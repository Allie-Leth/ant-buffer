#define CATCH_CONFIG_MAIN
#include <catch.hpp>
#include "ring_buffer.h"
#include <string>

// 1) Capacity check
TEST_CASE("capacity() returns compile-time capacity", "[RingBuffer][Capacity]") {
    constexpr size_t CAP = 4;
    RingBuffer<int, CAP> rb;
    REQUIRE(rb.capacity() == CAP);
}

// 2) Initial state: empty and not full
TEST_CASE("empty() and full() on new buffer", "[RingBuffer][State]") {
    RingBuffer<int, 3> rb;
    REQUIRE(rb.empty());
    REQUIRE_FALSE(rb.full());
}

// 3) Size tracking
TEST_CASE("size() reflects number of elements", "[RingBuffer][Size]") {
    RingBuffer<int, 3> rb;
    REQUIRE(rb.size() == 0);
    rb.push(1);
    REQUIRE(rb.size() == 1);
    int out;
    rb.pop(out);
    REQUIRE(rb.size() == 0);
}

// 4) Copy-push behavior
TEST_CASE("push(const T&) succeeds until full", "[RingBuffer][Push]") {
    RingBuffer<int, 2> rb;
    REQUIRE(rb.push(10));
    REQUIRE(rb.push(20));
    REQUIRE_FALSE(rb.push(30));  // buffer full
}

// 5) Move-push semantics
TEST_CASE("push(T&&) uses move semantics", "[RingBuffer][Move]") {
    RingBuffer<std::string, 2> rb;
    std::string foo = "foo";
    REQUIRE(rb.push(std::move(foo)));
    std::string popped;
    REQUIRE(rb.pop(popped));
    REQUIRE(popped == "foo");
    REQUIRE(foo.empty());
}

// 6) Pop behavior when empty
TEST_CASE("pop returns false on empty buffer", "[RingBuffer][Pop]") {
    RingBuffer<int, 3> rb;
    int out;
    REQUIRE_FALSE(rb.pop(out));
}

// 7) Wrap-around FIFO behavior
TEST_CASE("buffer wraps around correctly", "[RingBuffer][Wrap]") {
    RingBuffer<int, 3> rb;
    rb.push(1);
    rb.push(2);
    rb.push(3);
    REQUIRE(rb.full());
    int v;
    REQUIRE(rb.pop(v)); REQUIRE(v == 1);
    REQUIRE(rb.push(4));
    REQUIRE(rb.full());
    REQUIRE(rb.pop(v)); REQUIRE(v == 2);
    REQUIRE(rb.pop(v)); REQUIRE(v == 3);
    REQUIRE(rb.pop(v)); REQUIRE(v == 4);
    REQUIRE(rb.empty());
}

// 8) Clear resets state
TEST_CASE("clear() empties buffer and resets indices", "[RingBuffer][Clear]") {
    RingBuffer<int, 3> rb;
    rb.push(5);
    rb.push(6);
    rb.clear();
    REQUIRE(rb.empty());
    REQUIRE(rb.size() == 0);
    REQUIRE(rb.push(7));  // buffer reusable after clear
}
