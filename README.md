# Distributed ASCII Art System

[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue?style=flat-square&logo=c%2B%2B)](https://en.wikipedia.org/wiki/C%2B%2B17)
[![POSIX Sockets](https://img.shields.io/badge/Network-POSIX%20Sockets-informational?style=flat-square)](https://en.wikipedia.org/wiki/Berkeley_sockets)
[![Build: Make](https://img.shields.io/badge/Build-Makefile-brightgreen?style=flat-square)](Makefile)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow?style=flat-square)](LICENSE)

A multi-threaded ASCII art storage and retrieval server implemented in C++17. The project implements a custom block-based file system stored in a binary disk image (`disk.bin`), an HTTP/1.1-compatible server built on POSIX sockets that handles each connection in its own thread, a dedicated CLI client, and a retro CRT web terminal.

The running system is a single server node. The "distributed" part of the project is a design for proxy-to-server communication, documented in [`docs/Protocol_Specification.md`](docs/Protocol_Specification.md) and illustrated by a small walkthrough program in [`simulation/`](simulation/).

---

## Features

- **Multi-Threaded Server:** Each accepted connection is handled by its own detached `std::thread` (thread-per-connection). The listening socket is an IPv6 socket with `IPV6_V6ONLY` disabled, so it accepts both IPv6 and IPv4 clients.
- **Custom Block-Based Virtual File System (`disk.bin`):**
  - Fixed 256-byte blocks.
  - Directory blocks hold 7 entries of 36 bytes each (a 32-byte name plus a `uint32` pointer to the figure's first data block) and a `uint32` pointer to the next directory block (7 × 36 + 4 = 256 bytes). Directory blocks are chained as more figures are added.
  - Figure content is stored in contiguous 256-byte data blocks, starting at the block referenced by the directory entry. The disk image grows as blocks are allocated.
- **HTTP/1.1-Compatible Request Handling:** `GET` requests are routed by URL path (see [HTTP Endpoints](#http-endpoints)); any other method returns `400 Bad Request`.
- **Interactive Web Terminal:** Opening `http://localhost:1234/` in a browser serves a retro CRT terminal UI (see [Web Terminal](#web-terminal)).
- **Protocol Design and Walkthrough:** A text protocol for server discovery, figure requests and deregistration between servers and routing proxies ([`docs/Protocol_Specification.md`](docs/Protocol_Specification.md)), plus a two-thread walkthrough program in `simulation/` that prints the message sequence. The walkthrough uses `std::mutex` and `std::condition_variable` to coordinate the two threads; it does not open sockets.

---

## Architecture Overview

```text
       +--------------------------------------------------------+
       |                 Clients / HTTP Agents                  |
       |       (CLI Client / curl / wget / Web Browser)         |
       +---------------------------+----------------------------+
                                   |
                         TCP/IP (Port 1234)
                                   |
                                   v
       +--------------------------------------------------------+
       |               Multi-Threaded HTTP Server               |
       |  - Dual-stack Socket Listener (IPv6 / IPv4)            |
       |  - One detached std::thread per connection             |
       |  - Path-based routing: HTML / JSON / text/plain        |
       +---------------------------+----------------------------+
                                   |
                           C++ Virtual FS API
                                   |
                                   v
       +--------------------------------------------------------+
       |             Custom Block-Based File System             |
       |  - Directory Blocks (7 entries x 36 bytes + next ptr)  |
       |    entry = 32-byte name + uint32 first data block      |
       |  - Data Blocks (256 bytes, contiguous per figure)      |
       |  - Binary Storage Engine (disk.bin)                    |
       +--------------------------------------------------------+
```

---

## Available ASCII Art Assets

The server includes preloaded art assets in `ascii_figures/`, including:

- **Superheroes and Pop Culture:** `batman`, `mcqueen`, `spiderman_1`, `spiderman_2`.
- **Gaming:** `charizard`, `gengar`, `goomba`, `jigglypuff`, `mario`, `pikachu`, `yoshi`.
- **Technology:** `computer`.

---

## Project Structure

```text
distributed-ascii-art-system/
├── src/                             # Core C++ source files
│   ├── Client.cc                    # CLI Client implementation
│   ├── Server.cc                    # Multi-threaded server implementation
│   ├── FileSystem.h / .cc           # Virtual block-based file system engine
│   ├── Socket.h / .cc               # Stream socket wrapper
│   └── VSocket.h / .cc              # Base virtual socket wrapper
├── ascii_figures/                   # Repository of ASCII art text assets
├── web/                             # Interactive CRT Terminal Web UI (HTML5, CSS3, JS)
│   └── index.html                   # Standalone retro terminal & asset viewer
├── docs/                            # Technical guides & documentation
│   ├── Compilation_Guide.md         # Step-by-step build instructions
│   ├── Test_Cases.md                # Manual functional test scenarios (CLI & HTTP)
│   └── Protocol_Specification.md    # Proxy-to-server protocol design
├── simulation/                      # Two-thread protocol walkthrough (main.cc, Makefile)
├── Makefile                         # Build automation script
├── LICENSE                          # MIT License
└── README.md                        # Project documentation
```

---

## Getting Started

### Prerequisites

- A Linux environment (or WSL on Windows).
- `g++` with C++17 support.
- `make`.
- glibc 2.34 or newer. The root `Makefile` does not pass `-pthread`, which is only safe where `libpthread` is part of `libc`.

Tested environment: Ubuntu 22.04 (WSL 2), g++ 11.4.0, glibc 2.35.

### Build

```bash
# Clone the repository
git clone https://github.com/josuetorres27/distributed-ascii-art-system.git
cd distributed-ascii-art-system

# Compile binaries (exec/Server and exec/Client)
make

# Remove build artifacts (obj/ and exec/)
make clean
```

### Run

Run the server from the repository root: it opens `ascii_figures/`, `web/index.html` and `disk.bin` using relative paths.

```bash
./exec/Server
```

On launch, the server creates `disk.bin` only if it does not already exist, loads the `.txt` files from `ascii_figures/` (skipping figures already on disk), and listens on port `1234`. To rebuild the disk image from scratch, for example after editing or adding figures, delete it and restart:

```bash
rm disk.bin
./exec/Server
```

---

## Usage

### CLI Client

The client always connects to `::1` (IPv6 localhost) on port `1234`; the host and port are fixed in the source. It prints the raw HTTP response, including the status line and headers.

```bash
# List all available figures stored on disk
./exec/Client LIST

# Retrieve a specific figure
./exec/Client GET spiderman_1
```

Example output of `./exec/Client LIST` (truncated):

```text
HTTP/1.1 200 OK
Content-Type: text/plain; charset=utf-8
Access-Control-Allow-Origin: *
Connection: close

Figures stored on disk:
Directory block #0:
- charizard (19 data blocks)
- spiderman_1 (17 data blocks)
...
```

The order of figures follows the order in which the files were read from `ascii_figures/`. Requesting a missing figure (`./exec/Client GET ironman`) returns `HTTP/1.1 404 Not Found` with the body `Figure not found`.

### HTTP Tools

```bash
# List figures
curl http://localhost:1234/LIST

# Retrieve figure
curl http://localhost:1234/GET/spiderman_1

# Figure names as JSON
curl http://localhost:1234/api/figures

# Using wget
wget -qO- http://localhost:1234/GET/batman
```

### HTTP Endpoints

The server reads a single request, routes `GET` requests by URL path, writes the response and closes the connection (`Connection: close`).

| Path | Response |
| :--- | :--- |
| `/` or `/index.html` | Web terminal (`text/html`) |
| `/api/figures` or `/api/list` | JSON array of figure names (`application/json`, `Access-Control-Allow-Origin: *`) |
| `/LIST` | Directory listing with data-block counts (`text/plain`) |
| `/GET/<name>` | Figure content (`text/plain`), or `404 Not Found` |
| `/<name>` | Same as `/GET/<name>` |
| Any non-`GET` method | `400 Bad Request` |

### Web Terminal

Open `http://localhost:1234/` while the server is running. The page (`web/index.html`) includes:

- **Live and Standalone Modes:** Figures are loaded from the C++ server (`/api/figures`, `/GET/<name>`). If the server is unreachable, for example when `web/index.html` is opened directly from disk, the page falls back to a catalog of figures embedded in the file.
- **Search and Category Filters:** Filter the figure list by name and by category (All, Heroes, Gaming, Tech).
- **Deep Links:** `?fig=<name>` (or `?figure=<name>`) loads a specific figure when `web/index.html` is opened as a standalone file. The C++ server does not strip query strings, so `http://localhost:1234/?fig=<name>` returns `404 Not Found`.
- **Five Themes:** Matrix Phosphor, Amber 1982, Cyberpunk Neon, Cobalt VT220 and Monochrome.
- **Effects:** CRT effect toggle, optional typewriter animation, and optional sound effects synthesized with the Web Audio API.
- **Zoom Controls:** 40% to 250%.
- **Export:** Copy to clipboard and PNG export of the current figure.

### Protocol Walkthrough

```bash
cd simulation
make run
```

This compiles `simulation/main.cc` and runs a scripted, roughly 20-second sequence in which a proxy thread and a server thread print the discovery, figure request, `404 Not Found` and shutdown messages described in the protocol specification.

---

## Testing

The repository does not include automated tests. Manual CLI and HTTP test scenarios are documented in [docs/Test_Cases.md](docs/Test_Cases.md).

---

## Documentation

- [Compilation_Guide.md](docs/Compilation_Guide.md): compilation and execution guide.
- [Test_Cases.md](docs/Test_Cases.md): manual CLI and HTTP functional test scenarios.
- [Protocol_Specification.md](docs/Protocol_Specification.md): proxy-server communication protocol.

---

## License

This project is licensed under the [MIT License](LICENSE).

---

Created by **Josue Torres Sibaja** · [Portfolio](https://josuetorressibaja.com/) · [LinkedIn](https://www.linkedin.com/in/josue-torres-sibaja/) · [GitHub](https://github.com/josuetorres27)
