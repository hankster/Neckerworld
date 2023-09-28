#! /usr/bin/python3
"""
nweffdet.py -- A Python program to run inference on an image.

Sample usage:

 nweffdet.py

Complete specification:

 nweffdet.py -d -f filename -h -p -s -v --debug --file=filename --help --precision --show --version

 where

 -d, --debug          Turn debug statements on
 -f, --file           Input filename
 -h, --help           Print usage information
 -p, --precision      Precision test
 -s, --show           Show results (requires display)
 -v, --version        Report program version

Copyright (2021) H. S. Magnuski
All rights reserved

nweffdet.py: Model signature keys: ['serving_default']
nweffdet.py:        raw_detection_scores: TensorSpec(shape=(1, 49104, 10), dtype=tf.float32, name='raw_detection_scores')
nweffdet.py:           detection_classes: TensorSpec(shape=(1, 100), dtype=tf.float32, name='detection_classes')
nweffdet.py:              num_detections: TensorSpec(shape=(1,), dtype=tf.float32, name='num_detections')
nweffdet.py:             detection_boxes: TensorSpec(shape=(1, 100, 4), dtype=tf.float32, name='detection_boxes')
nweffdet.py: detection_multiclass_scores: TensorSpec(shape=(1, 100, 10), dtype=tf.float32, name='detection_multiclass_scores')
nweffdet.py:            detection_scores: TensorSpec(shape=(1, 100), dtype=tf.float32, name='detection_scores')
nweffdet.py:         raw_detection_boxes: TensorSpec(shape=(1, 49104, 4), dtype=tf.float32, name='raw_detection_boxes')
nweffdet.py:    detection_anchor_indices: TensorSpec(shape=(1, 100), dtype=tf.float32, name='detection_anchor_indices')

nweffdet.py: detection key - detection_boxes
nweffdet.py: detection key - detection_multiclass_scores
nweffdet.py: detection key - detection_scores
nweffdet.py: detection key - detection_anchor_indices
nweffdet.py: detection key - detection_classes
nweffdet.py: detection key - raw_detection_boxes
nweffdet.py: detection key - raw_detection_scores
nweffdet.py: detection key - num_detections

"""

import sys
import os, os.path
import io
import time
import getopt
import string
import math
import random
import base64

import numpy as np
import tensorflow as tf
import PIL
from PIL import Image
from PIL import ImageTk

import tkinter
from tkinter import *
from tkinter import ttk


debug = False
show_image = False
precision_test = False

class_names_fb = ['n/a', 'females_front', 'females_back', 'males_front', 'males_back', 'enbys_front', 'enbys_back', 'predators_front', 'predators_back', 'resources_front', 'resources_back'] 
class_names_fb_short = ['n/a', 'females', 'females', 'males', 'males', 'enbys', 'enbys', 'predators', 'predators', 'resources', 'resources'] 
class_names = ['n/a', 'females', 'males', 'enbys', 'predators', 'resources'] 
bounding_box_colors = {"females": "white", "males": "blue", "enbys": "purple", "predators": "red", "resources": "green",
                       "females_front": "white", "females_back": "white", "males_front": "blue", "males_back": "blue", "enbys_front": "purple", "enbys_back": "purple",
                       "predators_front": "red", "predators_back": "red", "resources_front": "green", "resources_back": "green"}

# filename = "/home/hankm/cube/training/trainers-set2-jpg/females/training-female-1f31d-hd-0.5986-512x512.jpg"
# filename = "/home/hankm/nwmodel/test.png"
filename = "/home/hankm/cube-training/Neckerworld-test-1280x720.jpg"

def Usage():
    print("Usage: nweffdet.py -d -f filename -h -p -s -v --debug --file=filename --help --precision --show --version")

# Create our display window
def display_window(w,h):

    global visual
    global visual_id
    
    window_title = "NW Efficient Detect"
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

    ttk.Button(mainframe, text="Quit", command=quit_play, style="C.TButton").grid(column=0, row=1, sticky=E)

    visual = tkinter.Canvas(mainframe, width=w, height=h, bg='#000')
    visual.grid(column=0, row=0, sticky=(W, E))
    xc = 0
    yc = 0
    visual_id = visual.create_image(xc, yc, anchor=NW, image=None)
    
    # Assign keys to this window
    root.bind("<Escape>", quit_play)

    # Window handlers
    root.protocol("WM_DELETE_WINDOW", display_window_closing)

    return root

def display_window_closing():
    window.destroy()
    sys.exit()
    
def quit_play(*args):
    window.destroy()
    sys.exit()

# Create our detected bounding boxes
def update_bounding_boxes(w, h, results):
    visual.delete("boxes")
    labeltext = ""
    scorenumber = 0.0
    for r in results["predictions"]:
        classname = r["classname"]
        pcolor = bounding_box_colors[classname]
        score = r["score"]
        box = r["bounding_vertices"]
        id = visual.create_rectangle(box[0] * w, box[1] * h, box[2] * w, box[3] * h, width = 4, outline = pcolor, tags = "boxes")
        if labeltext == "":
            labeltext = classname
            scorenumber = score
            visual.delete("results")
            visual.create_text(10, 10, anchor=tkinter.NW, text=labeltext + " -- " + ("%5.2f" % score), fill="white", font=('Helvetica 15 bold'), tags = "results")
    return

# Run through all images to check precision
def p_test():

    global window
    global visual

    print("nweffdet.py: Calculating precision")
    
    bbcsv = "/home/hankm/cube/training/training-bounding-box-set2.csv"
    dir = "/home/hankm/cube/training/trainers-set2-jpg"
    threshold = 0.45
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
            print("nweffdet.py: Predicted class name: {0:>9s}, score {1:0.3f} {2}".format(r["classname"], r["score"], r["bounding_vertices"]))
            classname = r["classname"]
            score = r["score"]
            if score >= threshold and cn[0:4] == classname[0:4]:
                p += 1.0
                
        if samples > 999.9:
            break

        print("nweffdet.py: Precision %0.3f using threshold %0.2f with %d samples" % (p/samples, threshold, int(samples)))

        if show_image:
            imgp = PIL.Image.open(image_file)
            width = imgp.size[0]
            height = imgp.size[1]
            imgtk = ImageTk.PhotoImage(imgp)
            visual.itemconfig(visual_id, image=imgtk)
            update_bounding_boxes(width, height, results)

            # Update the window
            window.update()
            time.sleep(0.5)
            
    precision = p/samples

    print("nweffdet.py: Precision %0.3f using threshold %0.2f with %d samples" % (precision, threshold, int(samples)))
          
    return

GRAPH_PB_PATH = '/home/hankm/cube-training/models/efficientdet-d0/saved_cube_model/saved_model'

model = tf.saved_model.load(GRAPH_PB_PATH, tags=None, options=None)

print("nweffdet.py: Tensorflow loaded model %s" % type(model))

print("nweffdet.py: Model signature keys: %s" % list(model.signatures.keys()))
infer = model.signatures["serving_default"]

so = infer.structured_outputs

for key, value  in so.items():
    print("nweffdet.py:%28s: %s" % (key, value))

#
# Make a prediction
#

def predict(image_filename):
    
    global img
    
    # Loads the image into memory
    print("nweffdet.py: predict image %s" % image_filename)
    img = np.array(PIL.Image.open(image_filename))
    # The input needs to be a tensor, convert it using `tf.convert_to_tensor`.
    input_tensor = tf.convert_to_tensor(img)
    # The model expects a batch of images, so add an axis with `tf.newaxis`.
    input_tensor = input_tensor[tf.newaxis, ...]

    # Call on the model, do the inference
    detections = model(input_tensor)

    num_detections = int(detections.pop('num_detections'))
    detections = {key: value[0, :num_detections].numpy() for key, value in detections.items()}
    detections['num_detections'] = num_detections

    # detection_classes should be ints.
    detections['detection_classes'] = detections['detection_classes'].astype(np.int64)

    if debug:
        print('')
    for key in detections.keys():
        if debug:
            print("nweffdet.py: detection key - %s" % key)
        if debug:
            print('Prediction key is "%s"' % key)
            print('Value returned:')
            print(detections[key])
            print('End of key\n')
        if key == "detection_scores":
            detection_scores = tf.make_ndarray(tf.make_tensor_proto(detections[key]))[:]
        if key == "detection_boxes":
            detection_boxes = tf.make_ndarray(tf.make_tensor_proto(detections[key]))[:]
        if key == "detection_classes":
            detection_classes = tf.make_ndarray(tf.make_tensor_proto(detections[key]))[:]

    # print("nweffdet.py: detection_classes - %s" % detection_classes)
    # print("nweffdet.py: detection_scores  - %s" % detection_scores)
    # print("nweffdet.py: detection_boxes   - %s" % detection_boxes)

    if not precision_test:
        for i in range(15):
            print("nweffdet.py: %15s %5.3f [%3.2f, %3.2f, %3.2f, %3.2f}" % (class_names_fb[detection_classes[i]], detection_scores[i], detection_boxes[i][0], detection_boxes[i][1], detection_boxes[i][2], detection_boxes[i][3] ))
    else:
        print("nweffdet.py: %15s %5.3f [%3.2f, %3.2f, %3.2f, %3.2f}" % (class_names_fb[detection_classes[0]], detection_scores[0], detection_boxes[0][0], detection_boxes[0][1], detection_boxes[0][2], detection_boxes[0][3] ))
            
    predictions = {"predictions": []}
    if precision_test:
        threshold = 0.45
    else:
        threshold = 0.75
    box_list = []
    
    for i in range(detection_scores.shape[0]):
        score = detection_scores[i]
        if score < threshold:
            continue
        classid = detection_classes[i]
        if precision_test:
            classname = class_names_fb[classid]
        else:
            classname = class_names_fb_short[classid]
        box = [detection_boxes[i][1], detection_boxes[i][0], detection_boxes[i][3], detection_boxes[i][2]]
        
        # Check for overlap and discard if needed
        xc = (box[2]+box[0])/2.0
        yc = (box[3]+box[1])/2.0
        overlap = False
        for b in box_list:
            # print("nweffdet.py: xc %0.3f yc %0.3f in list %s, new %s" % (xc, yc, b, box))
            if xc > b[0] and xc < b[2] and yc > b[1] and yc < b[3]:
                print("nweffdet.py: predict - Discarding box %9s %0.3f [%0.3f, %0.3f, %0.3f, %0.3f]" % (classname, score, box[0], box[1], box[2], box[3]))
                overlap = True
                break
        if not overlap:
            print("nweffdet.py: predict - Adding box     %9s %0.3f [%0.3f, %0.3f, %0.3f, %0.3f]" % (classname, score, box[0], box[1], box[2], box[3]))
            box_list.append(box)
            predictions["predictions"].append({"classname": classname, "score": score, "bounding_vertices": box})

    if debug and len(predictions["predictions"]) > 0:
        print("nweffdet.py: predictions %s" % predictions)

    return predictions
#
# Main program starts here
#

def main():

    global window
    
    if precision_test and show_image:
        # We will display our image on the screen
        width = 512
        height = 512
        window = display_window(width, height)

    if precision_test:
        p_test()
        if show_image:
            window.destroy()
        return

    results = predict(filename)

    if debug:
        for r in results["predictions"]:
            print("nweffdet.py: Predicted class name: {0:>9s}, score {1:0.3f} {2}".format(r["classname"], r["score"], r["bounding_vertices"]))

    if show_image:
        # Now display our image on the screen
        imgp = PIL.Image.open(filename)
        width = imgp.size[0]
        height = imgp.size[1]
        window = display_window(width, height)
        imgtk = ImageTk.PhotoImage(imgp)
        visual.itemconfig(visual_id, image=imgtk)
        update_bounding_boxes(width, height, results)
        
        # Update the window
        window.mainloop()
        
if __name__=='__main__':

    #
    # Get options and call the main program
    #                                                                                            

    try:
        options, args = getopt.getopt(sys.argv[1:], 'df:hpsv', ['debug', 'file=', 'help', 'precision', 'show', 'version'])
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
        if o in ("-s", "--show"):
            show_image = True
        if o in ("-v", "--version"):
            print("nweffdet.py: Version 1.0")
            sys.exit()
        
    main()

    sys.exit()
