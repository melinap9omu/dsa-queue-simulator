#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "dataManagement.h"
#include "dataManagement.c"

#define MAX_LINE_LENGTH 20
#define MAIN_FONT "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 800
#define SCALE 1
#define ROAD_WIDTH 150
#define LANE_WIDTH 50
#define ARROW_SIZE 15
#define QUEUE_SIZE 10
#define MAX_ROADS 4
#define MAX_VEHICLE_QUEUE_SIZE 15
#define MAX_LANE_SIZE 3
#define VEHICLE_WIDTH 30
#define VEHICLE_HEIGHT 20
#define VEHICLE_SPEED 2

const char* VEHICLE_FILE = "vehicles.data";

typedef struct {
    Vehicle vehicle;
    SDL_Rect rect;
    float x, y;           // Precise position for smooth movement
    float targetX, targetY; // Target position
    bool isMoving;
    bool hasArrived;
    int pathStep;         // Current step in the path
} VehicleUI;

typedef struct{
    int currentLight;
    int nextLight;
} SharedData;

typedef struct{
    SharedData sharedData;
    int greenDuration;
    int redDuration;
}TrafficLight;
typedef struct {
    Road* roads[MAX_ROADS];
} ThreadData;

VehicleUI activeVehicles[200];
int vehicleCount = 0;


// Function declarations

void initializeRoads(Road* roads[MAX_ROADS]);
bool initializeSDL(SDL_Window **window, SDL_Renderer **renderer);
void drawRoadsAndLane(SDL_Renderer *renderer, TTF_Font *font, Road* roads[MAX_ROADS]);
void displayText(SDL_Renderer *renderer, TTF_Font *font, char *text, int x, int y);
void drawLightForA(SDL_Renderer* renderer, bool isRed);
void drawLightForB(SDL_Renderer* renderer, bool isRed);
void drawLightForC(SDL_Renderer* renderer, bool isRed);
void drawLightForD(SDL_Renderer* renderer, bool isRed);
void refreshLight(SDL_Renderer *renderer, SharedData* sharedData);
void* chequeQueue(void* arg);
void* readAndParseFile(void* arg);
void calculatePath(Lane* sourceLane, Lane* destLane, int pathX[4], int pathY[4], int* numPoints, Road* roads[MAX_ROADS]);
void getLaneCoordinates(Lane* lane, int* startX, int* startY, int* endX, int* endY, Road* roads[MAX_ROADS]);
SDL_Color getVehicleColor(const char* vehicleName);
void addVehicleToUI(Vehicle vehicle, Road* roads[MAX_ROADS]);
void updateVehiclesPosition(Road* roads[MAX_ROADS]);
void renderVehicles(SDL_Renderer* renderer, TTF_Font* font);
void processVehicleQueues(Road* roads[MAX_ROADS], bool trafficLightStatus[MAX_ROADS]);
void updateTrafficLightStatus(bool trafficLightStatus[MAX_ROADS], SharedData* sharedData);







bool fileExists(const char* path) {

    struct stat buffer;

    return (stat(path, &buffer) == 0); // Returns true if file exists

}

void printMessageHelper(const char* message, int count) {
    for (int i = 0; i < count; i++) printf("%s\n", message);
}


int main() {
    if (fileExists(MAIN_FONT)) {

        printf("Font file exists at: %s\n", MAIN_FONT);

        // Proceed to load the font

    } else {

        printf("Font file not found at: %s\n", MAIN_FONT);

        // Handle the error (e.g., exit or use a default font)
    }
    
    pthread_t tQueue, tReadFile, tLight;
    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;
    SDL_Event event;
    bool trafficLightStatus[MAX_ROADS] = {false, false, false, false}; 
    Road* roads[MAX_ROADS]; // Declare the roads array

    initializeRoads(roads);

    if (!initializeSDL(&window, &renderer)) {
        return -1;
    }
    SDL_mutex* mutex = SDL_CreateMutex();
    SharedData sharedData = { 0, 0 }; // 0 => all red

    TTF_Font* font = TTF_OpenFont(MAIN_FONT, 24);
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {

        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());

        return -1;

    }


    // Initialize SDL_ttf

    if (TTF_Init() == -1) {

        printf("SDL_ttf could not initialize! TTF_Error: %s\n", TTF_GetError());

        SDL_Quit();

        return -1;

    }


    // Load font

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {

        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());

        return -1;

    }


    // Initialize SDL_ttf

    if (TTF_Init() == -1) {

        printf("SDL_ttf could not initialize! TTF_Error: %s\n", TTF_GetError());

        SDL_Quit();

        return -1;

    }


    
 

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);
    drawRoadsAndLane(renderer, font, roads);
    drawLightForA(renderer, sharedData.currentLight != 0);
    drawLightForB(renderer, sharedData.currentLight != 1);
    drawLightForC(renderer, sharedData.currentLight != 2);
    drawLightForD(renderer, sharedData.currentLight != 3);

    SDL_RenderPresent(renderer);
    ThreadData threadData;
for (int i = 0; i < MAX_ROADS; i++) {
    threadData.roads[i] = roads[i];
}


    // Create threads for queue processing and light changes
    pthread_create(&tQueue, NULL, chequeQueue, &sharedData);
    printf("upto chequeQueue is fine\n");
    pthread_create(&tReadFile, NULL, readAndParseFile, (void*)&threadData);

    printf("upto parsefile is okay\n");

    // Continue the UI thread
    bool running = true;
    Uint32 lastTime = SDL_GetTicks();
    
    while (running) {
        // Process SDL events
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
        }
        
        // Update light status
        refreshLight(renderer, &sharedData);
        
        // Frame timing for 60 fps
        Uint32 currentTime = SDL_GetTicks();
        if (currentTime - lastTime >= 16) {
            // Update traffic light statuses
            updateTrafficLightStatus(trafficLightStatus, &sharedData);
            
            // Process vehicle queues based on traffic lights
            processVehicleQueues(roads, trafficLightStatus);
            
            // Update vehicle positions
            updateVehiclesPosition(roads);
            
            // Clear screen and redraw everything
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderClear(renderer);
            
            // Redraw roads, lanes, vehicles, etc.
            drawRoadsAndLane(renderer, font, roads);
            // Draw traffic lights
            drawLightForA(renderer, sharedData.currentLight != 0);
            drawLightForB(renderer, sharedData.currentLight != 1);
            drawLightForC(renderer, sharedData.currentLight != 2);
            drawLightForD(renderer, sharedData.currentLight != 3);
            
            // Draw vehicles
            renderVehicles(renderer, font);
            
            // Present the rendered frame
            SDL_RenderPresent(renderer);
            
            lastTime = currentTime;
        }
    }
    
    // Cleanup
    SDL_DestroyMutex(mutex);
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    TTF_CloseFont(font);

    TTF_Quit();

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

 void drawLightForA(SDL_Renderer* renderer, bool isRed){
    // draw light box
       SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
    SDL_Rect lightBox = {375, 450, 50, 30};  
    SDL_RenderFillRect(renderer, &lightBox);

    if(isRed) SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); 
    else SDL_SetRenderDrawColor(renderer, 11, 156, 50, 255);    
    SDL_Rect straight_Light = {400, 455, 20, 20};
    SDL_RenderFillRect(renderer, &straight_Light);

    drawArrow(renderer, 380+10, 455, 380+10, 455+20, 380, 455+10);
}
void drawLightForB(SDL_Renderer* renderer, bool isRed){
    // draw light box
    SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
    SDL_Rect lightBox = {375, 300, 50, 30};
    SDL_RenderFillRect(renderer, &lightBox);
    if(isRed) SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); 
    else SDL_SetRenderDrawColor(renderer, 11, 156, 50, 255);    
    SDL_Rect straight_Light = {380, 305, 20, 20};
    SDL_RenderFillRect(renderer, &straight_Light);
    drawArrow(renderer, 410,305, 410, 305+20, 410+10, 305+10);
}

void drawLightForC(SDL_Renderer* renderer, bool isRed){
    SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
    SDL_Rect lightBox = {320, 375, 30, 50};  // Adjust position for road D
    SDL_RenderFillRect(renderer, &lightBox);

    if(isRed) SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); 
    else SDL_SetRenderDrawColor(renderer, 11, 156, 50, 255);    
    SDL_Rect straight_Light = {325, 400, 20, 20};
    SDL_RenderFillRect(renderer, &straight_Light);

    drawArrow(renderer, 325, 380+10, 325+20, 380+10, 325+10, 380);
}
void drawLightForD(SDL_Renderer* renderer, bool isRed){
      SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
    SDL_Rect lightBox = {450, 375, 30, 50};  // Adjust position for road C
    SDL_RenderFillRect(renderer, &lightBox);

    if(isRed) SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); 
    else SDL_SetRenderDrawColor(renderer, 11, 156, 50, 255);    
    SDL_Rect straight_Light = {455, 380, 20, 20};
    SDL_RenderFillRect(renderer, &straight_Light);

    drawArrow(renderer, 455, 405, 455+20, 405, 455+10, 405+10); // Adjust arrow direction for road C
}



void refreshLight(SDL_Renderer *renderer, SharedData* sharedData){
    if(sharedData->nextLight == sharedData->currentLight) return;

    // Set all lights to red first
    drawLightForA(renderer, true);  // A road light red
    drawLightForB(renderer, true);  // B road light red
    drawLightForC(renderer, true);  // C road light red
    drawLightForD(renderer, true);  // D road light red

    // Now set the appropriate light to green based on nextLight
    if(sharedData->nextLight == 0) {
        drawLightForA(renderer, false);  // A road light green
    }
    else if(sharedData->nextLight == 1) {
        drawLightForB(renderer, false);  // B road light green
    }
    else if(sharedData->nextLight == 2) {
        drawLightForC(renderer, false);  // C road light green
    }
    else if(sharedData->nextLight == 3) {
        drawLightForD(renderer, false);  // D road light green
    }

    // Present the updated renderer
    SDL_RenderPresent(renderer);

    printf("Light of queue updated from %d to %d\n", sharedData->currentLight,  sharedData->nextLight);
    sharedData->currentLight = sharedData->nextLight;
    fflush(stdout);
}



void drawRoadsAndLane(SDL_Renderer *renderer, TTF_Font *font, Road* roads[MAX_ROADS]) {
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
        displayText(renderer, font, roads[i]->roadName, (WINDOW_WIDTH/2)-36, (WINDOW_HEIGHT*i)-(30*i)); 
    }
    for (int i = 0; i < MAX_ROADS/2; i++) {
        displayText(renderer, font, roads[2+i]->roadName, (WINDOW_WIDTH*(1-i)-(96*(1-i))), (WINDOW_HEIGHT/2) - 16); 
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

// Color mapping for vehicles
SDL_Color getVehicleColor(const char* vehicleName) {
    // Use the first character of the vehicle name to determine color
    char firstChar = vehicleName[0];
    
    switch(firstChar) {
        case 'A': return (SDL_Color){255, 0, 0, 255};    // Red
        case 'B': return (SDL_Color){0, 0, 255, 255};    // Blue
        case 'C': return (SDL_Color){0, 255, 0, 255};    // Green
        case 'D': return (SDL_Color){255, 255, 0, 255};  // Yellow
        case 'E': return (SDL_Color){255, 0, 255, 255};  // Magenta
        default:  return (SDL_Color){100, 100, 100, 255}; // Gray
    }
}


// Get lane position in screen coordinates
void getLaneCoordinates(Lane* lane, int* startX, int* startY, int* endX, int* endY, Road* roads[MAX_ROADS]) {
    int roadIndex = -1;
    int laneIndex = -1;
    
    // Find the road and lane index
    for (int i = 0; i < MAX_ROADS; i++) {
        Road* currentRoad = roads[i];
        if (lane->road == currentRoad) {
            roadIndex = i;
            for (int j = 0; j < MAX_LANE_SIZE; j++) {
                if (lane == &(currentRoad->lanes[j])) {
                    laneIndex = j;
                    break;
                }
            }
            break;
        }
    }
    
    if (roadIndex == -1 || laneIndex == -1) {
        // Handle error: lane not found
        *startX = *startY = *endX = *endY = 0;
        return;
    }
    
    // Window center
    int centerX = WINDOW_WIDTH / 2;
    int centerY = WINDOW_HEIGHT / 2;
    int laneOffset = LANE_WIDTH * laneIndex + LANE_WIDTH / 2;
    
    // Calculate based on road orientation
    switch (roadIndex) {
        case 0: // Road A (bottom)
            *startX = centerX - ROAD_WIDTH / 2 + laneOffset;
            *startY = WINDOW_HEIGHT;
            *endX = centerX - ROAD_WIDTH / 2 + laneOffset;
            *endY = centerY + ROAD_WIDTH / 2;
            break;
        case 1: // Road B (top)
            *startX = centerX + ROAD_WIDTH / 2 - laneOffset;
            *startY = 0;
            *endX = centerX + ROAD_WIDTH / 2 - laneOffset;
            *endY = centerY - ROAD_WIDTH / 2;
            break;
        case 2: // Road C (right)
            *startX = WINDOW_WIDTH;
            *startY = centerY - ROAD_WIDTH / 2 + laneOffset;
            *endX = centerX + ROAD_WIDTH / 2;
            *endY = centerY - ROAD_WIDTH / 2 + laneOffset;
            break;
        case 3: // Road D (left)
            *startX = 0;
            *startY = centerY + ROAD_WIDTH / 2 - laneOffset;
            *endX = centerX - ROAD_WIDTH / 2;
            *endY = centerY + ROAD_WIDTH / 2 - laneOffset;
            break;
    }
}
// Calculate a path through the intersection based on source and destination lanes
void calculatePath(Lane* sourceLane, Lane* destLane, int pathX[4], int pathY[4], int* numPoints, Road* roads[MAX_ROADS]) {
    int sourceStartX, sourceStartY, sourceEndX, sourceEndY;
    int destStartX, destStartY, destEndX, destEndY;
    getLaneCoordinates(sourceLane, &sourceStartX, &sourceStartY, &sourceEndX, &sourceEndY, roads);
    getLaneCoordinates(destLane, &destStartX, &destStartY, &destEndX, &destEndY, roads);
    // First point is the entry point to the intersection
    pathX[0] = sourceEndX;
    pathY[0] = sourceEndY;
    // Last point is the exit point from the intersection
    pathX[3] = destStartX;
    pathY[3] = destStartY;
    // Calculate mid-points for a smooth curve through the intersection
    int centerX = WINDOW_WIDTH / 2;
    int centerY = WINDOW_HEIGHT / 2;
    // Create a curve by adding two control points
    pathX[1] = centerX;
    pathY[1] = sourceEndY;
    pathX[2] = destStartX;
    pathY[2] = centerY;
    *numPoints = 4;
    }
    void updateVehiclesPosition(Road* roads[MAX_ROADS]) {
        for (int i = 0; i < vehicleCount; i++) {
        VehicleUI* vui = &activeVehicles[i];
        if (!vui->isMoving || vui->hasArrived) continue;
        // Calculate direction vector
        float dx = vui->targetX - vui->x;
        float dy = vui->targetY - vui->y;
        float distance = sqrt(dx*dx + dy*dy);
        if (distance < VEHICLE_SPEED) {
        // Reached the target
        vui->x = vui->targetX;
        vui->y = vui->targetY;
        vui->pathStep++;
        // Check if the vehicle has reached its final destination
        if (vui->pathStep >= 4) {
        vui->hasArrived = true;
        vui->isMoving = false;
        } else {
        // Set the next target in the path
        int pathX[4], pathY[4], numPoints;
        calculatePath(vui->vehicle.currentLane, vui->vehicle.destinationLane, 
        pathX, pathY, &numPoints, roads);
        vui->targetX = pathX[vui->pathStep];
        vui->targetY = pathY[vui->pathStep];
        }
        } else {
        // Move towards the target
        float ratio = VEHICLE_SPEED / distance;
        vui->x += dx * ratio;
        vui->y += dy * ratio;
        }
        // Update the rectangle position
        vui->rect.x = (int)vui->x - VEHICLE_WIDTH / 2;
        vui->rect.y = (int)vui->y - VEHICLE_HEIGHT / 2;
        }
        // Clean up vehicles that have reached their destination
        int newCount = 0;
        for (int i = 0; i < vehicleCount; i++) {
        if (!activeVehicles[i].hasArrived) {
        if (i != newCount) {
        activeVehicles[newCount] = activeVehicles[i];
        }
        newCount++;
        }
        }
        vehicleCount = newCount;
        }  
  
 void renderVehicles(SDL_Renderer* renderer, TTF_Font* font) {
        for (int i = 0; i < vehicleCount; i++) {
        VehicleUI* vui = &activeVehicles[i];
        // Get color based on vehicle name
        SDL_Color color = getVehicleColor(vui->vehicle.VechicleName);
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        // Draw the vehicle rectangle
        SDL_RenderFillRect(renderer, &vui->rect);
        // Add a border
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderDrawRect(renderer, &vui->rect);
        // Display vehicle name
        SDL_Color textColor = {0, 0, 0, 255};
        SDL_Surface* textSurface = TTF_RenderText_Solid(font, vui->vehicle.VechicleName, textColor);
        if (textSurface) {
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, textSurface);
        if (texture) {
        SDL_Rect textRect = {
        vui->rect.x, 
        vui->rect.y - 20, 
        textSurface->w, 
        textSurface->h
        };
        SDL_RenderCopy(renderer, texture, NULL, &textRect);
        SDL_DestroyTexture(texture);
        }
        SDL_FreeSurface(textSurface);
        }
        }
        }
void updateTrafficLightStatus(bool trafficLightStatus[MAX_ROADS], SharedData* sharedData) {
    for (int i = 0; i < MAX_ROADS; i++) {
    trafficLightStatus[i] = (sharedData->currentLight == i);
    }
    }

void* readAndParseFile(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    Road** roads = data->roads;  
    printf("inside readAndParseFile\n");
    struct stat file_stat;
    off_t last_size = 0;
    
    while (1) {
        if(stat(VEHICLE_FILE, &file_stat) == 0 && file_stat.st_size == last_size) {
            sleep(2);
            continue;
        }
        
        last_size = file_stat.st_size;
        printf("checking file\n");
        FILE* file = fopen(VEHICLE_FILE, "r");
        if (!file) {
            perror("Error opening file");
            return NULL; // Handle error
        }
        
        printf("file is being open , starting to read\n");
        char line[MAX_LINE_LENGTH];
        char tempBuffer[MAX_LINE_LENGTH * 100]; // Consider dynamic allocation for large files
        tempBuffer[0] = '\0'; // Initialize buffer
        int firstLineProcessed = 0;
        
        while (fgets(line, sizeof(line), file)) {
            line[strcspn(line, "\n")] = 0; // Remove newline
            printf("Reading files\n");
            
            if (!firstLineProcessed) {
                firstLineProcessed = 1;
                Road* roadPassed = NULL;
                char* vehicleNumber = strtok(line, ":");
                char* road = strtok(NULL, ":");
                
                if (vehicleNumber == NULL || road == NULL) {
                    printf("Invalid line format: %s\n", line);
                    continue; // Skip this line
                }
                
                // Check road and assign roadPassed
                if (strcmp(road, "A") == 0) {
                    roadPassed = roads[0];
                } else if (strcmp(road, "B") == 0) {
                    roadPassed = roads[1];
                } else if (strcmp(road, "C") == 0) {
                    roadPassed = roads[2];
                } else if (strcmp(road, "D") == 0) {
                    roadPassed = roads[3];
                }
                
                if (roadPassed) {
                    Vehicle vehicle;
                    strncpy(vehicle.VechicleName, vehicleNumber, sizeof(vehicle.VechicleName));
                    vehicle.road = roadPassed;
                    printf("Vehicle: %s, Road: %s\n", vehicle.VechicleName, roadPassed->roadName);
                    Lane* lane = addVehicleToRandomLane(roadPassed, vehicle);
                    Lane* destinationLane = generateDestination(lane,  data->roads);
                    
                    if (destinationLane == NULL) {
                        printf("Warning: Generated null destination lane\n");
                        continue; // Handle appropriately
                    }
                    
                    printf("Generated destination lane\n");
                    vehicle.destinationLane = destinationLane;
                    printf("Passed destinationLane to vehicle\n");
                } if (!roadPassed) {
    printf("Warning: Invalid road in input: %s\n", road);
    continue;
}
            } else {
                strcat(tempBuffer, line);
                strcat(tempBuffer, "\n");
            }
        }
        
        fclose(file);
        
        // Overwrite the file with remaining lines
        FILE* outFile = fopen(VEHICLE_FILE, "w");
        if (!outFile) {
            perror("Error writing file");
        } else {
            fputs(tempBuffer, outFile);
            fclose(outFile);
        }
        
        sleep(2); // Manage timing
    }
}