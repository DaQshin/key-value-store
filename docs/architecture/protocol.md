# Protocol

The server uses a custom length-prefixed binary protocol over TCP. All multi-byte integers are **little-endian**.

---

## Overview

```
Client                          Server
  │                               │
  │── [request frame] ──────────► │
  │                               │  parse → dispatch → serialize
  │ ◄────────────── [response] ── │
  │                               │
```

The protocol is **pipelining-capable** — a client may send multiple request frames back-to-back without waiting for responses. The server processes them in order.

---

## Request Frame

```
┌─────────────────────┐
│  total_len  (4B u32)│  ← byte length of everything that follows
├─────────────────────┤
│  nstr       (4B u32)│  ← number of string arguments
├─────────────────────┤
│  len₁       (4B u32)│  ┐
│  arg₁       (len₁ B)│  │
├─────────────────────┤  │ repeated nstr times
│  len₂       (4B u32)│  │
│  arg₂       (len₂ B)│  ┘
└─────────────────────┘
```

- `total_len` covers everything after the first 4 bytes (i.e., `nstr` + all argument fields).
- Each argument is a raw byte string with a 4-byte length prefix. No null terminator.
- Maximum `total_len`: **32 MB** (`32 << 20`). Frames larger than this cause the connection to be closed.

### Example — encoding `SET foo bar`

```
total_len = 4 + (4+3) + (4+3) = 18   → [12 00 00 00]  (little-endian 18 - wait, see below)
```

Full byte layout:

```
[12 00 00 00]  total_len = 18
[03 00 00 00]  nstr = 3
[03 00 00 00]  len("SET") = 3
[53 45 54]     "SET"
[03 00 00 00]  len("foo") = 3
[66 6F 6F]     "foo"
[03 00 00 00]  len("bar") = 3
[62 61 72]     "bar"
```

---

## Response Frame

```
┌─────────────────────┐
│  total_len  (4B u32)│  ← byte length of everything that follows
├─────────────────────┤
│  tag        (1B u8) │  ← type of the value
├─────────────────────┤
│  payload    (varies)│  ← type-specific encoding
└─────────────────────┘
```

---

## Response Type Tags

### TAG_NIL (0x00)

No payload. Indicates a missing key or absent value.

```
[total_len][00]
```

---

### TAG_ERR (0x01)

An error response with a numeric code and message string.

```
[total_len][01][code: 4B u32][msg_len: 4B u32][msg: msg_len bytes]
```

Example — `ERR_UNKNOWN`:

```
[0E 00 00 00]  total_len = 14
[01]           TAG_ERR
[01 00 00 00]  code = ERR_UNKNOWN (1)
[0F 00 00 00]  msg_len = 15
[556E6B6E6F776E20436F6D6D616E642E]  "Unknown Command."
```

---

### TAG_INT (0x02)

A 64-bit signed integer.

```
[total_len][02][value: 8B i64]
```

---

### TAG_STRING (0x03)

A length-prefixed UTF-8 string.

```
[total_len][03][len: 4B u32][bytes: len bytes]
```

---

### TAG_DOUBLE (0x04)

An IEEE 754 64-bit double.

```
[total_len][04][value: 8B f64]
```

---

### TAG_ARRAY (0x05)

An array of `n` consecutive response values. Each element is a fully serialized response value (tag + payload), without its own `total_len` wrapper.

```
[total_len][05][count: 4B u32][element₁][element₂]...[elementₙ]
```

---

## Parsing — `parse_req()`

```cpp
static int32_t parse_req(const uint8_t*& data, size_t size, std::vector<std::string>& out);
```

Reads `nstr` then iterates, reading each `(len, bytes)` pair into `out`. Advances the `data` pointer as it goes. Returns:

- `0` — success, `data` pointer is now at the end of the frame
- `-1` — malformed input (truncated field or trailing bytes)

Trailing bytes after the last argument are treated as an error (`data != end`).

---

## Response Framing — Two-Phase Write

Since `total_len` must be at the front but the payload size is only known after serialization, a two-phase approach is used:

```cpp
// Phase 1 — reserve space for the header
size_t header_pos = 0;
resp_header_alloc(conn->outgoing, &header_pos);

// Phase 2 — write payload
do_request(cmd, conn->outgoing);

// Phase 3 — fill in the length retroactively
resp_header_assign(conn->outgoing, header_pos);
```

`resp_header_alloc()` records the current buffer offset and appends 4 zero bytes as a placeholder. `resp_header_assign()` computes `out.size() - header_pos - 4` and writes it back at the saved offset.

If the payload exceeds `k_max_msg`, the buffer is truncated back to `header_pos + 4` and replaced with a `TAG_ERR / ERR_TOO_BIG` response.

---

## Pipelining

The server handles pipelining via `try_one_request()`, which is called in a loop:

```cpp
while (!conn->want_close && try_one_request(conn)) {}
```

Each call processes one complete frame from `conn->incoming` and returns `true` if a frame was consumed. The loop continues until fewer than 4 bytes remain or no complete frame is available.

---

## Connection Closure

The server closes a connection by setting `conn->want_close = true` in any of these cases:

- Incoming frame `total_len` exceeds `k_max_msg`
- `parse_req()` returns `-1` (malformed frame)
- `read()` returns 0 (clean EOF from client)
- `read()` or `write()` returns an unrecoverable error (not `EAGAIN`/`EWOULDBLOCK`)
- The connection exceeds `k_idle_timeout_ms` without any IO activity

`want_close` is checked after each IO operation. The connection is destroyed at the end of that event loop iteration.
