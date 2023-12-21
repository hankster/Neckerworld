#! /usr/bin/python3
"""
nwplay.py -- Neckerworld View - display playing field remotely

Sample usage:

 nwview.py

Complete specification:

 nwview.py -a address -d -g uuid -h -i -p password -u username -v --address address --debug --ground=uuid --help --image --pswd=password  --user=username --version

 where

 -a, --address        Address of remote host
 -d, --debug          Turn debug statements on
 -g, --ground         Ground UUID
 -h, --help           Print usage information
 -i, --image          Image snapshot
 -p, --pswd           Password
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
from nwmessage import ground_view_request
from nwmessage import snapshot
from nwmessage import nwmessage_debug

# def create_socket(address, port, blocking)
# def shutdown_socket(s)
# def login_request(s, sequence, username, password, cube_uuid, ground_uuid)
# def logout_request(s, sequence, cube_uuid, ground_uuid)
# def ground_view_request(s, sequence, ground_uuid, groundview)
# def snapshot(filename, image)
# def nwmessage_debug(debug)

debug = False
address = "127.0.0.1"
port = 2020

username = "example.user"
password = "pw"
groundview = -1
image_snapshot = False

displayNumber = ":0"
displayWidth = 0
displayHeight = 0
window_name = "Neckerworld Playing Field View"
window_width = 1280
window_height = 720
window_channels = 4
window_image_length = window_width * window_height * window_channels

# Standard ground uuid's
ground_uuid_test = "9787fbe0-68f9-40e7-81b7-904d36e7a848"
ground_uuid_wide = "86521488-c1b7-493e-b82a-2473e34af982"
ground_ids = [ground_uuid_test, ground_uuid_wide]
ground_uuid = ""

# ground scale factor (1/2 width of the playing field)
ground_scale_factor = 10.0

# imagea we are retrieving from the server
image = []

# Options
vrloop = False
sequence = 0

# Window variables

# Runtime status messages
status_message = ""

def Usage():
    print("Usage: nwview.py -a addr -d -g uuid -h -i -p password -u username -v --address=address --debug --ground=uuid --help --image --pswd=password --user=username --version")

# Create our display window
def display_window(name, uuid):

    # https://stackoverflow.com/questions/459083/how-do-you-run-your-own-code-alongside-tkinters-event-loop
    # http://www.nmt.edu/tcc/help/pubs/tkinter/
    # https://anzeljg.github.io/rin2/book2/2405/docs/tkinter/relief.html
    
    global ground_uuid_field
    global visual
    global status_message
    
    window_title = name
    root = tkinter.Tk()
    root.title(window_title)
    root.columnconfigure(0, weight=1)
    root.rowconfigure(0, weight=1)

    # top = root.winfo_toplevel()
    # top.rowconfigure(0, weight=1)
    # top.columnconfigure(0, weight=1) 

    style = ttk.Style(root)
    style.map("C.TButton",
    foreground=[('pressed', 'red'), ('active', 'blue')],
    background=[('pressed', '!disabled', 'black'), ('active', 'white')]
    )

    mainframe = ttk.Frame(root, padding="3 3 3 3")
    mainframe.grid(column=0, row=0, sticky=(N, W, E, S))
    mainframe.columnconfigure(0, weight=1)
    mainframe.rowconfigure(0, weight=1)
    ground_uuid_field = StringVar()
    ttk.Label(mainframe, width=40, padding="5 5 5 5", foreground='#000', textvariable=ground_uuid_field).grid(column=0, row=1, sticky=(W, E))
    ground_uuid_field.set("UUID: %s" % uuid)
    ttk.Button(mainframe, text="Quit", command=quit_play, style="C.TButton").grid(column=1, row=1, sticky=E)

    visual = tkinter.Canvas(mainframe, width=window_width, height=window_height, bg='#000')
    visual.grid(column=0, columnspan=2, row=0, sticky=(N, W, E, S))
    
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

def update_panel():
    return
    
def trigger_snapshot(*args):
    global image_snapshot
    image_snapshot = True

def quit_play(*args):
    global vrloop
    vrloop = False

# Check for communications error
def check_error(msg, response):
    if response["message_type"] == "Error":
        print("nwview.py: %s - Error - %s" % (msg, response["error"]))
        return True
    return False

# Main program starts here

def main():
    
    global debug
    global displayNumber, displayWidth, displayHeight
    global ground_uuid
    global ground_scale_factor
    global s
    global sequence
    global view_response
    global image_snapshot
    global vrloop
    
    # Find display size from X
    displayNumber = os.getenv('DISPLAY', ':0')
    try:
        display = Xlib.display.Display(displayNumber)
    except Xlib.error.DisplayConnectionError as e:
        print("nwview.py: Xlib.display connection error - %s" % e)
        sys.exit(-1)
    except IOError as e:
        print("nwview.py: Xlib.display IOError - %s" % e)
        sys.exit(-1)
    root = display.screen().root
    displays = display.screen_count()
    displayWidth = root.get_geometry().width
    displayHeight = root.get_geometry().height

    if debug :
        print('Display: width = %d, height = %d' % (displayWidth, displayHeight))

    # Create an inet, streaming socket with blocking
    s = create_socket(address, port, True)

    # Try our list of ground uuid's for login
    attempts = 0
    for id in ground_ids:
        response = login_request(s, sequence, username, password, "", id)
        if response["message_type"] != "GoodBye" and (not check_error("login_request", response)):
            ground_uuid = id
            break
        attempts += 1
        if attempts == len(ground_ids):
            # We're done. Goodbye.    
            print("nwview.py: Login request failed. Verify the ground UUID in use.")
            shutdown_socket(s)
            return

    if "ground_scale_factor" in response.keys():
        ground_scale_factor = response["ground_scale_factor"]

    # Create a named window
    window = display_window(window_name, ground_uuid)

    vrloop = True
    vrloop_count = 0.0
    vrloop_start = time.time()
    xc = 0
    yc = 0
    visual_id = visual.create_image(xc, yc, anchor=NW, image=None)
    
    while vrloop:

        # Get the latest display
        sequence += 1
        view_response = ground_view_request(s, sequence, ground_uuid, 0)
        if check_error("ground_view_request", view_response):
            vrloop = False
            continue

        # If we have an image
        if "image_length" in view_response.keys() and view_response["image_length"] > 0:

            # Size of the canvas
            vw = visual.winfo_width()
            vh = visual.winfo_height()

            # Now display our image on the screen
            img = Image.fromarray(view_response["image"][:,:,::-1])
            # If the canvas changed size then resize
            if vw > 320 and vh > 180 and (vw != window_width+2 or vh != window_height+2):
                img = img.resize((vw-2,vh-2))
                w, h = img.size
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

        if debug:
            print("nwview.py: ViewRequests per second = %0.2f" % vrloops_per_second)

        # Update text fields on the control panel
        update_panel()

        # Update the field display window
        window.update()

        if debug:
            time.sleep(5)
        
    sequence += 1
    logout_request(s, sequence, "", ground_uuid)
        
    # Close the window
    window.destroy()

    # We're done. Goodbye.    
    shutdown_socket(s)
    print("nwview.py: Game over")

if __name__=='__main__':

    #
    # Get options and call the main program
    #                                                                                            

    try:
        options, args = getopt.getopt(sys.argv[1:], 'a:dg:hip:su:v', ['address=','debug','ground=','help','image', 'pswd=','sound','user=','version'])
    except getopt.GetoptError:
        Usage()
        sys.exit(2)

    for o, a in options:
        if o in ("-a", "--address"):
            address = a
        if o in ("-d", "--debug"):
            debug = True
            nwmessage_debug(debug)
        if o in ("-g", "--ground"):
            ground_ids = [a]
        if o in ("-h", "--help"):
            Usage()
            sys.exit()
        if o in ("-i", "--image"):
            image_snapshot = True
        if o in ("-p", "--pswd"):
            password = a
        if o in ("-u", "--user"):
            username = a
        if o in ("-v", "--version"):
            print("nwview.py Version 1.0")
            sys.exit()
        

    main()
    sys.exit()

