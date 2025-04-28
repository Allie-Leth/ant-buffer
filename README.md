# Buffer Utilities

Device-agnostic C++ header-only utilities for efficient in-memory buffering.  
Originally developed for the Ant Worker ESP32 firmware fleet, but designed to be fully portable across systems.

### Included Components

## Byte Buffer:
- Lightweight sequential reader/writer over a raw byte array.
    - No dynamic memory allocation
    - Little-endian and big-endian support
    - Separate read/write cursors for flexible use

## Message Buffer:
- Small framed-message reader/writer for packetized communication.
    - 1-byte type and 1-byte payload length header
    - Safe sequential writing and reading of payloads
    - Designed for small transports (LoRa, BLE, ESP-NOW)

## Ring Buffer:
- Fixed-capacity circular buffer (ring buffer) for any T type.
    - Non-blocking push/pop
    - Supports move and copy semantics
    - Constant memory overhead

## Highlights
- Zero dynamic allocation: All memory is user-supplied or static.
- Header-only: Just include and use.
- Embedded-safe: Minimal dependencies, predictable behavior.
- Cross-platform: No ESP32 or device-specific code.
