# Neckerworld - A Computer Vision Game

![Neckerworld Player Cubesl](../images/Neckerworld-players.png)

## Overview

The "setup" directory contains programs to create and establish a game space.
A game space consists of a ground plane, player cubes and lighting.

## Usage

```
./nw.py
```

## Summary of programs

### nw.py
This is the main program to create a Neckerworld game space.
A new JSON file "nw-yyyy-mm-dd.json" is created as a result of running "nw.py".
The objects are randomly selected and randomly placed on the playing field for each invocation of "nw.py".
To use it, change to the server directory (/server) and type "cube ../setup/nw-yyyy-mm-dd.json"
If you then go to the game window and type "E" followed by "S" you will see a simulation of game action.

### nwu.py
A utility helper program for nw.py

### nwtest.py
A five-player game space used to test functions of the game.
Produces the standard test file "nwtest.json".
To use it change to the server directory (/server) and type "cube ../setup/nwtest.json"

### nwtest.json
The json file created by running nwtest.py.
This is a small field with five objects used for testing and debugging programs.

### nwtest.csv
A tab-separated file which creates a wide playing field similar to nwtest.py.
It's designed to be used with the --cubelist=nwtest.csv option in nw.py.
This is a wide (large width and depth) field with five objects used for testing and debugging programs.
To control the male cube, invoke nwplay.py with its unique UUID:
```
./nwplay.py -c a5ba491d-85a9-4736-bc60-c39adc12a723
```

### tournament.csv
This is a spreadsheet file (tab separated) which may be used to create a game with a specified number of cubes taken from the standard list of game cubes (see "/training/cubedata.csv").
This allows new game datasets to be specified in a repeatable manner for tournaments.

```
./nw.py --cubelist=tournament.csv
```

Also included are some other example .csv files and the spreadsheet file (.ods) used to generate them. 

### prettyjson.py
A utility to print JSON files in a readable format

```
./prettyjson.py jsonfile
```
