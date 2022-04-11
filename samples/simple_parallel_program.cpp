#include <iostream>
#include <cstdlib>
#include <vector>
#include <pthread.h>
using namespace std;

void *PrintHello(void *threadid) {
    long tid;
    tid = *(int*)threadid;
    std::cout << "Hello World! Thread ID, " << tid << endl;
    pthread_exit(NULL);
}

int main () {
    int N = 3;
    pthread_t threads[N];
    int rc;
    int i;

    for( i = 0; i < N; i++ ) {
        std::cout << "main() : creating thread, " << i << endl;
        rc = pthread_create(&threads[i], NULL, PrintHello, (void *)&i);
        
        if (rc) {
            std::cout << "Error:unable to create thread," << rc << endl;
            exit(-1);
        }
    }
    pthread_exit(NULL);
}
