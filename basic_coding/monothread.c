#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define N 10

double rnorm (float *U, int n){
    int i;
    double somme = 0;

    for (i=0; i<n; i++){
        somme += sqrt(U[i]);
    }

    return somme;
}

int main(){
    float U[N];
    double somme;
    int i;

    printf("Root-norm of [");

    for (i=0; i<N; i++) {                       // for loop to generate and display our vector
        U[i] = (float) (rand() % 100) / 30.;
        printf("%.2f", U[i]);
        if (i != N-1) {
            printf(", ");
        }
        else {
            printf("]\n");
        }
    }

    somme = rnorm(U,N);
    printf("is %.2f", somme);

    return 0;
}