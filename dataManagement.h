
#ifndef DATAMANAGEMENT_H
#define DATAMANAGEMENT_H
#include <stdbool.h>
#include <SDL2/SDL.h>

#define QUEUE_SIZE 10
#define MAX_ROADS 4
#define MAX_VEHICLE_QUEUE_SIZE 15
#define MAX_LANE_SIZE 3
// Forward declarations

typedef struct VehicleQueue VehicleQueue;

typedef struct Lane Lane;

typedef struct Road Road;


// Vehicle struct

typedef struct {

    char VechicleName[7];

    Road* road; // Store the road name as a string

    char carDestination[7];

} Vehicle;


// VehicleQueue struct

struct VehicleQueue {

    Vehicle vehicles[MAX_VEHICLE_QUEUE_SIZE];

    int front;

    int rear;

    int count;

    SDL_mutex* mutex;

    SDL_cond* cond;

};


// Lane struct

struct Lane {

    bool isPriority;

    char laneName[30]; // Increased size to avoid overflow

    int VehiclesNo;

    VehicleQueue queue; // Now this is fully defined

};


// Road struct

struct Road {

    char roadName[20];

    Lane lanes[MAX_LANE_SIZE]; // Road contains an array of Lane

};


void initializeRoads(Road roads[MAX_ROADS]);
Road* findRoad(Road roads[MAX_ROADS], const char* roadName);
void initializeQueue(VehicleQueue* queue);
bool enqueue(VehicleQueue* queue, Vehicle vehicle) ;
Vehicle dequeue(VehicleQueue* queue);
void addVehicleToRandomLane(Road*  roadPassed, Vehicle vehicle);
void printRoads(Road roads[MAX_ROADS]);


#endif
