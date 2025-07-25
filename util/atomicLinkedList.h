/**
 * TODO: make this truly atomic.  For now it does use a mutex.
 *       Lock-free doubly-linked lists are complicated.
 */

#pragma once
#include "compiler_util.h"
#include <atomic>
#include <mutex>

namespace litestl::util {
namespace detail {
template <typename T> struct Link {
  T *next, *prev;
};
} // namespace detail

template <typename N, typename B> concept LinkedListNode = requires(N *node, B data)
{
  {
    node->next
  }
  ->std::same_as<N *&>;
  {
    node->prev
  }
  ->std::same_as<N *&>;
  {
    node->data()
  }
  ->std::same_as<B>;

  {
    N::wrapData(data)
  }
  ->std::same_as<N *>;
};

template <typename T, LinkedListNode<T> N> struct AtomicLinkedList {
  N *first = {nullptr};
  N *last = {nullptr};
  std::recursive_mutex mutex;

  AtomicLinkedList()
  {
  }

  N *push(T &&data)
  {
    std::lock_guard guard(mutex);
    N *node = N::wrapData(data);

    if (!first) {
      first = last = node;
      node->next = node->prev = nullptr;
    } else {
      last->next = node;
      node->prev = last;
      node->next = nullptr;
      last = node;
    }
    return node;
  }

  void remove(N *node)
  {
    std::lock_guard guard(mutex);
    if (node->prev) {
        node->prev->next = node->next;
    }
    if (node->next) {
        node->next->prev = node->prev;
    }
    if (node == first) {
        first = node->next;
    }
    if (node == last) {
        last = node->prev;
    }
  }
};
} // namespace litestl::util
