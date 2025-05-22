struct Point {
    int x;
    int y;
};

int f() {
    struct Point p1;
    struct Point p2;
    p1.x = 2;
    p2.x = 3;
    return p1.x<p2.x;
}
