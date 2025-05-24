struct Point {
    float x;
    float y;
};

float f()
{
    struct Point z;
    z.x = 1.0;
    z.y = 2.0;
    return z.x;
}
