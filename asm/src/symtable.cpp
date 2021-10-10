#include "symtable.hpp"

#include "general.hpp"

SymTable::SymTable() {
    for (size_t i = 0; i < NUM_BUCKETS; i++) {
        table[i] = nullptr;
    }
}

SymTable::~SymTable() {
    for (size_t i = 0; i < NUM_BUCKETS; i++) {
        SymEntry* cur = table[i];
        while (cur != nullptr) {
            SymEntry* next = cur->next;
            delete cur;
            cur = next;
        }
    }
}

void SymTable::insert(std::string_view& symbol, uint16_t address) {
    uint8_t symHash = hash(symbol);

    SymEntry* newEntry = new SymEntry(symbol, address, nullptr);

    SymEntry* cur = table[symHash];
    if (cur == nullptr) {
        table[symHash] = newEntry;
    } else {
        while (cur->next != nullptr && cur->symbol != symbol) {
            cur = cur->next;
        }
        if (cur->symbol == symbol) {
            fatal("Error: duplicate label '%s' at address x%04x and x%04x\n",
                  std::string(symbol).c_str(), cur->address, address);
            return;
        }
        cur->next = newEntry;
    }
}

SymEntry* SymTable::get(const std::string_view& findSym) {
    uint8_t symHash = hash(findSym);

    SymEntry* cur = table[symHash];
    while (cur != nullptr) {
        if (cur->symbol == findSym) {
            return cur;
        }
        cur = cur->next;
    }
    fatal("Could not find referenced offset label: '%s'\n", std::string(findSym).c_str());
}
