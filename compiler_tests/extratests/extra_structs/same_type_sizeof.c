struct x{
    int y;
    int a;
    int r;
    int f;
};

int f()
{
    struct x z;
    return sizeof(z);
}