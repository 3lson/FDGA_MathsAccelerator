/*-----------------------------------------------------------------------------

                       GLOBAL VARIABLE INITIALIZATIONS

-------------------------------------------------------------------------------*/

// float OUT_centroids_x[3];
// float OUT_centroids_y[3];

// float OUT_clusters_x[3][9];
// float OUT_clusters_y[3][9];
// int OUT_cluster_sizes[3];


int main(){
    float centroids_x[3];
    float centroids_y[3];
    float old_centroids_x[3];
    float old_centroids_y[3];
    int cluster_sizes[3];
    int max_iter = 2; // CHOSEN TO BE 2 FOR OPTIMAL USER ACCESSIBILITY IN UI
    int num_points =9; //EXPECTED TO CHANGE TO 100 PTS IN FINAL DEMO
    float clusters_x[3][num_points];
    float clusters_y[3][num_points];
    int cycle;
    int j;
    int l; //loop iterator
    int done =1;
    int best;

    /*-------------------------------------------------------------------------------

                        KEY KERNEL VARIABLE INITIALIZATIONS

    ---------------------------------------------------------------------------------*/

    int warpsize = 16; // dependant on number of SIMT lanes
    int blocksize = 4; // ([pts + warpsize-1] // warpsize) * warpsize MANUALLY RECALCULATE WHEN WARPSIZE ALTERED
    float distances[3][num_points];
    float total[3][num_points];
    float sum_x[3][num_points];
    float sum_y[3][num_points];
    float shortest_distance[num_points];
    int best_centroid_index[num_points];
    int i;
    int index;


    /*--------------------------------------------------------------------------------

                                    PROGAM BEGIN

    ----------------------------------------------------------------------------------*/

    float points_x[9] = {1.0, 2.0, 1.0, 8.0, 9.0, 8.0, -1.0, -2.0, -1.0};
    float points_y[9] = {1.0, 1.0, 2.0, 8.0, 8.0, 9.0, -1.0, -1.0, -2.0}; 

    for(cycle=0; cycle<max_iter; cycle++){
        done = 1;
        for(j=0; j<3; j++){
            old_centroids_x[j] = centroids_x[j];
            old_centroids_y[j] = centroids_y[j];
            cluster_sizes[j] = 0;
        }

    /*----------------------------------------------------------------------------------

                                KERNAL IMPLEMENTATION

    SYNTAX:
    KERNAL(<NUM_BLOCKS>,<BLOCK_SIZE>,<NUM_THREADS>, VARABLE LIST TO BE LOADED INTO CACHE....)

    THREAD ATTRIBUTES WHEN A KERNEL IS INITIALIZED:
    blockId.x - the block to which a thread belongs to
    threadId.x - each thread has its own threadId within its block

    blockId.x * blocksize + threadId.x - glocal thread Id within the entire kernel


    -------------------------------------------------------------------------------------*/

        kernel(num_points){
            //i = blockId.x * blocksize + threadId.x;
            i = threadId.x

            if(i < num_points){
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
                total[best_centroid_index[i]][i]  = 1.0;
                
                //syncs all threads in the same block
                sync;
                
                //h represents the number of times to half the 100 points in summing operations 
                for(int h = 0; h < 7; h++){
                    // 1 << h  = 2 ^ h;

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
           
        }

        for(int i = 0; i < 3; i++){
            centroids_x[i] = sum_x[i][0]/total[i][0];
            centroids_y[i] = sum_y[i][0]/total[i][0];
        }

        /*-----------------------------------------------------------------------------

        CONVERGENCE CONDITION FOR EARLY TERMINATION OF ALGORITHM NOW REDUNDANT!!!!!!

        ------------------------------------------------------------------------------*/
        
        // for (i = 0; i < 3; i++) {
        //     float side1 = fabsf(centroids_x[i] - old_centroids_x[i]);
        //     float side2 = fabsf(centroids_y[i] - old_centroids_y[i]);
        //     if (!(side1 < 0.000001 && side2 < 0.000001)) {
        //         done = 0;
        //         break;
        //     }
        // }
        // if (done) {
        //     break;
        // }
    }

    // for (l = 0; l < 3; l++) {
    //     OUT_centroids_x[l] = centroids_x[l];
    //     OUT_centroids_y[l] = centroids_y[l];
    //     OUT_cluster_sizes[l] = cluster_sizes[l];
    //     for (j = 0; j < cluster_sizes[l]; j++) {
    //         OUT_clusters_x[l][j] = clusters_x[l][j];
    //         OUT_clusters_y[l][j] = clusters_y[l][j];
    //     }
    // }
    
    return 5;
}