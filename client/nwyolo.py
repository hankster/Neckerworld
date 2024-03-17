#! /usr/bin/python3
"""
nwyolo.py -- A Python program to run inference on an image.

Sample usage:

 nwyolo.py

Complete specification:

 nwyolo.py -d -f filename -h -v --debug --file=filename --help --version

 where

 -d, --debug          Turn debug statements on
 -f, --file           Input filename
 -h, --help           Print usage information
 -v, --version        Report program version

Copyright (2024) H. S. Magnuski
All rights reserved

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
import torch

debug = False
precision_test = False

filename = "test.png"

yolo_nano = "yolov5n.pt"
yolo_small = "yolov5s.pt"
yolo_medium = "yolov5m.pt"
yolo_large = "yolov5l.pt"
yolo_extra_large = "yolov5x.pt"

# Check for a GPU
if torch.cuda.is_available():
    print('nwyolo.py: GPU is available.')
    print('nwyolo.py: Number of CUDA devices = %d' % torch.cuda.device_count())
    nw_device = torch.device('cuda')
else:
    print('nwyolo.py: GPU not available, CPU only')
    nw_device = torch.device('cpu')

# Load a local model
model = torch.hub.load('ultralytics/yolov5', 'custom', path='../nwmodel/yolov5/nw_weights/best.pt')

# model.conf = 0.25  # NMS confidence threshold
#       iou = 0.45  # NMS IoU threshold
#       agnostic = False  # NMS class-agnostic
#       multi_label = False  # NMS multiple labels per box
#       classes = None  # (optional list) filter by class, i.e. = [0, 15, 16] for COCO persons, cats and dogs
#       max_det = 1000  # maximum number of detections per image
#       amp = False  # Automatic Mixed Precision (AMP) inference
      
def Usage():
    print("Usage: nwyolo.py -d -f filename -h -v --debug --file=filename --help --version")

# Run through all images to check precision
def p_test():

    print("nwyolo.py: Calculating precision")
    
    bbcsv = "../training/training-bounding-box-set2.csv"
    dir = "../training/trainers-set2-jpg"
    threshold = 0.5
    samples = 0.0
    p = 0.0
    idx = -1
    
    with open(bbcsv, 'r') as f:
        lines = f.readlines()
    for line in lines:
        idx += 1
        if not ((idx % 10) == 0):
            continue
        fields = line.split(',')
        imfn = fields[0]
        cn = fields[1]
        image_file = dir + '/' + cn + '/' + imfn

        results = predict(image_file)
        samples += 1.0
        
        if len(results["predictions"]) > 0:
            r = results["predictions"][0]
            print("nwyolo.py: Predicted class name: {0:>9s}, score {1:0.3f} {2}".format(r["classname"], r["score"], r["bounding_vertices"]))
            classname = r["classname"]
            score = r["score"]
            if score >= threshold and cn[0:4] == classname[0:4]:
                p += 1.0
                
        if samples > 999.9:
            break

        print("nwyolo.py: Precision %0.3f using threshold %0.2f with %d samples" % (p/samples, threshold, int(samples)))

        
    precision = p/samples

    print("nwyolo.py: Precision %0.3f using threshold %0.2f with %d samples" % (precision, threshold, int(samples)))
          
    return

#
# Make a prediction
#

def predict(image_filename):
    
    players = {'male': 'males', 'female': 'females', 'enby': 'enbies', 'predator': 'predators', 'resource': 'resources'}
    classes = ['males', 'females', 'enbies', 'predators', 'resources']
    resolution = 512.0
    
    # Inference
    results = model([image_filename], size=512)
    
    # Convert to our bounding box format an move to CPU memory.
    r = results.xyxy[0].cpu()
    
    labels = r[:, 5].numpy()
    boxes =  r[:, 0:4].numpy() / resolution
    scores = r[:, 4].numpy()

    if debug:
        print(labels)
        print(boxes)
        print(scores)
        
    predictions = {"predictions": []}
    threshold = 0.5
    
    for i in range(labels.shape[0]):
        score = scores[i]
        if score < threshold:
            continue
        classname = classes[int(labels[i])]
        bounding_vertices = boxes[i].tolist()
        predictions["predictions"].append({"classname": classname, "score": score, "bounding_vertices": bounding_vertices})

    if len(predictions["predictions"]) > 0:
        for p in predictions["predictions"]:
            c = p["classname"]
            s = p["score"]
            b = p["bounding_vertices"]
            print("nwyolo.py: predict classname %9s score %0.3f box [%0.2f, %0.2f, %0.2f, %0.2f]" % (c, s, b[0], b[1], b[2], b[3]))

    return predictions

#
# Main program starts here
#

def main():

    if precision_test:
        p_test()
        return

    results = predict(filename)

    for r in results["predictions"]:
        print("nwyolo.py: Predicted class name: {}".format(r["classname"]))
        print("nwyolo.py: Predicted class score: {}".format(r["score"]))
        bounding_box = r["bounding_vertices"]
        print("nwyolo.py: Normalized Vertices: %s" % bounding_box)

if __name__=='__main__':

    #
    # Get options and call the main program
    #                                                                                            

    try:
        options, args = getopt.getopt(sys.argv[1:], 'df:hpv', ['debug', 'file=', 'help', 'precision', 'version'])
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
        if o in ("-v", "--version"):
            print("nwyolo.py: Version 1.0")
            sys.exit()
        
    main()

    sys.exit()
