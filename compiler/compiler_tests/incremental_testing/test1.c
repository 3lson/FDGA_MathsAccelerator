int f(){
    int output_array[1];
    
    kernel(1){
        output_array[0] = 123;
    }
    return 5;
}