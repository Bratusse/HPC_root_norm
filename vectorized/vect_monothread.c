#include <stdio.h>
#include <immintrin.h>
#include <math.h>
#include <time.h>
#include <sys/time.h> // for timing

#define N 10000000

double now(){
   // Returns time in seconds
   struct timeval t;
   double f_t;
   gettimeofday(&t, NULL);
   f_t = t.tv_usec;
   f_t = f_t/1000000.0;
   f_t +=t.tv_sec;
   return f_t;
}

float u[N] __attribute__((aligned(32)));            //align to 32 bytes to use 256 bits vectors

void init(){
    unsigned int i;
    for( i = 0; i < N; i++ ){
        u[i] = (float)rand () / RAND_MAX / N;
    }
}

double rnorm (float *U, int n){         //scalar for verification
    int i;
    double res = 0;
    for (i=0; i<n; i++){
        res += sqrt(U[i]);
    }
    return res;
}

#ifdef __AVX__

double vect_rnorm(float *U, int n){
    int i;
    double res = 0;
    float r256[8] __attribute__((aligned(32))) = {};
    __m256 *mm_u = (__m256 *)U;
    __m256 *mm_r = (__m256 *)r256;
    for( i = 0 ; i < n/8 ; i++ )
        mm_r[0] = _mm256_add_ps(_mm256_sqrt_ps(mm_u[i]),mm_r[0]);
    for( i = 0 ; i < 8 ; i++ )
        res += r256[i];
    return res;
}

double vect_rnorm_double(float *U, int n){
    int i;
    double res = 0;
    double r256[4] __attribute__((aligned(32))) = {};
    __m128 *mm_u = (__m128 *)U;
    __m256d *mm_r = (__m256d *)r256;
    for( i = 0 ; i < n/4 ; i++ )
        mm_r[0] = _mm256_add_pd(_mm256_sqrt_pd(_mm256_cvtps_pd(mm_u[i])),mm_r[0]);
    for( i = 0 ; i < 4 ; i++ )
        res += r256[i];
    return res;
}

#endif

double vect_rnorm_128(float *U, int n){
    int i;
    double res = 0;
    float r128[4] __attribute__((aligned(32))) = {};
    __m128 *mm_u = (__m128 *)U;
    __m128 *mm_r = (__m128 *)r128;
    for( i = 0 ; i < n/4 ; i++ )
        mm_r[0] = _mm_add_ps(_mm_sqrt_ps(mm_u[i]),mm_r[0]);
    for( i = 0 ; i < 4 ; i++ )
        res += r128[i];
    return res;
}

int main(){
    double t;
    double res;
    init();
    t = now();
    res = rnorm(u,N);
    t = now()-t;
    printf("Square-root norm (scalar) : %.10f\n in %fs\n", res, t);
    t = now();
    res = vect_rnorm_128(u,N);
    t = now()-t;
    printf("Square-root norm (128 bits) : %.10f\n in %fs\n", res, t);
    #ifdef __AVX__
    t = now();
    res = vect_rnorm(u,N);
    t = now()-t;
    printf("Square-root norm (256 bits) : %.10f\n in %fs\n", res, t);
    t = now();
    res = vect_rnorm_double(u,N);
    t = now()-t;
    printf("Square-root norm (256b double) : %.10f\n in %fs", res, t);
    #endif
    return 0;
}