int f(){
    int clusters[3][9];
    int i;
    int j;

    for (i = 0; i < 3; i++) {
        for (j = 0; j < 9; j++) {
            clusters[i][j] = i + j;
        }
    }

    clusters[1][1]= 2;

    return clusters[1][1];
}
