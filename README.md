# Distributed ASCII Art System

[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg?logo=c%2B%2B)](https://en.wikipedia.org/wiki/C%2B%2B17)
[![POSIX Sockets](https://img.shields.io/badge/Network-POSIX%20Sockets-informational.svg)](https://en.wikipedia.org/wiki/Berkeley_sockets)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![Build: Make](https://img.shields.io/badge/Build-Makefile-brightgreen.svg)](Makefile)

A multi-threaded networked storage and retrieval system implemented in **C++17**. The project implements a **custom block-based file system** mapped onto a binary disk image, an **HTTP/1.1-compatible server** managing concurrent client connections via POSIX threads, and a dedicated CLI client.

---

## Key Features

* **Multi-Threaded Server:** Handles concurrent incoming requests using dedicated worker threads and robust POSIX stream sockets (supporting both IPv4 and IPv6).
* **Custom Block-Based Virtual File System (`disk.bin`):**
  * Fixed 256-byte block allocation.
  * Directory indexing: directory blocks store metadata (up to 7 file entries of 32-byte names with data block pointers, plus forward pointers for directory chaining).
  * Chained data allocation for arbitrarily large ASCII text files without virtual disk size boundaries.
* **HTTP/1.1 Compatible Parser:** The server natively understands HTTP requests (`GET /LIST`, `GET /GET/<figure>`), allowing seamless interaction with standard tools like `curl`, `wget`, or web browsers.
* **Distributed Protocol Simulation:**
  * Inter-server discovery and proxy routing protocol specification and simulation (`docs/Protocol_Specification.md`, `simulation/`).

---

## Architecture Overview

```text
       +--------------------------------------------------------+
       |                 Clients / HTTP Agents                  |
       |  (CLI Client / curl / wget / Web Browser / Proxies)    |
       +---------------------------+----------------------------+
                                   |
                         TCP/IP (Port 1234)
                                   |
                                   v
       +--------------------------------------------------------+
       |               Multi-Threaded HTTP Server               |
       |  - Socket Listener (IPv4 / IPv6)                       |
       |  - Worker Thread Pool per Connection                   |
       |  - HTTP Request & Response Serializer                  |
       +---------------------------+----------------------------+
                                   |
                           C++ Virtual FS API
                                   |
                                   v
       +--------------------------------------------------------+
       |             Custom Block-Based File System             |
       |  - Directory Blocks (7 entries x 32 bytes + chain ptr) |
       |  - Data Blocks (256 bytes fixed payload)               |
       |  - Binary Storage Engine (disk.bin)                    |
       +--------------------------------------------------------+
```

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
├── docs/                            # Technical guides & documentation
│   ├── Compilation_Guide.md         # Step-by-step build instructions
│   ├── Test_Cases.md                # Comprehensive test matrix (CLI & HTTP)
│   └── Protocol_Specification.md    # Distributed proxy protocol design
├── simulation/                      # Inter-node protocol simulation program
├── Makefile                         # Build automation script
├── LICENSE                          # MIT License
└── README.md                        # Project documentation
```

---

## Quick Start

### Prerequisites
* A Linux environment (or **WSL** on Windows).
* `g++` with C++17 support.
* `make`.

### 1. Build
```bash
# Clone the repository
git clone https://github.com/josuetorres27/distributed-ascii-art-system.git
cd distributed-ascii-art-system

# Compile binaries
make
```

### 2. Run the Server
```bash
./exec/Server
```
*On launch, the server reads the `ascii_figures/` folder, initializes `disk.bin`, indexes all figures, and begins listening on port `1234`.*

### 3. Query the System

**Using the included CLI client:**
```bash
# List all available figures stored on disk
./exec/Client LIST

# Retrieve a specific figure
./exec/Client GET spiderman_1
```

**Using standard HTTP tools (`curl` / `wget`):**
```bash
# List figures
curl http://localhost:1234/LIST

# Retrieve figure
curl http://localhost:1234/GET/spiderman_1

# Using wget
wget -qO- http://localhost:1234/GET/batman
```

---

## Available ASCII Art Assets

The server includes preloaded art assets in `ascii_figures/`, including:
* **Superheroes & Pop Culture:** `batman`, `mcqueen`, `spiderman_1`, `spiderman_2`
* **Gaming:** `charizard`, `gengar`, `goomba`, `jigglypuff`, `mario`, `pikachu`, `yoshi`
* **Technology:** `computer`

---

## Author

* **Josue Torres Sibaja**
  * GitHub: [@josuetorres27](https://github.com/josuetorres27)

---

## License

This project is licensed under the **MIT License** - see the [LICENSE](LICENSE) file for details.
