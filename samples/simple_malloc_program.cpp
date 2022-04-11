#include <iostream>
#include <cstdlib>
#include <vector>
#include <pthread.h>
using namespace std;

int main(){
    int N = 2;
    int *my_arr = (int*)malloc(2 * sizeof(int));

    for (int i = 0; i < N; i++){
        my_arr[i] = i;
    }

    return 0;
}