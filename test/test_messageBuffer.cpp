#define CATCH_CONFIG_MAIN
#include <catch.hpp>
#include "message_buffer.h"
#include <cstdint>

/**
 * @brief Fixture providing an 8-byte buffer for default MessageBuffer tests.
 */
struct MessageBufferFixture {
    static constexpr size_t SIZE = 8;
    uint8_t raw[SIZE] = {};
    MessageBuffer mb{raw, SIZE};
};

/**
 * @brief Helper: finalize and check header correctness.
 */
inline void expectHeader(MessageBuffer &mb, uint8_t expectedType, uint8_t expectedLength) {
    REQUIRE(mb.data()[0] == expectedType);
    REQUIRE(mb.data()[1] == expectedLength);
}

TEST_CASE_METHOD(MessageBufferFixture, "Initial state", "[MessageBuffer][Init]") {
    // Raw buffer is untouched at start
    for (size_t i = 0; i < SIZE; ++i) {
        REQUIRE(raw[i] == 0);
    }
}

TEST_CASE_METHOD(MessageBufferFixture, "Begin and finalize message", "[MessageBuffer][Basic]") {
    REQUIRE(mb.beginMessage(0x42));
    REQUIRE(mb.writeByte(0x11));
    REQUIRE(mb.writeByte(0x22));
    mb.finalizeMessage();

    REQUIRE(mb.size() == 4); // [type][length][payload1][payload2]
    expectHeader(mb, 0x42, 2);
}

TEST_CASE_METHOD(MessageBufferFixture, "Message overflow prevention", "[MessageBuffer][Overflow]") {
    REQUIRE(mb.beginMessage(0x99));
    
    // Fill exactly to capacity
    for (size_t i = 0; i < MessageBufferFixture::SIZE - 2; ++i) {
        REQUIRE(mb.writeByte(static_cast<uint8_t>(i)));
    }

    // Further write must fail
    REQUIRE_FALSE(mb.writeByte(0xFF));

    // Finalize still succeeds
    mb.finalizeMessage();
    expectHeader(mb, 0x99, 6);
}

TEST_CASE_METHOD(MessageBufferFixture, "Begin read with valid message", "[MessageBuffer][Read]") {
    REQUIRE(mb.beginMessage(0xAB));
    REQUIRE(mb.writeByte(0xDE));
    REQUIRE(mb.writeByte(0xAD));
    mb.finalizeMessage();

    // Simulate receiving it
    REQUIRE(mb.beginRead(mb.size()));

    REQUIRE(mb.messageType()   == 0xAB);
    REQUIRE(mb.payloadLength() == 2);

    uint8_t v;
    REQUIRE(mb.readByte(v));
    REQUIRE(v == 0xDE);

    REQUIRE(mb.readByte(v));
    REQUIRE(v == 0xAD);

    // No more payload
    REQUIRE_FALSE(mb.readByte(v));
}

TEST_CASE_METHOD(MessageBufferFixture, "Begin read rejects invalid size", "[MessageBuffer][BadSize]") {
    REQUIRE_FALSE(mb.beginRead(0));          // Too small
    REQUIRE_FALSE(mb.beginRead(MessageBufferFixture::SIZE + 1)); // Too large
}

TEST_CASE_METHOD(MessageBufferFixture, "Payload readRemaining logic", "[MessageBuffer][Remaining]") {
    REQUIRE(mb.beginMessage(0x01));
    REQUIRE(mb.writeByte(0xAA));
    REQUIRE(mb.writeByte(0xBB));
    REQUIRE(mb.writeByte(0xCC));
    mb.finalizeMessage();

    REQUIRE(mb.beginRead(mb.size()));

    REQUIRE(mb.readRemaining() == 3);
    
    uint8_t v;
    REQUIRE(mb.readByte(v));
    REQUIRE(mb.readRemaining() == 2);

    REQUIRE(mb.readByte(v));
    REQUIRE(mb.readRemaining() == 1);

    REQUIRE(mb.readByte(v));
    REQUIRE(mb.readRemaining() == 0);
}

TEST_CASE_METHOD(MessageBufferFixture, "Payload length clamps at 255 bytes", "[MessageBuffer][Clamp]") {
    constexpr size_t largeSize = 300;
    uint8_t bigRaw[largeSize] = {};
    MessageBuffer big{bigRaw, largeSize};

    REQUIRE(big.beginMessage(0x77));
    for (size_t i = 0; i < 300; ++i) {
        big.writeByte(0x00);
    }
    big.finalizeMessage();

    // Should clamp at 255
    REQUIRE(big.payloadLength() == 255);
}
