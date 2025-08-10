float points_x[9];
float points_y[9];
float centroids_x[3];
float centroids_y[3];

int cluster_assignments[9];

float sum_x[3][9];
float sum_y[3][9];
float cluster_point_counts[3][9];

int main1(){
    int cycle;
    int k;
    int i;

    for (cycle = 0; cycle < 2; cycle++){
        kernel(9){
            int i = threadId.x;
            int k;
            int h;
            int index;

            float min_dist;
            int best_cluster_index;
            int my_assigned_cluster;
            float dist;

            min_dist = fabsf(centroids_x[0] - points_x[i]) + fabsf(centroids_y[0] - points_y[i]);
            best_cluster_index = 0;

            for(k=1; k<3; k++){
                dist = fabsf(centroids_x[k] - points_x[i]) + fabsf(centroids_y[k] - points_y[i]);
                if(dist < min_dist){
                    min_dist = dist;
                    best_cluster_index = k;
                }
            }
            cluster_assignments[i] = best_cluster_index;

            my_assigned_cluster = cluster_assignments[i];
            for (k=0; k<3; k++){
                if (k == my_assigned_cluster){
                    sum_x[k][i] = points_x[i];
                    sum_y[k][i] = points_y[i];
                    cluster_point_counts[k][i] = 1.0;
                } else {
                    sum_x[k][i] = 0.0;
                    sum_y[k][i] = 0.0;
                    cluster_point_counts[k][i] = 0.0;
                }
            }

            for (h=0; h<4; h++){
                sync;

                index = i + (1 << h);

                if (index < 9){
                    for (k=0; k<3; k++){
                        sum_x[k][i] = sum_x[k][i] + sum_x[k][index];
                        sum_y[k][i] = sum_y[k][i] + sum_y[k][index];
                        cluster_point_counts[k][i] = cluster_point_counts[k][i] + cluster_point_counts[k][index];
                    }
                }
            }
        }

        for (k=0;k<3;k++){
            if (cluster_point_counts[k][0] > 0.0){
                centroids_x[k] = sum_x[k][0] / cluster_point_counts[k][0];
                centroids_y[k] = sum_y[k][0] / cluster_point_counts[k][0];
            }
        }
    }
    return 5;
}