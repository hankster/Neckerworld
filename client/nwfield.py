#! /usr/bin/python3
"""
nwfield.py -- Neckerworld Field Utility. This program injects new predators and resources into the plaing field.

Sample usage:

 nwfield.py

Complete specification:

 nwfield.py -a address -c cube_uuid -d -f filename -g -h -i time -p password -r -t -u username -v --address address --cube cube_uuid --debug --file=filename --game --help --iloop time --pswd=password --predator --resource --user=username --version

 where

 -a, --address        Address of remote host
 -d, --debug          Turn debug statements on
 -f, --file           Input filename
 -h, --help           Print usage information
 -i, --iloop          Insertion loop timer
 -p, --pswd           Password
 -r, --resource       Insert resources
 -t, --predator       Insert predators
 -u, --user           Username
 -v, --version        Report program version

Copyright (2023 H. S. Magnuski
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
import socket
import base64
import zlib
import numpy as np

from nwmessage import create_socket
from nwmessage import shutdown_socket
from nwmessage import login_request
from nwmessage import logout_request
from nwmessage import import_json_file
from nwmessage import import_json_object
from nwmessage import move_request
from nwmessage import status_request
from nwmessage import cube_view_request
from nwmessage import ground_view_request
from nwmessage import snapshot
from nwmessage import nwmessage_debug

# def create_socket(address, port, blocking)
# def shutdown_socket(s)
# def login_request(s, sequence, username, password, cube_uuid, ground_uuid)
# def logout_request(s, sequence, cube_uuid)
# def import_json_file(s, sequence, jsonfile, cube_uuid)
# def import_json_object(s, sequence, jsonobject, cube_uuid)
# def move_request(s, sequence, cube_uuid, spatial_angle, spatial_direction, spatial_direction_active, distance, velocity, gaze)
# def status_request(s, sequence, cube_uuid)
# def cube_view_request(s, sequence, cube_uuid, spatial_angle, gaze)
# def ground_view_request(s, sequence, cube_uuid, groundview)
# def snapshot(filename, image)
# def nwmessage_debug(debug)

debug = False
matrix_test = False
filename = ""
address = "127.0.0.1"
port = 2020

username = "example.server"
password = "pw"
groundview = -1
jsonobject = ""

# ground_scale_factor
gsf = 10.0

# Options
InsertPredator = False
InsertResource = False
iloop = False
ilooptime = 60.0
sequence = 0

# Location of all our cube definitions
training = "../training"
cubedata = training + "/cubedata.csv"

# Lists of predators
predators = []
pidx = 0

# Lists of resources
resources = []
ridx = 0

# Some standard texture mappings
# Standard face order is Front, Top, Back, Bottom, Left, Right

cube_texture_map_6_panes  = [0.00000, 0.00000, 0.16666, 0.00000, 0.16666, 1.00000, 0.00000, 1.00000,
                             0.33333, 0.00000, 0.50000, 0.00000, 0.50000, 1.00000, 0.33333, 1.00000,
                             0.83333, 0.00000, 0.83333, 1.00000, 0.66666, 1.00000, 0.66666, 0.00000,
                             1.00000, 0.00000, 1.00000, 1.00000, 0.83333, 1.00000, 0.83333, 0.00000,
                             0.16666, 1.00000, 0.16666, 0.00000, 0.33333, 0.00000, 0.33333, 1.00000,
                             0.66666, 0.00000, 0.66666, 1.00000, 0.50000, 1.00000, 0.50000, 0.00000]

cube_texture_map_resource = [0.0, 0.0, 0.5, 0.0, 0.5, 1.0, 0.0, 1.0,
                             0.5, 0.0, 1.0, 0.0, 1.0, 1.0, 0.5, 1.0,
                             0.0, 0.0, 0.5, 0.0, 0.5, 1.0, 0.0, 1.0,
                             0.5, 0.0, 1.0, 0.0, 1.0, 1.0, 0.5, 1.0,
                             0.0, 0.0, 0.5, 0.0, 0.5, 1.0, 0.0, 1.0,
                             0.0, 0.0, 0.5, 0.0, 0.5, 1.0, 0.0, 1.0]

def Usage():
    print("Usage: nwfield.py -a addr -d -f filename -h -i time -p password -r -t -u username -v --address addressess --debug --file=filename -help --iloop time --pswd password --resource --predator --user username --version")

# Check for communications error
def check_error(msg, response):
    if response["message_type"] == "Error":
        print("nwfield.py: %s - Error - %s" % (msg, response["error"]))
        return True
    return False

# Read the list of predators and resources
def cube_list():
    global cubedata
    try:
        with open(cubedata) as f:
            cubes = f.read().splitlines()
    except IOError as e:
        print("nwfield.py: Unable to read cubedata file %s - %s" % (cubedata, e))
        sys.exit()

    for c in cubes:

        cs = c.split('\t')
        if "predator" in c:
            predators.append(cs)
        if "resource" in c:
            resources.append(cs)
    
# Assign a cube location
def cube_location(cube_scale_factor, ground_scale_factor):

    dimension_minimum = -0.9 * ground_scale_factor
    dimension_maximum =  0.9 * ground_scale_factor
    x = random.uniform(dimension_minimum, dimension_maximum)
    y = cube_scale_factor + 0.01
    z = random.uniform(dimension_minimum, dimension_maximum)
    r = math.sqrt(2 * cube_scale_factor * cube_scale_factor)
    
    return [x, y, z], r

# Create a cube JSON file

def create_cube(o, index):
    
#  0 dataset,
#  1 cube_player,
#  2 cube_uuid,
#  3 cube_emoticon,
#  4 cube_firstname,
#  5 cube_scale_factor,
#  6 cube_type,
#  7 cube_color_class,
#  8 cube_color[0],
#  9 cube_color[1],
# 10 cube_color[2],
# 11 cube_material,
# 12 cube_surface,
# 13 spatial_position[0],
# 14 spatial_position[1],
# 15 spatial_position[2],
# 16 spatial_radius,
# 17 resource_energy,
# 18 texture_filename

# String from nwtest.json
# {"cube_index": 3, "cube_player": "predator", "cube_uuid": "b5a49a67-60cf-4b10-b136-04f28d986368", "cube_emoticon": "1f63d", "cube_firstname": "Predator", "cube_active": true, "cube_display": true, "cube_scale_factor": 1.1587331717166975, "cube_type": 3, "cube_color_class": "r", "cube_color": [0.8980392156862745, 0.1843137254901961, 0.26666666666666666, 1.0], "cube_material": 0, "cube_surface": "surface-8-snakeskin-512x512", "cube_texture_index": 5, "cube_texture_map": [0.0, 0.0, 0.16666, 0.0, 0.16666, 1.0, 0.0, 1.0, 0.33333, 0.0, 0.5, 0.0, 0.5, 1.0, 0.33333, 1.0, 0.83333, 0.0, 0.83333, 1.0, 0.66666, 1.0, 0.66666, 0.0, 1.0, 0.0, 1.0, 1.0, 0.83333, 1.0, 0.83333, 0.0, 0.16666, 1.0, 0.16666, 0.0, 0.33333, 0.0, 0.33333, 1.0, 0.66666, 0.0, 0.66666, 1.0, 0.5, 1.0, 0.5, 0.0], "cube_parameters": {}, "spatial_position": [-5.869175668233945, 1.1687331717166975, -1.3895444125031053], "spatial_rotation": [0.0, 0.7880859605606255, 0.0], "spatial_gaze": [0.0, 0.0], "spatial_radius": 1.638696166613346, "spatial_destination": [0.0, 0.0, 0.0], "spatial_velocity": 0.0, "resource_energy": 100.0}

    cube_player = o[1]
    cube_uuid = o[2]
    cube_emoticon = o[3]
    cube_firstname = o[4]
    cube_active = True
    cube_display = True
    cube_scale_factor = float(o[5])
    cube_type = int(o[6])
    cube_color_class = o[7]
    cube_color = [float(o[8]), float(o[9]), float(o[10]), 1.0]
    cube_material = int(o[11])
    cube_surface = o[12]
    # -1 indicates we need to add a texture map
    cube_texture_index = -1
    cube_parameters = {}
    spatial_position, r = cube_location(cube_scale_factor, gsf)
    spatial_rotation = [0.0, 0.0, 0.0]
    spatial_radius = float(o[16])
    spatial_gaze = [0.0, 0.0]
    spatial_destination = [0.0, 0.0, 0.0]
    spatial_velocity = 0.0
    resource_energy = float(o[17])
    cube_texture_filename = "../" + o[18]

    if cube_player == "predator":
        cube_texture_map = cube_texture_map_6_panes
    else:
        cube_texture_map = cube_texture_map_resource
        
    cube = {"cubes":[{"cube_index": -1, "cube_player": cube_player, "cube_uuid": cube_uuid, "cube_emoticon": cube_emoticon, "cube_firstname": cube_firstname,
            "cube_active": cube_active, "cube_display": cube_display, "cube_scale_factor": cube_scale_factor,
            "cube_type": cube_type, "cube_color_class": cube_color_class, "cube_color": cube_color, "cube_material": cube_material,
            "cube_surface": cube_surface, "cube_texture_index": cube_texture_index, "cube_texture_map": cube_texture_map, "cube_texture_filename": cube_texture_filename,
            "cube_parameters": cube_parameters,
            "spatial_position": spatial_position, "spatial_rotation": spatial_rotation, "spatial_gaze": spatial_gaze, "spatial_radius": spatial_radius,
            "spatial_destination": spatial_destination, "spatial_velocity": spatial_velocity,
            "resource_energy": resource_energy}]}

    return json.dumps(cube)

def quit_play(*args):
    global iloop
    iloop = False

# Check for communications error
def check_error(msg, response):
    if response["message_type"] == "Error":
        print("nwfield.py: %s - Error - %s" % (msg, response["error"]))
        return True
    return False


# Main program starts here

def main():
    
    global debug
    global s
    global sequence
    global iloop
    global jsonobject
    global pidx, ridx
    
    # Read in our list of predators, resources
    cube_list()
    
    # Create an inet, streaming socket with blocking
    s = create_socket(address, port, True)

    server_uuid = "XXXX"
    response = login_request(s, sequence, username, password, "", server_uuid)
    if check_error("login_request", response):
        # We're done. Goodbye.    
        shutdown_socket(s)
        return

    iloop = True
    iloop_count = 0.0
    iloop_start = time.time()

    # Insertion loop -- upload new predators and resources to gaming server to put on the playing field
    while iloop:

        if not InsertPredator and not InsertResource:
            print("nwfield.py: No insertions specified, nothing to do.")
            break

        if InsertPredator and len(predators) > 0:
            p = predators[pidx]
            cube_uuid = p[2]
            jsonobject = create_cube(p, pidx)
            print("nwfield.py: Sending predator %d %s named %s" % (pidx, p[3], p[4]))
            if debug:
                print(jsonobject)
            ijo_response = import_json_object(s, sequence, jsonobject, cube_uuid)
            if check_error("import_json_object", ijo_response):
                break
            pidx += 1
            if pidx == len(predators):
                pidx = 0
            time.sleep(1.0)
            
        if InsertResource and len(resources) > 0:
            r = resources[ridx]
            cube_uuid = r[2]
            jsonobject = create_cube(r, ridx)
            print("nwfield.py: Sending resource %d %s named %s" % (ridx, r[3], r[4]))
            if debug:
                print(jsonobject)
            ijo_response = import_json_object(s, sequence, jsonobject, cube_uuid)
            if check_error("import_json_object", ijo_response):
                break
            ridx += 1
            if ridx == len(resources):
                ridx = 0
            time.sleep(1.0)
                
        # End of this processing loop
        iloop_count += 1.0
        iloop_elapsed_time = time.time() - iloop_start
        iloops_per_second = iloop_count / iloop_elapsed_time
            
        time.sleep(ilooptime)
        
        sequence += 1
        
    logout_request(s, sequence, "", server_uuid)
        
    # We're done. Goodbye.    
    shutdown_socket(s)
    print("nwfield.py: Game over")

        
if __name__=='__main__':

    #
    # Get options and call the main program
    #                                                                                            

    try:
        options, args = getopt.getopt(sys.argv[1:], 'a:df:hi:p:rtu:v', ['address=','debug','file=','help','iloop','pswd=','resource','predator','user=','version'])
    except getopt.GetoptError:
        Usage()
        sys.exit(2)

    for o, a in options:
        if o in ("-a", "--address"):
            address = a
        if o in ("-d", "--debug"):
            debug = True
            nwmessage_debug(debug)
        if o in ("-f", "--file"):
            filename = a
        if o in ("-h", "--help"):
            Usage()
            sys.exit()
        if o in ("-i", "--iloop"):
            ilooptime = float(a)
        if o in ("-p", "--pswd"):
            password = a
        if o in ("-r", "--resource"):
            InsertResource = True
        if o in ("-t", "--predator"):
            InsertPredator = True
        if o in ("-u", "--user"):
            username = a
        if o in ("-v", "--version"):
            print("nwfield.py Version 1.0")
            sys.exit()
        
    main()
    sys.exit()

