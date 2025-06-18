int f(){
    int i;
    int y;
    int arr[2] = {1,2};
    int mini[2][5];
    kernel(1){
        i = threadId.x;
        for(y = 0; y < 5; y++){
            i = i + 1;
        }
    }
    return 0;
}