# Key-Value Store

A fast, in-memory key-value store written in C++ with support for string operations and sorted sets.

---

## Table of Contents

- [Getting Started](#getting-started)
- [Architecture & Design](docs/architecture/protocol.md)
- [API Reference](docs/api-reference.md)

## Getting Started

### Run Locally

Build the project and start the server and client:

```bash
make all
make run_server_prod
make run_client
```

### Run with Docker

Start the server in detached mode:

```bash
docker compose up -d server
```

Start the client:

```bash
docker compose run --rm client
```

The `--rm` flag automatically removes the client container after it exits. If you want to keep the container for debugging or inspection, omit the flag:

```bash
docker compose run client
```

Stop the server:

```bash
docker compose down
```
