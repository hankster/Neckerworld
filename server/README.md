# Neckerworld - A Computer Vision Game

![Neckerworld server diagram](../images/Neckerworld-server.png)

## Overview

The "server" directory contains all the software and components needed to create the playing field server.
The "cube" program is the runtime server and uses opengl to render images of the playing field.
It's started by running a command with a JSON file argument describing the window and ground textures:

```
cube nw.json
```
This launches the game.
Port 2020 is opened up for receipt of messages from the client players.

## Summary of programs

### cube.cpp

This is the main C++ program and starts all activity.

### cube_data.cpp

Accepts and decodes JSON files to create data structures for the playing field.

### cube_engine.cpp

Contains the main display loop to render objects in opengl.

### cube_objects.cpp

Processes all requests for playing field objects.
Moves and rotates cubees as requested.
Checks for contact and conflicts in cube positions.
Changes camera and light positions.
Handles ground motions and positions.

### cube_simulation.cpp

Accepts incoming JSON messages from all clients and intrprets them.
Responds to clients with a JSON message update.

### cube.f.glsl

An opengl shader program.

### cube.v.glsl

An opengl shader program.

## Installation

Follow these steps to create and install the game server:
```
# Compile the multi-thread server code (has some errors)
cd Neckerworld/server/mtserver/mtserver
make
cd Neckerworld/server
make
# Start the server with a playing-field file
cube Neckerworld/client/nwtest.json
# To activate predators on the playing field select the server window and type "S"
# This will enable the server's basic strategy (control of predators and resources).
S
```

