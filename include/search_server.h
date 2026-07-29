#pragma once
#include "inverted_index.h"
#include <string>
#include <vector>

class SearchServer {
public:
    explicit SearchServer(const InvertedIndex& index) : index_(index) {}

    std::vector<Document> search(const std::string& query) const;

private:
    const InvertedIndex& index_;
};