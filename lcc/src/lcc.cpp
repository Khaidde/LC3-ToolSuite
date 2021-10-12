#include "general.hpp"
#include "string.h"

int main(int argc, const char** argv) {
    printf("lcc version 1.0\n");

    if (argc == 1) {
        printf("lcc: error: no input files");
        return -1;
    }
    if (argc != 4 || strcmp(argv[2], "-o") != 0) {
        printf("Usage: lcc [input_file].c -o [output_file].bin\n");
        return -1;
    }
    printf("Compiling %s...", argv[1]);
    return 0;
}
