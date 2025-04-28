#pragma once

#include "byte_buffer.h"
#include "message_buffer.h"
#include "ring_buffer.h"

namespace antBuffers {

    /**
     * @brief Factory for a ByteBuffer over external storage.
     * @param buffer   Pointer to the raw byte array.
     * @param capacity Total size of the array in bytes.
     * @return ByteBuffer instance operating over the provided buffer.
     */
    inline ByteBuffer make_byte_buffer(uint8_t* buffer, size_t capacity) {
        return ByteBuffer(buffer, capacity);
    }

    /**
     * @brief Factory for a MessageBuffer over external storage.
     * @param buffer   Pointer to the raw byte array.
     * @param capacity Total size of the array in bytes.
     * @return MessageBuffer instance operating over the provided buffer.
     */
    inline MessageBuffer make_message_buffer(uint8_t* buffer, size_t capacity) {
        return MessageBuffer(buffer, capacity);
    }

    // No make_ring() factory:
    // RingBuffer<T, N> must be constructed with template parameters at compile time.

} // namespace antBuffers
