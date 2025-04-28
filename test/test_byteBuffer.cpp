#define CATCH_CONFIG_MAIN
#include <catch.hpp>
#include <byte_buffer.h>
#include <cstdint>

/**
 * @brief Fixture providing an 8-byte buffer for default tests.
 */
struct ByteBufferFixture {
    static constexpr size_t SIZE = 8;
    uint8_t raw[SIZE] = {};
    ByteBuffer bb{raw, SIZE};
};

/**
 * @brief Test that the buffer overflows when writing more than its capacity.
 *
 * @tparam N Size of the buffer to test.
 * 
 * Iterates over the buffer, writing values from 0 to N-1, and then tries to write
 * one more value (0xFF). The test expects the first N writes to succeed and the
 * last one to fail.
 */
/**
 * @brief Reusable helper: push exactly capacity() bytes, then verify one more write fails
 * and that the write-cursor never advances past capacity().
 */
inline void expectOverflow(ByteBuffer &bb) {
    // fill to capacity
    for (size_t i = 0; i < bb.capacity(); ++i) {
        REQUIRE(bb.writeUInt8(static_cast<uint8_t>(i)));
    }
    // write-cursor must now be at capacity()
    REQUIRE(bb.writePosition()  == bb.capacity());
    REQUIRE(bb.writeRemaining() == 0);

    // one more write fails and cursor stays put
    REQUIRE_FALSE(bb.writeUInt8(0xFF));
    REQUIRE(bb.writePosition()  == bb.capacity());
    REQUIRE(bb.writeRemaining() == 0);
}

TEST_CASE_METHOD(ByteBufferFixture, "Initial state", "[ByteBuffer][Init]") {
    // nothing written yet
    REQUIRE(bb.capacity()      == SIZE);
    REQUIRE(bb.writePosition() == 0);
    REQUIRE(bb.writeRemaining() == SIZE);
    REQUIRE(bb.readPosition()  == 0);
    REQUIRE(bb.readRemaining() == 0);
}

TEST_CASE_METHOD(ByteBufferFixture, "8-bit write/read and cursors", "[ByteBuffer][8bit]") {
    // — write two known bytes —
    REQUIRE(bb.writeUInt8(0xAA));
    REQUIRE(bb.writeUInt8(0x55));

    // check write-cursor & remaining
    REQUIRE(bb.writePosition()   == 2);
    REQUIRE(bb.writeRemaining()  == ByteBufferFixture::SIZE - 2);

    // read-cursor hasn't advanced yet & remaining == bytes written
    REQUIRE(bb.readPosition()    == 0);
    REQUIRE(bb.readRemaining()   == 2);

    // — read them back —
    uint8_t v;
    REQUIRE(bb.readUInt8(v));
    REQUIRE(v == 0xAA);
    REQUIRE(bb.readPosition()  == 1);
    REQUIRE(bb.readRemaining() == 1);

    REQUIRE(bb.readUInt8(v));
    REQUIRE(v == 0x55);
    REQUIRE(bb.readPosition()  == 2);
    REQUIRE(bb.readRemaining() == 0);

    // no more to read
    REQUIRE_FALSE(bb.readUInt8(v));
}

TEST_CASE_METHOD(ByteBufferFixture, "8-bit buffer overflow semantics", "[ByteBuffer][Overflow]") {
    expectOverflow(bb);
}

TEMPLATE_TEST_CASE_SIG(
    "16-bit LE/BE read+write with cursors", "[ByteBuffer][16bit]",
    ((auto writeFn, auto readFn, uint16_t value, uint8_t b0, uint8_t b1),
     writeFn, readFn, value, b0, b1),
    (ByteBuffer::writeUInt16LE, ByteBuffer::readUInt16LE, 0x1234, 0x34, 0x12),
    (ByteBuffer::writeUInt16BE, ByteBuffer::readUInt16BE, 0xABCD, 0xAB, 0xCD)
) {
    uint8_t raw[4] = {};
    ByteBuffer bb{raw, sizeof(raw)};

    // initial
    REQUIRE(bb.writeRemaining() == 4);
    REQUIRE(bb.readRemaining()  == 0);

    // write two bytes
    REQUIRE((bb.*writeFn)(value));
    CHECK(raw[0] == b0);
    CHECK(raw[1] == b1);

    REQUIRE(bb.writePosition()   == 2);
    REQUIRE(bb.writeRemaining()  == 2);
    REQUIRE(bb.readPosition()    == 0);
    REQUIRE(bb.readRemaining()   == 2);

    // read them back
    uint16_t out = 0;
    REQUIRE((bb.*readFn)(out));
    CHECK(out == value);

    REQUIRE(bb.readPosition()    == 2);
    REQUIRE(bb.readRemaining()   == 0);
}

TEMPLATE_TEST_CASE_SIG(
    "32-bit LE/BE read+write with cursors", "[ByteBuffer][32bit]",
    ((auto writeFn, auto readFn, uint32_t value, uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3),
     writeFn, readFn, value, b0, b1, b2, b3),
    (ByteBuffer::writeUInt32LE, ByteBuffer::readUInt32LE, 0x11223344, 0x44, 0x33, 0x22, 0x11),
    (ByteBuffer::writeUInt32BE, ByteBuffer::readUInt32BE, 0xDEADBEEF, 0xDE, 0xAD, 0xBE, 0xEF)
) {
    uint8_t raw[8] = {};
    ByteBuffer bb{raw, sizeof(raw)};

    REQUIRE(bb.writeRemaining() == 8);
    REQUIRE(bb.readRemaining()  == 0);

    REQUIRE((bb.*writeFn)(value));
    CHECK(raw[0] == b0);
    CHECK(raw[1] == b1);
    CHECK(raw[2] == b2);
    CHECK(raw[3] == b3);

    REQUIRE(bb.writePosition()  == 4);
    REQUIRE(bb.writeRemaining() == 4);
    REQUIRE(bb.readRemaining()  == 4);

    uint32_t out = 0;
    REQUIRE((bb.*readFn)(out));
    CHECK(out == value);

    REQUIRE(bb.readPosition()  == 4);
    REQUIRE(bb.readRemaining() == 0);
}

TEST_CASE("Overflow conditions", "[ByteBuffer][Overflow]") {
    uint8_t small3[3] = {};
    ByteBuffer bb3{small3, sizeof(small3)};
    REQUIRE_FALSE(bb3.writeUInt32LE(0x12345678));
    REQUIRE_FALSE(bb3.writeUInt32BE(0x87654321));

    uint8_t small1[1] = {};
    ByteBuffer bb1{small1, sizeof(small1)};
    REQUIRE_FALSE(bb1.writeUInt16LE(0xFFFF));
    REQUIRE_FALSE(bb1.writeUInt16BE(0xFFFF));
}

TEST_CASE("Underflow conditions", "[ByteBuffer][Underflow]") {
    uint8_t buf2[2] = {0x00, 0x00};
    ByteBuffer bb2{buf2, sizeof(buf2)};
    uint32_t v32;
    REQUIRE_FALSE(bb2.readUInt32LE(v32));

    uint8_t buf1[1] = {0xAA};
    ByteBuffer bb1{buf1, sizeof(buf1)};
    uint16_t v16;
    REQUIRE_FALSE(bb1.readUInt16LE(v16));
    REQUIRE_FALSE(bb1.readUInt16BE(v16));
}

TEST_CASE_METHOD(ByteBufferFixture, "ResetRead / ResetWrite behavior", "[ByteBuffer][Reset]") {
    // write a couple bytes
    bb.writeUInt8(1);
    bb.writeUInt8(2);
    REQUIRE(bb.writePosition() == 2);
    REQUIRE(bb.writeRemaining() == 6);
    REQUIRE(bb.readRemaining()  == 2);

    // advancing readPos
    uint8_t dummy;
    SECTION("advance the read cursor") {
      REQUIRE(bb.readUInt8(dummy));
      REQUIRE(bb.readPosition()  == 1);
      REQUIRE(bb.readRemaining() == 1);
    }

    // reset only the read cursor
    bb.resetRead();
    REQUIRE(bb.readPosition()  == 0);
    REQUIRE(bb.readRemaining() == bb.writePosition());

    // reset only the write cursor (clears buffer)
    bb.resetWrite();
    REQUIRE(bb.writePosition()  == 0);
    REQUIRE(bb.writeRemaining() == bb.capacity());
    REQUIRE(bb.readRemaining()  == 0);
}
