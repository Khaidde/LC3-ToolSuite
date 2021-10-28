#pragma once

#include "general.hpp"

template <class T>
struct SymEntry {
    SymEntry(std::string_view& symbol, T&& data, SymEntry<T>* next)
        : symbol(symbol), data(std::move(data)), next(next) {}

    std::string_view symbol;
    T data;
    SymEntry<T>* next;
};

template <class T>
struct SymTable {
    SymTable();
    ~SymTable();

    static uint8_t hash(const std::string_view& key) {
        uint8_t hash = 0;
        for (const char& ch : key) {
            hash = (hash << 5) - hash + ch;
        }
        return hash & (NUM_BUCKETS - 1);
    }

    // Returns duplicate symbol entry if it exists, otherwise nullptr
    SymEntry<T>* put(std::string_view& symbol, T&& data);

    T* get(const std::string_view& symbol);

    static constexpr uint8_t NUM_BUCKETS_LOG2 = 5;
    static constexpr uint8_t NUM_BUCKETS = 1 << NUM_BUCKETS_LOG2;
    SymEntry<T>* table[NUM_BUCKETS];
    std::size_t size;
};

template <class T>
SymTable<T>::SymTable() {
    for (size_t i = 0; i < NUM_BUCKETS; i++) {
        table[i] = nullptr;
    }
}

template <class T>
SymTable<T>::~SymTable() {
    for (size_t i = 0; i < NUM_BUCKETS; i++) {
        SymEntry<T>* cur = table[i];
        while (cur != nullptr) {
            SymEntry<T>* next = cur->next;
            delete cur;
            cur = next;
        }
    }
}

template <class T>
SymEntry<T>* SymTable<T>::put(std::string_view& symbol, T&& data) {
    uint8_t symHash = hash(symbol);

    SymEntry<T>* newEntry = new SymEntry<T>(symbol, std::move(data), nullptr);

    SymEntry<T>* cur = table[symHash];
    if (cur == nullptr) {
        table[symHash] = newEntry;
    } else {
        while (cur->next != nullptr && cur->symbol != symbol) {
            cur = cur->next;
        }
        if (cur->symbol == symbol) {
            delete newEntry;
            return cur;
        }
        cur->next = newEntry;
    }
    size++;
    return nullptr;
}

template <class T>
T* SymTable<T>::get(const std::string_view& symbol) {
    uint8_t symHash = hash(symbol);

    SymEntry<T>* cur = table[symHash];
    while (cur != nullptr) {
        if (cur->symbol == symbol) {
            return &cur->data;
        }
        cur = cur->next;
    }
    return nullptr;
}
