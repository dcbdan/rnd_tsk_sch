#pragma once

#include <queue>
#include <limits>

template <typename T, typename F>
struct stable_priority_queue {
  stable_priority_queue():
    count(std::numeric_limits<uint64_t>::max()) {}

  T const& top() const {
    auto const& [ret, _0] = q.top();
    return ret;
  }

  void pop() {
    return q.pop();
  }

  bool empty() const {
    return q.empty();
  }

  size_t size() const {
    return q.size();
  }

  void push(T const& t) {
    q.emplace(t, count--);
  }

private:
  struct compare_t {
    bool operator()(
      std::tuple<T, uint64_t> const& lhs,
      std::tuple<T, uint64_t> const& rhs) const
    {
      auto const& [tl, cl] = lhs;
      auto const& [tr, cr] = rhs;

      auto vl = f(tl);
      auto vr = f(tr);

      if(vl == vr) {
        return cl < cr;
      }
      return vl < vr;
    }

    F f;
  };

  using _queue_t = std::priority_queue<
                     std::tuple<T, uint64_t>,
                     std::vector<std::tuple<T, uint64_t>>,
                     compare_t>;
  _queue_t q;
  uint64_t count;
};
