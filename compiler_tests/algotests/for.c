float OUT_centroids_x[3];
float OUT_centroids_y[3];

float OUT_clusters_x[3][9];
float OUT_clusters_y[3][9];
int OUT_cluster_sizes[3];

int main(){
    float centroids_x[3];
    float centroids_y[3];
    float old_centroids_x[3];
    float old_centroids_y[3];
    int cluster_sizes[3];
    int max_iter = 2;
    float clusters_x[3][9];
    float clusters_y[3][9];
    int cycle;
    int j;
    int i;
    int done =1;
    int best;
    int h;


    int warpsize = 32; 
    float distances[3][9];
    float total[3][9];
    float sum_x[3][9];
    float sum_y[3][9];
    float shortest_distance[9];
    int best_centroid_index[9];

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
            old_centroids_y[j] = centroids_y[j];
            cluster_sizes[j] = 0;
        }

        i = blockId.x * blocksize + threadId.x;

        if(i < 9){
            distances[0][i] = fabsf(centroids_x[0]-points_x[i]) + fabsf(centroids_y[0]-points_y[i]);
            distances[1][i] = fabsf(centroids_x[1]-points_x[i]) + fabsf(centroids_y[1]-points_y[i]);
            distances[2][i] = fabsf(centroids_x[2]-points_x[i]) + fabsf(centroids_y[2]-points_y[i]);

            if(distances[0][i] < distances[1][i]){
                shortest_distance[i] = distances[0][i];
                best_centroid_index[i] = 0;
            }
            else{
                shortest_distance[i] = distances[1][i];
                best_centroid_index[i] = 1;
            }

            if(distances[2][i] < shortest_distance[i]){
                shortest_distance[i] = distances[0][i];
                best_centroid_index[i] = 2;
            }

            clusters_x[best_centroid_index[i]][i] = points_x[i];
            clusters_y[best_centroid_index[i]][i] = points_y[i];
            total[best_centroid_index[i]][i]  = 1;

            sync;

            for(h = 0; h < 7; h++){
                if((9 - 1) < (1<<h)){ 
                    sum_x[0][i] = clusters_x[0][i] + clusters_x[0][i + (1 << h)];
                    sum_x[1][i] = clusters_x[1][i] + clusters_x[1][i + (1 << h)];
                    sum_x[2][i] = clusters_x[2][i] + clusters_x[2][i + (1 << h)];
                    sum_y[0][i] = clusters_y[0][i] + clusters_y[0][i + (1 << h)];
                    sum_y[1][i] = clusters_y[1][i] + clusters_y[1][i + (1 << h)];
                    sum_y[2][i] = clusters_y[2][i] + clusters_y[2][i + (1 << h)];

                    total[0][i] = total[0][i] + total[0][i + (1 << h)];
                    total[1][i] = total[1][i] + total[1][i + (1 << h)];
                    total[2][i] = total[2][i] + total[2][i + (1 << h)];
                }
                else{
                    sum_x[0][i] = clusters_x[0][i];
                    sum_x[1][i] = clusters_x[1][i];
                    sum_x[2][i] = clusters_x[2][i];
                    sum_y[0][i] = clusters_y[0][i];
                    sum_y[1][i] = clusters_y[1][i];
                    sum_y[2][i] = clusters_y[2][i];
                    total[0][i] = total[0][i];
                    total[1][i] = total[1][i];
                    total[2][i] = total[2][i];
                }
            }

        }

        for(i = 0; i < 3; i++){
            centroids_x[i] = sum_x[i][0]/total[i][0];
            centroids_y[i] = sum_y[i][0]/total[i][0];
        }
    }


    for (i = 0; i < 3; i++) {
        OUT_centroids_x[i] = centroids_x[i];
        OUT_centroids_y[i] = centroids_y[i];
        OUT_cluster_sizes[i] = cluster_sizes[i];
        for (j = 0; j < cluster_sizes[i]; j++) {
            OUT_clusters_x[i][j] = clusters_x[i][j];
            OUT_clusters_y[i][j] = clusters_y[i][j];
        }
    }

    return 5;
}