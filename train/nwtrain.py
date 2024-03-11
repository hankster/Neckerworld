#! /usr/bin/python3
"""
nwtrain.py -- Create training snapshots with a set of different groundviews x randomized different spatial angles

Sample usage:

 nwtrain.py

Complete specification:

  nwmtrain.py -a address -c cube_uuid -d -f filename -g ground -h -i -p password -r emoticon -u username -v --address address --cube cube_uuid --debug --file=filename --ground=ground  --help --image --pswd=password --restart=emoticon --user=username --version

 where

 -a, --address        Address of remote host
 -c, --cube           Cube UUID
 -d, --debug          Turn debug statements on
 -f, --file           Input filename
 -g, --ground         Ground view index
 -h, --help           Print usage information
 -i, --image          Image snapshot
 -j, --json           Import a JSON file
 -m, --move           Move requested
 -s, --status         Status requested
 -p, --pswd           Password
 -r, --restart        Restart with emoticon
 -u, --user           Username
 -v, --version        Report program version

Copyright (2024) H. S. Magnuski
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
import cv2
import numpy as np
import Xlib
import Xlib.display

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
# def import_json_file(s, sequence, jsonfile, cube_uuid, ground_uuid)
# def import_json_object(s, sequence, jsonobject, cube_uuid, ground_uuid)
# def move_request(s, sequence, cube_uuid, spatial_angle, spatial_direction, spatial_direction_active, distance, velocity, gaze)
# def status_request(s, sequence, cube_uuid)
# def cube_view_request(s, sequence, cube_uuid, spatial_angle, gaze)
# def ground_view_request(s, sequence, cube_uuid, groundview)
# def snapshot(filename, image)
# def nwmessage_debug(debug)

training = "../training"
cubes = "cubes"
trainers = "trainers"
groundviews = "groundviews"
ground_uuid = "9787fbe0-68f9-40e7-81b7-904d36e7a848"
cube_assets = training + "/" + cubes
trainer_assets = training + "/" + trainers
trainer_assets_png = training + "/" + trainers + "-png"
trainer_assets_jpg = training + "/" + trainers + "-jpg"
groundview_assets = training + "/" + groundviews
bounding_box_txt_file = training + "/" + "training-bounding-box.txt"
bounding_box_tmp_file = training + "/" + "training-bounding-box-tmp.txt"

# views to be processed for training
trainers_list = ["females", "males", "enbies", "predators", "resources"]

gv_list = ["nwgview-low-near.json", "nwgview-medium-near.json", "nwgview-high-near.json", "nwgview-medium-distant.json", "nwgview-high-distant.json", "nwgview-medium-remote.json", "nwgview-medium-periphery.json"]
camera_positions_index = 0
camera_positions = [[0.0, 0.5, 4.0], [0.0, 1.0, 4.0], [0.0, 3.0, 4.0], [0.0, 1.0, 8.0], [0.0, 3.0, 8.0], [0.0, 1.0, 12.0], [0.0, 1.0, 16.0], [0.0, 1.0, 24.0], [0.0, 1.0, 32.0], [0.0, 1.0, 40.0]]
camera_positions_description = ['ln', 'mn', 'hn', 'md', 'hd', 'mr', 'mp', 'ml', 'me', 'mz']

debug = False
restart = False
restart_emoticon = ""
filename = ""
address = "127.0.0.1"
port = 2020

username = "hankm"
password = "123456"
cube_uuid = ""
groundview = -1
image_snapshot = False
jsonfile = ""

window_name = "NW Trainer"
window_width = 1280
window_height = 720
window_channels = 4
window_image = window_width * window_height * window_channels

# image we are retrieving from the server
image = []
training_resolution = 512.0
    
def Usage():
    print("Usage: nwtrain.py -a addr -c cube_uuid -d -f filename -h -i -m -p password -r emoticon -s -u username -v --address addressess --cube cube_uuid --debug --file=filename --help --image --move --pswd password --restart=emoticon --status --user username --version")

# Setup a new camera view
def camera_view(i, s, sequence):

    global camera_positions_index, camera_positions, camera_positions_description
    
    cp = camera_positions[i]
    cpd = camera_positions_description[i]
    camera_target = [0.0, 1.0, 0.0]
    camera_up = [0.0, 1.0, 0.0]
    camera = {"camera": {"camera_position": cp, "camera_target": camera_target, "camera_up": camera_up}}
    response = import_json_object(s, sequence, json.dumps(camera), "", "")
    check_error("import_json_object", response)
    time.sleep(0.5)
    
# Retrieve json training files

def training_jsons(start):

    # dirpath is a string for the path to the directory.
    # dirnames is a list of the names of the subdirectories in dirpath (excluding '.' and '..').
    # filenames is a list of the names of the non-directory files in dirpath.
    # Note that the names in the lists contain no path components.
    # To get a full path (which begins with top) to a file or directory in dirpath, do os.path.join(dirpath, name).

    for dirpath, dirnames, filenames in os.walk(start):
        print("nwtrain.py: json file directory %s" % dirpath)
        if debug:
            print(filenames)
            print(dirnames)
        return filenames

# Check for communications error
def check_error(msg, response):
    if response["message_type"] == "Error":
        print("nwtrain.py: %s - %s" % (msg, response["error"]))
        sys.exit(-1)

#
# Main program starts here
#

def main():
    
    global debug
    global restart
    global displayWidth, displayHeight
    global window_name
    global camera_positions_index, camera_positions_description

    # Store the bounding box for each created image
    bounding_box_list = []
    
    # Find display size from X
    display = Xlib.display.Display(':0')
    root = display.screen().root
    displays = display.screen_count()
    displayWidth = root.get_geometry().width
    displayHeight = root.get_geometry().height

    if debug :
        print('Display: width = %d, height = %d' % (displayWidth, displayHeight))

    # Check if the output file already exists
    if os.path.exists(bounding_box_txt_file):
        print("nwtrain.py: Output file %s already exists. Delete it if you wish to overwrite." % bounding_box_txt_file)
        sys.exit(-1)

    # Check our directories
    if not restart:
        for category in trainers_list:
            td = trainer_assets_png + "/" + category
            if not os.path.exists(td):
                os.mkdir(td)
            os.system("rm -f %s/*.png" % td)
            td = trainer_assets_jpg + "/" + category
            if not os.path.exists(td):
                os.mkdir(td)
            os.system("rm -f %s/*.jpg" % td)

    # Get the json training files
    json_trainers = training_jsons(cube_assets)
    json_trainers.sort()
    if debug:
        print(json_trainers)
          
    # Create an inet, streaming socket with blocking
    s = create_socket(address, port, True)

    sequence = 100

    bbtmp = open(bounding_box_tmp_file, 'w')

    # Process all the cubes in the system
    for trainee in json_trainers:
        if restart:
            if restart_emoticon in trainee:
                restart = False
                #continue
            else:
                continue

        # Load the next cube
        training_file = cube_assets + '/' + trainee
        with open(training_file, 'r') as jfile:
            training_json = jfile.read()
        j = json.loads(training_json)
        c = j["cubes"]
        json_cube_player = c[0]['cube_player']
        json_cube_uuid = c[0]['cube_uuid']

        # Send the next cube to the playing field
        sequence += 1
        response = import_json_file(s, sequence, training_file, json_cube_uuid, "")
        check_error("import_json_file", response)
        time.sleep(0.5)

        for camera_positions_index in range(len(camera_positions)):
            sequence += 1
            time.sleep(0.5)
            camera_view(camera_positions_index, s, sequence)
            time.sleep(0.5)

            # Do a ground view login
            sequence += 1
            response = login_request(s, sequence, username, password, "", ground_uuid)
            if response["message_type"] == "GoodBye" or check_error("login_request", response):
                print("nwtrain.py: groundview login failed")
                sys.exit(-1)

            # Login for this cube
            sequence += 1
            response = login_request(s, sequence, username, password, json_cube_uuid, "")
            check_error("login", response)

            for a in range(8):
                angle = (math.pi/4.0) * (a + (-0.5 + random.random()))
                if angle < 0.0:
                    angle += 2.0 * math.pi
                spatial_direction = 0.0
                spatial_direction_active = False
                distance = 0.0
                velocity = 0.0
                gaze = [0.0, 0.0]
                sequence += 1
                time.sleep(0.5)
                response = move_request(s, sequence, json_cube_uuid, angle, spatial_direction, spatial_direction_active, distance, velocity, gaze)
                check_error("move_request", response)
                if debug:
                    print("nwtrain.py: Move - sequence %d response %s" % (sequence, response))
                time.sleep(1.0)
                groundview = 0
                image_length = 0
                while image_length == 0:
                    sequence += 1
                    response = ground_view_request(s, sequence, ground_uuid, groundview)
                    check_error("ground_view_request", response)
                    image_length = response["image_length"]
                    # print("nwtrain.py: image_length %d" % image_length)

                # Get window dimensions
                ww = response["width"]
                wh = response["height"]
                ch = response["channels"]
                wx0 = ww/2
                wy0 = wh/2
                
                # Write a cropped image
                wc = training_resolution
                hc = training_resolution
                xc = wx0 - wc/2
                yc = wy0 - hc/2
                wci = int(wc)
                hci = int(hc)
                xci = int(xc)
                yci = int(yc)

                # Get the bounding boxes
                xmin = response["bounding_box"][0]
                ymin = response["bounding_box"][1]
                xmax = response["bounding_box"][2]
                ymax = response["bounding_box"][3]

                x1 = xmin-xc
                y1 = ymin-yc
                x2 = xmax-xc
                y2 = ymax-yc

                # These are the 8 corners of the projection, needed for debugging
                corners = []
                for corner in range(2, 10):
                    cx = corner * 2
                    cy = cx + 1
                    corners.append(int(response["bounding_box"][cx]-xc))
                    corners.append(int(response["bounding_box"][cy]-yc))

                if debug:
                    print("nwtrain.py: ", xc, yc, ch, xmin, ymin, xmax, ymax, x1, y1, x2, y2)
                
                vid = camera_positions_description[camera_positions_index]
                
                if json_cube_player == "enby":
                    json_cube_players = "enbies"
                else:
                    json_cube_players = json_cube_player + "s"

                trainee_file = trainer_assets_png + "/" + json_cube_players + "/" + trainee[:-5] + "-" + vid + "-%1.4f" % angle + "-%sx%s.png" % (wci, hci)
                snapshot(trainee_file, response["image"][yci:yci+hci, xci:xci+wci])
                if debug:
                    print("nwtrain.py: Created training image %s" % trainee_file)
                trainee_file = trainer_assets_jpg + "/" + json_cube_players + "/" + trainee[:-5] + "-" + vid + "-%1.4f" % angle + "-%sx%s.jpg" % (wci, hci)
                snapshot(trainee_file, response["image"][yci:yci+hci, xci:xci+wci])
                if debug:
                    print("nwtrain.py: Created training image %s" % trainee_file)
                trainee_file_base = trainee[:-5] + "-" + vid + "-%1.4f" % angle + "-%sx%s" % (wci, hci)
                trainee_dictionary = '{"trainee_file": "%s", "bounding_box": [%0.3f, %0.3f, %0.3f, %0.3f], "corners": %s}' % (trainee_file_base, x1, y1, x2, y2, corners)
                bounding_box_list.append(trainee_dictionary)
                bbtmp.write(trainee_dictionary + "\n")
                bbtmp.flush()
                print("nwtrain.py: %s" % trainee_dictionary)
                # break

            sequence += 1
            logout_request(s, sequence, "", ground_uuid)
            # break

        # break
        
    shutdown_socket(s)

    bbtmp.close()

    # Write out the bounding box list for each image

    with open(bounding_box_txt_file, 'w') as bbfile:
        for box in bounding_box_list:
            bbfile.write(str(box)+"\n")
    
    print("nwtrain.py: All training images created.")
    
    
if __name__=='__main__':

    #
    # Get options and call the main program
    #                                                                                            

    try:
        options, args = getopt.getopt(sys.argv[1:], 'a:c:df:g:hij:mp:r:su:v', ['address=','cube=','debug','file=','ground=', 'help', 'image', 'json=', 'move', 'pswd=', 'restart=', 'status', 'user=','version'])
    except getopt.GetoptError:
        Usage()
        sys.exit(2)

    for o, a in options:
        if o in ("-a", "--address"):
            address = a
        if o in ("-c", "--cube"):
            cube_uuid = a
        if o in ("-d", "--debug"):
            debug = True
        if o in ("-f", "--file"):
            filename = a
        if o in ("-g", "--ground"):
            groundview = int(a)
        if o in ("-h", "--help"):
            Usage()
            sys.exit()
        if o in ("-i", "--image"):
            image_snapshot = True
        if o in ("-j", "--json"):
            jsonfile = a
        if o in ("-m", "--move"):
            MoveRequested = True
        if o in ("-p", "--pswd"):
            password = a
        if o in ("-r", "--restart"):
            restart = True
            restart_emoticon = a
        if o in ("-s", "--status"):
            StatusRequested = True
        if o in ("-u", "--user"):
            username = a
        if o in ("-v", "--version"):
            print("nwmsg.py Version 1.0")
            sys.exit()
        
    main()
    sys.exit()


