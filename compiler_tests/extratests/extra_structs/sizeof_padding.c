struct y {
    char c;
    int i;
    short s;
};

int g() {
    struct y v;
    return sizeof(v);
}