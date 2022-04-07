#pragma once

#include "aaa.h"

using nid_t = int;

struct dag_t {
  struct node_t {
    int priority;
    function<double()> time;

    vector<int> downs;
    vector<int> ups;

    nid_t nid;
  };

  vector<nid_t> inputs() const { return _inputs; }

  size_t size() const { return dag.size(); }

  node_t const& operator[](nid_t nid) const { return dag[nid]; }

  nid_t insert_to_top(int priority, function<double()> time, vector<nid_t> downs) {
    node_t new_node = node_t{
      .priority = priority,
      .time     = time,
      .downs    = downs,
      .ups      = vector<int>(),
      .nid      = static_cast<int>(dag.size())
    };
    dag.push_back(new_node);
    if(new_node.downs.size() == 0) {
      _inputs.push_back(new_node.nid);
    } else {
      for(nid_t down: downs) {
        dag[down].ups.push_back(new_node.nid);
      }
    }
    return new_node.nid;
  }

private:
  vector<node_t> dag;
  vector<nid_t> _inputs;
};

