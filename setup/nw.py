#! /usr/bin/python3
"""
nw.py -- A Python program to create JSON files for cube.cpp

Sample usage:

 nw.py

Complete specification:

 nw.py -d -f filename -h -s dataset -v --camera=T/F --cubes=T/F --debug --file=filename --grounds=T/F --help --lights=T/F --materials=T/F --set=dataset --textures=T/F --version --window=T/F

 where

 --camera=True/False    Setup camera position
 --cubes=True/False     Setup cubes
 --debug, -d            Turn debug statements on
 --file, -f             Output filename
 --grounds=True/False   Setup grounds
 --help, -h             Print usage information
 --lights=True/False    Setup lights
 --materials=True/False Setup materials
 --set, -s              Dataset
 --textures=True/False  Setup textures
 --version, -v          Report program version
 --window=True/False    Setup main window

Copyright (2023) H. S. Magnuski
All rights reserved

"""

import sys
import os, os.path
import time
import getopt
import string
import math
import random
import uuid
import json

from nwu import new_cube
from nwu import new_texture
from nwu import cube_colors
from nwu import males_list
from nwu import get_male
from nwu import females_list
from nwu import get_female
from nwu import enbies_list
from nwu import get_enby
from nwu import predators_list
from nwu import get_predator
from nwu import resources_list
from nwu import get_resource
from nwu import surfaces_list
from nwu import get_surface
from nwu import cube_color_patch
from nwu import cube_color_texture
from nwu import cube_size
from nwu import cube_location
from nwu import cube_placement
from nwu import cube_rotation
from nwu import setup_window
from nwu import setup_camera
from nwu import setup_grounds
from nwu import setup_cubes
from nwu import setup_lights
from nwu import setup_materials
from nwu import setup_textures

local = time.localtime()
ymd = time.strftime('%Y-%m-%d', local)
filename = "nw-%s.json" % ymd
dataset = "Neckerworld %s" % ymd

# Number of players to create
nw_player_count = 20

# Choose wide field of play
wide_field = True

# Camera view parameters
camera_position = [0.0, 5.0, 14.50]
camera_position_wide = [0.0, 16.0, 56.50]
camera_target = [0.0, 0.0, 0.0]
camera_up = [0.0, 1.0, 0.0]

# Ground parameters
nw_ground_count = 1
nw_ground_textures = 2
nw_ground_scale_factor = 10
nw_ground_scale_factor_wide = 50
ground_texture_map_reference = [0.0, 0.0, 20.0, 0.0, 20.0, 20.0, 0.0, 20.0]
ground_texture_map_reference_wide = [0.0, 0.0, 100.0, 0.0, 100.0, 100.0, 0.0, 100.0]
ground_texture_map_background = [0.875, 1.0, 0.125, 1.0, 0.125, 0.0, 0.875, 0.0]

if wide_field:
    # Overwrite previous values
    camera_position = camera_position_wide
    nw_ground_scale_factor = nw_ground_scale_factor_wide
    ground_texture_map_reference = ground_texture_map_reference_wide
    
# Lighting and colors
# Greenish background color
color_green_mud = [0.25490, 0.313725, 0.282352, 1.0]
background_color = color_green_mud

ambient = [0.40, 0.40, 0.40]

# Components to set up
do_camera = True
do_cubes = True
do_grounds = True
do_lights = True
do_materials = True
do_textures = True
do_window = True

debug = False

def Usage():
    print("Usage: nw.py -d -f filename -h -s dataset -v --camera=True/False --cubes=True/False --debug --file=filename --grounds=True/False --help --lights=True/False --materials=True/False --set=dataset --textures=True/False --version --window=True/False")

# Create a new JSON data file
def create_JSON(jsonName):

    # Initialize JSON object
    data = {"dataset":dataset}

    # Setup our window
    if do_window:
        data["window"] = setup_window(dataset, background_color)
            
    # Setup the camera position and target
    if do_camera:
        data["camera"] = setup_camera(camera_position, camera_target, camera_up)

    # Setup the grounds
    if do_grounds:
        data["grounds"] = setup_grounds(nw_ground_count, nw_ground_scale_factor, ground_texture_map_reference, ground_texture_map_background, camera_position, camera_target, camera_up)

    # Setup cubes
    if do_cubes:
        data["cubes"] = setup_cubes(nw_player_count, nw_ground_scale_factor, nw_ground_textures)

    # Setup lights
    if do_lights:
        data["lights"] = setup_lights(ambient)

    # Setup materials
    if do_materials:
        data["materials"] = setup_materials()

    # Setup textures
    if do_cubes and do_textures:
        data["textures"] = setup_textures(nw_player_count, nw_ground_textures, data["cubes"])

    if debug :
        print(json.dumps(data, indent=4, sort_keys=True))

    # Write the new JSON file
    try:
        with open(jsonName, 'w') as f:
            json.dump(data, f)
    except (IOError, ValueError) as e:
        print("nw.py: error -- %s" % (e.args[0]))
        sys.exit(1)

#
# Main program starts here
#

def main():
    
    print("nw.py -- Create dataset %s for the cube program" % dataset)

    # Read in lists of males, females, enbies, predators, resources and surfaces
    males_list()
    females_list()
    enbies_list()
    predators_list()
    resources_list()
    surfaces_list()

    # Now start creating our big JSON file
    create_JSON(filename)

    return

if __name__=='__main__':

    #
    # Get options and call the main program
    #                                                                                            

    try:
        options, args = getopt.getopt(sys.argv[1:], 'df:hs:v', ['camera=', 'cubes=', 'debug', 'file=', 'grounds=', 'help', 'lights=', 'materials=', 'set=', 'textures=', 'version', 'window='])
    except getopt.GetoptError:
        Usage()
        sys.exit(2)

    for o, a in options:
        if o in ("--camera"):
            do_camera = True if a == 'True' else False
        if o in ("--cubes"):
            do_cubes = True if a == 'True' else False
        if o in ("-d", "--debug"):
            debug = True
        if o in ("-f", "--file"):
            filename = a
        if o in ("--grounds"):
            do_grounds = True if a == 'True' else False
        if o in ("-h", "--help"):
            Usage()
            sys.exit()
        if o in ("--lights"):
            do_lights = True if a == 'True' else False
        if o in ("--materials"):
            do_materials = True if a == 'True' else False
        if o in ("-s", "--set"):
            dataset = a
        if o in ("--textures"):
            do_textures = True if a == 'True' else False
        if o in ("-v", "--version"):
            print("nw.py Version 1.0")
            sys.exit()
        if o in ("--window"):
            do_window = True if a == 'True' else False
        
    main()
    sys.exit()
