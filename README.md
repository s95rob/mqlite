# MQLite

Lightweight, cross-platform and asynchronous publish-subscribe message broker implemented in modern C++20. Utilizes [Asio](https://github.com/chriskohlhoff/asio) to handle concurrent client connections powered by coroutines, and features multiplexed message routing. 

## Protocol Specification

Clients interact with a temporary text-based protocol. This makes feature mocking easy and testing can be done simply with `netcat`.  

- `CONNECT`: "Authenticates" and opens the session. All commands require "authentication" (there is no real form of authentication yet, other than this handshake).
- `SUBSCRIBE <channel>`: Registers the session to receive broadcasts on the target channel. 
- `PUBLISH <channel>:<message>`: Broadcasts payload data to all active subscribers on the specified channel. 
- `DISCONNECT`: Requests the session to be gracefully torn down. 

## Usage

This repository includes a Dockerfile for easy deployment. Clone this repository, then build and run with Docker:

```sh
docker build -t mqlite .
docker run -p 9090:9090 mqlite
```

## Building

### Requirements
- C++20 compiler (GCC, Clang, MSVC)
- CMake 3.20+

MQLite uses CMake to generate build systems across platforms.  

1. Clone this repository
2. Configure your build system with CMake, build the project
3. Run MQLite: 
    `mqlite` **or** `mqlite <port>` (default port is `9090`)