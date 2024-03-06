# Neckerworld - A Computer Vision Game

## Overview

The "train" directory contains programs used for training a neural network model.

### nwtrain.py

Used to generate training images. Cycles through thousands of different camera views and projections.
Creates a list of views with their calculated bounding boxes and corner points.

### nwdata.py

Generates cube.json files from a spreadsheet. The JSON files are needed by nwtrain.py to display individual cubes on the screen.
Creates a spreadsheet fro a directory of cube.json files.
Using the -s option allows you to show each training image with its bounding box.

training/cubedata.csv --> training/cubes/*.json
training/cubes/*.json --> training/cubedata.csv
