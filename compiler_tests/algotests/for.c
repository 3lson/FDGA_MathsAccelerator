float OUT_centroids_x[3];
float OUT_centroids_y[3];

float OUT_clusters_x[3][9];
float OUT_clusters_y[3][9];
int OUT_cluster_sizes[3];


int main2(){
    float centroids_x[3];
    float centroids_y[3];
    float old_centroids_x[3];
    float old_centroids_y[3];
    float clusters_x[3][9];
    float clusters_y[3][9];
    int cluster_sizes[3];
    int max_iter =1;
    int num_points =9;
    int cycle;
    int j;
    int i;
    int done =1;

    float points_x[9] = {1.0, 2.0, 1.0, 8.0, 9.0, 8.0, -1.0, -2.0, -1.0};
    float points_y[9] = {1.0, 1.0, 2.0, 8.0, 8.0, 9.0, -1.0, -1.0, -2.0}; 

    for(i=0; i<3; i++){
        centroids_x[i] = points_x[i];
        centroids_y[i] = points_y[i];
    }

    for(cycle=0; cycle<max_iter; cycle++){
        done = 1;
        for(j=0; j<3; j++){
            old_centroids_x[j] = centroids_x[j];
        }
    }
    return 5;
}