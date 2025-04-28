#pragma once

#include <cstdint>
#include <cstddef>

/**
 * @file message_buffer.h
 * @brief Minimal framed message reader/writer over a byte buffer.
 *
 * Wraps a ByteBuffer to provide simple framed message handling with a
 * type field and payload length. Designed for packetized communication
 * over unreliable or size-limited links like LoRa, BLE, or UART streams.
 *
 * No dynamic allocation; assumes caller provides external buffer memory.
 */
class MessageBuffer
{
public:
    /**
     * @brief Construct a MessageBuffer over an existing byte array.
     *
     * @param buffer   Pointer to the raw byte array.
     * @param capacity Total size of the array in bytes.
     */
    MessageBuffer(uint8_t *buffer, size_t capacity)
        : data_(buffer), capacity_(capacity) {}

    //-------------------------------------------------------------------------
    // Write-side API
    //-------------------------------------------------------------------------
    /**
     * @brief Begin writing a new message.
     *
     * Resets the internal cursor and writes the type field.
     *
     * @param type Message type identifier (user-defined meaning).
     * @return true if message start was successful; false if buffer too small.
     */
    bool beginMessage(uint8_t type)
    {
        if (capacity_ < headerSize_) return false;
        head_ = headerSize_; // Reserve space for header
        tail_ = 0;
        data_[0] = type;
        data_[1] = 0; // Length placeholder, filled on finalize
        return true;
    }

    /**
     * @brief Append a byte to the message payload.
     *
     * @param v Byte value to append.
     * @return true if byte appended; false if buffer full.
     */
    bool writeByte(uint8_t v)
    {
        if (head_ >= capacity_) return false;
        data_[head_++] = v;
        return true;
    }

    /**
     * @brief Finalize the message by writing the payload length.
     *
     * Must be called after writing the payload before transmitting.
     */
    void finalizeMessage()
    {
        size_t payloadLength = head_ - headerSize_;
        if (payloadLength > 255) payloadLength = 255; // Clamp
        data_[1] = static_cast<uint8_t>(payloadLength);
    }

    /**
     * @brief Get pointer to the start of the full message (header + payload).
     *
     * @return Pointer to the beginning of the message.
     */
    uint8_t *data() const { return data_; }

    /**
     * @brief Get total size of the message (header + payload).
     *
     * @return Number of bytes to send.
     */
    size_t size() const { return head_; }

    //-------------------------------------------------------------------------
    // Read-side API
    //-------------------------------------------------------------------------
    /**
     * @brief Begin reading an incoming message.
     *
     * Sets up internal cursors based on provided message size.
     *
     * @param size Number of bytes in received message (including header).
     * @return true if valid message; false if too short or invalid length.
     */
    bool beginRead(size_t size)
    {
        if (size < headerSize_ || size > capacity_) return false;
        head_ = size;
        tail_ = headerSize_;
        return true;
    }

    /**
     * @brief Get the message type identifier.
     *
     * @return Message type byte.
     */
    uint8_t messageType() const { return data_[0]; }

    /**
     * @brief Get the message payload length.
     *
     * @return Number of payload bytes.
     */
    uint8_t payloadLength() const { return data_[1]; }

    /**
     * @brief Read a byte from the message payload.
     *
     * @param[out] out Where the byte will be stored.
     * @return true if byte read; false if no more payload bytes.
     */
    bool readByte(uint8_t &out)
    {
        if (tail_ >= head_) return false;
        out = data_[tail_++];
        return true;
    }

    /**
     * @brief How many payload bytes remain unread.
     *
     * @return Remaining payload bytes.
     */
    size_t readRemaining() const
    {
        size_t payloadEnd = headerSize_ + payloadLength();
        return (tail_ < payloadEnd) ? (payloadEnd - tail_) : 0;
    }

private:
    uint8_t *data_;         /**< Pointer to external byte array. */
    size_t   capacity_;     /**< Total size of the array in bytes. */
    size_t   head_ = 0;     /**< Write/read cursor. */
    size_t   tail_ = 0;     /**< Read cursor during payload reads. */

    static constexpr size_t headerSize_ = 2; /**< Bytes reserved for header: [type][length]. */
};
