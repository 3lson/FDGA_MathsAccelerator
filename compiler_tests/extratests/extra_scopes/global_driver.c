int f();

int main(){
    printf("Return Value: %d", f());
    return ! (f() == 1);
}
