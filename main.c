//
//  main.c
//  
//
//  Created by Bratusse on march 1st, 2022.
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <immintrin.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>

#define N 1048576
#define NB_THREADS 4

double now(){
    // Returns time in seconds
    struct timeval t;
    double f_t;
    gettimeofday(&t, NULL);
    f_t = t.tv_usec;
    f_t = f_t/1000000.0;
    f_t += t.tv_sec;
    return f_t;
}

struct thread_data{
    float *U;
    bool mode;
    long a;
    long b;
};

float u[N] __attribute__((aligned(32)));
double res = 0;

pthread_mutex_t res_mutex = PTHREAD_MUTEX_INITIALIZER;
struct thread_data thread_data_array[NB_THREADS];
pthread_t thread_ptr[NB_THREADS];

void init(){
    unsigned int i;
    for( i = 0 ; i < N ; i++ ){
        u[i] = (float)rand () / RAND_MAX;
    }
}

double rnorm(float *U, int n){         //scalar for comparison
    int i;
    double res_l = 0;
    for (i=0; i<n; i++){
        res_l += (double) sqrt(U[i]);
    }
    return res_l;
}

double vect_rnorm(float *U, int n){
    int i;
    double r256[4] __attribute__((aligned(32))) = {};
    double res_l = 0;
    __m128 *mm_u = (__m128 *)U;
    __m256d *mm_r = (__m256d *)r256;
    for( i = 0 ; i < n/4 ; i++ )
        mm_r[0] = _mm256_add_pd(_mm256_sqrt_pd(_mm256_cvtps_pd(mm_u[i])),mm_r[0]);
    for( i = 0 ; i < 4 ; i++ )
        res_l += r256[i];
    return res_l;
}

void *thread_function(void *threadarg){
    /* Local variables */
    double s;
    long i;
    /* Shared variables correspondances */
    float *U;
    bool mode;
    long a, b;
 
    /* Association between shared variables and their correspondances */
    struct thread_data *thread_pointer_data;
    thread_pointer_data = (struct thread_data *)threadarg;

    /* Shared variables */
    U = thread_pointer_data->U;
    mode = thread_pointer_data->mode;
    a = thread_pointer_data->a;
    b = thread_pointer_data->b;
 
    /* Body of the thread */
    if (mode) {
        s = vect_rnorm(U+a, b-a);
    }
    else {
        s = rnorm(U+a, b-a);
    }

    /* Mutex to add the partial result */
    pthread_mutex_lock(&res_mutex);
    res += s;
    pthread_mutex_unlock(&res_mutex);

    pthread_exit(NULL);
    return 0;
}

double rnormPar(float *U, int n, int nb_threads, bool mode){
    int i;
    res = 0;

    for( i=0 ; i<nb_threads ; i++ ){

        thread_data_array[i].U = U;
        thread_data_array[i].mode = mode;
        thread_data_array[i].a = i*(n/nb_threads);
        thread_data_array[i].b = (i+1)*(n/nb_threads);
        
        pthread_create(&thread_ptr[i], NULL, thread_function, (void *) &thread_data_array[i]);
    }

    for( i=0 ; i<nb_threads ; i++ ){
        pthread_join(thread_ptr[i], NULL);
    }

    return res;
}

int main(){
    double results[4];
    double times[4];
    int i;

    init();

    times[0] = now();
    for ( i=0 ; i<100 ; i++ )
    results[0] = rnorm(u, N);                           //100 scalar monothread
    times[0] = now()-times[0];

    times[1] = now();
    for ( i=0 ; i<100 ; i++ )
    results[1] = vect_rnorm(u, N);                      //100 vectorized monothread
    times[1] = now()-times[1];

    times[2] = now();
    for ( i=0 ; i<100 ; i++ )
    results[2] = rnormPar(u, N, NB_THREADS, false);     //100 scalar multithread
    times[2] = now()-times[2];

    times[3] = now();
    for ( i=0 ; i<100 ; i++ )
    results[3] = rnormPar(u, N, NB_THREADS, true);     //100 vectorized multithread
    times[3] = now()-times[3];

    printf("VALUES\nSequential (Scalar : %f ; Vectorized : %f)\n", results[0], results[1]);
    printf("Parallel (%d threads ; Scalar : %f ; Vectorized : %f)\n", NB_THREADS, results[2], results[3]);
    printf("TIME OF EXECUTION\nSequential (Scalar : %f ms ; Vectorized : %f ms)\n", times[0]*10, times[1]*10);
    printf("Parallel (%d threads ; Scalar : %f ms ; Vectorized : %f ms)\n", NB_THREADS, times[2]*10, times[3]*10);
    printf("Acceleration (Vectorized : %f ; Multithreaded : %f ; Vectorized+Multithreaded : %f)", times[0]/times[1], times[0]/times[2], times[0]/times[3]);

    return 0;
}
