struct x{
    float c;
    int a;
    char b;
    double d;
};

float f()
{
    struct x z;
    z.c = 30.0;
    z.a = 1;
    z.b = 'b';
    z.d = 2.5;
    return z.c;
}