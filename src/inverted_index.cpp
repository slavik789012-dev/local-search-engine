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
    size_t pos = text.find_first_not_of(' ');

    while (pos != std::string_view::npos) {
        size_t space_pos = text.find(' ', pos);

        result.push_back(text.substr(pos, space_pos - pos));

        pos = text.find_first_not_of(' ', space_pos);
    }
    return result;
}

void InvertedIndex::AddDocument(const std::string& document_name, const std::string& text) {
    size_t doc_id = docs_.size();
    docs_.push_back(document_name);
    std::vector<std::string_view> token_views = SplitBySpaces(text);
    std::unordered_map<std::string, size_t> words_count;
    for (const auto& view : token_views) {
        std::string word = NormalizeWord(view);
        if (!word.empty()) words_count[std::move(word)]++;
    }
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

const std::vector<IndexEntry>& InvertedIndex::GetWordEntries (const std::string& word) const
{
    auto it = index_.find(word);
    if (it == index_.end()) {
        static const std::vector<IndexEntry> empty_result;
        return empty_result;
    }
    return it->second;
}
