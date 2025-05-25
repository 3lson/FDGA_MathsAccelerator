int f() {
    int centroids_x[3];
    int centroids_y[3];
    int sum_x = 10;
    int sum_y = 20;
    int size = 5;

    if (size > 0) {
        centroids_x[0] = sum_x / size;
        centroids_y[0] = sum_y / size;  
    }

    return centroids_x[0];
}
