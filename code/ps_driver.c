/*-----------------------------------------------------------------------------
                      K-MEANS HOST DRIVER FOR ZYNQ PS
        To be compiled with arm-linux-gnueabihf-gcc for the ARM processor.
-------------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

// --- Constants for the driver ---
#define NUM_POINTS 9
#define NUM_CLUSTERS 3
#define MAX_ITER 2

/*-----------------------------------------------------------------------------
                      THE HARDWARE MEMORY MAP "CONTRACT"
   These addresses and offsets MUST match the design from Vivado and the layout
   expected by the PL hardware. This will be changed based on the Vivado addresses.
-------------------------------------------------------------------------------*/
// Base physical address of your accelerator's memory space, from Vivado Address Editor
#define BRAM_BASE_PHYSICAL_ADDR 0x40000000
// Total size of the memory space to map
#define BRAM_SIZE 0x10000 // 64 KB, more than enough

// --- Control and Status Registers ---
#define CONTROL_REG_OFFSET 0x00 // Offset 0: Write 1 to start, 0 to clear
#define STATUS_REG_OFFSET  0x04 // Offset 4: Read bit 0 for done status (1 = done)

// --- Data Array Offsets (in bytes, calculated from the PL memory layout) ---
// These must be calculated based on the order and size of arrays in kmeans_kernel.c
#define CENTROIDS_X_OFFSET  0x100  // Start data at an offset for clarity
#define CENTROIDS_Y_OFFSET  (CENTROIDS_X_OFFSET + NUM_CLUSTERS * sizeof(float))
#define POINTS_X_OFFSET     (CENTROIDS_Y_OFFSET + NUM_CLUSTERS * sizeof(float))
#define POINTS_Y_OFFSET     (POINTS_X_OFFSET + NUM_POINTS * sizeof(float))
// Note: Skipping scratchpad arrays, as PS doesn't need to access them directly.
#define TOTAL_OFFSET        (POINTS_Y_OFFSET + NUM_POINTS * sizeof(float))
#define SUM_X_OFFSET        (TOTAL_OFFSET + NUM_CLUSTERS * NUM_POINTS * sizeof(float))
#define SUM_Y_OFFSET        (SUM_X_OFFSET + NUM_CLUSTERS * NUM_POINTS * sizeof(float))


int main() {
    printf("--- K-Means Host Application Starting ---\n");

    // 1. Open /dev/mem to get access to physical memory
    int mem_fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (mem_fd < 0) {
        perror("Failed to open /dev/mem");
        return -1;
    }

    // 2. Memory-map the hardware's BRAM into the PS's virtual address space
    void* bram_virt_base = mmap(NULL, BRAM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, BRAM_BASE_PHYSICAL_ADDR);
    if (bram_virt_base == MAP_FAILED) {
        perror("mmap failed");
        close(mem_fd);
        return -1;
    }
    printf("Successfully mapped PL BRAM at physical 0x%X to virtual address %p\n", BRAM_BASE_PHYSICAL_ADDR, bram_virt_base);

    // 3. Create C pointers to the specific hardware registers and data arrays
    volatile unsigned int* gpu_ctrl_ptr = (unsigned int*)(bram_virt_base + CONTROL_REG_OFFSET);
    volatile unsigned int* gpu_status_ptr = (unsigned int*)(bram_virt_base + STATUS_REG_OFFSET);

    float* centroids_x_ptr = (float*)(bram_virt_base + CENTROIDS_X_OFFSET);
    float* centroids_y_ptr = (float*)(bram_virt_base + CENTROIDS_Y_OFFSET);
    float* points_x_ptr = (float*)(bram_virt_base + POINTS_X_OFFSET);
    float* points_y_ptr = (float*)(bram_virt_base + POINTS_Y_OFFSET);
    float* total_ptr = (float*)(bram_virt_base + TOTAL_OFFSET);
    float* sum_x_ptr = (float*)(bram_virt_base + SUM_X_OFFSET);
    float* sum_y_ptr = (float*)(bram_virt_base + SUM_Y_OFFSET);

    // 4. Initialize Input Data: Write data from PS to PL's BRAM
    // TODO: Replace this with actual dataset
    printf("Writing initial data to PL BRAM...\n");
    for (int i = 0; i < NUM_POINTS; i++) {
        points_x_ptr[i] = (float)(i * 2.0); // Example data
        points_y_ptr[i] = (float)(i * -1.5);
    }
    centroids_x_ptr[0] = 0.0; centroids_y_ptr[0] = 0.0;
    centroids_x_ptr[1] = 5.0; centroids_y_ptr[1] = -5.0;
    centroids_x_ptr[2] = 10.0; centroids_y_ptr[2] = -10.0;

    // --- Main K-Means Application Loop ---
    for (int cycle = 0; cycle < MAX_ITER; cycle++) {
        printf("\n--- Cycle %d ---\n", cycle);

        // 5. Start the PL Accelerator
        printf("Telling PL to start kernel...\n");
        *gpu_ctrl_ptr = 1; // Write '1' to the start register

        // 6. Wait for the PL to finish by polling the status register
        while ((*gpu_status_ptr & 0x1) == 0) {
            // Wait until the 'done' bit (bit 0) is set to 1 by the PL
            usleep(10); // Sleep for 10 microseconds to avoid wasting CPU cycles
        }
        printf("PL has finished execution.\n");

        // Important: Acknowledge and reset the hardware for the next run
        *gpu_ctrl_ptr = 0;

        // 7. Process Results: The CPU reads the reduction results and calculates the new centroids
        printf("Updating centroids on PS...\n");
        for (int k = 0; k < NUM_CLUSTERS; k++) {
            // Access the final sums, which are in index [0] of each cluster's reduction buffer
            float final_total = *(total_ptr + k * NUM_POINTS); // Accesses total[k][0]
            float final_sum_x = *(sum_x_ptr + k * NUM_POINTS); // Accesses sum_x[k][0]
            float final_sum_y = *(sum_y_ptr + k * NUM_POINTS); // Accesses sum_y[k][0]

            if (final_total > 0.0) {
                // Calculate and write the new centroid back to the PL's memory for the next iteration
                centroids_x_ptr[k] = final_sum_x / final_total;
                centroids_y_ptr[k] = final_sum_y / final_total;
                printf("  New Centroid %d: (%f, %f)\n", k, centroids_x_ptr[k], centroids_y_ptr[k]);
            } else {
                printf("  Centroid %d is empty, not updating.\n", k);
            }
        }
    }

    // 8. Clean up
    printf("\nK-Means complete. Unmapping memory.\n");
    munmap(bram_virt_base, BRAM_SIZE);
    close(mem_fd);

    return 0;
}