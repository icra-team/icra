#include <pthread.h>

int count;

void* thread(void *arg) {
    count = count + 1;
}

void main() {
    pthread_t t;
    int num = 2;
    int i;
    count = 0;
    for (i = 0; i < num; i++) {
	pthread_create(&t, NULL, thread, NULL);
    }
    assert(count <= num);
}
