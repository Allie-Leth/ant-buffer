#pragma once

#include <cstdint>
#include <cstddef>

/**
 * @file byte_buffer.h
 * @brief Simple sequential reader/writer over a raw byte buffer with endianness helpers.
 *
 * Provides a non-owning, index-based interface to a byte array for reading and
 * writing unsigned integer types in little- or big-endian formats. Designed for
 * embedded and real-time systems with zero dynamic allocation.
 */
class ByteBuffer
{
public:
    /**
     * @brief Construct a ByteBuffer over an existing byte array.
     *
     * @param buffer   Pointer to the raw byte array.
     * @param capacity Total size of the array in bytes.
     */
    ByteBuffer(uint8_t *buffer, size_t capacity)
        : data_(buffer), capacity_(capacity) {}

    //-------------------------------------------------------------------------
    // Write-side API
    //-------------------------------------------------------------------------
    /**
     * @brief Reset the write cursor back to the start (clears all data).
     *
     * After this call, writePosition() == 0 and writeRemaining() == capacity().
     */
    void resetWrite() { head_ = 0; }

    /**
     * @brief Get the current write index.
     *
     * @return Zero-based index where the next write will occur.
     */
    size_t writePosition() const { return head_; }

    /**
     * @brief How many bytes you can still write before overflowing.
     *
     * @return capacity() - writePosition().
     */
    size_t writeRemaining() const { return (head_ < capacity_) ? (capacity_ - head_) : 0; }

    //-------------------------------------------------------------------------
    // Read-side API
    //-------------------------------------------------------------------------
    /**
     * @brief Reset the read cursor back to the start.
     *
     * Does _not_ erase data—you can re-read what was previously written.
     * After this call, readPosition() == 0 and readRemaining() == writePosition().
     */
    void resetRead() { tail_ = 0; }

    /**
     * @brief Get the current read index.
     *
     * @return Zero-based index where the next read will occur.
     */
    size_t readPosition() const { return tail_; }

    /**
     * @brief How many bytes remain available to read.
     *
     * @return writePosition() - readPosition().
     */
    size_t readRemaining() const { return (tail_ < head_) ? (head_ - tail_) : 0; }

    //-------------------------------------------------------------------------
    // Shared
    //-------------------------------------------------------------------------
    /**
     * @brief Get the total capacity of the buffer.
     *
     * @return Maximum number of bytes that can ever be held.
     */
    size_t capacity() const { return capacity_; }

    //-------------------------------------------------------------------------
    // 8-bit
    //-------------------------------------------------------------------------
    /**
     * @brief Read one byte.
     *
     * @param[out] out Where the value will be stored.
     * @return true if one byte was read; false if no data remains.
     */
    bool readUInt8(uint8_t &out)
    {
        if (readRemaining() < 1) return false;
        out = data_[tail_++];
        return true;
    }

    /**
     * @brief Write one byte.
     *
     * @param[in] v Value to write.
     * @return true if one byte was written; false if buffer full.
     */
    bool writeUInt8(uint8_t v)
    {
        if (writeRemaining() < 1) return false;
        data_[head_++] = v;
        return true;
    }

    //-------------------------------------------------------------------------
    // 16-bit
    //-------------------------------------------------------------------------
    /**
     * @brief Read a little-endian 16-bit value.
     *
     * @param[out] out Where the 16-bit value will be stored.
     * @return true if two bytes were read; false if underflow.
     */
    bool readUInt16LE(uint16_t &out)
    {
        if (readRemaining() < 2) return false;
        out = uint16_t(data_[tail_])
            | (uint16_t(data_[tail_ + 1]) << 8);
        tail_ += 2;
        return true;
    }

    /**
     * @brief Read a big-endian 16-bit value.
     *
     * @param[out] out Where the 16-bit value will be stored.
     * @return true if two bytes were read; false if underflow.
     */
    bool readUInt16BE(uint16_t &out)
    {
        if (readRemaining() < 2) return false;
        out = (uint16_t(data_[tail_]) << 8)
            |  uint16_t(data_[tail_ + 1]);
        tail_ += 2;
        return true;
    }

    /**
     * @brief Write a little-endian 16-bit value.
     *
     * @param[in] v Value to write.
     * @return true if two bytes were written; false if overflow.
     */
    bool writeUInt16LE(uint16_t v)
    {
        if (writeRemaining() < 2) return false;
        data_[head_]     = uint8_t( v        & 0xFF);
        data_[head_ + 1] = uint8_t((v >> 8)  & 0xFF);
        head_ += 2;
        return true;
    }

    /**
     * @brief Write a big-endian 16-bit value.
     *
     * @param[in] v Value to write.
     * @return true if two bytes were written; false if overflow.
     */
    bool writeUInt16BE(uint16_t v)
    {
        if (writeRemaining() < 2) return false;
        data_[head_]     = uint8_t((v >> 8)  & 0xFF);
        data_[head_ + 1] = uint8_t( v        & 0xFF);
        head_ += 2;
        return true;
    }

    //-------------------------------------------------------------------------
    // 32-bit
    //-------------------------------------------------------------------------
    /**
     * @brief Read a little-endian 32-bit value.
     *
     * @param[out] out Where the 32-bit value will be stored.
     * @return true if four bytes were read; false if underflow.
     */
    bool readUInt32LE(uint32_t &out)
    {
        if (readRemaining() < 4) return false;
        out =  uint32_t(data_[tail_])
            |  (uint32_t(data_[tail_ + 1]) << 8)
            |  (uint32_t(data_[tail_ + 2]) << 16)
            |  (uint32_t(data_[tail_ + 3]) << 24);
        tail_ += 4;
        return true;
    }

    /**
     * @brief Read a big-endian 32-bit value.
     *
     * @param[out] out Where the 32-bit value will be stored.
     * @return true if four bytes were read; false if underflow.
     */
    bool readUInt32BE(uint32_t &out)
    {
        if (readRemaining() < 4) return false;
        out = (uint32_t(data_[tail_]) << 24)
            | (uint32_t(data_[tail_ + 1]) << 16)
            | (uint32_t(data_[tail_ + 2]) << 8)
            |  uint32_t(data_[tail_ + 3]);
        tail_ += 4;
        return true;
    }

    /**
     * @brief Write a little-endian 32-bit value.
     *
     * @param[in] v Value to write.
     * @return true if four bytes were written; false if overflow.
     */
    bool writeUInt32LE(uint32_t v)
    {
        if (writeRemaining() < 4) return false;
        data_[head_]     = uint8_t( v        & 0xFF);
        data_[head_ + 1] = uint8_t((v >> 8)  & 0xFF);
        data_[head_ + 2] = uint8_t((v >> 16) & 0xFF);
        data_[head_ + 3] = uint8_t((v >> 24) & 0xFF);
        head_ += 4;
        return true;
    }

    /**
     * @brief Write a big-endian 32-bit value.
     *
     * @param[in] v Value to write.
     * @return true if four bytes were written; false if overflow.
     */
    bool writeUInt32BE(uint32_t v)
    {
        if (writeRemaining() < 4) return false;
        data_[head_]     = uint8_t((v >> 24) & 0xFF);
        data_[head_ + 1] = uint8_t((v >> 16) & 0xFF);
        data_[head_ + 2] = uint8_t((v >> 8)  & 0xFF);
        data_[head_ + 3] = uint8_t( v        & 0xFF);
        head_ += 4;
        return true;
    }

private:
    uint8_t *data_;       /**< Pointer to the external byte array. */
    size_t   capacity_;   /**< Total size of the array in bytes. */
    size_t   head_ = 0;  /**< Next index to write. */
    size_t   tail_  = 0;  /**< Next index to read. */
};
