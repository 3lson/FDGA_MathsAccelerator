int f(int a, int b) {
    int sum = 0;
    while (a < b) {
        int temp = a * 2;
        sum += temp;
        a++;
    }
    return sum;
}