#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <fstream>

struct Document {
    size_t id;
    double relevance;
};

struct IndexEntry {
    size_t doc_id;
    size_t term_frequency;
};

class InvertedIndex {
private:
    std::vector<std::string> docs_;
    std::unordered_map<std::string, std::vector<IndexEntry>> index_;
    std::mutex mtx_;
public:
    InvertedIndex() = default;
    static std::vector<std::string_view> SplitBySpaces(std::string_view text);
    static std::string NormalizeWord(std::string_view word_view);
    void AddDocument(const std::string& document_name, const std::string& text);
    size_t GetSize() const;
    const std::string& GetDocumentName(size_t id) const;
    const std::vector<IndexEntry>& GetWordEntries(const std::string& word) const;
    bool save(const std::string& filename);
    bool load(const std::string& filename);
};