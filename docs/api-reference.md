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
