#include <iostream>
#include <cstdlib>
#include <vector>
#include <pthread.h>
using namespace std;

// Set ROI flag
void startroi() {}

// Set ROI flag
void stoproi() {}

struct args {
    int tid;
    int *arr;
};

void *thread(void *cur_args) {
    
    args *casted = (args*)cur_args;
    for (int i = 0; i < 4; i++){
        casted->arr[casted->tid + i * 64] += 1;
    }

    pthread_exit(NULL);
}


int main () {

    startroi();
    int threadNum = 64;
    pthread_t threads[threadNum];
    int rc;
    int i;

    int arr_size = 256;
    int *shared_arr = (int*)malloc(arr_size * sizeof(int));

    for (int idx = 0; idx < 256; idx++){
        shared_arr[idx] = idx;
    }

    for( i = 0; i < threadNum; i++ ) {
        args *cur_args = (args*)malloc(sizeof(args));
        cur_args->tid = i;
        cur_args->arr = shared_arr;

        rc = pthread_create(&threads[i], NULL, thread, (void *)cur_args);
        
        if (rc) {
            std::cout << "Error:unable to create thread," << rc << endl;
            exit(-1);
        }
    }
    
    for (i = 0; i < threadNum; i++){
        pthread_join(threads[i], NULL);
    }
    stoproi();

    return 0;
}