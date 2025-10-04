#! /usr/bin/bash

# update with the latest firmware
sudo apt-get update
sudo apt-get upgrade

# dpkgs needed
sudo apt-get -y install make cmake pkg-config
sudo apt-get -y install g++
sudo apt-get -y install libglfw3 libglfw3-dev
sudo apt-get -y install mesa-utils libglu1-mesa-dev mesa-common-dev
sudo apt-get -y install freeglut3 freeglut3-dev
sudo apt-get -y install libglew-dev libglm-dev
sudo apt-get -y install libpng-dev
sudo apt-get -y install imagemagick

# Python packages needed
sudo apt-get -y install python3
sudo apt-get -y install python3-pip
sudo apt-get -y install python3-tk
sudo apt-get -y install python3-opencv
sudo apt-get -y install python3-matplotlib

# Note: For Ubuntu 24.04 LTS and later, Python package installs are "Managed"
# This requires use of a Python "Virtual Environment" to install packages for the user.
# See the section below for information on creating this environment.
sudo python3 -m pip install tensorflow
sudo python3 -m pip install xlib
sudo python3 -m pip install playsound
sudo python3 -m pip install torch
sudo python3 -m pip install torchvision
sudo python3 -m pip install pandas
sudo python3 -m pip install dill

# These links are required for training programs to run
cd ~/Neckerworld/server
ln -s ../assets assets
ln -s ../training training

# To install on Ubuntu 24.04 LTS and later, create a virtual environment
sudo apt install python3-venv
cd ~/Neckerworld
python3 -m venv nw

# For Tensorflow to work this environment variable might be needed
# To add it automatically, edit ".profile" and add this export statement
# Restart your shell session
export XLA_FLAGS="--xla_gpu_cuda_data_dir=/usr/lib/cuda"

# Activate the virtual environment after every login for each client
cd ~/Neckerworld
source nw/bin/activate

# These packages will now be installed in the user's directory
pip3 install ultralytics
pip3 install tensorflow
pip3 install opencv-python
pip3 install matplotlib
pip3 install dill
pip3 install Xlib
pip3 install pandas
pip3 install tqdm
pip3 install seaborn

# Create and start the server
cd ~/Neckerworld/server
make clean
make
cube ../setup/nwtest.json

# In a new terminal start the player
cd ~/Neckerworld/client
source ../nw/bin/activate
python3 nwplay.py -y

# To disable the virtual environment
deactivate
