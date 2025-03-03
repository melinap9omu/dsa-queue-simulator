#include "dataManagement.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void initializeQueue(VehicleQueue* queue) {
    queue->front = 0;
    queue->rear = 0;
    queue->count = 0;
    queue->mutex = SDL_CreateMutex();
    queue->cond = SDL_CreateCond();
}

bool enqueue(VehicleQueue* queue, Vehicle vehicle) {
    if(queue==NULL){
        printf("Error:Queue is  not initialized in lane\n");
        return false;
    }
    if(queue->mutex==NULL){
        printf("Error: Mutex is not initialized for queue\n");
        return false;
    }
    SDL_LockMutex(queue->mutex);

    if (queue->count < QUEUE_SIZE) {
        queue->vehicles[queue->rear] = vehicle; // Add vehicle to the queue
        queue->rear = (queue->rear + 1) % QUEUE_SIZE; // Move rear index forward
        queue->count++; 
        printf("Vehicle hase been successfully added:\n");// Increment vehicle count
        SDL_CondSignal(queue->cond); // Signal that a vehicle has been added
        SDL_UnlockMutex(queue->mutex);
        return true; // Successfully added
    }
    

    SDL_UnlockMutex(queue->mutex);
    return false; // Queue is full
}

Vehicle dequeue(VehicleQueue* queue) {
    SDL_LockMutex(queue->mutex);

    while (queue->count == 0) {
        SDL_CondWait(queue->cond, queue->mutex);
    }

    Vehicle vehicle = queue->vehicles[queue->front]; // Get the vehicle from the front
    queue->front = (queue->front + 1) % QUEUE_SIZE; // Move front index forward
    queue->count--; // Decrement vehicle count
    SDL_UnlockMutex(queue->mutex);
    return vehicle;
}

void initializeRoads(Road* roads[MAX_ROADS]) {
    const char* roadNames[MAX_ROADS] = {"Road A", "Road B", "Road C", "Road D"};

    // Allocate memory for each road
    for (int i = 0; i < MAX_ROADS; i++) {
        roads[i] = (Road*)malloc(sizeof(Road));
        if (roads[i] == NULL) {
            printf("Memory allocation failed for road %d\n", i);
            return;
        }
        
        strcpy(roads[i]->roadName, roadNames[i]);
        printf("%s\n", roads[i]->roadName);
        
        for (int j = 0; j < MAX_LANE_SIZE; j++) {
            initializeQueue(&(roads[i]->lanes[j].queue));
            roads[i]->lanes[j].isPriority = false;
            snprintf(roads[i]->lanes[j].laneName, sizeof(roads[i]->lanes[j].laneName), 
                    "%s%d", roads[i]->roadName, j + 1);
            printf("%s", roads[i]->lanes[j].laneName);
            roads[i]->lanes[j].road = roads[i];
        }
    }
}

Road* findRoad(Road* roads[MAX_ROADS], const char* roadName) {
    for (int i = 0; i < MAX_ROADS; i++) {
        if (strcmp(roads[i]->roadName, roadName) == 0) {
            return roads[i];
        }
    }
    return NULL;
}

void addVehicleToRandomLaneWithDestinationLane(Road* roads[MAX_ROADS],Road* roadPassed, Vehicle vehicle) {
    if (roadPassed == NULL) {
        printf("Road not found.\n");
        return ;
    }  
    
    int laneIndex = rand() % MAX_LANE_SIZE;   
    Lane* selectedLane=&(roadPassed->lanes[laneIndex]);

    printf("Attempting to add Vehicle %s to Lane %d of Road %s\n", 
           vehicle.VechicleName, laneIndex + 1, roadPassed->roadName);
    Lane* destinationLane = generateDestination(selectedLane, roads);

    printf("Generated destination Lane %s\n",destinationLane->laneName);
    if(destinationLane!=NULL){
        vehicle.destinationLane=destinationLane;
    }
    
    if (enqueue(&selectedLane->queue, vehicle)) {
        printf("enqueuing vehicle after updating destinationLane.\n");
        
    }
}

Lane* generateDestination(Lane* randomSourceLane, Road* roads[MAX_ROADS]) {
    Lane* destinationLane = NULL;
    
    if (randomSourceLane == &roads[0]->lanes[3]) {
        destinationLane = &roads[2]->lanes[0];
        return destinationLane;
    }
    else if (randomSourceLane == &roads[0]->lanes[1]) {
        Lane* options[] = {&roads[1]->lanes[1], &roads[3]->lanes[1]};
        destinationLane = options[rand() % 2];
        return destinationLane;
    }
    else if (randomSourceLane == &roads[1]->lanes[2]) {
        destinationLane = &roads[2]->lanes[0];
        return destinationLane;
    } 
    else if (randomSourceLane == &roads[1]->lanes[1]) {
        Lane* options[] = {&roads[0]->lanes[1], &roads[2]->lanes[1]};
        destinationLane = options[rand() % 2];
        return destinationLane;
    }
    else if (randomSourceLane == &roads[2]->lanes[2]) {
        destinationLane = &roads[1]->lanes[0];
        return destinationLane;
    }
    else if (randomSourceLane == &roads[2]->lanes[1]) {
        Lane* options[] = {&roads[0]->lanes[1], &roads[3]->lanes[1]};
        destinationLane = options[rand() % 2];
        return destinationLane;
    }
    else if (randomSourceLane == &roads[3]->lanes[2]) {
        destinationLane = &roads[0]->lanes[0];
        return destinationLane;
    }
    else if (randomSourceLane == &roads[3]->lanes[1]) {
        Lane* options[] = {&roads[2]->lanes[1], &roads[1]->lanes[1]};
        destinationLane = options[rand() % 2];
        return destinationLane;
    }
    
    return NULL;
}

void printRoads(Road* roads[MAX_ROADS]) {
    for (int i = 0; i < MAX_ROADS; i++) {
        printf("%s:\n", roads[i]->roadName);
        for (int j = 0; j < MAX_LANE_SIZE; j++) {
            Lane* lane = &roads[i]->lanes[j];
            printf("  Lane %d - Vehicles in Queue: %d, Priority: %s\n",
                   j + 1, lane->queue.count, lane->isPriority ? "Yes" : "No");
        }
    }
}