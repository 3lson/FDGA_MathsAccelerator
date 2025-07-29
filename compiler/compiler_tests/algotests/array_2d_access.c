float f(){
    float clusters[3][9];
    int cluster_sizes[3] = {1, 2, 3};
    clusters[cluster_sizes[1]][5] = 42.0;

    return clusters[2][5];
}