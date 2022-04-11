#include <iostream>
#include <cstdlib>
using namespace std;

int main(){
    int *my_arr = (int*)malloc(42 * sizeof(int));

    for (int i = 0; i < 42; i++){
        my_arr[i] = i;
    }

    for (int i = 0; i < 42; i++){
        my_arr[i] += 1;
    }

    free(my_arr);

    return 0;
}