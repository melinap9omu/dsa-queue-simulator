# Traffic Junction Simulation

## Overview
This project simulates traffic flow through a four-way junction with traffic lights. The simulation visualizes vehicles approaching from different directions, waiting at traffic lights, and traversing the intersection based on traffic rules.

## Features
- Real-time visualization of a four-way junction with traffic lights
- Dynamic vehicle generation from external file input
- Multi-lane roads with vehicle queuing system
- Traffic light cycle management
- Vehicle path calculation through the intersection
- Thread-based concurrent processing

## Requirements
- SDL2 library
- SDL2_ttf library
- C compiler (GCC recommended)
- POSIX-compliant system (for pthread support)

## Project Structure
The project consists of the following key files:

1. **Main Simulation File (`main.c`):**
   - Contains the main simulation loop and UI rendering
   - Handles SDL initialization and event processing
   - Manages traffic light states and vehicle movement

2. **Data Management (`dataManagement.h` and `dataManagement.c`):**
   - Defines data structures for roads, lanes, and vehicles
   - Implements queue operations for vehicle management
   - Handles road and lane initialization

3. **Vehicle Generator (`vehicleGenerator.c`):**
   - Creates random vehicles and writes them to a data file
   - Specifies vehicle origin roads

## Building the Project
To compile the project, use the following command:

```bash
 gcc simulator.c -o simulator $(sdl2-config --cflags --libs) -lSDL2_ttf -lm`$ 
 gcc traffic_generator.c -o traffic_generator $(sdl2-config --cflags --libs) -lSDL2_ttf -lm```

## Running the Simulation
1. Start the vehicle generator:
   ```bash
   ./traffic_generator
   ```

2. Run the simulation:
   ```bash
   ./simulator
   ```

## How It Works

### Traffic System
The simulation models a four-way junction with roads labeled A, B, C, and D:
- Road A: Bottom road (northbound)
- Road B: Top road (southbound)
- Road C: Right road (westbound)
- Road D: Left road (eastbound)

Each road has multiple lanes that can hold vehicles in a queue.

### Vehicle Generation
Vehicles are randomly generated by the vehicle generator program and written to a data file (`vehicles.data`). Each vehicle has:
- A unique identifier
- A source road
- A randomly assigned lane on that road
- A destination lane that determines its path through the junction

### Traffic Light System
The traffic light system cycles through different states, allowing vehicles from different roads to pass through the intersection. Currently, the cycle alternates between:
- Road A (green for 5 seconds)
- Road C (green for 5 seconds)

### Vehicle Movement
Vehicles follow these steps:
1. Queue in their assigned lane on the source road
2. Wait for a green light
3. Follow a calculated path through the intersection
4. Exit via their destination lane

### Multithreading
The program uses multiple threads to handle:
- Main rendering and simulation loop
- Traffic light state management
- Vehicle file monitoring and processing

## Extending the Project
To extend this project, you might consider:
1. Adding more complex traffic light patterns
2. Implementing pedestrian crossings
3. Adding traffic congestion visualization
4. Creating more sophisticated path-finding algorithms
5. Adding collision detection
6. Implementing statistical analysis of traffic flow

## Troubleshooting
- **Font Loading Issues:** Ensure the DejaVuSans.ttf font is available at the specified path or modify the `MAIN_FONT` constant
- **Vehicle File Access:** Check that the program has read/write permissions for the vehicles.data file
- **SDL Initialization Errors:** Make sure SDL2 and SDL2_ttf libraries are correctly installed


![Demo GIF](https://github.com/melinap9omu/dsa-queue-simulator/blob/main/Untitled%20design.gif)
