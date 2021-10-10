#pragma once

#include "general.hpp"

struct SymEntry {
    SymEntry(std::string_view& symbol, uint16_t address, SymEntry* next)
        : symbol(symbol), address(address), next(next) {}

    std::string_view symbol;
    uint16_t address;
    SymEntry* next;
};

struct SymTable {
    static constexpr uint8_t NUM_BUCKETS_LOG2 = 5;
    static constexpr uint8_t NUM_BUCKETS = 1 << NUM_BUCKETS_LOG2;
    SymEntry* table[NUM_BUCKETS];

    SymTable();
    ~SymTable();

    static uint8_t hash(const std::string_view& key) {
        uint8_t hash = 0;
        for (const char& ch : key) {
            hash = (hash << 5) - hash + ch;
        }
        return hash & (NUM_BUCKETS - 1);
    }

    void insert(std::string_view& symbol, uint16_t address);
    SymEntry* get(const std::string_view& findSym);
};
