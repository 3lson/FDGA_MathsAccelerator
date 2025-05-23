// need to remowe the use of that library and the #define as I dont want to implement them in a compiler
#include <stdio.h>

// can make those numbers larger later
#define MAX_POINTS 50
#define MAX_K 3

struct Point {
    float x, y;
};

float fabsf(float x) {
    return x < 0 ? -x : x;
}
// Manhattan distance between two points
float MDistance(struct Point a, struct Point b) {
    return fabsf(a.x - b.x) + fabsf(a.y - b.y);
}

// K-means clustering using Manhattan distance
void Clustering(struct Point centroids[], struct Point points[], int num_points, struct Point clusters[][MAX_POINTS], int cluster_sizes[], int k) {
    for (int i = 0; i < k; i++) {
        cluster_sizes[i] = 0;
    }

    for (int i = 0; i < num_points; i++) {
        float min_dist = MDistance(centroids[0], points[i]);
        int best = 0;

        for (int j = 1; j < k; j++) {
            float d = MDistance(centroids[j], points[i]);
            if (d < min_dist) {
                min_dist = d;
                best = j;
            }
        }

        clusters[best][cluster_sizes[best]++] = points[i];
    }
}

// recalculate centroids
int UpdateCentroids(struct Point centroids[], struct Point clusters[][MAX_POINTS], int cluster_sizes[], int k) {
    for (int i = 0; i < k; i++) {
        float sum_x = 0, sum_y = 0;
        int size = cluster_sizes[i];
        for (int j = 0; j < size; j++) {
            sum_x += clusters[i][j].x;
            sum_y += clusters[i][j].y;
        }

        if (size > 0) {
            centroids[i].x = sum_x / size;
            centroids[i].y = sum_y / size;
        }
    }
    return 0;
}

//Had to add that cause != gave me wrong results sometimes with floats chat said that it is because of floating point precision issues
int Equal(struct Point a, struct Point b) {
    float EPS = 0.000001;
    return (fabsf(a.x - b.x) < EPS) && (fabsf(a.y - b.y) < EPS);
}

int main() {
    int k;
    struct Point centroids[MAX_K];
    struct Point points[MAX_POINTS];
    int num_points;

    printf("Enter number of centroids: ");
    scanf("%d", &k);

    printf("Enter %d initial centroids (x y):\n", k);
    for (int i = 0; i < k; i++) {
        scanf("%f %f", &centroids[i].x, &centroids[i].y);
    }

    printf("Enter number of points: ");
    scanf("%d", &num_points);

    printf("Enter %d points (x y):\n", num_points);
    for (int i = 0; i < num_points; i++) {
        scanf("%f %f", &points[i].x, &points[i].y);
    }

    struct Point clusters[MAX_K][MAX_POINTS];
    int cluster_sizes[MAX_K];

    int max_iter = 100;
    for (int cycle = 0; cycle < max_iter; cycle++) {
        struct Point old_centroids[MAX_K];

        for (int i = 0; i < k; i++) old_centroids[i] = centroids[i];
        Clustering(centroids, points, num_points, clusters, cluster_sizes, k);
        UpdateCentroids(centroids, clusters, cluster_sizes, k);
        int done = 1;

        for (int i = 0; i < k; i++) {
            if (!Equal(centroids[i], old_centroids[i])) {
                done = 0;
                break;
            }
        }
        if (done) break;
    }
// changed it so it outputs the result in a file instead of a terminal
FILE *fout = fopen("output.txt", "w");
if (!fout) {
    printf("Error opening output.txt for writing.\n");
    return 1;
}

for (int i = 0; i < k; i++) {
    fprintf(fout, "C%d (%.2f, %.2f): ", i, centroids[i].x, centroids[i].y);
    for (int j = 0; j < cluster_sizes[i]; j++) {
        fprintf(fout, "(%.2f, %.2f) ", clusters[i][j].x, clusters[i][j].y);
    }
    fprintf(fout, "\n");
}

fclose(fout);

    return 0;
}
