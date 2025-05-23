float fabsf(float x) {
    return x < 0 ? -x : x;
}

int main() {
    float a = -5.5;
    float b = fabsf(a);
    return 0;
}
