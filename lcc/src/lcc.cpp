#include "codegen.hpp"
#include "general.hpp"
#include "parser.hpp"
#include "string.h"

int main(int argc, const char** argv) {
    printf("lcc version 1.0\n");

    if (argc == 1) {
        printf("lcc: error: no input files");
        return -1;
    }
    if (argc != 3 && argc != 2) {
        printf("Usage: lcc [input_file].c [output_file].asm\n");
        return -1;
    }

    try {
        auto parser = std::make_unique<Parser>();
        parser->load_source(read_file(argv[1]));
        auto program = parser->parse_program();
        dump_ast(*program);

        auto generator = std::make_unique<CodeGen>();
        generator->gen_program(argv[2] ? argv[2] : nullptr, *program);
    } catch (const std::exception& e) {
        printf("\nFATAL ERROR: %s\n", e.what());
    }
    return 0;
}
