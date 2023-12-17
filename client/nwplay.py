#! /usr/bin/python3
"""
nwplay.py -- Neckerworld Play prototype

Sample usage:

 nwplay.py

Complete specification:

 nwplay.py -a address -c cube_uuid -d -f filename -g -h -i -k -m -p password -r -s -u username -v --address address --cube=cube_uuid --debug --file=filename --game --help --image --kill --mate --pswd=password --resource --sound --user=username --version

 where

 -a, --address        Address of remote host
 -c, --cube           Cube UUID
 -d, --debug          Turn debug statements on
 -f, --file           Input filename
 -g, --game           Game play
 -h, --help           Print usage information
 -i, --image          Image snapshot
 -k, --kill           Find a predator and kill it
 -m, --mate           Find a mate
 -s, --status         Status requested
 -p, --pswd           Password
 -r, --resource       Find a resource
 -s, --sound          Enable sound
 -u, --user           Username
 -v, --version        Report program version

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
import socket
import base64
import zlib
import cv2
import numpy as np
import matplotlib
#from matplotlib import pyplot as plt
import Xlib
import Xlib.display
import tkinter
from tkinter import *
from tkinter import ttk
from PIL import Image, ImageTk

# This will be conditionally imported
# from playsound import playsound

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
# def logout_request(s, sequence, cube_uuid, ground_uuid)
# def import_json_file(s, sequence, jsonfile, cube_uuid)
# def move_request(s, sequence, cube_uuid, spatial_angle, spatial_direction, spatial_direction_active, distance, velocity, gaze)
# def status_request(s, sequence, cube_uuid)
# def cube_view_request(s, sequence, cube_uuid, spatial_angle, gaze)
# def ground_view_request(s, sequence, cube_uuid, groundview)
# def snapshot(filename, image)
# def nwmessage_debug(debug)

nweffdet_active = False
gvision_active = False
nwvision_active = True

if nweffdet_active:
    from nweffdet import predict
if gvision_active:
    from gvision import predict
if nwvision_active:
    from nwvision import predict

debug = False
matrix_test = False
filename = ""
address = "127.0.0.1"
port = 2020

username = "example.user"
password = "pw"
groundview = -1
image_snapshot = False
jsonfile = ""

displayNumber = ":0"
displayWidth = 0
displayHeight = 0
window_name = "Neckerworld Cube View - Game Action - "
window_width = 512
window_height = 512
window_channels = 4
window_image_length = window_width * window_height * window_channels

# ground scale factor (1/2 width of the playing field)
ground_scale_factor = 10.0

# imagea we are retrieving from the server
image = []

# probability density function weighting

pdf_filter = False
u_mean = 0.0
std_dev = 0.6
y_max = 0.665

# Sound effects
sound = False
sounds = "../sounds"

# Options
FindPredator = False
FindResource = False
FindMate = False
GameOn = False
vrloop = False
sequence = 0

# This is a small delay to insure we have the correct image.
# It's a temporary fixup
scan_delay = 2

# Window variables

# Cube state variables from status call
# These are updated from the playing field server
cube_player = ""
cube_uuid = '1354b75f-9ca5-4da5-80b2-8f0056f9e08e'
cube_firstname = ""
cube_active = True
spatial_angle = 0.0
spatial_direction = 0.0
spatial_direction_active = False
spatial_distance = 0.0
spatial_velocity = 0.0
spatial_position_blocked = False
resource_energy = 0.0
total_points = 0.0
total_mates = 0.0
total_resources = 0.0
total_predators = 0.0

# Runtime status messages
status_message = ""

# Cube state variables needed for stategy execution
# These are set and controller locally
cube_state = "Idle"
current_location = [0.0, 0.0, 0.0]
starting_location = [0.0, 0.0, 0.0]
scan_state_control = 0
spatial_angle_control = 0.0
spatial_distance_control = 0.0
spatial_direction_control = 0.0
spatial_direction_active_control = False
gaze_control = [0.0, 0.0]

# List of all objects detected in scanning tasks
player_field_list = []
target_class = ""

# Field state

player_colors = {"female": "white", "male": "blue", "enby": "purple", "predator": "red", "resource": "green"}
class_colors = {"females": "white", "males": "blue", "enbies": "purple", "predators": "red", "resources": "green"}
bounding_box__colors = {"females": "white", "males": "blue", "enbies": "purple", "predators": "red", "resources": "green"}

def Usage():
    print("Usage: nwplay.py -a addr -c cube_uuid -d -f filename -g -h -i -m -p password -s -u username -v --address=address --cube=cube_uuid --debug --file=filename --game --help --image --move --pswd=password --sound --user=username --version")

# probability density function

def pdf(x, u, s):

    ss2 = 2.0 * s**2
    y = math.exp(-(x-u)**2/(ss2))/(math.sqrt(ss2 * math.pi))
    return y

# Create our display window
def display_window(cube_uuid):

    # https://stackoverflow.com/questions/459083/how-do-you-run-your-own-code-alongside-tkinters-event-loop
    # http://www.nmt.edu/tcc/help/pubs/tkinter/
    
    global visual
    global field
    global multi_use_field
    global cube_firstname_field
    global spatial_angle_field
    global spatial_direction_field
    global spatial_velocity_field
    global spatial_position_blocked_field
    global resource_energy_field
    global total_points_field
    global vrloops_per_second_field
    global cube_state_field
    global btn_mate, btn_predator, btn_resource, btn_game
    global multi_use_field
    global status_message
    
    window_title = window_name + cube_uuid
    root = tkinter.Tk()
    root.title(window_title)
    style = ttk.Style(root)

    style.map("C.TButton",
    foreground=[('pressed', 'red'), ('active', 'blue')],
    background=[('pressed', '!disabled', 'black'), ('active', 'white')]
    )

    mainframe = ttk.Frame(root, padding="3 3 3 3")
    mainframe.grid(column=0, row=0, sticky=(N, W, E, S))
    root.columnconfigure(0, weight=1)
    root.rowconfigure(0, weight=1)
    resource_energy_field = StringVar()
    ttk.Label(mainframe, width=14, padding="5 5 5 5", foreground='#080', textvariable=resource_energy_field).grid(column=0, row=2, sticky=(W, E))
    total_points_field = StringVar()
    ttk.Label(mainframe, width=14, padding="5 5 5 5", foreground='#008', textvariable=total_points_field).grid(column=1, row=2, sticky=(W, E))
    spatial_angle_field = StringVar()
    ttk.Label(mainframe, width=14, padding="5 5 5 5", foreground='#008', textvariable=spatial_angle_field).grid(column=2, row=2, sticky=(W, E))
    spatial_direction_field = StringVar()
    ttk.Label(mainframe, width=14, padding="5 5 5 5", foreground='#008', textvariable=spatial_direction_field).grid(column=3, row=2, sticky=(W, E))
    spatial_velocity_field = StringVar()
    ttk.Label(mainframe, width=14, padding="5 5 5 5", foreground='#008', textvariable=spatial_velocity_field).grid(column=4, row=2, sticky=(W, E))
    vrloops_per_second_field = StringVar()
    ttk.Label(mainframe, width=14, padding="5 5 5 5", foreground='#008', textvariable=vrloops_per_second_field).grid(column=5, row=2, sticky=(W, E))
    spatial_position_blocked_field = StringVar()
    ttk.Label(mainframe, width=14, padding="5 5 5 5", foreground='#800', textvariable=spatial_position_blocked_field).grid(column=6, row=2, sticky=(W, E))
    cube_state_field = StringVar()
    ttk.Label(mainframe, width=14, padding="5 5 5 5", foreground='#008', textvariable=cube_state_field).grid(column=7, row=2, sticky=(W, E))
    cube_firstname_field = StringVar()
    ttk.Label(mainframe, width=14, padding="5 5 5 5", foreground='#000', textvariable=cube_firstname_field).grid(column=4, row=4, sticky=(W, E))
    btn_mate = ttk.Button(mainframe, text="Mate", command=find_mate, style="C.TButton")
    btn_mate.grid(column=0, row=4, sticky=W)
    btn_predator = ttk.Button(mainframe, text="Predator", command=find_predator, style="C.TButton")
    btn_predator.grid(column=1, row=4, sticky=W)
    btn_resource = ttk.Button(mainframe, text="Resource", command=find_resource, style="C.TButton")
    btn_resource.grid(column=2, row=4, sticky=W)
    btn_game = ttk.Button(mainframe, text="Game", command=game_on, style="C.TButton")
    btn_game.grid(column=3, row=4, sticky=W)
    multi_use_field = StringVar()
    option_entry = ttk.Entry(mainframe, width=35, textvariable=multi_use_field)
    option_entry.grid(column=5, row=4, columnspan=2, sticky=W)
    option_entry.focus()
    ttk.Button(mainframe, text="Quit", command=quit_play, style="C.TButton").grid(column=7, row=4, sticky=E)

    visual = tkinter.Canvas(mainframe, width=window_width, height=window_height, bg='#000')
    visual.grid(column=0, columnspan=4, row=0, sticky=(W, E))
    field = tkinter.Canvas(mainframe, width=window_width, height=window_height, bg='#060')
    field.grid(column=4, columnspan=4, row=0, sticky=(W, E))
    
    # Assign keys to this window
    root.bind("P", trigger_snapshot)
    root.bind("p", trigger_snapshot)
    root.bind("<Escape>", quit_play)

    # Window handlers
    root.protocol("WM_DELETE_WINDOW", display_window_closing)

    return root

def display_window_closing():

    global vrloop
    
    #if messagebox.askokcancel("Quit", "Do you want to quit?"):
    #    vrloop = False
    vrloop = False
    
# Action routines from button pushes
def find_mate(*args):
    global FindMate
    FindMate = not FindMate
    
def find_predator(*args):
    global FindPredator
    FindPredator = not FindPredator

def find_resource(*args):
    global FindResource
    FindResource = not FindResource

def game_on(*args):
    global GameOn
    GameOn = not GameOn

def update_panel():
    btn_mate.config(text = "Mate %s" % FindMate)
    btn_predator.config(text = "Predator %s" % FindPredator)
    btn_resource.config(text = "Resource %s" % FindResource)
    btn_game.config(text = "Game %s" % ("On" if GameOn else "Off"))
    cube_state_field.set("%s" % cube_state)
    multi_use_field.set(status_message)
    
# Create our detected bounding boxes
def update_bounding_boxes(plist):
    visual.delete("boxes")
    for p in plist:
        classname = p["classname"]
        score = p["score"]
        pcolor = class_colors[classname]
        boxes = p["bounding_box"]
        id = visual.create_rectangle(boxes[0] * window_width, boxes[1] * window_height, boxes[2] * window_width, boxes[3] * window_height, width = 4, outline = pcolor, tags = "boxes")
    return


# Update our field plot
def update_field(angle, state):

    global ground_scale_factor
    global field
    
    scale = 2.0 * ground_scale_factor
    xw = float(window_width)
    yw = float(window_height)
    xwc = xw/2.0
    ywc = yw/2.0
    
    # Erase previous icons
    field.delete("all")

    # World coordinates: +x goes left, +z is up
    # Field coordinates: +x goes right, +y goes down
    
    for p in player_field_list:
        classname = p["classname"]
        if classname == "removed":
            continue
        score = p["score"]
        pcolor = class_colors[classname]
        csf = p["scale_factor"]
        target_location = p["target_location"]
        pxc = int(-xwc * (target_location[0]/scale) + xwc) 
        pyc = int(-ywc * (target_location[2]/scale) + ywc)
        dx = int(xwc * csf/scale)
        dy = int(ywc * csf/scale)
        player = field.create_rectangle(pxc-dx, pyc-dy, pxc+dx, pyc+dy, fill=pcolor, outline='yellow', width=1)
        
    player_x = int(-xwc * (current_location[0]/scale) + xwc)
    player_y = int(-ywc * (current_location[2]/scale) + ywc)
    dx = int(xwc * cube_scale_factor/scale)
    dy = int(ywc * cube_scale_factor/scale)
    r = math.sqrt(dx*dx + dy*dy)
    pi = math.pi
    x0, y0 =  box_rotate(r, angle, pi/4.0)
    x1, y1 =  box_rotate(r, angle, 3.0*pi/4.0)
    x2, y2 =  box_rotate(r, angle, 5.0*pi/4.0)
    x3, y3 =  box_rotate(r, angle, 7.0*pi/4.0)
    rv0 = dx * 1.05
    rv1 = dx * 2.5
    vlx0, vly0 = box_rotate(rv0, angle, 0.0)
    vlx1, vly1 = box_rotate(rv1, angle, pi/12.0)
    vrx0, vry0 = box_rotate(rv0, angle, 0.0)
    vrx1, vry1 = box_rotate(rv1, angle, -pi/12.0)
    
    player = field.create_polygon(player_x+x0, player_y+y0, player_x+x1, player_y+y1, player_x+x2, player_y+y2, player_x+x3, player_y+y3, fill=player_colors[cube_player], outline='white', width=1)
    if state == "Scanning":
        player_view = field.create_polygon(player_x+vlx0, player_y+vly0, player_x+vlx1, player_y+vly1, player_x+vrx1, player_y+vry1, fill='light grey', outline='white', width=1)
    #view_left = field.create_line(player_x+vlx0, player_y+vly0, player_x+vlx1, player_y+vly1, fill='white', width=1)
    #view_right = field.create_line(player_x+vrx0, player_y+vry0, player_x+vrx1, player_y+vry1, fill='white', width=1)
    
    return

def box_rotate(r, angle, offset):
    dx = -r * math.sin(angle + offset)
    dy = -r * math.cos(angle + offset)
    return dx, dy

def trigger_snapshot(*args):
    global image_snapshot
    image_snapshot = True

def quit_play(*args):
    global vrloop
    vrloop = False

# Check for communications error
def check_error(msg, response):
    if response["message_type"] == "Error":
        print("nwplay.py: %s - Error - %s" % (msg, response["error"]))
        return True
    return False

# Move to the next position
def cube_move(s, sequence, cube_uuid, spatial_angle, spatial_direction, spatial_direction_active, distance):

    velocity = 1.0
    gaze = [0.0, 0.0]

    response = move_request(s, sequence, cube_uuid, spatial_angle, spatial_direction, spatial_direction_active, distance, velocity, gaze)
    return response

# Get the current status
def cube_status(s, sequence, cube_uuid):

    global cube_player
    global cube_firstname
    global cube_active
    global cube_scale_factor
    global spatial_angle
    global spatial_direction, spatial_direction_active
    global spatial_distance, spatial_velocity
    global spatial_gaze
    global spatial_position_blocked
    global resource_energy
    global total_points
    
    response = status_request(s, sequence, cube_uuid)
    if check_error("cube_status", response):
        return response

    sequence_status = response["sequence"]
    timestamp = response["timestamp"]
    frame = response["frame"]
    cube_player = response["cube_player"]
    cube_uuid_status = response["cube_uuid"]
    cube_firstname =  response["cube_firstname"]
    cube_active = response["cube_active"]
    cube_display = response["cube_display"]
    cube_remote = response["cube_remote"]
    cube_scale_factor = response["cube_scale_factor"]
    spatial_angle = response["spatial_angle"]
    spatial_direction = response["spatial_direction"]
    spatial_direction_active = response["spatial_direction_active"]
    spatial_gaze = response["spatial_gaze"]
    spatial_radius = response["spatial_radius"]
    spatial_distance = response["spatial_distance"]
    spatial_velocity = response["spatial_velocity"]
    spatial_position_blocked = response["spatial_position_blocked"]
    life_birth = response["life_birth"]
    life_death = response["life_death"]
    life_father = response["life_father"]
    life_mother = response["life_mother"]
    resource_energy = response["resource_energy"]
    total_points = response["total_points"]

    resource_energy_field.set("Energy = %0.2f" % resource_energy)
    total_points_field.set("Points = %0.2f" % total_points[0])
    spatial_angle_field.set("Angle = %0.2f" % (math.degrees(spatial_angle)))
    spatial_direction_field.set("Direction = %0.2f" % (math.degrees(spatial_direction)))
    spatial_velocity_field.set("Velocity = %0.2f" % spatial_velocity)
    if spatial_position_blocked:
        spatial_position_blocked_field.set("Blocked")
    else:
        spatial_position_blocked_field.set("")
    cube_firstname_field.set(cube_firstname)

    return response

# Game strategy routines for the players

def execute_strategy(state):

    global s
    global sequence
    global scan_state_control, scan_delay
    global spatial_angle_control
    global spatial_distance_control
    global spatial_direction_control, spatial_direction_active_control
    global current_location
    global starting_location
    global view_response
    global player_field_list
    global FindMate, FindPredator, FindResource, GameOn
    global target_class
    global total_points, total_mates, total_resources, total_predators, resource_energy
    global status_message
    
    if state == "Idle":
        if spatial_position_blocked:
            spatial_distance_control = 1.0
            spatial_direction_control = spatial_angle_control-math.pi
            spatial_direction_active_control = True
            starting_location = current_location[:]
            sequence += 1
            status_message = "Unblock distance %0.2f, angle %0.2f" % (spatial_distance_control, spatial_direction_control)
            print("nwplay.py: unblock distance %0.2f, angle %0.2f" % (spatial_distance_control, spatial_direction_control))
            move_response = cube_move(s, sequence, cube_uuid, spatial_angle_control, spatial_direction_control, spatial_direction_active_control, spatial_distance_control)
            return "Moving"
        if resource_energy < 50.0:
                FindResource = True
        if resource_energy > 1000.0:
                FindResource = False
        if cube_player == "male" or cube_player == "female" or cube_player == "enby":
            if FindMate or FindPredator or FindResource or GameOn:
                scan_state_control = 0
                # This is a hack, needs fixing.
                scan_delay = 2
                spatial_angle_control = 0.0
                return "Scanning"
            return state
        return state
        
    if state == "Scanning":
        # This is a hack, fix it
        if scan_delay > 0:
            scan_delay -= 1
            return "Scanning"
        status_message = "Current state %s" % state
        print("nwplay.py: Current state %s" % state)
        p_filename = "p_files/prediction-%05d.jpg" % sequence
        snapshot(p_filename, view_response["image"])
        if nweffdet_active or gvision_active or nwvision_active:
            predictions = predict(p_filename)
        else:
            predictions = predict_test(p_filename)
        plist = analyze_scene(predictions)
        update_bounding_boxes(plist)
        for p in plist:
            player_field_list.append(p)
        scan_state_control = (scan_state_control + 1) % 8
        spatial_angle_control -= math.pi/4.0
        # This is a hack. Needs to be fixed
        scan_delay = 2
        if scan_state_control == 0:
            return "ScanDone"
        return "Scanning"
    
    if state == "ScanDone":
        status_message = "Current state %s" % state
        print("nwplay.py: Current state %s" % state)

        player_field_list_merge()
        target_class = ""

        if FindPredator:
            target_class = "predators"
        if FindResource:
            target_class = "resources"
        if cube_player == "male"  and FindMate:
            target_class = "females"
        if cube_player == "female" and FindMate:
            target_class = "males"

        if GameOn and target_class == "":
            for p in player_field_list:
                if target_class == "" and p["classname"] == "males" and cube_player == "female":
                    target_class = "males"
                if target_class == "" and p["classname"] == "females" and cube_player == "male":
                    target_class = "females"
                if p["classname"] == "resources" and resource_energy < 1000.0:
                    target_class = "resources"
                if p["classname"] == "predators":
                    target_class = "predators"
                    break
                
        # Find the nearest target
        return move_to_target(target_class)

    if state == "Moving":

        update_location()

        if debug:
            print("nwplay.py: Current state %s, spatial_distance = %0.2f sdc %0.2f x %0.2f z %0.2f (%0.2f, %0.2f)"
              % (state, spatial_distance, spatial_distance_control, current_location[0], current_location[2], starting_location[0], starting_location[2]))

        # Update states and cancel targets if reached. Use total points for the class as an indicator of objectives met.
        if (FindMate or target_class == "males" or target_class == "females") and total_points[1] > total_mates:
            total_mates = total_points[1]
            FindMate = False
            target_class = ""
        
        if FindResource and resource_energy > 1000.0:
            FindResource = False
            target_class = ""

        if GameOn and target_class == "resources" and resource_energy > 1000.0:
            target_class = ""

        if total_points[2] > total_resources:
            total_resources = total_points[2]

        # How to know we've taken out a predator? Let's look at the points
        if total_points[3] > total_predators:
            total_predators = total_points[3]
            near = get_nearest(current_location, "predators")
            if near >= 0:
                print("nwplay.py: total_points removing %s" % player_field_list[near])
                player_field_list[near]["classname"] = "removed"

        if spatial_position_blocked:
            spatial_direction_active_control = False
            return "Idle"

        if spatial_velocity == 0.0:
            spatial_direction_active_control = False
            p_filename = "p_files/prediction-%05d.jpg" % sequence
            snapshot(p_filename, view_response["image"])
            if nweffdet_active or gvision_active or nwvision_active:
                predictions = predict(p_filename)
            else:
                predictions = predict_test_one(p_filename)
            plist = analyze_scene(predictions)
            update_bounding_boxes(plist)
            for p in plist:
                player_field_list.append(p)
            player_field_list_merge()

            return move_to_target(target_class)

        return state

    if state == "NoTargets":
        status_message = "Current state %s" % state
        print("nwplay.py: Current state %s" % state)
        FindMate = False
        FindPredator = False
        FindResource = False
        return "Idle"

    print("Unknown state %s" % state)
    return "State undefined"

# Move on target
def move_to_target(tclass):

    global sequence
    global spatial_angle_control, spatial_distance_control
    global spatial_direction_control, spatial_direction_active_control
    global current_location
    global starting_location
    global status_message
    
    current_target = get_nearest(current_location, tclass)

    if current_target >= 0:
        classname = player_field_list[current_target]["classname"]
        score = player_field_list[current_target]["score"]
        distance, angle = player_distance(current_location, player_field_list[current_target]["target_location"])
        if distance < 0.1:
            return "NoTargets"
        if score > 0.60 or score > 0.05:
            spatial_angle_control = angle
            spatial_distance_control = distance
            spatial_direction_control = spatial_angle_control
            spatial_direction_active_control = False
            starting_location = current_location[:]
            sequence += 1
            status_message = "Move to target %s distance %0.2f" % (classname, distance)
            print("nwplay.py: move_to_target class %s distance %0.2f, angle %0.2f" % (classname, distance, angle))
            move_response = cube_move(s, sequence, cube_uuid, spatial_angle_control, spatial_direction_control, spatial_direction_active_control, distance)
            return "Moving"

    return "NoTargets"
    
# Calculate player distance and angle
def player_distance(me, target):

    dx = target[0] - me[0]
    dz = target[2] - me[2]

    distance = math.sqrt(dx*dx + dz*dz)

    angle = math.atan2(dx, dz)

    return distance, angle
    
# Get nearest player
def get_nearest(location, player):

    d = 99999.0
    nearest = -1

    for i in range(len(player_field_list)):
        if player_field_list[i]["classname"] != player:
            continue
        dx, a  =  player_distance(location, player_field_list[i]["target_location"])
        if dx < d:
            d = dx
            nearest = i
    
    return nearest

# Merge possible duplicate entries in the player field list
def player_field_list_merge():

    global player_field_list
    
    player_list_new = []

    p_index = 0
    pfl_len = len(player_field_list)
    for p in player_field_list:
        classname = p["classname"]
        if classname == "removed":
            p_index += 1
            continue
        score = p["score"]
        location = p["target_location"]
        scale_factor = p["scale_factor"]
        box = p["bounding_box"]
        if p_index + 1 < pfl_len:
            for i in range(p_index+1, pfl_len):
                cn = player_field_list[i]["classname"]
                if cn != classname:
                    continue
                sc = player_field_list[i]["score"]
                tl = player_field_list[i]["target_location"]
                sf = player_field_list[i]["scale_factor"]
                distance, angle = player_distance(location, tl)
                if distance < cube_scale_factor+sf:
                    location[0] = (location[0]+tl[0])/2.0
                    location[2] = (location[2]+tl[2])/2.0
                    score = (score+sc)/2.0
                    scale_factor = (scale_factor+sf)/2.0
                    player_field_list[i]["classname"] = "removed"

        np = {"classname": classname, "score": score, "bounding_box": box, "scale_factor": scale_factor, "target_location": location}
        player_list_new.append(np)
        p_index += 1

    player_field_list = player_list_new[:]
                    
    return

# Update our current location
def update_location():

    global current_location
    global starting_location
    global spatial_angle_control, spatial_distance_control
    global spatial_direction_control, spatial_direction_active_control

    if spatial_direction_active_control:
        angle = spatial_direction_control
    else:
        angle = spatial_angle_control
    
    current_location[0] = starting_location[0] + (spatial_distance_control-spatial_distance) * math.sin(angle)
    current_location[2] = starting_location[2] + (spatial_distance_control-spatial_distance) * math.cos(angle)

# Analyze the predictions and bounding boxes returned from the object detection process
def analyze_scene(predictions):

    global matrix_test
    
    mpp = perspective_projection_matrix(60.0, window_width/window_height, 0.1, 100.0)
    # mi = perspective_projection_inverse(mpp)
    a00 = mpp[0][0]
    a11 = mpp[1][1]
    csf = cube_scale_factor

    if matrix_test:
        m_test(mpp, mi)
        matrix_test = False
        
    plist = predictions["predictions"]

    player_list = []
    
    for p in plist:
        classname = p["classname"]
        score = p["score"]
        bbox = p["bounding_vertices"]

        bbdx = bbox[2]-bbox[0]
        bbdy = bbox[3]-bbox[1]
        if bbdx > 0.95 or bbdy > 0.95:
            if debug:
                print("nwplay.py: analyze_scene Discarding suspect player %s bounding box limits exceeded (%0.3f, %0.3f)" % (classname, bbdx, bbdy))
            continue

        # Window coordinates - (x0, y0) upper left corner, (512, 512) lower right corner
        w = float(window_width)
        h = float(window_height)
        xmin = w * bbox[0]
        ymin = h * bbox[1]
        xmax = w * bbox[2]
        ymax = h * bbox[3]
        xc = (xmin + xmax)/2.0
        xw = xmax-xmin

        # Bounding box coordinates - (x0, y0) upper left corner, (1.0, 1.0) lower right corner
        xcbb = (bbox[2]+bbox[0])/2.0
        ymbb = bbox[3]

        # Perspective transform coordinates - (x0, y0) at (0.0, 0,0), upper right (1.0, 1.0) and z right-hand coordinate system
        px = (xcbb - 0.5) * 2.0
        pxleft = (bbox[0] - 0.5) * 2.0
        pxright = (bbox[2] - 0.5) * 2.0
        py = (0.5 - ymbb) * 2.0
        # We assume the center-point of the lower bounding box line is where the cube touches the ground-plane.
        # Without this assumption we could not solve for distance from the image plane.
        z = a11 * csf / py
        x = -(z * px) / a00
        xleft = -(z * pxleft) / a00
        xright = -(z * pxright) / a00
        xwidth = xright - xleft
        if xwidth > 3.0 and score < 0.8:
            if debug:
                print("nwplay.py: analyze_scene Discarding suspect player %s detection width %0.2f score %0.2f" % (classname, xwidth, score))
            continue
        sf = min(xwidth/2.0, 1.2)
        sf = max(sf, 0.5)
        # Add the estimated scale factor to the computed distance to get a final estimate of the cube's position.  
        distance = -z + sf
        angle = math.atan(-x/distance)
        target_angle = spatial_angle + angle
        # Figure out the world coordinates of the target
        target_location_x = current_location[0] + distance * math.sin(target_angle)
        target_location_y = sf
        target_location_z = current_location[2] + distance * math.cos(target_angle)
        target_location = [target_location_x, target_location_y, target_location_z]
        
        if debug:
            print("nwplay.py: analyze_scene Box coordinate: xcbb %0.2f, ymbb %0.2f, csf %0.2f, px %0.2f, py %0.2f, x %0.2f, z %0.2f, a00 %0.2f, a11 %0.2f" % (xcbb, ymbb, csf, px, py, x, z, a00, a11))
            print("nwplay.py: analyze_scene View coordinate: x %0.2f, y %0.2f, z %0.2f, distance %0.2f angle %0.2f" % (x, 0.0, z, distance, angle))
            print("nwplay.py: analyze_scene %s xmin %0.2f ymin %0.2f xmax %0.2f  ymax %0.2f xc %0.2f xw %0.2f sf %0.2f distance %0.2f angle %0.2f spatial_angle %0.2f target_angle %0.2f tx %0.2f tz %0.2f" %
                  (classname, xmin, ymin, xmax, ymax, xc, xw, sf, distance, angle, spatial_angle, target_angle, target_location_x, target_location_z))

        player = {"classname": classname, "score": score, "bounding_box": bbox, "scale_factor": sf, "target_location": target_location}

        player_list.append(player)
        
    return player_list

def perspective_projection_matrix(fov, aspect, zNear, zFar):

    # from glm::perspective
    # tanHalfFovy = tan(rad / 2.0);
    # detail::tmat4x4<valType, defaultp> Result(0.0);
    # Result[0][0] = 1.0 / (aspect * tanHalfFovy);
    # Result[1][1] = 1.0 / tanHalfFovy;
    # Result[2][2] = - (zFar + zNear) / (zFar - zNear);
    # Result[2][3] = - 1.0;
    # Result[3][2] = - 2.0 * zFar * zNear) / (zFar - zNear);

    fovrad = math.radians(fov)
    tanHalfFovy = math.tan(fovrad / 2.0)
    r00 = 1.0 / (aspect * tanHalfFovy)
    r11 = 1.0 / tanHalfFovy
    r22 = - (zFar + zNear) / (zFar - zNear)
    r23 = -1.0
    r32 = - (2.0 * zFar * zNear) / (zFar - zNear)

    # Construct the perspective matrix, and switch to column major for numpy (swaps r23 x r32)
    mpp = np.array([[r00, 0.0, 0.0, 0.0], [0.0, r11, 0.0, 0.0], [0.0, 0.0, r22, r32], [0.0, 0.0, r23, 0.0]])
    
    # cube.cpp: update_projection
    # [1.7321, 0.0000,  0.0000,  0.0000]
    # [0.0000, 1.7321,  0.0000,  0.0000]
    # [0.0000, 0.0000, -1.0020, -1.0000]
    # [0.0000, 0.0000, -0.2002,  0.0000]
    
    # nwplay.py: perspective [ 1.732,  0.000,  0.000,  0.000]
    # nwplay.py: perspective [ 0.000,  1.732,  0.000,  0.000]
    # nwplay.py: perspective [ 0.000,  0.000, -1.002, -0.200]
    # nwplay.py: perspective [ 0.000,  0.000, -1.000,  0.000]

    if debug:
        print("nwplay.py: perspective [%6.3f, %6.3f, %6.3f, %6.3f]" % (mpp[0][0], mpp[0][1], mpp[0][2], mpp[0][3]))
        print("nwplay.py: perspective [%6.3f, %6.3f, %6.3f, %6.3f]" % (mpp[1][0], mpp[1][1], mpp[1][2], mpp[1][3]))
        print("nwplay.py: perspective [%6.3f, %6.3f, %6.3f, %6.3f]" % (mpp[2][0], mpp[2][1], mpp[2][2], mpp[2][3]))
        print("nwplay.py: perspective [%6.3f, %6.3f, %6.3f, %6.3f]" % (mpp[3][0], mpp[3][1], mpp[3][2], mpp[3][3]))
    
    return mpp

def perspective_projection_inverse(m):

    mi = np.linalg.inv(m) 
    return mi
    
def m_test(m, mi):

    pts =  [[0.58, -0.58,  -1.0, 1.0], [0.58, 0.58,  -1.0, 1.0], [-0.58, 0.58,  -1.0, 1.0], [-0.58, -0.58,  -1.0, 1.0],
            [1.0,  -1.0,  -10.0, 1.0], [ 1.0, 1.0,  -10.0, 1.0], [-1.0,  1.0,  -10.0, 1.0], [-1.0,  -1.0,  -10.0, 1.0],
            [1.0,  -1.0, -100.0, 1.0], [ 1.0, 1.0, -100.0, 1.0], [-1.0,  1.0, -100.0, 1.0], [-1.0,  -1.0, -100.0, 1.0]]

    for p in pts:
        uv = np.dot(m, p)
        w = uv[3]
        print("mtest p x %7.2f y %7.2f z %7.2f w %7.2f, uv.x %7.2f uv.y %7.2f uv.z %7.2f uv.w %7.2f, uvw.x %7.2f uvw.y %7.2f uvw.z %7.2f uvw.w %7.2f"
              % (p[0], p[1], p[2], p[3], uv[0], uv[1], uv[2], uv[3], uv[0]/w, uv[1]/w, uv[2]/w, uv[3]))
        # xy = np.dot(mi, uv)
        # print("mtest mi x %7.2f y %7.2f z %7.2f w %7.2f" % (xy[0], xy[1], xy[2], xy[3]))

# Get sound file for our current state
def get_sound(state, player, velocity, sdac, FR, FM, FP):

    global scan_delay
    
    if state == "Scanning" and scan_delay == 0:
        return sounds + "/chirp-440-1320-250.wav"
    if state == "Moving":
        if sdac:
            return sounds + "/440-125.wav"
        else:
            return sounds + "/1000-125.wav"
    return ""

# Main program starts here

def main():
    
    global debug
    global displayNumber, displayWidth, displayHeight
    global ground_scale_factor
    global s
    global sequence
    global view_response
    global image_snapshot
    global vrloop
    global FindMate, FindResource, FindPredator, GameOn
    global cube_state
    global scan_state_control
    global spatial_angle_control
    global gaze_control
    
    # Find display size from X
    displayNumber = os.getenv('DISPLAY', ':0')
    try:
        display = Xlib.display.Display(displayNumber)
    except Xlib.error.DisplayConnectionError as e:
        print("nwplay.py: Xlib.display connection error - %s" % e)
        sys.exit(-1)
    except IOError as e:
        print("nwplay.py: Xlib.display IOError - %s" % e)
        sys.exit(-1)
    root = display.screen().root
    displays = display.screen_count()
    displayWidth = root.get_geometry().width
    displayHeight = root.get_geometry().height

    if debug :
        print('Display: width = %d, height = %d' % (displayWidth, displayHeight))

    # Create a named window
    window = display_window(cube_uuid)

    # Create an inet, streaming socket with blocking
    s = create_socket(address, port, True)

    response = login_request(s, sequence, username, password, cube_uuid, "")
    if check_error("login_request", response) or response["message_type"] == "GoodBye":
        # We're done. Goodbye.    
        print("nwplay.py: Login request failed. Check credentials and cube UUID in use.")
        shutdown_socket(s)
        return

    if "ground_scale_factor" in response.keys():
        ground_scale_factor = response["ground_scale_factor"]

    vrloop = True
    vrloop_count = 0.0
    vrloop_start = time.time()
    xc = 0
    yc = 0
    visual_id = visual.create_image(xc, yc, anchor=NW, image=None)
    
    while vrloop:

        # Get the latest view for our cube
        sequence += 1
        view_response = cube_view_request(s, sequence, cube_uuid, spatial_angle_control, gaze_control)
        if check_error("cube_view_request", view_response):
            vrloop = False
            continue

        # Get the latest status for our cube
        sequence += 1
        status_response = cube_status(s, sequence, cube_uuid)
        if check_error("cube_status", status_response) or not cube_active:
            vrloop = False
            continue

        # If we have an image
        if "image_length" in view_response.keys() and view_response["image_length"] > 0:

            # We have a new image. Get rid of any bounding box outlines.
            visual.delete("boxes")

            # Figure out what to do next
            cube_state = execute_strategy(cube_state)
            
            # Now display our image on the screen
            # RGB then BGR then RGB, what a mess.
            img = Image.fromarray(view_response["image"][:,:,::-1])

            if pdf_filter:
                wwc =  window_width / 2.0
                whc = window_height / 2.0
                
                for j in range(window_height):
                    for i in range(window_width):
                        dx = i - wwc
                        dy = j - whc
                        dr = math.sqrt(dx**2+dy**2)
                        drn = dr/wwc
                        pscale = pdf(drn, u_mean, std_dev)/y_max
                        pixel = img.getpixel((i, j))
                        pixels = (int(pscale * pixel[0]), int(pscale * pixel[1]), int(pscale * pixel[2]))
                        img.putpixel((i, j), pixels)
                            
            imgtk = ImageTk.PhotoImage(img)
            visual.itemconfig(visual_id, image=imgtk)

            if image_snapshot:
                ymd = time.strftime("%Y-%m-%d_%H.%M.%S")
                snapshot("IMG-%s.%05d.png" % (ymd, sequence), view_response["image"])
                image_snapshot = False

        # End of this processing loop
        vrloop_count += 1.0

        vrloop_elapsed_time = time.time() - vrloop_start
        vrloops_per_second = vrloop_count / vrloop_elapsed_time
        ivrct = int(vrloop_count)
        if (ivrct % 60 == 0):
            vrloops_per_second_field.set("Views/sec = %0.2f" % vrloops_per_second)  

        if debug:
            print("nwplay.py: ViewRequests per second = %0.2f" % vrloops_per_second)

        # Update text fields on the control panel
        update_panel()

        # Update the playing field diagram
        update_field(spatial_angle, cube_state)

        # Update the control panel window
        window.update()

        # Play a sound
        if sound:
            sound_file = get_sound(cube_state, cube_player, spatial_velocity, spatial_direction_active_control, FindResource, FindMate, FindPredator)
            if sound_file != "":
                playsound(sound_file, block=True)
            
        if debug:
            time.sleep(5)
        
    sequence += 1
    logout_request(s, sequence, cube_uuid, "")
        
    # Close the window
    window.destroy()

    # We're done. Goodbye.    
    shutdown_socket(s)
    print("nwplay.py: Game over")

        
# ------- Could be removed -----------------------------------

# Test data for predictions
def predict_test(image_filename):

    if scan_state_control == 0:
        classname0 = "females"
        score0 = 0.9992648959159851
        bounding_vertices0 = [0.7455063462257385, 0.3876940608024597, 0.997381865978241, 0.767550528049469]
        classname1 = "enbies"
        score1 = 0.9981234669685364
        bounding_vertices1 = [0.1775604635477066, 0.45660680532455444, 0.39429670572280884, 0.6395054459571838]
        p = {"predictions": [{"classname": classname0, "score": score0, "bounding_vertices": bounding_vertices0}, {"classname": classname1, "score": score1, "bounding_vertices": bounding_vertices1}]}
        return p
    if scan_state_control == 1:
        classname0 = "resources"
        score0 = 0.9994663596153259
        bounding_vertices0 = [0.3310697078704834, 0.4611090123653412, 0.46154356002807617, 0.5913728475570679]
        classname1 = "females"
        score1 = 0.994785487651825
        bounding_vertices1 = [0.0, 0.39253750443458557, 0.25295698642730713, 0.7403668165206909]
        p = {"predictions": [{"classname": classname0, "score": score0, "bounding_vertices": bounding_vertices0}, {"classname": classname1, "score": score1, "bounding_vertices": bounding_vertices1}]}
        return p
    if scan_state_control == 2:
        classname0 = "predators"
        score0 = 0.997676432132721
        bounding_vertices0 = [0.20436035096645355, 0.34890514612197876, 0.5519095659255981, 0.6472323536872864]
        p = {"predictions": [{"classname": classname0, "score": score0, "bounding_vertices": bounding_vertices0}]}
        return p
    
    p = {"predictions": []}
    return p

def predict_test_one(image_filename):
    if total_points[0] < 10.0:
        classname = "predators"
        score = 0.95
        bounding_vertices = [0.25, 0.25, 0.75, 0.75]
        p = {"predictions": [{"classname": classname, "score": score, "bounding_vertices": bounding_vertices}]}
    else:
        #  We killed off the predator already
        p = {"predictions": []}
        
    return p


if __name__=='__main__':

    #
    # Get options and call the main program
    #                                                                                            

    try:
        options, args = getopt.getopt(sys.argv[1:], 'a:c:df:ghikmp:rsu:v', ['address=','cube=','debug','file=','game','help','image','kill','mate','pswd=','resource','sound','user=','version'])
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
            nwmessage_debug(debug)
        if o in ("-f", "--file"):
            filename = a
        if o in ("-g", "--game"):
            GameOn = True
        if o in ("-h", "--help"):
            Usage()
            sys.exit()
        if o in ("-i", "--image"):
            image_snapshot = True
        if o in ("-k", "--kill"):
            FindPredator  = True
        if o in ("-m", "--mate"):
            FindMate = True
        if o in ("-p", "--pswd"):
            password = a
        if o in ("-r", "--resource"):
            FindResource = True
        if o in ("-s", "--sound"):
            sound = True
        if o in ("-u", "--user"):
            username = a
        if o in ("-v", "--version"):
            print("nwplay.py Version 1.0")
            sys.exit()
        
        # If sound effects are desired
        if sound:
            from playsound import playsound

    main()
    sys.exit()

