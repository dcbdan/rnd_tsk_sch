#pragma once

#include "aaa.h"
#include "dag.h"
#include "stable_priority_queue.h"

#include <queue>
#include <cassert>
#include <algorithm>

struct sch_t {
  sch_t(int num_workers, dag_t const& dag): num_workers(num_workers), dag(dag), time(0.0)
  {
    for(nid_t input: dag.inputs()) {
      pending_tasks.push(make_pending_task(input));
    }

    num_remaining.reserve(dag.size());
    for(nid_t nid = 0; nid != dag.size(); ++nid) {
      num_remaining.push_back(dag[nid].downs.size());
    }
  }

  enum step_type {
    waited_cuz_all_busy,
    waited_cuz_no_tasks,
    added_task,
    completed_all_tasks
  };

  step_type step() {
    if(has_worker() && has_task()) {
      // we have a worker and we have a task, so get the worker busy
      pending_task_t const& task = pending_tasks.top();

      pending_workers.push(pending_worker_t{
        .time_complete = task.duration + time,
        .task          = task.task
      });

      pending_tasks.pop();

      return step_type::added_task;
    }

    if(!has_worker() ||
       (is_working() && !has_task()))
    {
      // Case 1: all the workers are busy
      // Case 2: we're waiting for tasks to be available
      step_type ret =
        (!has_worker())                ?
        step_type::waited_cuz_all_busy :
        step_type::waited_cuz_no_tasks ;

      pending_worker_t worker = pending_workers.top();

      // increment time
      assert(worker.time_complete >= time);
      time = worker.time_complete;

      // add tasks that can start to pending
      for(nid_t up: dag[worker.task].ups) {
        num_remaining[up]--;
        if(num_remaining[up] == 0) {
          pending_tasks.push(make_pending_task(up));
        }
      }

      return ret;
    }

    // If we get here, we better be done
    assert(done());
    // Just in case, if we're done, this better also be true
    assert(std::all_of(
      num_remaining.begin(),
      num_remaining.end(),
      [](int i){ return i == 0; }));
    return step_type::completed_all_tasks;
  }

  bool is_working() const { return !pending_workers.empty();             }
  bool has_worker() const { return pending_workers.size() < num_workers; }
  bool has_task()   const { return !pending_tasks.empty();               }
  bool done()       const { return pending_workers.empty() &&
                                   pending_tasks.empty();                }

  double current_time() const { return time; }

private:
  struct pending_worker_t {
    double time_complete;
    nid_t task;
  };

  struct pending_task_t {
    double duration;
    int priority;
    nid_t task;
  };

  pending_task_t make_pending_task(nid_t nid) const {
    auto const& node = dag[nid];
    return pending_task_t {
      .duration = node.time(),
      .priority = node.priority,
      .task     = node.nid};
  }

  int const num_workers;
  dag_t const& dag;

  double time;

  struct get_priority_t {
    // higher priority will come first
    int operator()(pending_task_t const& t) const { return t.priority; }
  };

  struct compare_pending_worker_t {
    bool operator()(pending_worker_t const& lhs, pending_worker_t const& rhs) const
    {
      // this way lower times come first
      return lhs.time_complete > rhs.time_complete;
    }
  };
  using pending_tasks_queue_t = stable_priority_queue<
          pending_task_t,
          get_priority_t>;
  using pending_workers_queue_t = std::priority_queue<
          pending_worker_t,
          vector<pending_worker_t>,
          compare_pending_worker_t>;

  pending_tasks_queue_t   pending_tasks;
  pending_workers_queue_t pending_workers;

  vector<int> num_remaining;
};

double schedule(int num_workers, dag_t const& dag) {
  sch_t sch(num_workers, dag);
  while(sch.step() != sch_t::step_type::completed_all_tasks) {}
  return sch.current_time();
}
