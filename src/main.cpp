#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <thread>
#include "inverted_index.h"
#include "search_server.h"

namespace fs = std::filesystem;

std::string ReadFile(const fs::path& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filepath.string());
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

int main(int argc, char* argv[]) {
    try {
        if (argc != 3 || std::string(argv[1]) != "--dir") {
            throw std::invalid_argument("Usage: ./search_engine --dir <path>");
        }

        std::string dir_path = argv[2];

        std::cout << "DEBUG: Looking for directory at: " << fs::absolute(dir_path) << std::endl;

        if (!fs::exists(dir_path) || !fs::is_directory(dir_path)) {
            throw std::invalid_argument("Invalid directory path provided.");
        }

        std::vector<fs::path> text_files;
        for (const auto& entry : fs::recursive_directory_iterator(dir_path)) {
            if (entry.is_regular_file() && entry.path().extension() == ".txt") {
                text_files.push_back(entry.path());
            }
        }
        if (text_files.empty()) {
            throw std::runtime_error("Directory is empty or contains no .txt files.");
        }
        size_t num_threads = std::thread::hardware_concurrency();
        if (num_threads == 0) num_threads = 4;
        if (num_threads > text_files.size()) num_threads = text_files.size();
        std::vector<std::thread> threads;
        size_t chunk_size = text_files.size() / num_threads;


        InvertedIndex index;
        const std::string cache_filename = "index.bin";

        std::cout << "Trying to load the index from cache (" << cache_filename << ")...\n";

        if (index.load(cache_filename)) {
            std::cout << "Success! Index loaded from disk.\n";
        }
        else {
            std::cout << "Cache not found. Starting multithreaded indexing...\n";

            for (size_t i = 0; i < num_threads; ++i) {
                size_t start_idx = i * chunk_size;
                size_t end_idx = (i == num_threads - 1) ? text_files.size() : start_idx + chunk_size;
                threads.emplace_back([start_idx, end_idx, &text_files, &index]() {
                    for (size_t j = start_idx; j < end_idx; ++j) {
                        std::string text = ReadFile(text_files[j]);
                        std::cout << "Thread read " << text.size() << " bytes from file\n";
                        index.AddDocument(text_files[j].string(), text);
                    }
                    });
            }

            for (auto& t : threads) {
                if (t.joinable()) t.join();
            }
            std::cout << "Total number of documents in the index: " << index.GetSize() << "\n";

            std::cout << "Indexing completed. Saving to cache...\n";
            if (index.save(cache_filename)) {
                std::cout << "Cache saved successfully.\n";
            }
            else {
                std::cerr << "Error: failed to save cache to file!\n";
            }
        }
        std::cout << "Total documents in index: " << index.GetSize() << "\n";
        std::cout << "Search engine is ready!\n";

        SearchServer server(index);
        std::string query;
        while (true) {
            std::cout << "Search > ";
            if (!std::getline(std::cin, query)) break;
            if (query.empty()) continue;
            if (query == "!exit") break;

            std::cout << "\n[DEBUG] Original query: {" << query << "}\n";
            std::vector<std::string_view> tokens = InvertedIndex::SplitBySpaces(query);

            std::cout << "[DEBUG] The query has been split into: " << tokens.size() << " word(s).\n";
            for (std::string_view view : tokens) {
                std::string word = InvertedIndex::NormalizeWord(view);
                std::cout << "[DEBUG] A note after normalization: {" << word << "}\n";

                const auto& entries = index.GetWordEntries(word);
                if (entries.empty()) {
                    std::cout << "[DEBUG] ERROR: There isn't {" << word << "} not even in the dictionary!\n";
                }
                else {
                    std::cout << "[DEBUG] SUCCESS: The Word {" << word << "} found in " << entries.size() << " documents.\n";
                }
            }
            std::cout << "-----------------------\n";

            auto results = server.search(query);
            if (results.empty()) std::cout << "No matches found.\n";
            else {
                std::cout << "Found documents (Top-" << results.size() << "):\n";
                for (const auto& doc : results) {
                    std::cout << "  File: " << index.GetDocumentName(doc.id)
                        << " (Relevance: " << doc.relevance << ")\n";
                }
            }
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}