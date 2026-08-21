#include "inverted_index.h"
#include "inverted_index.h"
#include "inverted_index.h"
#include "inverted_index.h"
#include <sstream>
#include <cctype>
#include <cmath>
#include <algorithm>

std::string InvertedIndex::NormalizeWord(std::string_view word_view)
{
    std::string clean_word;
    clean_word.reserve(word_view.length());
    for (char ch : word_view) {
        unsigned char u_ch = static_cast<unsigned char>(ch);
        if (!std::ispunct(u_ch)) {
            clean_word.push_back(static_cast<char>(std::tolower(u_ch)));
        }
    }
    return clean_word;
}

std::vector<std::string_view> InvertedIndex::SplitBySpaces(std::string_view text)
{
    std::vector<std::string_view> result;
    const std::string_view delimiters = " \t\n\r\f\v";
    size_t pos = text.find_first_not_of(delimiters);

    while (pos != std::string_view::npos) {
        size_t space_pos = text.find_first_of(delimiters, pos);

        result.push_back(text.substr(pos, space_pos - pos));

        pos = text.find_first_not_of(delimiters, space_pos);
    }
    return result;
}

void InvertedIndex::AddDocument(const std::string& document_name, const std::string& text) {
    std::vector<std::string_view> token_views = SplitBySpaces(text);
    std::unordered_map<std::string, size_t> words_count;
    for (const auto& view : token_views) {
        std::string word = NormalizeWord(view);
        if (!word.empty()) words_count[std::move(word)]++;
    }
    std::lock_guard<std::mutex> lock(mtx_);
    size_t doc_id = docs_.size();
    docs_.push_back(document_name);
    for (const auto& [word, freq] : words_count) {
        index_[word].push_back({ doc_id, freq });
    }
}

size_t InvertedIndex::GetSize() const
{
    return docs_.size();
}

const std::string& InvertedIndex::GetDocumentName(size_t id) const
{
    return docs_[id];
}

const std::vector<IndexEntry>& InvertedIndex::GetWordEntries(const std::string& word) const
{
    auto it = index_.find(word);
    if (it == index_.end()) {
        static const std::vector<IndexEntry> empty_result;
        return empty_result;
    }
    return it->second;
}

bool InvertedIndex::save(const std::string& filename)
{
    std::lock_guard<std::mutex> lock(mtx_);
    std::ofstream out(filename, std::ios::binary);
    if (!out) return false;
    size_t docs_size = docs_.size();
    out.write(reinterpret_cast<const char*>(&docs_size), sizeof(docs_size));
    for (size_t i = 0; i < docs_size; ++i) {
        size_t docs_name_size = docs_[i].size();
        out.write(reinterpret_cast<const char*>(&docs_name_size), sizeof(docs_name_size));
        out.write(reinterpret_cast<const char*>(docs_[i].data()), docs_name_size*sizeof(char));
    }
    size_t index_size = index_.size();
    out.write(reinterpret_cast<const char*>(&index_size), sizeof(index_size));
    for (const auto& [word, vec] : index_) {
        size_t word_size = word.size();
        out.write(reinterpret_cast<const char*>(&word_size), sizeof(word_size));
        out.write(reinterpret_cast<const char*>(word.data()), word_size);
        size_t vec_size = vec.size();
        out.write(reinterpret_cast<const char*>(&vec_size), sizeof(vec_size));
        if(vec_size > 0) out.write(reinterpret_cast<const char*>(vec.data()), vec_size * sizeof(IndexEntry));
    }
    out.close();
    return true;
}

bool InvertedIndex::load(const std::string& filename)
{
    std::lock_guard<std::mutex> lock(mtx_);
    std::ifstream in(filename, std::ios::binary);
    if (!in) return false;
    docs_.clear();
    index_.clear();
    size_t docs_size = 0;
    in.read(reinterpret_cast<char*>(&docs_size), sizeof(docs_size));
    docs_.resize(docs_size);
    for (size_t i = 0; i < docs_size; ++i) {
        size_t docs_name_size = 0;
        in.read(reinterpret_cast<char*>(&docs_name_size), sizeof(docs_name_size));
        docs_[i].resize(docs_name_size);
        in.read(&docs_[i][0], docs_name_size);
    }
    size_t index_size = 0;
    in.read(reinterpret_cast<char*>(&index_size), sizeof(index_size));
    for (size_t i = 0; i < index_size; ++i) {
        std::string word;
        size_t word_size = 0;
        in.read(reinterpret_cast<char*>(&word_size), sizeof(word_size));
        word.resize(word_size);
        in.read(&word[0], word_size);
        size_t vec_size = 0;
        in.read(reinterpret_cast<char*>(&vec_size), sizeof(vec_size));
        std::vector<IndexEntry> entries(vec_size);
        if(vec_size > 0) in.read(reinterpret_cast<char*>(entries.data()), vec_size * sizeof(IndexEntry));
        index_[std::move(word)] = std::move(entries);
    }
    in.close();
    return true;
}


