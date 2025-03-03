

renderVehicles(renderer, font);
updateTrafficLightStatus(trafficLightStatus, &sharedData);
            
// Process vehicle queues based on traffic lights
processVehicleQueues(roads, trafficLightStatus);

// Update vehicle positions
updateVehiclesPosition(roads);
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

// Add a new vehicle to the UI
void addVehicleToUI(Vehicle vehicle, Road* roads[MAX_ROADS]) {
    if (vehicleCount >= 100) return; // Avoid overflow
    VehicleUI* vui = &activeVehicles[vehicleCount++];
    vui->vehicle = vehicle;
    // Set initial position based on the source lane
    int startX, startY, endX, endY;
    getLaneCoordinates(vehicle.currentLane, &startX, &startY, &endX, &endY, roads);
    vui->x = startX;
    vui->y = startY;
    vui->rect.x = (int)vui->x;
    vui->rect.y = (int)vui->y;
    vui->rect.w = VEHICLE_WIDTH;
    vui->rect.h = VEHICLE_HEIGHT;
    vui->isMoving = true;
    vui->hasArrived = false;
    vui->pathStep = 0;
    // Calculate the full path through the intersection
    int pathX[4], pathY[4], numPoints;
    calculatePath(vehicle.currentLane, vehicle.destinationLane, pathX, pathY, &numPoints, roads);
    // Set initial target to the entry of the intersection
    vui->targetX = pathX[0];
    vui->targetY = pathY[0];
    }  

// Check if we need to dequeue vehicles from the lanes
void processVehicleQueues(Road* roads[MAX_ROADS], bool trafficLightStatus[MAX_ROADS]) {
    for (int i = 0; i < MAX_ROADS; i++) {
    if (trafficLightStatus[i]) { // If green light
    for (int j = 0; j < MAX_LANE_SIZE; j++) {
    Lane* lane = &roads[i]->lanes[j];
    if (lane->queue.count > 0) {
    // Check if not too many vehicles are already in the intersection
    if (vehicleCount < 10) {
    SDL_LockMutex(lane->queue.mutex);
    if (lane->queue.count > 0) {
    Vehicle vehicle = dequeue(&lane->queue);
    vehicle.currentLane = lane;
    addVehicleToUI(vehicle, roads);
    }
    SDL_UnlockMutex(lane->queue.mutex);
    }
    }
    }
    }
    }
    }
    
void updateTrafficLightStatus(bool trafficLightStatus[MAX_ROADS], SharedData* sharedData) {
    for (int i = 0; i < MAX_ROADS; i++) {
    trafficLightStatus[i] = (sharedData->currentLight == i);
    }
    }