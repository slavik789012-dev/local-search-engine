#include <gtest/gtest.h>
#include "inverted_index.h"
#include "search_server.h"

// Тест 1: Проверка базового поиска
TEST(SearchEngineTest, BasicSearch) {
    InvertedIndex index;
    index.AddDocument("doc1", "hello world");
    index.AddDocument("doc2", "hello C++");

    SearchServer server(index);
    auto results = server.search("hello");

    ASSERT_EQ(results.size(), 2);
    EXPECT_EQ(results[0].id, 0);
    EXPECT_EQ(results[1].id, 1);
}

// Тест 2: Проверка игнорирования регистра и знаков препинания
TEST(SearchEngineTest, CaseInsensitiveAndPunctuation) {
    InvertedIndex index;
    index.AddDocument("doc1", "Hello, World!!!");

    SearchServer server(index);
    auto results = server.search("hello");

    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0].id, 0);
}

// Тест 3: Поиск по пустому запросу
TEST(SearchEngineTest, EmptyQueryReturnsEmptyResult) {
    InvertedIndex index;
    index.AddDocument("doc1", "some text here");

    SearchServer server(index);
    auto results = server.search(""); // Пустой запрос

    // Проверяем, что результаты действительно пустые
    ASSERT_TRUE(results.empty());
}