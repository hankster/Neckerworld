#! /usr/bin/python3
"""
nwvision.py -- A Python program to run inference on an image.

Sample usage:

 nwvision.py

Complete specification:

 nwvision.py -d -f filename -h -v --debug --file=filename --help --version

 where

 -d, --debug          Turn debug statements on
 -f, --file           Input filename
 -h, --help           Print usage information
 -v, --version        Report program version

Copyright (2021) H. S. Magnuski
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
import base64

import cv2
import tensorflow as tf

debug = False
precision_test = False

filename = "/home/hankm/cube/training/trainers-set2-jpg/females/training-female-1f31d-hd-0.5986-512x512.jpg"
filename = "/home/hankm/nwmodel/test.png"

def Usage():
    print("Usage: nwvision.py -d -f filename -h -v --debug --file=filename --help --version")

# Run through all images to check precision
def p_test():

    print("nweffdet.py: Calculating precision")
    
    bbcsv = "/home/hankm/cube/training/training-bounding-box-set2.csv"
    dir = "/home/hankm/cube/training/trainers-set2-jpg"
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
            print("nweffdet.py: Predicted class name: {0:>9s}, score {1:0.3f} {2}".format(r["classname"], r["score"], r["bounding_vertices"]))
            classname = r["classname"]
            score = r["score"]
            if score >= threshold and cn[0:4] == classname[0:4]:
                p += 1.0
                
        if samples > 999.9:
            break

        print("nweffdet.py: Precision %0.3f using threshold %0.2f with %d samples" % (p/samples, threshold, int(samples)))

        
    precision = p/samples

    print("nweffdet.py: Precision %0.3f using threshold %0.2f with %d samples" % (precision, threshold, int(samples)))
          
    return

GRAPH_PB_PATH = 'nw_model'

model = tf.saved_model.load(GRAPH_PB_PATH, tags=None, options=None)

print("nwvision.py: Tensorflow loaded model %s" % type(model))

print("nwvision.py: Model signature keys: %s" % list(model.signatures.keys()))
infer = model.signatures["serving_default"]

so = infer.structured_outputs

for key, value  in so.items():
    print("nwvision.py:%28s: %s" % (key, value))

#
# Make a prediction
#

def predict(image_filename):
    
    export_path = "/home/hankm/nwmodel/nw_model"

    # Loads the image into memory
    #with open(image_filename, "rb") as content_file:
    #    content = content_file.read()

    img = cv2.imread(image_filename)
    flag, bts = cv2.imencode('.jpg', img)
    inp = [bts.tobytes()]
    p = infer(key=tf.constant('nw_cube'), image_bytes=tf.constant(inp))

    
    if debug:
        print('')
    for key in p.keys():
        if debug:
            print('Prediction key is "%s"' % key)
            print('Value returned:')
            print(p[key])
            print('End of key\n')
        if key == "detection_scores":
            detection_scores = tf.make_ndarray(tf.make_tensor_proto(p[key]))[0,:]
        if key == "detection_boxes":
            detection_boxes = tf.make_ndarray(tf.make_tensor_proto(p[key]))[0,:]
        if key == "detection_classes_as_text":
            detection_classes_as_text = tf.make_ndarray(tf.make_tensor_proto(p[key]))[0,:]
    
    predictions = {"predictions": []}
    threshold = 0.6
    
    for i in range(detection_scores.shape[0]):
        score = detection_scores[i]
        if score < threshold:
            continue
        classname = detection_classes_as_text[i].decode("UTF-8")
        # classname change
        if classname == "others":
            classname = "enbys"
        bounding_vertices = [detection_boxes[i][1], detection_boxes[i][0], detection_boxes[i][3], detection_boxes[i][2]]
        predictions["predictions"].append({"classname": classname, "score": score, "bounding_vertices": bounding_vertices})

    if len(predictions["predictions"]) > 0:
        print("nwvision.py: predictions %s" % predictions)

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
        print("nwvision.py: Predicted class name: {}".format(r["classname"]))
        print("nwvision.py: Predicted class score: {}".format(r["score"]))
        bounding_box = r["bounding_vertices"]
        print("nwvision.py: Normalized Vertices: %s" % bounding_box)

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
            print("nwvision.py: Version 1.0")
            sys.exit()
        
    main()

    sys.exit()
