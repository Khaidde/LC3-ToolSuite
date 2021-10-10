#include <memory>

#include "parser.hpp"

void assemble(const char* srcPath, const char* dest) {
    auto parser = std::make_unique<Parser>();
    parser->load_source(read_file(srcPath));

    printf("source file: ");
    if (parser->lex.src.size() >= 1 << 10) {
        printf("%d kilobytes\n", parser->lex.src.size() >> 10);
    } else {
        printf("%d bytes\n", parser->lex.src.size());
    }

    parser->first_pass_parse();
    parser->second_pass_parse(dest);
}

int main(int argc, char** argv) {
    printf("lcasm version 1.0\n");

    if (argc != 2 && argc != 3) {
        printf("Usage: lcasm [source_file].asm [dest_file].bin\n");
        return -1;
    }
    try {
        if (argc == 2) {
            assemble(argv[1], nullptr);
        } else {
            assemble(argv[1], argv[2]);
        }
    } catch (const std::exception& e) {
        printf("\nFATAL ERROR: %s\n", e.what());
    }

    return 0;
}
