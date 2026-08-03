# Local Search Engine (Inverted Index)

A fast, multithreaded local search engine written in modern C++. This engine indexes `.txt` files in a specified directory and allows you to perform fast word searches using an Inverted Index data structure.

## 🚀 Features

* **Inverted Index Architecture:** Efficiently maps words to the documents they appear in, ensuring lightning-fast search queries.
* **Multithreaded Indexing (New!):** Utilizes `std::thread` and hardware concurrency to parse multiple documents in parallel. This drastically reduces the initial indexing time for large sets of files. Thread-safe operations are guaranteed via `std::mutex`.
* **Binary Caching & Serialization (New!):** Once the files are indexed, the engine saves the entire dictionary to a binary cache file (`index.bin`). On subsequent runs, the program loads instantly from the disk, completely skipping the expensive parsing phase.
* **Smart Text Parsing:** Automatically handles whitespaces, tabs, newlines (`\n`, `\r`), and normalizes text by ignoring punctuation.

## 🧠 How It Works

1. **Initialization:** The program first checks for the existence of `index.bin`.
2. **Fast Boot:** If the cache exists, the engine loads it directly into memory in milliseconds.
3. **Parallel Parsing:** If the cache is missing (e.g., first run or deleted), the engine collects all `.txt` files and distributes them evenly across available CPU cores. Each thread reads and normalizes its chunk of files, safely merging the results into the global index.
4. **Search:** The user enters a query. The engine normalizes the input and instantly returns the documents containing the requested words.

## 🛠️ Prerequisites

* A C++ compiler that supports **C++17** or higher (GCC, Clang, or MSVC).
* CMake (optional, but recommended for building).

## 💻 Usage

1. Place your text files (`.txt`) inside the target directory (e.g., `docs/`).
2. Build and run the executable.
3. On the first run, you will see the multithreaded indexing process in action.
4. Type a word to search across all indexed documents.
5. Restart the application to see the instant cache-loading feature!

## 📝 Example Output

```text
Trying to load the index from cache (index.bin)...
Cache not found. Starting multithreaded indexing...
Indexing completed. Saving to cache...
Cache saved successfully.
Total documents in index: 154
Search engine is ready!
Enter word to search: apple
Found in: doc1.txt (3 occurrences), doc4.txt (1 occurrence)