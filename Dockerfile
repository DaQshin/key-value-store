FROM ubuntu:24.04 AS builder

WORKDIR /app

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    libgtest-dev \
    g++ \
    make \
    && rm -rf /var/lib/apt/lists/*

COPY Makefile .
COPY include/ include/
COPY src/ src/

RUN make all MODE=release

FROM ubuntu:24.04 AS server

WORKDIR /app
COPY --from=builder /app/build/server ./build/server
EXPOSE 5000

CMD ["./build/server"]

FROM ubuntu:24.04 AS client
WORKDIR /app
COPY --from=builder /app/build/client ./build/client
EXPOSE 5000

CMD ["./build/client"]