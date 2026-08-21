# 🚀 Local Search Engine & Asynchronous TCP Server

A high-performance, multithreaded local search engine running on a custom asynchronous TCP server. Written entirely in modern C++ (C++17) without relying on heavy third-party frameworks. 

This project is designed to demonstrate core backend engineering concepts: **system programming, non-blocking I/O (epoll), multithreading, algorithmic ranking (TF-IDF), and binary data serialization.**

---

## 🧠 Architecture Overview

The system is strictly divided into two main layers:

### 1. The Search Engine Core
* **Multithreaded Indexing:** Reads multiple `.txt` files concurrently using `std::thread` and `std::thread::hardware_concurrency` to prevent I/O bottlenecks.
* **Inverted Index:** Maps words to the documents they appear in, ensuring $O(1)$ or $O(\log N)$ lookup times.
* **TF-IDF Ranking:** Implements the Term Frequency-Inverse Document Frequency mathematical algorithm to calculate the true relevance of a query to a document, ignoring stop words and rewarding rare terms.
* **Binary Caching:** Serializes the computed index into a raw binary file (`index.bin`). On subsequent restarts, the engine loads the cache directly from disk, bypassing the indexing phase and starting instantly.

### 2. The Network Layer (Event Loop)
* **Linux `epoll`:** Utilizes the highly efficient `epoll` system call for I/O multiplexing, allowing the server to monitor multiple file descriptors simultaneously without allocating a thread per client.
* **Non-Blocking Sockets:** All TCP sockets are configured with `O_NONBLOCK` via `fcntl`, ensuring the main event loop is never blocked by slow clients.
* **Custom Protocol:** Currently processes raw TCP streams, maintaining active sessions with clients until explicitly disconnected.

---

## 🛠️ Tech Stack & Requirements

* **Language:** C++17
* **Build System:** CMake (3.10+)
* **OS:** Linux (Requires POSIX API for sockets and `epoll`)
* **Compiler:** GCC / Clang

---

## 📁 Project Structure

```text
local-search-engine/
├── CMakeLists.txt       # CMake build configuration
├── README.md            # Project documentation
├── include/             # Header files (.h)
│   ├── search_server.h  # TF-IDF logic and inverted index API
│   ├── tcp_server.h     # epoll event loop and socket management
│   └── ...
├── src/                 # Source files (.cpp)
│   ├── main.cpp         # Entry point, argument parsing, threading
│   ├── search_server.cpp
│   ├── tcp_server.cpp
│   └── ...
└── docs/                # Directory containing target .txt files for indexing


🚀 How to Build and Run
1. Build the project
Clone the repository and compile it using CMake:

Bash
git clone [https://github.com/slavik789012-dev/local-search-engine.git](https://github.com/slavik789012-dev/local-search-engine.git)
cd local-search-engine
mkdir build && cd build
cmake ..
make

2. Start the Server
Run the executable. You must provide the absolute path to the directory containing your text files using the --dir flag. If the flag is omitted, the server will ask for the path interactively.

Bash
./search_engine --dir /absolute/path/to/your/docs
Expected Server Output:

Plaintext
Cache not found. Starting multithreaded indexing...
Thread read 1024 bytes from file
Thread read 2048 bytes from file
Total number of documents in the index: 15
Indexing completed. Saving to cache...
Search engine server is running and listening on port 8080...
3. Send Queries (Client Side)
Once the server is running, you can connect to it using netcat (nc) or any raw TCP client from another terminal:

Bash
nc 127.0.0.1 8080
Type your search query (e.g., linux backend) and press Enter.

Expected Client Output:

Plaintext
Found documents (Top-2):
  DocID: 3 (Relevance: 0.842)
  DocID: 1 (Relevance: 0.125)
-----------------------
🔮 Roadmap / Future Improvements
[ ] HTTP Support: Upgrade the raw TCP stream to parse HTTP GET requests, allowing users to query the engine directly from a web browser.

[ ] Graceful Shutdown: Implement signal handling (SIGINT, SIGTERM) to safely close file descriptors and sockets upon termination.

[ ] Load Testing: Benchmark the epoll loop under high concurrency using tools like wrk or Apache Benchmark.