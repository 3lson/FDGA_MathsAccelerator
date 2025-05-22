double add(double a, double b);

double multiply(double x, double y);

double complex_compute(int a, double b);

int main() {
    return !(complex_compute(3.0, 4.5) == 21.0);
}