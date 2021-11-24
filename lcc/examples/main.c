
int addIfGreater(int a, int b) {
    if (a > b) {
        return a + b;
    } else {
        return 0;
    }
}

// LC3 does not support multiplication
int mult(int a, int b) {
    int res = 0;
    while (a--) {
        res += b;
    }
    return res;
}

int fact(int n) {
    if (n <= 1) {
        return 1;
    }
    return mult(n, fact(n - 1));
}

void strcpy(int* dest, int* src) {
    while (*src) {
        *dest = *src;
        src++;
        dest++;
    }
}

int* staticAdr = 0x4000;

int* testStr = "Hello World";

/*
 * This is a test block comment
 */
int main() {
    strcpy(staticAdr, testStr);

    int* fourK = 0x4000;

    int ptrArithmetic = *(fourK + 5);  // ptrArithmetic = ' ' = 32
    int f = fact(7) + ptrArithmetic;   // f = 5040 + 32 = 5072

    int arrayIndex = testStr[7];  // arrayIndex = 'o' = 111
    f -= arrayIndex;              // f = 5183

    return addIfGreater(f, 0x1D);  // Should return 4990
}
