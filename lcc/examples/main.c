
// clang-format off


int max(int a, int b) {
    if (a > b) {
        return 0;
    }
    return 1;
}

int div(int x, int y) {
    if (y > x) {
        return 0;
    }
    return div(x - y, y) + 1;
}

int map(int* array, int length) {
    int i = 0;
    int first;
    int second;
    int d;
    int off;
    while (i < length) {
        first= array[i];
        second= array[i + 1];
        d = div(first, second);
        off = max(first, second);
        array[i + off] = d;
        i = i + 2;
    }
    return 0;
}

int main() {
    return 0;
}
