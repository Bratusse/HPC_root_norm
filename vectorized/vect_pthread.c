#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <immintrin.h>
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
    long a;
    long b;
};

float u[N] __attribute__((aligned(32)));
double res = 0;

pthread_mutex_t res_mutex = PTHREAD_MUTEX_INITIALIZER;
struct thread_data thread_data_array[NB_THREADS];
unsigned int thread_i;
pthread_t thread_ptr[NB_THREADS];

void init(){
    unsigned int i;
    for( i = 0 ; i < N ; i++ ){
        u[i] = (float)rand () / RAND_MAX / N;
    }
}

double rnorm(float *U, int n){         //scalar for verification
    int i;
    double res = 0;
    for (i=0; i<n; i++){
        res += sqrt(U[i]);
    }
    return res;
}

void *thread_function(void *threadarg){
    /* Local variables */
    double s;
    long i;
    double r256[4] __attribute__((aligned(32))) = {};
    /* Shared variables correspondances */
    float *U;
    long a, b;
 
    /* Association between shared variables and their correspondances */
    struct thread_data *thread_pointer_data;
    thread_pointer_data = (struct thread_data *)threadarg;

    /* Shared variables */
    U = thread_pointer_data->U;
    a = thread_pointer_data->a;
    b = thread_pointer_data->b;
 
    /* Body of the thread */
    __m128 *mm_u = (__m128 *)U;
    __m256d *mm_r = (__m256d *)r256;
    for( i = a/4 ; i < b/4 ; i++ )
        mm_r[0] = _mm256_add_pd(_mm256_sqrt_pd(_mm256_cvtps_pd(mm_u[i])),mm_r[0]);
    for( i = 0 ; i < 4 ; i++ )
        s += r256[i];

    /* Mutex to add the partial result */
    pthread_mutex_lock(&res_mutex);
    res += s;
    pthread_mutex_unlock(&res_mutex);

    pthread_exit(NULL);
    return 0;
}

double rnormPar(float *U, int n, int nb_threads){
    int i;
    res = 0;

    for( i=0 ; i<nb_threads ; i++ ){
        thread_i = i;

        thread_data_array[thread_i].U = U;
        thread_data_array[thread_i].a = i*(n/nb_threads);
        thread_data_array[thread_i].b = (i+1)*(n/nb_threads);
        
        pthread_create(&thread_ptr[thread_i], NULL, thread_function, (void *) &thread_data_array[thread_i]);
    }

    for( i=0 ; i<nb_threads ; i++ ){
        pthread_join(thread_ptr[i], NULL);
    }

    return res;
}

int main(){
    double res_l;
    double t;
    int i;
    init();

    t = now();
    for ( i=0 ; i<100 ; i++ )
    res_l = rnorm(u, N);
    t = now()-t;
    printf("Square-root norm (scalar) : %.10f\n in %f ms\n", res_l, t*10);
    t = now();
    for ( i=0 ; i<100 ; i++ )
    res_l = rnormPar(u, N, NB_THREADS);
    t = now()-t;
    printf("Square-root norm (%d threads) : %.10f\n in %f ms", NB_THREADS, res_l, t*10);

    return 0;
}