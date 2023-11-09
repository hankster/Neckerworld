#! /usr/bin/bash

# update with the latest firmware
sudo apt-get update

# dpkgs needed
sudo apt-get -y install python3
sudo apt-get -y install make cmake pkg-config
sudo apt-get -y install gcc
sudo apt-get -y install libglfw3 libglfw3-dev
sudo apt-get -y install mesa-utils libglu1-mesa-dev mesa-common-dev
sudo apt-get -y install freeglut3 freeglut3-dev
sudo apt-get -y install libglew-dev libglm-dev
sudo apt-get -y install python3-pip
sudo apt-get -y install python3-tk

# Python packages needed
sudo python3 -m pip install tensorflow

