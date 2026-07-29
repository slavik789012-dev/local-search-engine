#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
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

        InvertedIndex index;
        bool has_files = false;

        for (const auto& entry : fs::recursive_directory_iterator(dir_path)) {
            if (entry.is_regular_file() && entry.path().extension() == ".txt") {
                std::string text = ReadFile(entry.path());
                index.AddDocument(entry.path().string(), text);
                has_files = true;
            }
        }

        if (!has_files) {
            throw std::runtime_error("Directory is empty or contains no .txt files.");
        }

        SearchServer server(index);
        std::string query;
        while (true) {
            std::cout << "Search > ";
            if (!std::getline(std::cin, query)) break;
            if (query.empty()) continue;
            if (query == "!exit") break;
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