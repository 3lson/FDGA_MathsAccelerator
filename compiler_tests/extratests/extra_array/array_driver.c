int f(int i);

int main()
{

    return !((f(0) == 1) &&
             (f(1) == 10) &&
             (f(2) == 6) &&
             (f(3) == 5));
}
