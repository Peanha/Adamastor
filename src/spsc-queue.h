#ifndef SPSC_QUEUE_H
#define SPSC_QUEUE_H

#include <vector>
#include <atomic>
#include <cstddef>
#include "event.h"
#include <new>

template <typename T>
class SPSCQueue {
private:
    std::vector<T> buffer;
    const std::size_t capacity;

    /*
    *   Enforce alignment to avoid False sharing.
    *   To 128 so different machines avoid it
    */

    alignas(128) std::atomic<std::size_t> head;
    alignas(128) std::atomic<std::size_t> tail;

public:
    SPSCQueue(size_t size) :
        buffer(size + 1),
        capacity(size + 1),
        head(0),
        tail(0) {}

    /*
    *   Copying/Assignments operations are not thread safe
    */

    SPSCQueue(const SPSCQueue&) = delete;
    SPSCQueue& operator=(const SPSCQueue&) = delete;

    [[nodiscard]] bool try_push(const T& item) {
        const std::size_t current_tail = tail.load(std::memory_order_relaxed);
        const std::size_t next_tail = (current_tail + 1) % capacity;

        /*
        *   Checks if there is any space available
        */

        if (next_tail == head.load(std::memory_order_acquire)) {
            return false;
        }

        /*
        *   Store the event in the actual slot, and then go to the next slot
        */

       buffer[current_tail] = item;

        tail.store(next_tail, std::memory_order_release);
        return true;
    }

    [[nodiscard]] bool try_pop(T& item) {

        const std::size_t current_head = head.load(std::memory_order_relaxed);

        /*
        *   Checks if there is any value to read
        */


        if (current_head == tail.load(std::memory_order_acquire)) {
            return false;
        }

        item = buffer[current_head];

        const std::size_t next_head = (current_head + 1 ) % capacity;

        head.store(next_head, std::memory_order_release);
        return true;

    }
};

#endif

