#define CATCH_CONFIG_MAIN
#include <catch.hpp>
#include "ant_buffers.h"
#include <string>
#include <cstdint>

/**
 * @brief Fixture providing an 8-byte array for ByteBuffer/MessageBuffer tests.
 */
struct RawBufferFixture {
    static constexpr size_t SIZE = 8;
    uint8_t raw[SIZE] = {};
};

//-------------------------------------------------------------------------
// ByteBuffer Factory Tests
//-------------------------------------------------------------------------
TEST_CASE_METHOD(RawBufferFixture, "make_byte_buffer() constructs correctly", "[antBuffers][ByteBuffer]") {
    auto bb = antBuffers::make_byte_buffer(raw, SIZE);
    REQUIRE(bb.capacity() == SIZE);
    REQUIRE(bb.writeRemaining() == SIZE);
}

TEST_CASE_METHOD(RawBufferFixture, "antBuffers::ByteBuffer read/write roundtrip", "[antBuffers][ByteBuffer]") {
    antBuffers::ByteBuffer bb(raw, SIZE);
    REQUIRE(bb.writeUInt8(0x12));
    REQUIRE(bb.writeUInt8(0x34));

    bb.resetRead();
    uint8_t v;
    REQUIRE(bb.readUInt8(v));
    REQUIRE(v == 0x12);
    REQUIRE(bb.readUInt8(v));
    REQUIRE(v == 0x34);
}

//-------------------------------------------------------------------------
// MessageBuffer Factory Tests
//-------------------------------------------------------------------------
TEST_CASE_METHOD(RawBufferFixture, "make_message_buffer() constructs correctly", "[antBuffers][MessageBuffer]") {
    auto mb = antBuffers::make_message_buffer(raw, SIZE);
    REQUIRE(mb.size() == 0);
}

TEST_CASE_METHOD(RawBufferFixture, "antBuffers::MessageBuffer basic framing", "[antBuffers][MessageBuffer]") {
    antBuffers::MessageBuffer mb(raw, SIZE);
    REQUIRE(mb.beginMessage(0x42));
    REQUIRE(mb.writeByte(0xAA));
    REQUIRE(mb.writeByte(0xBB));
    mb.finalizeMessage();

    REQUIRE(mb.data()[0] == 0x42);  // type
    REQUIRE(mb.data()[1] == 2);     // length
    REQUIRE(mb.size() == 4);        // header + payload
}

//-------------------------------------------------------------------------
// RingBuffer Usage Tests
//-------------------------------------------------------------------------
TEST_CASE("antBuffers::RingBuffer basic push/pop", "[antBuffers][RingBuffer]") {
    antBuffers::RingBuffer<std::string, 3> rb;
    
    REQUIRE(rb.push("Hello"));
    REQUIRE(rb.push("World"));
    
    std::string out;
    REQUIRE(rb.pop(out));
    REQUIRE(out == "Hello");

    REQUIRE(rb.pop(out));
    REQUIRE(out == "World");

    REQUIRE(rb.empty());
}

TEST_CASE("antBuffers::RingBuffer respects capacity", "[antBuffers][RingBuffer]") {
    antBuffers::RingBuffer<int, 2> rb;
    
    REQUIRE(rb.push(1));
    REQUIRE(rb.push(2));
    REQUIRE_FALSE(rb.push(3));  // Full now

    int out;
    REQUIRE(rb.pop(out));
    REQUIRE(rb.pop(out));
    REQUIRE(rb.empty());
}
