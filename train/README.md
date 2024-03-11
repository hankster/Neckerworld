# Neckerworld - A Computer Vision Game

## Overview

The "train" directory contains programs used for training a neural network model. The client game players receive, by request, a small image, (currently 512x512), which is the "Cube's Eye" view of Neckerworld. This is the only information a player receives about the playing field and other game participants.

To decode the cube's eye view of the world an object identification neural network is used. Three models are supported:

* A model created on Google Cloud Machine Learning. This is the default and requires Tensorflow.
* A model based on efficientdet-d0. Requires Tensorflow.
* A model based on YOLOv5 which requires PyTorch and can utilize an NVidia CUDA GPU for custom training and inference. This model was trained on 13,920 images using a GPU and is currently the best performing. It handles wide-field game play and GPU-inference, which the other models lack.

Model selection is changed by editing nwplay.py (e.g. "nwyolo_active = True")

### nwtrain.py

Used to generate training images. Cycles through thousands of different camera views and projections.
Creates a list of views with their calculated bounding boxes and corner points.
Completion time can be the better part of a day.

### nwdata.py

This is a small utility program for creating and manipulating .json, .txt, .csv and .jpg files. The six functions available:

* --audit    Verifies that the created training images match the training list.
* --csv      Creates the cubedata.csv master file from a directory of cube.json files (training/cubes/*.json --> training/cubedata.csv).
* --generate Generate cube json files from a csv list (training/cubedata.csv --> training/cubes/*.json).
* --show     Display the training image with its bounding box.
* --txt      Create a tab-separated list of training images, classes, bounding boxes and corners from the training txt file.
* --yolo     Setup directories, images and labels needed for YOLOv5 custom training.

### Custom training for YOLOv5

There is no neural net in this universe that can properly identify Neckerworld Cubes. We need to create one. Top performance and speed of inference will be needed for game-winning play. The following is an outline of setting up and doing custom training for YOLOv5 models. There are hundreds of neural network models to choose from. This example was easy to do in PyTorch. A rough step-by-step outline is given, but it is a complicated process and the user will need to research the tutorials if there are problems.

```
# Download the YOLOv5 model

cd
git clone https://github.com/ultralytics/yolov5

# Run one of the example programs to check if all requirements have been met.
# See, for example, "https://docs.ultralytics.com/models/yolov5/#supported-tasks-and-modes"

# Setup necessary components for our training run

cd ~/Neckerworld/train

# Copy the labels file

cp nw.yaml ~/yolov5/data

# Copy the training script. The memory available in your GPU will determine the model size you can use.

cp trainnw.sh ~/yolov5

# Create the label and bounding box text files needed for training.

./nwdata.py --yolo

# Run custom training

cd ~/yolov5
./trainnw.sh

# Training should now begin using 13,920 images.
# A useful tool for checking on your GPU from NVidia

nvidia-smi

# Results will be found in the runs directory.
# An example set of results is in ~/Neckerworld/train/runs
# In yolov5 look at:

ls -al runs/exp/*

# Edit ~/Neckerworld/client/nwplay.py
# Set "nwyolo_active = True" and all others False

```
