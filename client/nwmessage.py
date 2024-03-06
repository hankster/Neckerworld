#! /usr/bin/python3
"""
nwmessage.py -- Utilities to send a JSON message to cube

Copyright (2020) H. S. Magnuski
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

debug = False

# Set debug flag
def nwmessage_debug(d):
    global debug
    debug = d
    
# Create an inet, streaming socket
def create_socket(address, port, blocking):
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.settimeout(3.0)
        s.setblocking(blocking)
        # now connect to the server on the port
        s.connect((address, port))
    except ConnectionRefusedError as e:
        print("nwmessage.py: create_socket connection refused %s" % e)
        sys.exit(-1)
    except socket.timeout as e:
        print("nwmessage.py: create_socket timeout %s" % e)
        sys.exit(-1)
    except socket.error as e:
        print("nwmessage.py: create_socket error %s" % e)
        sys.exit(-1)
    except IOError as e:
        print("nwmessage.py: create_socket IOError %s" % e)
        sys.exit(-1)
    return s

# Shutdown a socket
def shutdown_socket(s):
    try:
        s.shutdown(socket.SHUT_RDWR)
        s.close()
    except IOError as e:
        print("nwmessage.py: %s" % e)
        return {"message_type": "Error", "error": e}
        
def login_request(s, sequence, username, password, cube_uuid, ground_uuid):

    message_type = "LoginRequest"
    timestamp = time.time()
    message = {"message_type": message_type, "sequence": sequence, "timestamp": timestamp, "cube_uuid": cube_uuid, "ground_uuid": ground_uuid, "username": username, "password": password}
    data = json.dumps(message)
    if debug:
        print("nwmessage.py: Sending   " + data)
    bytes = data.encode(encoding='UTF-8')

    # send the login request
    try:
        s.send(bytes)
        response = str(s.recv(1024), 'utf-8')
        if debug:
            print("nwmessage.py: Receiving " + response)
    except IOError as e:
        print("nwmessage.py: %s" % e)
        return {"message_type": "Error", "error": e}
    try:
        j = json.loads(response)
    except json.JSONDecodeError as e:
        return {"message_type": "Error", "error": e}
    return j
        
def logout_request(s, sequence, cube_uuid, ground_uuid):
    
    message_type = "LogoutRequest"

    timestamp = time.time()
    message = {"message_type": message_type, "sequence": sequence, "timestamp": timestamp, "cube_uuid": cube_uuid, "ground_uuid": ground_uuid}
    data = json.dumps(message)
    if debug:
        print("nwmessage.py: Sending   " + data)
    bytes = data.encode(encoding='UTF-8')
    try:
        s.send(bytes)
        response = str(s.recv(1024), 'utf-8')
        if debug:
            print("nwmessage.py: Receiving " + response)
    except IOError as e:
        print("nwmessage.py: %s" % e)
        return {"message_type": "Error", "error": e}
    try:
        j = json.loads(response)
    except json.JSONDecodeError as e:
        return {"message_type": "Error", "error": e}
    return j

def import_json_file(s, sequence, jsonfile, cube_uuid, ground_uuid):
    
    message_type = "ImportJSONFileRequest"

    timestamp = time.time()
    message = {"message_type": message_type, "sequence": sequence, "timestamp": timestamp, "jsonfilename": jsonfile, "cube_uuid": cube_uuid, "ground_uuid": ground_uuid}
    data = json.dumps(message)
    if debug:
        print("nwmessage.py: Sending   " + data)
    bytes = data.encode(encoding='UTF-8')
    try:
        s.send(bytes)
        response = str(s.recv(1024), 'utf-8')
        if debug:
            print("nwmessage.py: Receiving " + response)
    except socket.timeout as e:
        print("nwmessage.py: import_json_file timeout %s" % e)
        return {"message_type": "Error", "error": e}
    except socket.error as e:
        print("nwmessage.py: import_json_file socket error %s" % e)
        return {"message_type": "Error", "error": e}
    except IOError as e:
        print("nwmessage.py: import_json_file %s" % e)
        return {"message_type": "Error", "error": e}

    try:
        j = json.loads(response)
    except json.JSONDecodeError as e:
        return {"message_type": "Error", "error": e}
    return j

def import_json_object(s, sequence, jsonobject, cube_uuid, ground_uuid):
    
    message_type = "ImportJSONObjectRequest"

    timestamp = time.time()
    message = {"message_type": message_type, "sequence": sequence, "timestamp": timestamp, "jsonobject": jsonobject, "cube_uuid": cube_uuid, "ground_uuid": ground_uuid}
    data = json.dumps(message)
    if debug:
        print("nwmessage.py: Sending   " + data)
    bytes = data.encode(encoding='UTF-8')
    try:
        s.send(bytes)
        response = str(s.recv(1024), 'utf-8')
        if debug:
            print("nwmessage.py: Receiving " + response)
    except socket.timeout as e:
        print("nwmessage.py: import_json_object timeout %s" % e)
        return {"message_type": "Error", "error": e}
    except socket.error as e:
        print("nwmessage.py: import_json_object socket error %s" % e)
        return {"message_type": "Error", "error": e}
    except IOError as e:
        print("nwmessage.py: import_json_object %s" % e)
        return {"message_type": "Error", "error": e}

    try:
        j = json.loads(response)
    except json.JSONDecodeError as e:
        return {"message_type": "Error", "error": e}
    return j

def move_request(s, sequence, cube_uuid, spatial_angle, spatial_direction, spatial_direction_active, distance, velocity, gaze):
    
    message_type = "MoveRequest"
    timestamp = time.time()
    message = {"message_type": message_type, "sequence": sequence, "timestamp": timestamp, "cube_uuid": cube_uuid,
               "spatial_angle": spatial_angle, "spatial_direction": spatial_direction, "spatial_direction_active": spatial_direction_active,
               "distance": distance, "velocity": velocity, "gaze": gaze}
    data = json.dumps(message)
    if debug:
        print("nwmessage.py: Sending   " + data)
    bytes = data.encode(encoding='UTF-8')
    try:
        s.send(bytes)
        response = str(s.recv(1024), 'utf-8')
        if debug:
            print("nwmessage.py: Receiving " + response)
    except socket.timeout as e:
        print("nwmessage.py: move_request timeout %s" % e)
        return {"message_type": "Error", "error": e}
    except socket.error as e:
        print("nwmessage.py: move_request socket error %s" % e)
        return {"message_type": "Error", "error": e}
    except IOError as e:
        print("nwmessage.py: move_request %s" % e)
        return {"message_type": "Error", "error": e}

    try:
        j = json.loads(response)
    except json.JSONDecodeError as e:
        return {"message_type": "Error", "error": e}
    return j

def status_request(s, sequence, cube_uuid):
    
    message_type = "StatusRequest"
    timestamp = time.time()
    message = {"message_type": message_type, "sequence": sequence, "timestamp": timestamp, "cube_uuid": cube_uuid}
    data = json.dumps(message)
    if debug:
        print("nwmessage.py: Sending   " + data)
    bytes = data.encode(encoding='UTF-8')
    try:
        s.send(bytes)
        response = str(s.recv(1024), 'utf-8')
        if debug:
            print("nwmessage.py: Receiving " + response)
    except socket.timeout as e:
        print("nwmessage.py: status_request timeout %s" % e)
        return {"message_type": "Error", "error": e}
    except socket.error as e:
        print("nwmessage.py: status_request socket error %s" % e)
        return {"message_type": "Error", "error": e}
    except IOError as e:
        print("nwmessage.py: status_request %s" % e)
        return {"message_type": "Error", "error": e}

    try:
        j = json.loads(response)
    except json.JSONDecodeError as e:
        return {"message_type": "Error", "error": e}
    return j

def cube_view_request(s, sequence, cube_uuid, spatial_angle, gaze):
    
    message_type = "ViewRequest"
    timestamp = time.time()
    message = {"message_type": message_type, "sequence": sequence, "timestamp": timestamp, "cube_uuid": cube_uuid, "spatial_angle": spatial_angle, "gaze": gaze}
    response = send_view_message(s, message)
    return response
    
def ground_view_request(s, sequence, ground_uuid, groundview):
    
    message_type = "GroundViewRequest"
    timestamp = time.time()
    message = {"message_type": message_type, "sequence": sequence, "timestamp": timestamp, "ground_uuid": ground_uuid, "groundview": groundview}
    response = send_view_message(s, message)
    if response["message_type"] != "Error":
        response["message_type"] = "GroundViewResponse"
    return response

def send_view_message(s, message):

    image = []
    response = ""
    data = json.dumps(message)
    if debug:
        print("nwmessage.py: Sending   " + data)
    bytes = data.encode(encoding='UTF-8')

    # ready_to_read, ready_to_write, in_error = select.select(potential_readers, potential_writers, potential_errs, timeout)

    try:
        # print("nwmessage.py: send_view_message  %s" % bytes)

        s.send(bytes)
        while(True):
            msg = str(s.recv(4096), 'utf-8')
            message_length = len(msg)
            response += msg
            response_length = len(response)

            # print("nwmessage.py: Received %d bytes" % message_length)
            # if response_length > 0 :
            #    rend = response[-1:]
            #else:
            #    rend = "!!!"
            # print("nwmessage.py: Message length %d Response received %d bytes, rend [%s]" % (message_length, response_length, rend))

            # if we didn't get a full buffer, check if we're done. Now check if if the response is complete
            if message_length < 4096:
                if response_length > 0:
                    # Test for JSON closing brace
                    if response[-1:] == '}':
                        break
                    else:
                        # At times a buffer is not packed with a full 4096 bytes. It's a shorter buffer.
                        continue
                else:
                    # The response looks incomplete and the connection is probably closed.
                    print("nwmessage.py: Error in message length %d, response length %d, [%s]" % (message_length, response_length, response))
                    return {"message_type": "Error", "error": "view request response is incomplete"}

            ## If the message length == 4096 and we find a brace at the end, we're done (braces are not allowed in base64 encoding character set).
            if message_length == 4096 and response[-1:] == '}':
                break
    
    except socket.timeout as e:
        print("nwmessage.py: send_view_message timeout %s" % e)
        return {"message_type": "Error", "error": e}
    except socket.error as e:
        print("nwmessage.py: send_view_message socket error %s" % e)
        return {"message_type": "Error", "error": e}
    except IOError as e:
        print("nwmessage.py: send_view_message %s" % e)
        return {"message_type": "Error", "error": e}

    if debug:
        print("nwmessage.py: Received response - length %d at %0.5f " % (len(response), time.time()))
        # print("nwmessage.py: Received response - %s" % response)

    if "Error" in response:
        return json.loads(response)
        
    try:
        j = json.loads(response)
        window_width = j["width"]
        window_height = j["height"]
        window_channels = j["channels"]
        window_mode = j["mode"]
        window_extension = j["extension"]
        window_image_length = window_width * window_height * window_channels
        bounding_box = j["bounding_box"]
        current_frame = 0
        pixels_frame = 0
        npimage = []
        if "pixels_b64" in j:
            current_frame = j["frame"]
            pixels_frame = j["pixels_frame"]
            # First change b64 encoded zip to the original zip
            gz = base64.standard_b64decode(j["pixels_b64"])
            # Now unzip the image into its raw bitmap format
            image = zlib.decompress(gz) 
            if debug:
                print("nwmessage.py: Length of decoded image is %d (type=%s). Current frame %u. Captured frame %u. Frame lag %d time %0.5f." %
                      (len(image), type(image), current_frame, pixels_frame, current_frame-pixels_frame, time.time()) )
    except (IOError, json.decoder.JSONDecodeError, zlib.error)  as e:
        print("nwmessage.py: JSON decode error - %s" % e)
        return {"message_type": "Error", "error": e}

    if len(image) == window_image_length:

        # image[y, x, c] or equivalently image[y][x][c].
        # and it will return the value of the pixel in the x,y,c coordinates.
        # Notice that indexing begins at 0. So, if you want to access the third BGR (note: not RGB) 

        # npimage = cv2.imdecode(np.frombuffer(image, np.uint8), -1)
        # npimage = npimage.reshape((window_height, window_width, window_channels))
            
        npimage = np.frombuffer(image, np.uint8).reshape((window_height, window_width, window_channels))
        if window_channels == 3 and window_mode == "RGB":
            npimage = cv2.cvtColor(npimage, cv2.COLOR_RGB2BGR)
        if window_channels == 4 and window_mode == "RGBA":
            npimage = cv2.cvtColor(npimage, cv2.COLOR_RGBA2BGRA)
       
    return {"message_type": "ViewResponse", "frame": current_frame, "pixels_frame": pixels_frame, "image_length": len(image), "width": window_width, "height": window_height, "channels": window_channels, "mode": window_mode, "extension": window_extension, "bounding_box": bounding_box, "image": npimage}

# Take a snapshot of a window
def snapshot(filename, image):
    cv2.imwrite(filename, image)
    return

