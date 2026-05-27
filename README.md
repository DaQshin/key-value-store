# Key-Value Store

A fast, in-memory key-value store written in C++ with support for string operations and sorted sets.

---

## Table of Contents

- [Architecture & Design](#architecture--design)
- [Getting Started](#getting-started)
- [API Reference](#api-reference)

---

## API Reference

All commands are case-insensitive. Keys and values are UTF-8 strings. Scores are IEEE 754 double-precision floats.

---

### String Commands

---

#### `GET`

Retrieve the value associated with a key.

**Syntax**

```
GET <key>
```

**Returns**

- The value string if the key exists
- `(nil)` if the key does not exist
  **Example**

```
SET language C++
GET language     # → "C++"
GET missing      # → (nil)
```

---

#### `SET`

Set a key to a string value. Creates the key if it does not exist; overwrites if it does.

**Syntax**

```
SET <key> <value>
```

**Returns**

- `OK` on success
  **Example**

```
SET version 1.0.0   # → OK
SET version 2.0.0   # → OK  (overwrites)
```

---

#### `DEL`

Delete a key and its associated value (string or sorted set).

**Syntax**

```
DEL <key>
```

**Returns**

- `1` if the key existed and was deleted
- `0` if the key did not exist
  **Example**

```
SET temp value
DEL temp    # → 1
DEL temp    # → 0
```

---

#### `EXISTS`

Check whether a key exists in the store.

**Syntax**

```
EXISTS <key>
```

**Returns**

- `1` if the key exists
- `0` if the key does not exist
  **Example**

```
SET x 42
EXISTS x       # → 1
EXISTS y       # → 0
```

---

#### `FLUSH`

Remove all keys from the store. This operation is irreversible.

**Syntax**

```
FLUSH
```

**Returns**

- `OK`
  **Example**

```
SET a 1
SET b 2
FLUSH       # → OK
EXISTS a    # → 0
```

---

### Sorted Set Commands

Sorted sets store a collection of unique string members each associated with a `double` score. Members are ordered by score from lowest to highest.

---

#### `ZADD`

Add a new (name, score) pair to a sorted set, or update the score of an existing pair by name.

**Syntax**

```
ZADD <key> <score> <name>
```

**Parameters**

- `key` — The name of the sorted set
- `score` — A floating-point number
- `name` — A unique string identifier within this set
  **Returns**
- `1` if the pair was newly added
- `0` if the name already existed and its score was updated
  **Example**

```
ZADD scores 95.5 alice    # → 1
ZADD scores 87.0 bob      # → 1
ZADD scores 99.0 alice    # → 0  (score updated)
```

---

#### `ZSCORE`

Find a pair by name and return its score.

**Syntax**

```
ZSCORE <key> <name>
```

**Returns**

- The score as a double if the name exists
- `(nil)` if the key or name does not exist
  **Example**

```
ZADD temps 36.6 patient1
ZSCORE temps patient1    # → 36.6
ZSCORE temps patient2    # → (nil)
```

---

#### `ZREM`

Find a pair by name and delete it from the sorted set.

**Syntax**

```
ZREM <key> <name>
```

**Returns**

- `1` if the name existed and was removed
- `0` if the name did not exist
  **Example**

```
ZADD rankings 10.0 alpha
ZREM rankings alpha    # → 1
ZREM rankings alpha    # → 0
```

---

#### `ZQUERY`

Return all members whose scores fall within a given range, in ascending score order.

**Syntax**

```
ZQUERY <key> <min_score> <max_score>
```

**Parameters**

- `min_score` — Lower bound (inclusive)
- `max_score` — Upper bound (inclusive)
  **Returns**
- An ordered list of `(member, score)` pairs within the range
- An empty list if no members match
  **Example**

```
ZADD points 10.0 a
ZADD points 20.0 b
ZADD points 30.0 c
ZADD points 40.0 d

ZQUERY points 15.0 35.0
# → b  20.0
# → c  30.0
```

---
