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
    int best_centroid;


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
            distances[0][threadId.x] = fabsf(centroids_x[0]-points_x[threadId.x]) + fabsf(centroids_y[0]-points_y[threadId.x]);
            distances[1][threadId.x] = fabsf(centroids_x[1]-points_x[threadId.x]) + fabsf(centroids_y[1]-points_y[threadId.x]);
            distances[2][threadId.x] = fabsf(centroids_x[2]-points_x[threadId.x]) + fabsf(centroids_y[2]-points_y[threadId.x]);

            if(distances[0][threadId.x] < distances[1][threadId.x]){
                shortest_distance[threadId.x] = distances[0][threadId.x];
                best_centroid_index[threadId.x] = 0;
            }
            else{
                shortest_distance[threadId.x] = distances[1][threadId.x];
                best_centroid_index[threadId.x] = 1;
            }

            if(distances[2][threadId.x] < shortest_distance[threadId.x]){
                shortest_distance[threadId.x] = distances[0][threadId.x];
                best_centroid_index[threadId.x] = 2;
            }
            
            best_centroid = best_centroid_index[threadId.x];
            clusters_x[best_centroid][threadId.x] = points_x[threadId.x];
            clusters_y[best_centroid][threadId.x] = points_y[threadId.x];
            total[best_centroid][threadId.x]  = 1.0;
            
            //syncs all threads in the same block
            sync;
            
            //h represents the number of times to half the 100 points in summing operations 
            for(int h = 0; h < 7; h++){
                // 1 << h  = 2 ^ h;

                index = threadId.x + (1 << h);
                if((num_points - 1) < (1 << h)){ 
                    sum_x[0][threadId.x] = clusters_x[0][threadId.x] + clusters_x[0][index];
                    sum_x[1][threadId.x] = clusters_x[1][threadId.x] + clusters_x[1][index];
                    sum_x[2][threadId.x] = clusters_x[2][threadId.x] + clusters_x[2][index];
                    sum_y[0][threadId.x] = clusters_y[0][threadId.x] + clusters_y[0][index];
                    sum_y[1][threadId.x] = clusters_y[1][threadId.x] + clusters_y[1][index];
                    sum_y[2][threadId.x] = clusters_y[2][threadId.x] + clusters_y[2][index];

                    total[0][threadId.x] = total[0][threadId.x] + total[0][index];
                    total[1][threadId.x] = total[1][threadId.x] + total[1][index];
                    total[2][threadId.x] = total[2][threadId.x] + total[2][index];
                }
                else{
                    sum_x[0][threadId.x] = clusters_x[0][threadId.x];
                    sum_x[1][threadId.x] = clusters_x[1][threadId.x];
                    sum_x[2][threadId.x] = clusters_x[2][threadId.x];
                    sum_y[0][threadId.x] = clusters_y[0][threadId.x];
                    sum_y[1][threadId.x] = clusters_y[1][threadId.x];
                    sum_y[2][threadId.x] = clusters_y[2][threadId.x];
                    total[0][threadId.x] = total[0][threadId.x];
                    total[1][threadId.x] = total[1][threadId.x];
                    total[2][threadId.x] = total[2][threadId.x];
                }
            }
        }

        for(int i = 0; i < 3; i++){
            centroids_x[i] = sum_x[i][0]/total[i][0];
            centroids_y[i] = sum_y[i][0]/total[i][0];
        }

    }

    
    return 5;
}