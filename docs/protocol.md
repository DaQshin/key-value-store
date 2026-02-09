# Protocol Documentation

This document describes the protocol format and I/O guarantees used by the system for request and response communication.

---

## 1. Protocol Format

All messages use a simple length-prefixed framing:

```
[ 4 bytes length ][ payload ]
```

* The first 4 bytes encode the length of the payload.
* The payload immediately follows the length field.
* There are no delimiters inside the payload.

---

## 2. Byte Order

* The length field is encoded in **little-endian** byte order.
* Both sender and receiver must interpret the length using the same endianness.

---

## 3. Invariants

The following invariants are enforced during serialization and deserialization:

* `length <= k_max_msg`
* The payload is **binary-safe** and may contain arbitrary bytes, including `\0`.
* Every message **must** include a 4-byte length prefix.

Any message violating these invariants is considered invalid and must be rejected.

---

## 4. I/O Guarantees

To ensure correctness over stream-oriented transports, the following guarantees are provided:

* `write_all`

  * Retries on partial writes.
  * Retries when interrupted by signals (`EINTR`).
  * Only returns success after all bytes have been written.

* `read_full`

  * Retries when interrupted by signals (`EINTR`).
  * Reads exactly the requested number of bytes unless EOF or a real error occurs.

These helpers ensure that logical messages are transmitted atomically at the protocol level.

---

## 5. Design Rationale

This protocol design intentionally favors simplicity and robustness:

* Avoids buffer overflows by validating message length before allocation or reads.
* Avoids undefined behavior by using explicit byte copies instead of casting.
* Keeps parsing logic simple, fast, and unambiguous.
* Eliminates delimiter-based ambiguity and escaping requirements.
