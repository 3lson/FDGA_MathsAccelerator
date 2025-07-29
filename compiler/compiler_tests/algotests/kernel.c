int f(){
    float centroids_x[3];
    float centroids_y[3]; 
    float points_x[9];
    float points_y[9]; 
    float old_centroids_x[3];
    float old_centroids_y[3];
    int cluster_sizes[3];
    int max_iter = 2; 
    int num_points = 9; 
    float clusters_x[3][9];
    float clusters_y[3][9];
    int cycle;
    int j;
    int l; 
    int done =1;
    int best;

    float distances[3][9];
    float total[3][9];
    float sum_x[3][9];
    float sum_y[3][9];
    float shortest_distance[9];
    int best_centroid_index[9];
    int i;
    int h;
    int w;
    int index;
    int best_centroid;


    for(j=0; j<3; j++){
        old_centroids_x[j] = centroids_x[j];
        old_centroids_y[j] = centroids_y[j];
        cluster_sizes[j] = 0;
    }

    kernel(4){
        i = threadId.x;
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
        
        best_centroid = best_centroid_index[i];
        clusters_x[best_centroid][i] = points_x[i];
        clusters_y[best_centroid][i] = points_y[i];
        total[best_centroid][i]  = 1.0;

        sync;
        
        for(h = 0; h < 7; h++){

            index = i + (1 << h);
            if((num_points - 1) < (1 << h)){ 
                sum_x[0][i] = clusters_x[0][i] + clusters_x[0][index];
                sum_x[1][i] = clusters_x[1][i] + clusters_x[1][index];
                sum_x[2][i] = clusters_x[2][i] + clusters_x[2][index];
                sum_y[0][i] = clusters_y[0][i] + clusters_y[0][index];
                sum_y[1][i] = clusters_y[1][i] + clusters_y[1][index];
                sum_y[2][i] = clusters_y[2][i] + clusters_y[2][index];

                total[0][i] = total[0][i] + total[0][index];
                total[1][i] = total[1][i] + total[1][index];
                total[2][i] = total[2][i] + total[2][index];
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

    for(w = 0; w < 3; w++){
        centroids_x[w] = sum_x[w][0]/total[w][0];
        centroids_y[w] = sum_y[w][0]/total[w][0];
    }

    OUT centroids_x[3];
    OUT centroids_y[3];

    OUT clusters_x[3][9];
    OUT clusters_y[3][9];

    return 5;
}