#include "dataManagement.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>


void initializeQueue(VehicleQueue* queue){
    queue->front=0;
    queue->rear=0;
    queue->count=0;
    queue->mutex=SDL_CreateMutex();
    queue->cond=SDL_CreateCond();
}
bool enqueue(VehicleQueue* queue, Vehicle vehicle) {

    SDL_LockMutex(queue->mutex);

    if (queue->count < QUEUE_SIZE) {

        queue->vehicles[queue->rear] = vehicle; // Add vehicle to the queue

        queue->rear = (queue->rear + 1) % QUEUE_SIZE; // Move rear index forward

        queue->count++; // Increment vehicle count

        SDL_CondSignal(queue->cond); // Signal that a vehicle has been added

        SDL_UnlockMutex(queue->mutex);

        return true; // Successfully added

    }

    SDL_UnlockMutex(queue->mutex);

    return false; // Queue is full

}

// Function to remove a vehicle from the queue

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

void initializeRoads(Road roads[MAX_ROADS]){
    const char* roadNames[MAX_ROADS] ={"Road A","Road B","Road C","Road D"};


    for (int i=0;i<MAX_ROADS;i++){
        strcpy(roads[i].roadName, roadNames[i]);
        printf("%s\n",roads[i].roadName);
        for(int j=0;j<MAX_LANE_SIZE;j++){
              initializeQueue(&(roads[i].lanes[j].queue));
              roads[i].lanes[j].isPriority=false;
            snprintf(roads[i].lanes[j].laneName, sizeof(roads[i].lanes[j].laneName), "%s%d", roads[i].roadName, j + 1);     
            printf("%s",roads[i].lanes[j].laneName);
              
        }
    }
}

 Road* findRoad(Road roads[MAX_ROADS], const char* roadName)
 {
     for (int i=0;i< MAX_ROADS;i++){
         if(strcmp(roads[i].roadName, roadName)==0){
            return &roads[i];
         }
     }
     return NULL;
 }

Lane* addVehicleToRandomLane(Road* roadPassed, Vehicle vehicle){
     if (roadPassed == NULL) {
    printf("Road not found.\n");
    return NULL;
}  
    if (roadPassed!=NULL){
        int laneIndex=rand()%MAX_LANE_SIZE;    
        printf("Attempting to add Vehicle %s to Lane %d of Road %s\n", vehicle.VechicleName, laneIndex + 1, roadPassed->roadName);
    if(enqueue(&roadPassed->lanes[laneIndex].queue,vehicle))
        {
         printf("Vehicle %s added to %s, Lane %s\n", vehicle.VechicleName, roadPassed->roadName,roadPassed->lanes[laneIndex].laneName);
         return &roadPassed->lanes[laneIndex];
        }
        else {

            printf("Lane %d of %s is full. Vehicle %s could not be added.\n", laneIndex + 1, roadPassed->roadName, vehicle.VechicleName);
            return NULL;
    }
    }

}
Lane* generateDestination(Lane* randomSourceLane, Road* roads[MAX_ROADS]){

    Lane* destinationLane=NULL;
     Lane* options=NULL;
    for (int j=0;j<MAX_ROADS;j++){
        if(randomSourceLane==&roads[0].lanes[3]){
            destinationLane=&roads[2].lanes[0];
            return destinationLane;
        }
        else if(randomSourceLane==&roads[0].lanes[1])
        {
           options =[&roads[1].lanes[1],&roads[3].lanes[1]]
         destinationLane=options(rand()%2);
         return destinationLane;
        }
        else if(randomSourceLane==&roads[1].lanes[2]){
             destinationLane=&roads[2].lanes[0];
             return destinationLane;
        } 
        else if (randomSourceLane==&roads[1].lanes[1]){
            options=[&roads[0].lane[1],&roads[2].lanes[1]];
            destinationLane=options(rand()%2);
            return destinationLane;

        }
        else if (randomSourceLane==&roads[2].lanes[2]){
            destinationLane=&roads[1].lanes[0];
            return destinationLane;
        }

        else if (randomSourceLane==&roads[2].lanes[1]){
            options=[&roads[0].lanes[1],&roads[3].lanes[1]];
            destinationLane=options(rand()%2);
            return destinationLane;
        }
        else if (randomSourceLane=&roads[3].lanes[2]){
            destinationLane= &roads[0].lanes[0];
            return destinationLane;
        }
        else if (randomSourceLane==&roads[3].lanes[2]){
            options=[&roads[2].lanes[1],&roads[1].lanes[1]];
            destinationLane=options(rand()%2);
            return destinationLane;
        }
        else{
            return NULL;
        }
    }
   

}

  

void printRoads(Road roads[MAX_ROADS]) {

    for (int i = 0; i < MAX_ROADS; i++) {

        printf("%s:\n", roads[i].roadName);

        for (int j = 0; j < MAX_LANE_SIZE; j++) {

            Lane* lane = &roads[i].lanes[j];

            printf("  Lane %d - Vehicles in Queue: %d, Priority: %s\n",

                   j + 1, lane->queue.count, lane->isPriority ? "Yes" : "No");

        }

    }

}

