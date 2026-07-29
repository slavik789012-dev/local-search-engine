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

    // Ожидаем, что нашлись оба документа
    ASSERT_EQ(results.size(), 2);
    // Проверяем, что первый по релевантности/ID документ правильный
    EXPECT_EQ(results[0].id, 0);
    EXPECT_EQ(results[1].id, 1);
}

// Тест 2: Проверка игнорирования регистра и знаков препинания
TEST(SearchEngineTest, CaseInsensitiveAndPunctuation) {
    InvertedIndex index;
    index.AddDocument("doc1", "Hello, World!!!");

    SearchServer server(index);
    // Ищем в другом регистре
    auto results = server.search("hello");

    ASSERT_EQ(results.size(), 1);
    EXPECT_EQ(results[0].id, 0);
}

// Тест 3: Твое домашнее задание :)
TEST(SearchEngineTest, EmptyQueryReturnsEmptyResult) {
    // Напиши код, который добавляет документ, 
    // делает поиск по пустой строке "" 
    // и через ASSERT_TRUE проверяет, что results.empty()
}