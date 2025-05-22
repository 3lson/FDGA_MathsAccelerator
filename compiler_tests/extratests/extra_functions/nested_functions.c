double add(double a, double b) {
    return a + b;
}

double multiply(double x, double y) {
    return x * y;
}

double complex_compute(double a, double b) {
    return add(multiply(a, b), add(a, b));
}