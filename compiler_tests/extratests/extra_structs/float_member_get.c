struct x{
    float c;
};

float f()
{
    struct x z;
    z.c = 30.0;
    return z.c;
}