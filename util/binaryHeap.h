#pragma once

#include "function.h"
#include "vector.h"

// Binary heap implementation from:
// http://eloquentjavascript.net/appendix2.html
namespace litestl::util {
template <typename T = void *, int static_size = 64> class BinaryHeap {
public:
  void push(T element, double weight)
  {
    content.append(element);
    weights.append(weight);

    // Allow it to bubble up.
    bubbleUp(content.size() - 1);
  }

  /** It is undefined behaviour to call .pop() on an empty heap. */
  T pop()
  {
    // Store the first element so we can return it later.
    T result = std::move(content[0]);

    // Get the element at the end of the array.
    T end = std::move(content.pop_back());
    double weight = weights.pop_back();

    // If there are any elements left, put the end element at the
    // start, and let it sink down.
    if (content.size() > 0) {
      content[0] = std::move(end);
      weights[0] = weight;
      sinkDown(0);
    }
    return result;
  }

  Vector<T> popAll(bool unsorted = false)
  {
    if (!unsorted) {
      Vector<T> result;
      result.ensure_capacity(content.size());

      while (size() > 0) {
        result.push(std::move(pop()));
      }
      return;
    }

    Vector<T> result = std::move(content);
    content.clear_and_contract();
    weights.clear_and_contract();
    return result;
  }

  T &peek()
  {
    return content[0];
  }

  /** returns false on failure. */
  bool remove(T &node)
  {
    int len = content.size();
    double nodeWeight = scoreFunction(node);
    // To remove a value, we must search through the array to find
    // it.
    for (int i = 0; i < len; i++) {
      if (content[i] == node) {
        // When it is found, the process seen in 'pop' is repeated
        // to fill up the hole.
        T end = std::move(content.pop_back());
        double weight = weights.pop_back();
        if (i != len - 1) {
          content[i] = std::move(end);
          weights[i] = weight;
          if (weight < nodeWeight) {
            bubbleUp(i);
          } else {
            sinkDown(i);
          }
        }
        return true;
      }
    }
    return false;
  }

  int size()
  {
    return content.size();
  }

  bool empty()
  {
    return !content.size();
  }

private:
  Vector<T> content;
  Vector<double> weights;
  void swap(int a, int b)
  {
    std::swap(content[a], content[b]);
    std::swap(weights[a], weights[b]);
  }

  void bubbleUp(int n)
  {
    // Fetch the element that has to be moved.
    // When at 0, an element can not go up any further.
    while (n > 0) {
      // Compute the parent element's index, and fetch it.
      int parentN = std::floor((n + 1) / 2) - 1;
      // Swap the elements if the parent is greater.
      if (weights[n] < weights[parentN]) {
        swap(parentN, n);
        // Update 'n' to continue at the new position.
        n = parentN;
      }
      // Found a parent that is less, no need to move it further.
      else
      {
        break;
      }
    }
  }

  void sinkDown(int n)
  {
    // Look up the target element and its score.
    int length = content.size();
    double elemScore = weights[n];

    // eslint-disable-next-line no-constant-condition
    while (true) {
      // Compute the indices of the child elements.
      int child2N = (n + 1) * 2, child1N = child2N - 1;

      // This is used to store the new position of the element,
      // if any.
      int swap = -1;
      double child1Score = 0.0;

      // If the first child exists (is inside the array)...
      if (child1N < length) {
        // Look it up and compute its score.
        child1Score = weights[child1N];
        // If the score is less than our element's, we need to swap.
        if (child1Score < elemScore) {
          swap = child1N;
        }
      }
      // Do the same checks for the other child.
      if (child2N < length) {
        double child2Score = weights[child2N];
        if (child2Score < (swap == -1 ? elemScore : child1Score)) {
          swap = child2N;
        }
      }

      // If the element needs to be moved, swap it, and continue.
      if (swap != -1) {
        std::swap(n, swap);
        n = swap;
      }
      // Otherwise, we are done.
      else
      {
        break;
      }
    }
  }
};

} // namespace litestl::util