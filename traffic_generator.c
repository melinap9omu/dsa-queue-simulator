#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h> // For sleep()

#define FILENAME "vehicles.data"

// Function to generate a random vehicle number
void generateVehicleNumber(char* buffer) {
    buffer[0] = 'A' + rand() % 26;
    buffer[1] = 'A' + rand() % 26;
    buffer[2] = '0' + rand() % 10;
    buffer[3] = 'A' + rand() % 26;
    buffer[4] = 'A' + rand() % 26;
    buffer[5] = '0' + rand() % 10;
    buffer[6] = '0' + rand() % 10;
    buffer[7] = '0' + rand() % 10;
    buffer[8] = '\0';
}

// Function to generate a random lane
char generateLane() {
    char roads[] = {'A', 'B', 'C', 'D'};
    return roads[rand() % 4];
}
int main() {
    FILE* file = fopen(FILENAME, "a");
    if (!file) {
        perror("Error opening file");
        return 1;
    }

    srand(time(NULL)); // Initialize random seed

    while (1) {
        char vehicle[9];
        generateVehicleNumber(vehicle);
        char road = generateLane();

        // Write to file
        fprintf(file, "%s:%c\n", vehicle, road);
        fflush(file); // Ensure data is written immediately

        printf("Generated: %s:%c\n", vehicle, road); // Print to console

        int delay = 1000 + (rand() % 2000); // Delay between 1-3 seconds
        usleep(delay * 1000); 
    }

    fclose(file);
    return 0;
}