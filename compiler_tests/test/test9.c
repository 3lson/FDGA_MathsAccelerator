void zero(int arr[], int size) {
    for (int i = 0; i < size; i++) {
        arr[i] = 0;
    }
}

int main() {
    int a[5];
    zero(a, 5);
    return 0;
}
