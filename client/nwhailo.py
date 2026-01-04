#! /usr/bin/env python3
"""
nwhailo.py -- A Python program to run inference on an image using the Hailo chip running on a Raspberry Pi 5

Sample usage:

 nwhailo.py

Complete specification:

 nwhailo.py -d -f filename -h -r repo -s -v -w weights --debug --file=filename --help --repo=repo --show --source=source --version --weights=weights

 where

 -d, --debug          Turn debug statements on
 -f, --file           Input filename
 -h, --help           Print usage information
 -r, --repo           Repository or local directory
 -s, --show           Show bounding boxes
     --source         Source of model ('github' or 'local')
 -v, --version        Report program version
 -w, --weights        Trained model and weights (usually a .pt file)

Copyright (2025) H. S. Magnuski
All rights reserved

Credit for original code goes to:
File Name:   simple_infer.py
Author:      Nadav
Date:        June 30, 2025

"""

import sys
import os, os.path
import io
import time
import getopt
import string
import math
import random
import cv2
import numpy as np
from pathlib import Path

from hailo_platform import (HEF, VDevice, HailoStreamInterface, InferVStreams, ConfigureParams, InputVStreamParams, OutputVStreamParams, FormatType)

from PIL import Image

debug = False
precision_test = False
show_boxes = False

# filename = "test.png"
filename = "/home/hankm/yolov11/training-female-1f31d-hd-1.0690-512x512.jpg"

repo = '../nwmodel/hailo8l'
weights = 'yolov11m-nw.hef'
hef_path = repo + "/" + weights

players = {'male': 'males', 'female': 'females', 'enby': 'enbies', 'predator': 'predators', 'resource': 'resources'}
classes = ['males', 'females', 'enbies', 'predators', 'resources']
labels_defined = ["male", "female", "enby", "predator", "resource"]

target = VDevice()
hef = HEF(hef_path)
    
# Configure network groups
configure_params = ConfigureParams.create_from_hef(hef, interface=HailoStreamInterface.PCIe)
network_group = target.configure(hef, configure_params)[0]
network_group_params = network_group.create_params()

# Create input and output virtual streams params
input_vstreams_params = InputVStreamParams.make_from_network_group(network_group, quantized=False, format_type=FormatType.UINT8)
output_vstreams_params = OutputVStreamParams.make_from_network_group(network_group, quantized=False, format_type=FormatType.FLOAT32)

# Define dataset params
input_vstream_info = hef.get_input_vstream_infos()[0]
output_vstream_info = hef.get_output_vstream_infos()[0]

def Usage():
    print("Usage: nwhailo.py -d -f filename -h -r repo -s -v -w weights --debug --file=filename --help --repo=repo --show --source=source --version --weights=weights")

# Run through all images to check precision
def p_test():

    print("nwhailo.py: Calculating precision")
    
    bbcsv = "../training/training-bounding-box.csv"
    dir = "../training/trainers-jpg"
    threshold = 0.5
    samples = 0.0
    p = 0.0
    idx = -1
    
    with open(bbcsv, 'r') as f:
        lines = f.readlines()

    start_time = time.time()

    for line in lines:
        idx += 1
        if not ((idx % 10) == 0):
            continue
        fields = line.split('\t')
        imfn = fields[0]
        cn = players[fields[1]]
        image_file = dir + '/' + cn + '/' + imfn

        results = predict(image_file)
        samples += 1.0
        
        if len(results["predictions"]) > 0:
            r = results["predictions"][0]
            if debug:
                print("nwhailo.py: Predicted class name: {0:>9s}, score {1:0.3f} {2}".format(r["classname"], r["score"], r["bounding_vertices"]))
            classname = r["classname"]
            score = r["score"]
            if score >= threshold and cn[0:4] == classname[0:4]:
                p += 1.0
                
        if samples > 99.9:
            break

        if debug:
            print("nwhailo.py: Precision %0.3f using threshold %0.2f with %d samples" % (p/samples, threshold, int(samples)))

        
    precision = p/samples

    end_time = time.time()
    test_time = end_time - start_time
    prediction_time_ms = (test_time/samples) * 1000.0

    print("nwhailo.py: Precision %0.3f using threshold %0.2f with %d samples" % (precision, threshold, int(samples)))
    print("nwhailo.py: Prediction time per sample %0.3f milliseconds." % prediction_time_ms)
      
    return

#
# Make a prediction
#

# Run inference (this part uses the Hailo hardware)
# The Hailo SDK provides methods to push data to the configured model and receive results
# The output format will be specific to the model (e.g., YOLO output tensor)

def infer(image):
    with InferVStreams(network_group, input_vstreams_params, output_vstreams_params) as infer_pipeline:
        input_data = {input_vstream_info.name: np.expand_dims(image, axis=0).astype(np.uint8)}    
        with network_group.activate(network_group_params):
            infer_results = infer_pipeline.infer(input_data)
    return infer_results

def predict(image_filename):
    
    resolution = 512.0
    
    # Get the image
    image = Image.open(image_filename)

    # Get the detections from the buffer
    infer_results = infer(image)

    labels = []
    scores = []
    boxes = []
    
    # Parse the detections
    detection_count = 0

    for idx, class_detections in enumerate(infer_results[list(infer_results.keys())[0]][0]):
        if class_detections.shape[0]>0:
            for det in class_detections:
                labels.append(labels_defined[idx])
                scores.append(det[4])
                scale = 640.0/resolution
                boxes.append([scale * float(det[1]), scale * float(det[0]), scale * float(det[3]), scale * float(det[2])])
                detection_count += 1
                
    if debug:
        print("nwhailo.py: Found %d detections." % (detection_count))
        
    if debug:
        print(labels)
        print(scores)
        print(boxes)
        
    predictions = {"predictions": []}
    threshold = 0.5
    
    for i in range(len(labels)):
        score = scores[i]
        if score < threshold:
            continue
        classname = players[labels[i]]
        bounding_vertices = boxes[i]
        predictions["predictions"].append({"classname": classname, "score": score, "bounding_vertices": bounding_vertices})

    if len(predictions["predictions"]) > 0:
        for p in predictions["predictions"]:
            c = p["classname"]
            s = p["score"]
            b = p["bounding_vertices"]
            print("nwhailo.py: predict classname %9s score %0.3f box [%0.2f, %0.2f, %0.2f, %0.2f]" % (c, s, b[0], b[1], b[2], b[3]))

    return predictions

#
# Main program starts here
#

def main():

    global user_data

    if precision_test:
        p_test()
        return

    player_colors = {"female": "white", "male": "blue", "enby": "purple", "predator": "red", "resource": "green"}
    class_colors = {"females": "white", "males": "blue", "enbies": "purple", "predators": "red", "resources": "green", "removed": "black"}
    bounding_box_colors = {"females": (255, 255, 255), "males": (255, 0, 0), "enbies": (240, 32, 160), "predators": (0, 0, 255), "resources": (0, 255, 0), "removed": (0, 0, 0)}
    res = 512.0
    
    results = predict(filename)

    if show_boxes:
        img = cv2.imread(filename)
    
    for r in results["predictions"]:
        classname = r["classname"]
        score = r["score"]
        bounding_box = r["bounding_vertices"]
        print("nwhailo.py: Predicted class name: {}".format(classname))
        print("nwhailo.py: Predicted class score: {}".format(score))
        print("nwhailo.py: Normalized Vertices: %s" % bounding_box)

        if show_boxes:
            color = bounding_box_colors[classname]
            thickness = 1
        
            x1 = int(res * bounding_box[0]) 
            y1 = int(res * bounding_box[1]) 
            x2 = int(res * bounding_box[2]) 
            y2 = int(res * bounding_box[3]) 
            if x1 < 0 or y1 < 0 or x2 > 512 or y2 > 512:
                print("nwhailo.py: Bounding box error (%3d, %3d), (%3d, %3d)" % (x1, y1, x2, y2))

            cv2.rectangle(img, (x1, y1), (x2, y2), color, thickness)
            print("nwhailo.py: Bounding box for %s (%d, %d), (%d, %d)" % (classname, x1, y1, x2, y2))
            
    if show_boxes:
        cv2.imshow(filename, img)
        cv2.waitKey(0)
        cv2.destroyWindow(filename)

if __name__=='__main__':

    #
    # Get options and call the main program
    #                                                                                            
    try:
        options, args = getopt.getopt(sys.argv[1:], 'df:hpr:svw:', ['debug', 'file=', 'help', 'precision', 'repo=', 'show', 'source=', 'version', 'weights='])
    except getopt.GetoptError:
        Usage()
        sys.exit(-1)

    for o, a in options:
        if o in ("-d", "--debug"):
            debug = True
        if o in ("-f", "--file"):
            filename = a
        if o in ("-h", "--help"):
            Usage()
            sys.exit()
        if o in ("-p", "--precision"):
            precision_test = True
        if o in ("-r", "--repo"):
            repo = a
        if o in ("-s", "--show"):
            show_boxes = True
        if o in ("--source"):
            source = a
        if o in ("-v", "--version"):
            print("nwhailo.py: Version 1.0")
            sys.exit()
        if o in ("-w", "--weights"):
            weights = a
        
    main()

    sys.exit()

else:

    global user_data

