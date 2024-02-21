#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#define NUM_STAGES 5
#define PAGE_SIZE 4096 // Assuming page size is 4KB

void safe_malloc(size_t size) {
    void *ptr = malloc(size);
    if (ptr == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    printf("Allocated %lu bytes of memory\n", size);
    free(ptr);
}

int main() {
    int stages[NUM_STAGES] = {1, 2, 4, 8, 16}; // Each stage doubles the size of memory allocation
    int i;
    // Load the LKM with the obtained PID as a parameter
    char load_command[100];
    // Get the PID of the current process
    pid_t pid = getpid();
    sprintf(load_command, "insmod lkm4.ko pid=%d", pid);

    char unload_command[100];
    sprintf(unload_command, "rmmod lkm4");

    system(load_command);
    printf("Sleeping for 2 seconds...\n");
    sleep(2);
    system(unload_command);
    // Run through each stage
    for (i = 0; i < NUM_STAGES; i++) {
        size_t size = stages[i] * PAGE_SIZE;
        safe_malloc(size);
        printf("Sleeping for 2 seconds...\n");
        sleep(2);

        // Sleep for a while to observe memory stats using LKM
        system(load_command);
        printf("Sleeping for 2 seconds...\n");
        sleep(2);
        system(unload_command);
    }

    return 0;
}
