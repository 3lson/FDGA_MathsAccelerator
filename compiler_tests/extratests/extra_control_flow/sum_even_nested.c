int sum_even_nested(int n) {
    int sum = 0;
    int i = 1;

    while (i <= n) {
        int j = 1;
        while (j <= n) {
            if ((i * j) % 2 != 0) {
                j++;
                continue;
            }
            sum += i * j;
            j++;
        }
        i++;
    }

    return sum;
}
