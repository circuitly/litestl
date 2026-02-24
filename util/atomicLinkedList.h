/**
 * TODO: make this truly atomic.  For now it does use a mutex.
 *       Lock-free doubly-linked lists are complicated.
 */

#pragma once
#include "compiler_util.h"
#include <atomic>
#include <mutex>
#include <type_traits>

namespace litestl::util {
namespace detail {
template <typename T> struct Link {
  T *next, *prev;
};
} // namespace detail

template <typename NodeData, typename Node>
concept LinkedListNode = requires(Node *node, NodeData data) {
  { std::remove_reference_t<decltype(node->next)>() } -> std::same_as<Node *>;
  { std::remove_reference_t<decltype(node->prev)>() } -> std::same_as<Node *>;
  { node->data() } -> std::same_as<NodeData>;
  { Node::wrapData(data) } -> std::same_as<Node *>;
};

template <typename T, LinkedListNode<T> NodeData=T*> struct AtomicLinkedList {
  T *first = {nullptr};
  T *last = {nullptr};
  std::recursive_mutex mutex;

  AtomicLinkedList()
  {
  }

  T *push(NodeData &&data)
  {
    std::lock_guard guard(mutex);
    T *node = T::wrapData(data);

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

  void remove(T *node)
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
