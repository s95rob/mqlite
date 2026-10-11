# --- Stage 1: Build ---
FROM debian:bookworm-slim AS builder

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    git \
    ca-certificates \
    && rm -rf /var/lib/apt/lists*
    # NOTE: Deliberately not installing `libasio-dev` because it pulls in 
    # Boost dependency bloat and the CMake script explicitly fetches the standalone version

WORKDIR /app

COPY . .

RUN cmake -B build -DCMAKE_BUILD_TYPE=Release && \
    cmake --build build --config Release

# --- Stage 2: Runner ---
FROM debian:bookworm-slim

WORKDIR /app

# Only copy the compiled binary
COPY --from=builder /app/build/mqlite /app/mqlite

# Expose the default TCP port
EXPOSE 9090

ENTRYPOINT ["./mqlite"]