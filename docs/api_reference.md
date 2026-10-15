# API Reference

Keys and values are arbitrary byte strings. Command names are **case-sensitive** and must be uppercase. A known command with the wrong number of arguments returns `ERR_UNKNOWN`.

Response types (`nil`, `int`, `string`, `error`) are the wire tags described in the [protocol](architecture/protocol.md). The REPL client prints them as `(nil)`, `(int) n`, `(str) …`, `(err) code msg`.

## Summary

| Command             | Syntax              | Returns                        |
| ------------------- | ------------------- | ------------------------------ |
| [`PING`](#ping)     | `PING`              | string `hello client`          |
| [`SET`](#set)       | `SET key value`     | string `Operation successful.` |
| [`GET`](#get)       | `GET key`           | string value, or nil           |
| [`DEL`](#del)       | `DEL key`           | string (see below)             |
| [`EXISTS`](#exists) | `EXISTS key`        | int `1` or `0`                 |
| [`FLUSH`](#flush)   | `FLUSH`             | string `Operation successful.` |
| [`EXPIRE`](#expire) | `EXPIRE key ttl_ms` | int `1` or `0`                 |
| [`PTTL`](#pttl)     | `PTTL key`          | int (ms, `-1`, or `-2`)        |

---

## PING

Health check.

```
PING            → "hello client"
```

## SET

Create or overwrite a key. Overwriting keeps any TTL the key already has.

```
SET language C++    → "Operation successful."
```

## GET

```
GET language        → "C++"
GET missing         → (nil)
```

## DEL

Delete a key. The reply is a string, not a count.

| Situation                     | Reply                   |
| ----------------------------- | ----------------------- |
| Key existed                   | `Operation successful.` |
| Key missing                   | `Value Not Found.`      |
| Database has never held a key | `Empty DB.`             |

```
SET temp x
DEL temp            → "Operation successful."
DEL temp            → "Value Not Found."
```

## EXISTS

```
EXISTS language     → (int) 1
EXISTS missing      → (int) 0
```

## FLUSH

Remove every key. Irreversible.

```
FLUSH               → "Operation successful."
```

---

## Expiry (TTL)

Expiry times are in **milliseconds** and measured on a monotonic clock. They are in-memory only and are lost on restart.

### EXPIRE

Set a key to expire `ttl_ms` milliseconds from now. A **negative** value removes the TTL.

| Situation                  | Reply                                |
| -------------------------- | ------------------------------------ |
| Key exists                 | `(int) 1`                            |
| Key missing                | `(int) 0`                            |
| `ttl_ms` is not an integer | error `ERR_BAD_ARG` (`expect int64`) |

```
SET session abc
EXPIRE session 5000     → (int) 1
EXPIRE session -1       → (int) 1    # TTL removed
```

### PTTL

Remaining time to live.

| Reply   | Meaning            |
| ------- | ------------------ |
| `n ≥ 0` | `n` ms left        |
| `-1`    | Key exists, no TTL |
| `-2`    | Key does not exist |

```
EXPIRE session 5000
PTTL session            → (int) 4987
PTTL nothing            → (int) -2
```

An expired key is removed by the server's timer step, which runs at the end of each event-loop iteration. Expiry can lag the deadline if the server is busy, and at most 2000 keys are expired per iteration.

---

## Errors

| Code | Message                | Cause                                                     |
| ---- | ---------------------- | --------------------------------------------------------- |
| 1    | `Unknown Command.`     | Unrecognised command, wrong case, or wrong argument count |
| 2    | `Response is too big.` | Reply larger than 32 MiB                                  |
| 3    | `expect int64`         | Non-integer TTL passed to `EXPIRE`                        |

---

## Using the REPL client

```
make all
make run_client
cacheline> SET name alice
cacheline> GET name
cacheline> exit()
```
