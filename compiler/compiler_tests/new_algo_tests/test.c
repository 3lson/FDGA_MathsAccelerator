int f(){
    float centroids_x[3];
    float centroids_y[3];
    float points_x[9];
    float points_y[9];

    float distances[3][9];
    float shortest_distance[9];
    int best_centroid_index[9];

    float total[3][9];
    float sum_x[3][9];
    float sum_y[3][9];

    kernel(9) {
        int i = threadId.x;
        int k;
        int h;
        int index;
        int best_centroid;

        distances[0][i] = fabsf(centroids_x[0] - points_x[i]) + fabsf(centroids_y[0] - points_y[i]);
        distances[1][i] = fabsf(centroids_x[1] - points_x[i]) + fabsf(centroids_y[1] - points_y[i]);
        distances[2][i] = fabsf(centroids_x[2] - points_x[i]) + fabsf(centroids_y[2] - points_y[i]);

        if (distances[0][i] < distances[1][i]) {
            shortest_distance[i] = distances[0][i];
            best_centroid_index[i] = 0;
        } else {
            shortest_distance[i] = distances[1][i];
            best_centroid_index[i] = 1;
        }

        if (distances[2][i] < shortest_distance[i]) {
            best_centroid_index[i] = 2;
        }

        best_centroid = best_centroid_index[i];

        for (k = 0; k < 3; k++) {
            if (k == best_centroid) {
                sum_x[k][i] = points_x[i];
                sum_y[k][i] = points_y[i];
                total[k][i] = 1.0;
            } else {
                sum_x[k][i] = 0.0;
                sum_y[k][i] = 0.0;
                total[k][i] = 0.0;
            }
        }

        for (h = 0; h < 4; h++) {
            sync;

            index = i + (1 << h);

            if (index < 9) {
                for (k = 0; k < 3; k++) {
                    sum_x[k][i] += sum_x[k][index];
                    sum_y[k][i] += sum_y[k][index];
                    total[k][i] += total[k][index];
                }
            }
        }
    }

    return 5;
}