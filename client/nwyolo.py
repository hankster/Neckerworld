#! /usr/bin/python3
"""
nwyolo.py -- A Python program to run inference on an image.

Sample usage:

 nwyolo.py

Complete specification:

 nwyolo.py -d -f filename -h -r repo -s -v -w weights --debug --file=filename --help --repo=repo --show --source=source --version --weights=weights

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

"""

import sys
import os, os.path
import io
import time
import getopt
import string
import math
import random
import json
import cv2
import torch
from ultralytics import YOLO

debug = False
precision_test = False
show_boxes = False

players = {'male': 'males', 'female': 'females', 'enby': 'enbies', 'predator': 'predators', 'resource': 'resources'}
classes = ['males', 'females', 'enbies', 'predators', 'resources']
resolution = 512.0

# Image file for testing
filename = "test.png"

#repo = '../nwmodel/yolov5/nw_weights'
#weights = 'yolov5l-nw.pt'
repo = '../nwmodel/yolov11/nw_weights'
# weights = 'yolov11x-nw.pt'
# weights = 'yolov11l-nw.pt'
# weights = 'yolov11m-nw.pt'
weights = 'yolov11s-nw.pt'
# weights = 'yolov11n-nw.pt'
source = 'local'

# Github repositories
# repo = 'ultralytics/yolov3'
# repo = 'ultralytics/yolov5'
# repo = 'ultralytics/ultralytics'
# model = torch.hub.load('ultralytics/yolov5', 'yolov5s')  # official model
# model = torch.hub.load('ultralytics/yolov5:master', 'yolov5s')  # from branch
# model = torch.hub.load('ultralytics/yolov5', 'custom', 'yolov5s.pt')  # custom/local model
# model = torch.hub.load('.', 'custom', 'yolov5s.pt', source='local')  # local repo
# models
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

def Usage():
    print("Usage: nwyolo.py -d -f filename -h -r repo -s -v -w weights --debug --file=filename --help --repo=repo --show --source=source --version --weights=weights")

# Run through all images to check precision
def p_test():

    print("nwyolo.py: Calculating precision")
    
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
                print("nwyolo.py: Predicted class name: {0:>9s}, score {1:0.3f} {2}".format(r["classname"], r["score"], r["bounding_vertices"]))
            classname = r["classname"]
            score = r["score"]
            if score >= threshold and cn[0:4] == classname[0:4]:
                p += 1.0
                
        if samples > 999.9:
            break

        if debug:
            print("nwyolo.py: Precision %0.3f using threshold %0.2f with %d samples" % (p/samples, threshold, int(samples)))

        
    precision = p/samples

    end_time = time.time()
    test_time = end_time - start_time
    prediction_time_ms = (test_time/samples) * 1000.0

    print("nwyolo.py: Precision %0.3f using threshold %0.2f with %d samples" % (precision, threshold, int(samples)))
    print("nwyolo.py: Prediction time per sample %0.3f milliseconds." % prediction_time_ms)
          
    return

#
# Make a prediction
#

def predict(image_filename):
    
    # Inference
    # results = model([image_filename], size=resolution)
    results = model(source=image_filename, imgsz=int(resolution), max_det=15, verbose=debug)
    
    labels = []
    scores = []
    boxes = []
    
    if debug:
        print("nwyolo.py: Found %d results. Type of results is %s." % (len(results), type(results)))
        # print(results)
        
    for result in results:

        lines = result.to_csv().split('\n')
        if not 'name' in lines[0]:
            continue
        for i in range(1, len(lines)-1):
            details = lines[i].split(',')
            if debug:
                print(details)
            label = classes[int(details[1])]
            labels.append(label)
            score = float(details[2])
            scores.append(score)
            bounds_index = lines[i].index('"')
            bounds = lines[i][bounds_index:]
            bounds = bounds[1:-1].replace("'",'"')
            boxes_dict = json.loads(bounds)
            box_list = []
            box_list.append(boxes_dict['x1']/resolution)
            box_list.append(boxes_dict['y1']/resolution)
            box_list.append(boxes_dict['x2']/resolution)
            box_list.append(boxes_dict['y2']/resolution)
            boxes.append(box_list)
            
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
        classname = labels[i]
        bounding_vertices = boxes[i]
        predictions["predictions"].append({"classname": classname, "score": score, "bounding_vertices": bounding_vertices})

    if len(predictions["predictions"]) > 0:
        for p in predictions["predictions"]:
            c = p["classname"]
            s = p["score"]
            b = p["bounding_vertices"]
            if debug:
                print("nwyolo.py: predict classname %9s score %0.3f box [%0.2f, %0.2f, %0.2f, %0.2f]" % (c, s, b[0], b[1], b[2], b[3]))

    return predictions

#
# Main program starts here
#

def main():

    global model
    
    # Load a local model
    model = YOLO(repo + '/' + weights)

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

        if debug:
            print("nwyolo.py: Predicted class name: {}".format(classname))
            print("nwyolo.py: Predicted class score: {}".format(score))
            print("nwyolo.py: Predicted normalized vertices: %s" % bounding_box)

        if show_boxes:
            color = bounding_box_colors[classname]
            thickness = 1
        
            x1 = int(res * bounding_box[0]) 
            y1 = int(res * bounding_box[1]) 
            x2 = int(res * bounding_box[2]) 
            y2 = int(res * bounding_box[3]) 
            if x1 < 0 or y1 < 0 or x2 > 512 or y2 > 512:
                print("nwyolo.py: Bounding box error (%3d, %3d), (%3d, %3d)" % (x1, y1, x2, y2))

            cv2.rectangle(img, (x1, y1), (x2, y2), color, thickness)
            print("nwyolo.py: Bounding box for %s (%d, %d), (%d, %d)" % (classname, x1, y1, x2, y2))
            
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
            print("nwyolo.py: Version 1.0")
            sys.exit()
        if o in ("-w", "--weights"):
            weights = a
        
    main()

    sys.exit()

else:

    global model
    
    # Load a local model (We are running as a module, not a main program)

    # model = torch.hub.load(repo_or_dir, model, *args, source='github', trust_repo=None, force_reload=False, verbose=True, skip_validation=False, **kwargs)
    # model = torch.hub.load('ultralytics/yolov5', 'custom', path='../nwmodel/yolov5/nw_weights/yolov5l-nw.pt')
    model = YOLO(repo + '/' + weights)
    # model = torch.hub.load(repo, weights, source=source)
    
    # model.conf = 0.25  # NMS confidence threshold
    #       iou = 0.45  # NMS IoU threshold
    #       agnostic = False  # NMS class-agnostic
    #       multi_label = False  # NMS multiple labels per box
    #       classes = None  # (optional list) filter by class, i.e. = [0, 15, 16] for COCO persons, cats and dogs
    #       max_det = 1000  # maximum number of detections per image
    #       amp = False  # Automatic Mixed Precision (AMP) inference
      
