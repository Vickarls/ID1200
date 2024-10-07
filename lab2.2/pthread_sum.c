#include <pthread.h>  
#include <stdio.h>     
#include <unistd.h>   
#include <stdlib.h>    
#include <time.h>      

int num_threads = 0;    
int array[1000];       
int *partial_sums;      

void *thread_func(void *id);

int main(int argc, char *argv[])
{

    num_threads = atoi(argv[1]); 

    srand(time(0));

    // Initialize the array with random values between 0 and 99
    for (int i = 0; i < 1000; i++) {
        array[i] = rand() % 100;
    }

    struct timespec start_time, end_time; // For measuring execution time
    double serial_time, parallel_time;    // To store time taken for sums

    // Start timing for serial sum
    clock_gettime(CLOCK_MONOTONIC, &start_time);

    int sum = 0; 

    for (int i = 0; i < 1000; i++) {
        sum += array[i];
    }

    // End timing for serial sum
    clock_gettime(CLOCK_MONOTONIC, &end_time);

    serial_time = (end_time.tv_sec - start_time.tv_sec) +
                  (end_time.tv_nsec - start_time.tv_nsec) / 1e9;

    printf("Serial sum is %d\n", sum);
    printf("Time taken for serial sum: %f seconds\n", serial_time);

    partial_sums = malloc(num_threads * sizeof(int));

    pthread_t *workers = malloc(num_threads * sizeof(pthread_t));

    // Start timing for parallel sum
    clock_gettime(CLOCK_MONOTONIC, &start_time);

    // Create the worker threads
    for (int i = 0; i < num_threads; i++) {
        // Allocate memory for the thread's ID
        int *id = malloc(sizeof(int));
        *id = i; 

        pthread_create(&workers[i], NULL, thread_func, (void *)id);
    }

    // Wait for all threads to finish execution
    for (int i = 0; i < num_threads; i++) {
        pthread_join(workers[i], NULL);
    }

    // Aggregate the partial sums from all threads
    int parallel_sum = 0;
    for (int i = 0; i < num_threads; i++) {
        parallel_sum += partial_sums[i];
    }

    // End timing for parallel sum
    clock_gettime(CLOCK_MONOTONIC, &end_time);

    parallel_time = (end_time.tv_sec - start_time.tv_sec) +
                    (end_time.tv_nsec - start_time.tv_nsec) / 1e9;

    printf("Parallel sum is %d\n", parallel_sum);
    printf("Time taken for parallel sum: %f seconds\n", parallel_time);

    // Free allocated memory
    free(partial_sums);
    free(workers);

    return 0; 
}

// Thread function to compute partial sums
void *thread_func(void *id) {
    // Retrieve and cast the thread ID from the argument
    int my_id = *(int *)id;
    free(id); 

    // Calculate the start and end indices for this thread's portion of the array
    int start = (1000 / num_threads) * my_id;         
    int end = (1000 / num_threads) * (my_id + 1);     

    // Adjust the end index for the last thread to include any remaining elements
    if (my_id + 1 == num_threads) {
        end = 1000; 
    }

    int sum = 0; 

    // Compute the sum of the assigned portion of the array
    for (int i = start; i < end; i++) {
        sum += array[i];
    }

    // Store the partial sum in the shared array
    partial_sums[my_id] = sum;

    printf("I am Thread %d / %d. My sum is %d\n", my_id, num_threads, sum);

    pthread_exit(0);
}
