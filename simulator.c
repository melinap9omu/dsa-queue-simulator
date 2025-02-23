#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#define MAX_LINE_LENGTH 20
#define MAIN_FONT "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 800
#define SCALE 1
#define ROAD_WIDTH 150
#define LANE_WIDTH 50
#define ARROW_SIZE 15
#define QUEUE_SIZE 10
#define MAX_VEHICLE_QUEUE_SIZE 15
#define MAX_ROADS 4
#define MAX_LANE_SIZE 3

const char* VEHICLE_FILE = "vehicles.data";





typedef struct{
    char VechicleName[7];
    char road[20];

}Vehicle;

typedef struct {
    Vehicle vehicles[MAX_VEHICLE_QUEUE_SIZE];
    int front;
    int rear;
    int count;
    SDL_mutex* mutex;
    SDL_cond* cond;

}VehicleQueue;

typedef struct {

    bool isPriority;
    int VehiclesNo;
    VehicleQueue queue;
} Lane;

typedef struct {
    char roadName[20];
    Lane lanes[MAX_LANE_SIZE];
}Road;

typedef struct{
    int currentLight;
    int nextLight;
} SharedData;

void initializeRoads(Road roads[MAX_ROADS]);
bool initializeSDL(SDL_Window **window, SDL_Renderer **renderer);
void drawRoadsAndLane(SDL_Renderer *renderer, TTF_Font *font, Road roads[MAX_ROADS]);
void displayText(SDL_Renderer *renderer, TTF_Font *font, char *text, int x, int y);
void drawLightForB(SDL_Renderer* renderer, bool isRed);
void refreshLight(SDL_Renderer *renderer, SharedData* sharedData);
void* chequeQueue(void* arg);
void* readAndParseFile(void* arg);
void initializeQueue(VehicleQueue* queue);
bool enqueue(VehicleQueue* queue, Vehicle vehicle) ;
Vehicle dequeue(VehicleQueue* queue);
Road* findRoad(Road roads[MAX_ROADS], const char* roadName);
Road* findRoad(Road roads[MAX_ROADS], const char* roadName);




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
}

void initializeRoads(Road roads[MAX_ROADS]){
    const char* roadNames[MAX_ROADS] ={"Road A","Road B","Road C","Road D"};

    for (int i=0;i<MAX_ROADS;i++){
        strcpy(roads[i].roadName, roadNames[i]);
        printf("%s\n",roads[i].roadName);
        for(int j=0;j<MAX_LANE_SIZE;j++){
              initializeQueue(&(roads[i].lanes[j].queue));
              roads[i].lanes[j].isPriority=false;
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

void addVehicleToRandomLane(Road roads[MAX_ROADS],const char* roadName, Vehicle vehicle){
    Road* road=findRoad(roads,roadName);
    if (road!=NULL){
        int laneIndex=rand()%MAX_LANE_SIZE;
        if(enqueue(&road->lanes[laneIndex].queue,vehicle))
        {
         printf("Vehicle %s added to %s, Lane %d\n", vehicle.VechicleName, roadName, laneIndex + 1);
        }
        else {

            printf("Lane %d of %s is full. Vehicle %s could not be added.\n", laneIndex + 1, roadName, vehicle.VechicleName);
    }
    }
    else {

        printf("Road %s not found.\n", roadName);
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






// Function declarations



void printMessageHelper(const char* message, int count) {
    for (int i = 0; i < count; i++) printf("%s\n", message);
}

int main() {
    pthread_t tQueue, tReadFile;
    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;
    SDL_Event event;

    Road roads[MAX_ROADS]; // Declare the roads array

    initializeRoads(roads);;



    if (!initializeSDL(&window, &renderer)) {
        return -1;
    }
    SDL_mutex* mutex = SDL_CreateMutex();
    SharedData sharedData = { 0, 0 }; // 0 => all red

    TTF_Font* font = TTF_OpenFont(MAIN_FONT, 24);
    if (!font) SDL_Log("Failed to load font: %s", TTF_GetError());

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);
    drawRoadsAndLane(renderer, font,roads);
    drawLightForB(renderer, false);

    SDL_RenderPresent(renderer);

    // we need to create seprate long running thread for the queue processing and light
   // pthread_create(&tLight, NULL, refreshLight, &sharedData);
    pthread_create(&tQueue, NULL, chequeQueue, &sharedData);
    pthread_create(&tReadFile, NULL, readAndParseFile, (void*) roads);


    // Continue the UI thread
    bool running = true;
    while (running) {
        // update light
        refreshLight(renderer, &sharedData);
        while (SDL_PollEvent(&event))
            if (event.type == SDL_QUIT) running = false;
    }
    SDL_DestroyMutex(mutex);
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    // pthread_kil
    SDL_Quit();
    return 0;
}

bool initializeSDL(SDL_Window **window, SDL_Renderer **renderer) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
        return false;
    }
    // font init
    if (TTF_Init() < 0) {
        SDL_Log("SDL_ttf could not initialize! TTF_Error: %s\n", TTF_GetError());
        return false;
    }


    *window = SDL_CreateWindow("Junction Diagram",
                               SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                               WINDOW_WIDTH*SCALE, WINDOW_HEIGHT*SCALE,
                               SDL_WINDOW_SHOWN);
    if (!*window) {
        SDL_Log("Failed to create window: %s", SDL_GetError());
        SDL_Quit();
        return false;
    }

    *renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_ACCELERATED);
    // if you have high resolution monitor 2K or 4K then scale
    SDL_RenderSetScale(*renderer, SCALE, SCALE);

    if (!*renderer) {
        SDL_Log("Failed to create renderer: %s", SDL_GetError());
        SDL_DestroyWindow(*window);
        TTF_Quit();
        SDL_Quit();
        return false;
    }

    return true;
}


void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}


void drawArrow(SDL_Renderer* renderer, int x1, int y1, int x2, int y2, int x3, int y3) {
    // Sort vertices by ascending Y (bubble sort approach)
    if (y1 > y2) { swap(&y1, &y2); swap(&x1, &x2); }
    if (y1 > y3) { swap(&y1, &y3); swap(&x1, &x3); }
    if (y2 > y3) { swap(&y2, &y3); swap(&x2, &x3); }

    // Compute slopes
    float dx1 = (y2 - y1) ? (float)(x2 - x1) / (y2 - y1) : 0;
    float dx2 = (y3 - y1) ? (float)(x3 - x1) / (y3 - y1) : 0;
    float dx3 = (y3 - y2) ? (float)(x3 - x2) / (y3 - y2) : 0;

    float sx1 = x1, sx2 = x1;

    // Fill first part (top to middle)
    for (int y = y1; y < y2; y++) {
        SDL_RenderDrawLine(renderer, (int)sx1, y, (int)sx2, y);
        sx1 += dx1;
        sx2 += dx2;
    }

    sx1 = x2;

    // Fill second part (middle to bottom)
    for (int y = y2; y <= y3; y++) {
        SDL_RenderDrawLine(renderer, (int)sx1, y, (int)sx2, y);
        sx1 += dx3;
        sx2 += dx2;
    }
}
 void drawLightForB(SDL_Renderer* renderer, bool isRed){
    // draw light box
    SDL_SetRenderDrawColor(renderer, 150, 75,0, 255);
    SDL_Rect lightBox = {400, 300, 50, 30};
    SDL_RenderFillRect(renderer, &lightBox);
    // draw light
    if(isRed) SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // red
    else SDL_SetRenderDrawColor(renderer, 11, 156,
    50, 255);    // green
    SDL_Rect straight_Light = {405, 305, 20, 20};
    SDL_RenderFillRect(renderer, &straight_Light);
    drawArrow(renderer, 435,305, 435, 305+20, 435+10, 305+10);
}






void drawRoadsAndLane(SDL_Renderer *renderer, TTF_Font *font, Road roads[MAX_ROADS]) {
    SDL_SetRenderDrawColor(renderer, 211,211,211,255);
    // Vertical road

    SDL_Rect verticalRoad = {WINDOW_WIDTH / 2 - ROAD_WIDTH / 2, 0, ROAD_WIDTH, WINDOW_HEIGHT};
    SDL_RenderFillRect(renderer, &verticalRoad);

    // Horizontal road
    SDL_Rect horizontalRoad = {0, WINDOW_HEIGHT / 2 - ROAD_WIDTH / 2, WINDOW_WIDTH, ROAD_WIDTH};
    SDL_RenderFillRect(renderer, &horizontalRoad);
    // draw horizontal lanes
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    for(int i=0; i<=3; i++){
        // Horizontal lanes
        SDL_RenderDrawLine(renderer,
            0, WINDOW_HEIGHT/2 - ROAD_WIDTH/2 + LANE_WIDTH*i,  // x1,y1
            WINDOW_WIDTH/2 - ROAD_WIDTH/2, WINDOW_HEIGHT/2 - ROAD_WIDTH/2 + LANE_WIDTH*i // x2, y2
        );
        SDL_RenderDrawLine(renderer,
            800, WINDOW_HEIGHT/2 - ROAD_WIDTH/2 + LANE_WIDTH*i,
            WINDOW_WIDTH/2 + ROAD_WIDTH/2, WINDOW_HEIGHT/2 - ROAD_WIDTH/2 + LANE_WIDTH*i
        );
        // Vertical lanes
        SDL_RenderDrawLine(renderer,
            WINDOW_WIDTH/2 - ROAD_WIDTH/2 + LANE_WIDTH*i, 0,
            WINDOW_WIDTH/2 - ROAD_WIDTH/2 + LANE_WIDTH*i, WINDOW_HEIGHT/2 - ROAD_WIDTH/2
        );
        SDL_RenderDrawLine(renderer,
            WINDOW_WIDTH/2 - ROAD_WIDTH/2 + LANE_WIDTH*i, 800,
            WINDOW_WIDTH/2 - ROAD_WIDTH/2 + LANE_WIDTH*i, WINDOW_HEIGHT/2 + ROAD_WIDTH/2
        );
    }
  
        for (int i = 0; i < MAX_ROADS/2; i++) {
         displayText(renderer, font, roads[i].roadName, (WINDOW_WIDTH/2)-36,  (WINDOW_HEIGHT*i)-(30*i)); 
        }
      for (int i = 0; i < MAX_ROADS/2; i++) {
         displayText(renderer, font, roads[2+i].roadName, (WINDOW_WIDTH*(1-i)-(96*(1-i))),  (WINDOW_HEIGHT/2) - 16); 
       }

}


void displayText(SDL_Renderer *renderer, TTF_Font *font, char *text, int x, int y){
    // display necessary text
    SDL_Color textColor = {0, 0, 0, 255}; // black color
    SDL_Surface *textSurface = TTF_RenderText_Solid(font, text, textColor);
    if(!textSurface){
        SDL_Log("failed to create text surface: %s", TTF_GetError());
    }
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, textSurface);
     if (!texture) {

        SDL_Log("Failed to create texture: %s", SDL_GetError());

        SDL_FreeSurface(textSurface);

        return; // Handle the error appropriately

    }
    SDL_FreeSurface(textSurface);
    SDL_Rect textRect = {x,y,0,0 };
    SDL_QueryTexture(texture, NULL, NULL, &textRect.w, &textRect.h);
    SDL_Log("DIM of SDL_Rect %d %d %d %d", textRect.x, textRect.y, textRect.h, textRect.w);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_Log("TTF_Error: %s\n", TTF_GetError());
    SDL_RenderCopy(renderer, texture, NULL, &textRect);
    // SDL_Log("TTF_Error: %s\n", TTF_GetError());
}


void refreshLight(SDL_Renderer *renderer, SharedData* sharedData){
    if(sharedData->nextLight == sharedData->currentLight) return; // early return

    if(sharedData->nextLight == 0){ // trun off all lights
        drawLightForB(renderer, false);
    }
    if(sharedData->nextLight == 2) drawLightForB(renderer, true);
    else drawLightForB(renderer, false);
    SDL_RenderPresent(renderer);
    printf("Light of queue updated from %d to %d\n", sharedData->currentLight,  sharedData->nextLight);
    // update the light
    sharedData->currentLight = sharedData->nextLight;
    fflush(stdout);
}


void* chequeQueue(void* arg){
    SharedData* sharedData = (SharedData*)arg;
    int i = 1;
    while (1) {
        sharedData->nextLight = 0;
        sleep(5);
        sharedData->nextLight = 2;
        sleep(5);
    }
}

// you may need to pass the queue on this function for sharing the data
void* readAndParseFile(void* arg) {
Road* roads = (Road*)arg;
    while(1){
        FILE* file = fopen(VEHICLE_FILE, "r");
        if (!file) {
            perror("Error opening file");
            continue;
        }

        char line[MAX_LINE_LENGTH];
        while (fgets(line, sizeof(line), file)) {
            // Remove newline if present
            line[strcspn(line, "\n")] = 0;
             char roadName[7];
            // Split using ':'
            char* vehicleNumber = strtok(line, ":");
            char* road = strtok(NULL, ":"); // read next item resulted from split

      if (vehicleNumber && road) {

                // Determine the road name based on the input

                if (strcmp(road, "A") == 0) {

                    strcpy(roadName, "Road A");

                } else if (strcmp(road, "B") == 0) {

                    strcpy(roadName, "Road B");

                } else if (strcmp(road, "C") == 0) {

                    strcpy(roadName, "Road C");

                } else if (strcmp(road, "D") == 0) {

                    strcpy(roadName, "Road D");

                } else {

                    printf("Invalid road: %s\n", road);

                    continue; // Skip to the next line if the road is invalid

                }

        Vehicle vehicle;

        strncpy(vehicle.VechicleName, vehicleNumber, sizeof(vehicle.VechicleName));

        strncpy(vehicle.road, road, sizeof(vehicle.road)); // Assuming 'road' is the road name



        printf("Vehicle: %s, Road: %s\n", vehicle.VechicleName, road);

        addVehicleToRandomLane(roads, roadName, vehicle); 
        

    } else {

        printf("Invalid format: %s\n", line);

    }
        fclose(file);
        sleep(2); // manage this time
    }
}
}