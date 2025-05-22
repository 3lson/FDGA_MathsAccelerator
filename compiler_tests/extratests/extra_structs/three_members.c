struct x{
    int y;
    int z;
    int a;
};

int f()
{
    struct x g;
    g.y=17;
    g.z=13;
    g.a=10;
    return g.a;
}