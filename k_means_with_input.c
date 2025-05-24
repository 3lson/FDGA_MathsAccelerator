#include <stdio.h>
#include <stdlib.h>

float fabsf(float x) {
    return x < 0 ? -x : x;
}

int main() {
    float centroids_x[3];
    float centroids_y[3];
    float old_centroids_x[3];
    float old_centroids_y[3];
    float points_x[9];
    float points_y[9];

    FILE *input = fopen("input.txt", "r");
    if (!input) {
        perror("Failed to open input.txt");
        return 1;
    }

    int num_points = 0;
    while (fscanf(input, "%f %f", &points_x[num_points], &points_y[num_points]) == 2) {
        num_points++;
        if (num_points >= 9) break;
    }
    fclose(input);

    // Initialize centroids with first 3 points
    for (int i = 0; i < 3; i++) {
        centroids_x[i] = points_x[i];
        centroids_y[i] = points_y[i];
    }

    float clusters_x[3][9];
    float clusters_y[3][9];
    int cluster_sizes[3];

    int max_iter = 100;
    for (int cycle = 0; cycle < max_iter; cycle++) {
        for (int i = 0; i < 3; i++) {
            old_centroids_x[i] = centroids_x[i];
            old_centroids_y[i] = centroids_y[i];
            cluster_sizes[i] = 0;
        }

        for (int i = 0; i < num_points; i++) {
            float min_dist = fabsf(centroids_x[0] - points_x[i]) + fabsf(centroids_y[0] - points_y[i]);
            int best = 0;

            for (int j = 1; j < 3; j++) {
                float d = fabsf(centroids_x[j] - points_x[i]) + fabsf(centroids_y[j] - points_y[i]);
                if (d < min_dist) {
                    min_dist = d;
                    best = j;
                }
            }

            clusters_x[best][cluster_sizes[best]] = points_x[i];
            clusters_y[best][cluster_sizes[best]] = points_y[i];
            cluster_sizes[best]++;
        }

        for (int i = 0; i < 3; i++) {
            float sum_x = 0, sum_y = 0;
            int size = cluster_sizes[i];
            for (int j = 0; j < size; j++) {
                sum_x += clusters_x[i][j];
                sum_y += clusters_y[i][j];
            }

            if (size > 0) {
                centroids_x[i] = sum_x / size;
                centroids_y[i] = sum_y / size;
            }
        }

        int done = 1;
        for (int i = 0; i < 3; i++) {
            if (!(fabsf(centroids_x[i] - old_centroids_x[i]) < 0.000001 &&
                  fabsf(centroids_y[i] - old_centroids_y[i]) < 0.000001)) {
                done = 0;
                break;
            }
        }
        if (done) break;
    }

    FILE *output = fopen("output.txt", "w");
    if (!output) {
        perror("Failed to open output.txt");
        return 1;
    }

    for (int i = 0; i < 3; i++) {
        fprintf(output, "Centroid %d: %.6f %.6f\n", i, centroids_x[i], centroids_y[i]);
    }

    fclose(output);
    return 0;
}
