struct Point {
    int x;
    int y;
};

int f() {
    struct Point p1;
    struct Point p2;
    p1.x = 3;
    p2 = p1;
    return p2.x;
}
