#ifndef SPSC_QUEUE_H
#define SPSC_QUEUE_H

#include "util.h"
#include <atomic>
#include <cstddef>
#include <vector>
#include <bit>

template <typename T> class SPSCQueue {
  private:
    /*
     * Cache line size differs by architecture: 64 bytes on x86, 128 on Apple Silicon.
     * Using 128 bytes keeps head and tail on separate cache lines on both,
     * avoiding false sharing between producer and consumer cores.
     */
    static constexpr std::size_t CACHE_LINE = 128;
    std::vector<T> buffer;
    const std::size_t capacity_mask;

    alignas(CACHE_LINE) std::atomic<std::size_t> head;
    alignas(CACHE_LINE) std::atomic<std::size_t> tail;

  public:
    explicit SPSCQueue(std::size_t capacity) : buffer(capacity), capacity_mask(capacity - 1), head(0), tail(0) {
        if (!std::has_single_bit(capacity)) {
            die("SPSCQueue capacity must be a power of two");
        }
    }
    /*
     * Copying/Assignment operations are not thread safe.
     */
    SPSCQueue(const SPSCQueue &) = delete;
    SPSCQueue &operator=(const SPSCQueue &) = delete;
    /*
     * Tries to push an item into the SPSCQueue. Returns false if it is full
     * without blocking. The caller (ingest) decides what to do on false.
     */
    [[nodiscard]] bool try_push(const T &item) {
        const std::size_t current_tail = tail.load(std::memory_order_relaxed);
        const std::size_t next_tail = (current_tail + 1) & capacity_mask;
        /*
         * Checks if there is any space available
         */
        if (next_tail == head.load(std::memory_order_acquire))
            return false;
        /*
         * Store the item in the current slot, and then go to the next slot.
         */
        buffer[current_tail] = item;

        tail.store(next_tail, std::memory_order_release);
        return true;
    }
    /*
     * Tries to pop an item, writing it into the item in the parameters.
     * Returns false if the queue is empty, without blocking. The caller (order book)
     * decides what to do on false.
     */
    [[nodiscard]] bool try_pop(T &item) {
        const std::size_t current_head = head.load(std::memory_order_relaxed);
        /*
         * Checks if there is any value to read
         */
        if (current_head == tail.load(std::memory_order_acquire))
            return false;
        /*
        * Read the item from the current slot, and then go to the next slot.
        */
        item = buffer[current_head];

        const std::size_t next_head = (current_head + 1) & capacity_mask;

        head.store(next_head, std::memory_order_release);
        return true;
    }
};

#endif
