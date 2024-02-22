#pragma once
/**
  1. 一个生产者消费者
  2. 指针操作为原子类型
*/
#include <atomic>
namespace xrtc {

template<class T>
class LockFreeQueue {
  private:
    struct Node {
        T value;
        Node *next;
        explicit Node(const T &value) : value(value), next(nullptr) {}
    };
    Node *_head;
    Node *_divider;
    Node *_tail;
    std::atomic<int> _size;

  public:
    LockFreeQueue() {
        _head = _tail = _divider = new Node(T());
        _size = 0;
    }

    ~LockFreeQueue() {
        Node *cur = nullptr;
        while (_head != nullptr) {
            cur = _head;
            _head = _head->next;
            delete cur;
        }
        _size = 0;
    }

    void Produce(const T &val) {
        _tail->next = new Node(val);
        _tail = _tail->next;
        _size++;

        Node *cur = nullptr;
        while (_head != _divider) {
            cur = _head;
            _head = _head->next;
            delete cur;
        }
    }

    bool Consume(T &val) {
        if (_divider != _tail) {
            val = _divider->next->value;
            _divider = _divider->next;
            _size--;
            return true;
        }
        return false;
    }

    bool empty() { return _size == 0; }

    int size() { return _size; }
};
} // namespace xrtc
