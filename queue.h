#ifndef QUEUE_H
#define QUEUE_H
#include <stdbool.h>


/* defines */

/* for now each lane can accomodate 10 vehicles */
#define MAX_VEHICLE_QUEUE_SIZE 15
/* there are 4 road   */
#define MAX_LANE_QUEUE_SIZE 4

/* there are 3 lane for each road*/
#define MAX_LANE_SIZE

typedef struct{
    char VechicleName;
    char source[5];
    char destination1[5];
    char destination2[5];
}Vehicle;

typedef struct {
    Vehicle vehicles[MAX_LANE_QUEUE_SIZE];
    int front;
    int rear;
    int count;
}VehicleQueue;

typedef struct {

    bool ispriority;
    int VehiclesNo;
    VehicleQueue queue;
} Lane;

typedef struct {
    char Road;
    Lane lanes[MAX_LANE_SIZE];
}Road;





/* vehicle queue */
void Init_Vehicle_Queue(VehicleQueue *);
int Is_Vehicle_Queue_Empty(VehicleQueue *);
int Is_Vehicle_Queue_Full(VehicleQueue *);
void Enqueue_Vehicle(VehicleQueue *, Vehicle);
Vehicle Dequeue_Vehicle(VehicleQueue *);


#endif