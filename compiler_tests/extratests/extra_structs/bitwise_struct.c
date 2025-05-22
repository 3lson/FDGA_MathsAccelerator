struct Point {
    int x;
    int y;
};

int f() {
    struct Point p1;
    struct Point p2;
    p1.x = 0xFFFF;
    p2.x = 0xFFFF00;
    return p1.x&p2.x;
}
