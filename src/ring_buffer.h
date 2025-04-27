#pragma once

#include <cstddef>
#include <utility>

/**
 * @brief Fixed-capacity, in-memory circular buffer (ring buffer) template.
 *
 * Provides non-blocking push and pop operations without dynamic allocation.
 * Ideal for embedded or real-time systems where predictability and minimal
 * overhead are required.
 *
 * @tparam T Element type stored in the buffer. Must be MoveAssignable for move overload.
 * @tparam N Compile-time capacity of the buffer (maximum number of elements).
 */
template<typename T, size_t N>
class RingBuffer {
public:
    /**
     * @brief Default constructor.
     *
     * Initializes an empty buffer.
     */
    RingBuffer() = default;

    /**
     * @brief Default destructor.
     */
    ~RingBuffer() = default;

    /**
     * @brief Push a copy of a value into the buffer.
     *
     * @param v Reference to the value to copy into the buffer.
     * @return true if the value was successfully pushed; false if buffer is full.
     */
    bool push(const T& v) {
        if (count_ == N) return false;
        buf_[head_] = v;
        head_ = (head_ + 1) % N;
        ++count_;
        return true;
    }

    /**
     * @brief Push a movable value into the buffer
     *
     * @param v Rvalue reference to the value to move into the buffer.
     * @return true if the value was successfully pushed; false if buffer is full.
     */
    bool push(T&& v) {
        if (count_ == N) return false;
        buf_[head_] = std::move(v);
        head_ = (head_ + 1) % N;
        ++count_;
        return true;
    }

    /**
     * @brief Pop the oldest element from the buffer.
     *
     * Moves the oldest element into the provided output reference.
     *
     * @param out Reference where the popped value will be stored.
     * @return true if an element was successfully popped; false if buffer is empty.
     */
    bool pop(T& out) {
        if (count_ == 0) return false;
        out = std::move(buf_[tail_]);
        tail_ = (tail_ + 1) % N;
        --count_;
        return true;
    }

    /**
     * @brief Get the current number of stored elements.
     *
     * @return Number of elements currently in the buffer.
     */
    size_t size() const {
        return count_;
    }

    /**
     * @brief Get the maximum capacity of the buffer.
     *
     * @return Compile-time maximum number of elements.
     */
    constexpr size_t capacity() const {
        return N;
    }

    /**
     * @brief Check if the buffer is empty.
     *
     * @return true if there are no elements stored; false otherwise.
     */
    bool empty() const {
        return count_ == 0;
    }

    /**
     * @brief Check if the buffer is full.
     *
     * @return true if buffer has reached its capacity; false otherwise.
     */
    bool full() const {
        return count_ == N;
    }

    /**
     * @brief Clear all contents of the buffer.
     *
     * Resets head, tail, and count to zero. Does not destruct stored elements,
     * they will be overwritten on subsequent pushes.
     */
    void clear() {
        head_ = tail_ = count_ = 0;
    }

private:
    T      buf_[N];   /**< Internal storage array of size N. */
    size_t head_  = 0;/**< Index for next push. */
    size_t tail_  = 0;/**< Index for next pop. */
    size_t count_ = 0;/**< Current number of elements. */
};
