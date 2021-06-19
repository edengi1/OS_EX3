#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

static int primes[4793];  /* There are 4792 primes <= sqrt(0x7fffffff) */

int isPrime(int num) {
    if ((num & 1) == 0)
        return num == 2;
    for (int *p = primes + 1; *p && (*p) * (*p) <= num; p++) {
        if (num % *p == 0)
            return 0;
    }
    return 1;
}

void initPrimes(int max) {
    int i = 0, p;
    primes[i++] = 2;
    for (p = 3; p <= max / p; p += 2) {
        if (isPrime(p))
            primes[i++] = p;
    }
}

struct primeCalcArgs {
    pthread_t tid;
    int *array;
    int start, end;
    long sum, count;
};

void *primeCalcTask(void *opaque) {
    struct primeCalcArgs *p = opaque;
    int *array = p->array;
    p->sum = 0;
    p->count = 0;
    for (int i = p->start; i < p->end; i++) {
        if (isPrime(array[i])) {
            p->sum += array[i];
            p->count++;
        }
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "primeCalc: Too few arguments\n");
        fprintf(stderr, "usage: ./primeCalc <prime pivot> <num of random numbers>\n");
        return 1;
    }

    int randomPivot = atoi(argv[1]);
    int numOfRandomNumbers = atoi(argv[2]);
    int numOfThreads = 4;
    long sum = 0;
    long primeCounter = 0;

    if (numOfThreads < 1)
        numOfThreads = 1;

    if (numOfRandomNumbers > 0) {
        int i, n;

        initPrimes(RAND_MAX);

        int *array = malloc(sizeof(*array) * numOfRandomNumbers);
        if (!array) {
            fprintf(stderr, "primeCalc: cannot allocate random number array\n");
            return 1;
        }
        srand(randomPivot);  //init rundom generator
        for (i = 0; i < numOfRandomNumbers; i++) {
            array[i] = rand();
        }
        struct primeCalcArgs args[numOfThreads];
        for (n = 0; n < numOfThreads; n++) {
            args[n].array = array;
            args[n].start = (long long)numOfRandomNumbers * n / numOfThreads;
            args[n].end = (long long)numOfRandomNumbers * (n + 1) / numOfThreads;
            args[n].sum = 0;
            args[n].count = 0;
            if (pthread_create(&args[n].tid, NULL, primeCalcTask, &args[n])) {
                fprintf(stderr, "primeCalc: cannot create thread %d\n", n + 1);
                break;
            }
        }
        for (i = 0; i < n; i++) {
            pthread_join(args[i].tid, NULL);
            sum += args[i].sum;
            primeCounter += args[i].count;
        }
        free(array);
    }
    printf("%ld,%ld\n", sum, primeCounter);
    return 0;
}
