int f();

int main()
{
    double y = 0;
    double *x = &y;
    return !(f() == sizeof(x + 1));
}
