#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>

const int MODE_MUTEX = 1;
const int MODE_CAS = 0;

int num_iterations = 500;
int num_threads = 1;
int next_thread_id = 0;
int sync_mode = 0;
int sync_lock = 0;

pthread_mutex_t mutex;

typedef struct node {
	int data;
	struct node *next;
} Node;

Node *top; // top of stack

void push(int dataitem) {
	Node *new_node = malloc(sizeof(Node));
	new_node->data = dataitem;

	// Update top of the stack below.
	/* Option 1: Mutex Lock */
	if (sync_mode == MODE_MUTEX) {
		pthread_mutex_lock(&mutex);

		new_node->next = top;
		top = new_node;

		pthread_mutex_unlock(&mutex);
	}

	/* Option 2: Compare-and-Swap (CAS) */
	if (sync_mode == MODE_CAS) {
		while(__sync_bool_compare_and_swap(&sync_lock, 0, 1));

		new_node->next = top;
		top = new_node;

		sync_lock = 0;
	}
}

int pop() {
    Node *old_node;

    if (sync_mode == MODE_MUTEX) {
        pthread_mutex_lock(&mutex);
        if (top == NULL) {
            pthread_mutex_unlock(&mutex);
            return -1; // Return an error code if the stack is empty
        }

        old_node = top;
        top = top->next;
        pthread_mutex_unlock(&mutex);
        free(old_node);
    }

    if (sync_mode == MODE_CAS) {
        while(__sync_bool_compare_and_swap(&sync_lock, 0, 1));
        if (top == NULL) {
            sync_lock = 0;
            return -1; // Return an error code if the stack is empty
        }

        old_node = top;
        top = top->next;
        sync_lock = 0;
        free(old_node);
    }

    return old_node->data;
}


void *thread_func() {
	/* Assign each thread an id so that they are unique in range [0, num_thread -1] */
	
	int my_id = __atomic_fetch_add(&next_thread_id, 1, __ATOMIC_ACQ_REL);
	
	printf("I am Thread %d / %d\n", my_id, num_threads);
	
	push(my_id);
	push(my_id);
	push(my_id);
	
	pop();
	pop();

	pthread_exit(0);

	return NULL;
}

long long timeMs(void) {
    struct timeval tv;

    gettimeofday(&tv,NULL);
    return (((long long)tv.tv_sec)*1000)+(tv.tv_usec/1000);
}

int main(int argc, char *argv[]) {

	/* Start timer */
	long long start = timeMs();

	if (argc >= 2) num_threads = atoi(argv[1]);
	if (argc >= 3) sync_mode = atoi(argv[2]);
	
	/* Initialize mutex variable */
	pthread_mutex_init(&mutex, NULL);

	/* Create a pool of num_threads workers and keep them in workers */
	pthread_t *workers = malloc(sizeof(pthread_t) * num_threads);

	for (int j = 0; j < num_iterations; j++) {

		for (int i = 0; i < num_threads; i++) {
			pthread_create(&workers[i], NULL, thread_func, NULL);
		}

		for (int i = 0; i < num_threads; i++) {
			pthread_join(workers[i], NULL);
		}

		// Print out the number of nodes remaining in Stack
		Node *current_node = top;
		int count = 0;

		while (current_node != NULL) {
			current_node = current_node->next;
			count++;
			pop();
		}

		printf("Number of nodes remaining in stack: %d\n", count);
		__atomic_store_n(&next_thread_id, 0, __ATOMIC_RELAXED);
	}

	/* Free up resources properly */
	free(workers);
	
	/* Stop timer */
	long long stop = timeMs();
	
	printf("Took %llu ms\n", stop - start);
	printf("Synchronized using %s (%d)\n", sync_mode == MODE_CAS ? "Compare-and-swap" : "Mutexes", sync_mode);
}
