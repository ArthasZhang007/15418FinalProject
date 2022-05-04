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

void *PrintHello(void *threadid) {
    long tid;
    tid = *(int*)threadid;

    int N = 2000;
    int *my_arr = (int*)malloc(N * sizeof(int));

    // StartROI();
    for (int i = 0; i < N; i++){
        my_arr[i] = i;
    }
    // StopROI();

    //std::cout << "Hello World! Thread ID, " << tid << endl;
    pthread_exit(NULL);
}


int main () {

    startroi();
    int N = 2;
    pthread_t threads[N];
    int rc;
    int i;

    // StartROI();
    for( i = 0; i < N; i++ ) {
        // StartROI();
        //std::cout << "main() : creating thread, " << i << endl;
        rc = pthread_create(&threads[i], NULL, PrintHello, (void *)&i);
        
        if (rc) {
            std::cout << "Error:unable to create thread," << rc << endl;
            exit(-1);
        }
        // StopROI();
    }
    // StopROI();
    
    for (i = 0; i < N; i++){
        pthread_join(threads[i], NULL);
    }
    stoproi();

    return 0;
}