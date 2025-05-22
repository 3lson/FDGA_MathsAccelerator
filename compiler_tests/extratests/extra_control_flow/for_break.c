int f(int x) {
    int y = 0;
    int i;
    for (i = 1; i < x; i++) {
        if (i == 6) {
            break;
        }
        y += i;
    }
    return y;
}
