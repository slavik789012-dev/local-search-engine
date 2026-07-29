#include "search_server.h"
#include <cmath>
#include <algorithm>

std::vector<Document> SearchServer::search(const std::string& query) const
{
	std::vector<std::string_view> token_views = InvertedIndex::SplitBySpaces(query);
	size_t N = index_.GetSize();
	if (N == 0) {
		return {};
	}
	std::vector<double> doc_scores(N, 0.0);
	for (std::string_view view : token_views) {
		std::string word = InvertedIndex::NormalizeWord(view);
		if (word.empty()) continue;
		const auto& entries = index_.GetWordEntries(word);
		if (!entries.empty()) {
			double idf = (N * 1.0 / entries.size());
			for (auto& entry : entries) {
				doc_scores[entry.doc_id] += entry.term_frequency * idf;
			}
		}
	}
	std::vector<Document> result;
	for (size_t id = 0; id < N; ++id) {
		if (doc_scores[id] > 0) result.push_back({ id, doc_scores[id] });
	}
	size_t top_count = std::min<size_t>(5, result.size());
	std::partial_sort(result.begin(), result.begin() + top_count, result.end(), [](const Document& lhs, const Document& rhs) {
		const double EPSILON = 1e-6;
		if (std::abs(lhs.relevance - rhs.relevance) < EPSILON) return lhs.id < rhs.id;
		return lhs.relevance > rhs.relevance;
	});
	result.resize(top_count);
	return result;
}
