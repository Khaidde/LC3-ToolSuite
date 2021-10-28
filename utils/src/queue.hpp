
// T -> type of data stored in queue
// N -> initial capacity
template <class T, int N = 16>
struct Queue {
    void push(T& item);
    void push(T&& item);
    T pop();

    int front = 0;
    int size = 0;
    T backing[N];
};

template <class T, int N>
void Queue<T, N>::push(T& item) {
    backing[(front + size) % N] = item;
    size++;
}

template <class T, int N>
void Queue<T, N>::push(T&& item) {
    backing[(front + size) % N] = item;
    size++;
}

template <class T, int N>
T Queue<T, N>::pop() {
    if (size == 0) {
        return 0;
    }
    T&& temp = (T &&) backing[front];
    front = (front + 1) % N;
    size--;
    return temp;
}
