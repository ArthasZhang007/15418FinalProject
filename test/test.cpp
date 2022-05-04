#include <iostream>
#include <cstdlib>
#include <vector>
#include <pthread.h>
using namespace std;

// static int test1(void){
//     std::vector <int> x;
//     for (int i = 0; i <= 10000; i++){
//         x.push_back(i);
//     }
//     return 1;
// }

// int main(){
//     int N = 2;
//     int *my_arr = (int*)malloc(2 * sizeof(int));

//     for (int i = 0; i < N; i++){
//         my_arr[i] = i;
//         //std::cout << "addr of arr[" << i << "]: " << &(my_arr[i]) << '\n';
//     }

//     // int y = test1();

//     return 0;
// }

// bool isROI = false;

// Set ROI flag
void startroi() {}

// Set ROI flag
void stoproi() {}

// const CHAR * ROI_BEGIN = "__parsec_roi_begin";
// const CHAR * ROI_END = "__parsec_roi_end";

struct args {
    int tid;
    int *arr;
};

void *thread(void *cur_args) {
    //int N = 10;
    //int *my_arr = (int*)malloc(N * sizeof(int));

    // for (int i = 0; i < N; i++){
    //     my_arr[i] = i;
    // }

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