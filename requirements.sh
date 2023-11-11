#! /usr/bin/bash

# update with the latest firmware
sudo apt-get update

# dpkgs needed
sudo apt-get -y install python3
sudo apt-get -y install python3-pip
sudo apt-get -y install python3-tk
sudo apt-get -y install python3-opencv
sudo apt-get -y install python3-matplotlib
sudo apt-get -y install make cmake pkg-config
sudo apt-get -y install g++
sudo apt-get -y install libglfw3 libglfw3-dev
sudo apt-get -y install mesa-utils libglu1-mesa-dev mesa-common-dev
sudo apt-get -y install freeglut3 freeglut3-dev
sudo apt-get -y install libglew-dev libglm-dev
sudo apt-get -y install libpng-dev
sudo apt-get -y install imagemagick

# Python packages needed
sudo python3 -m pip install tensorflow
sudo python3 -m pip install xlib
