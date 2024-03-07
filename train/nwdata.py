#! /usr/bin/python3
"""
nwdata.py -- Python utilities for data management. 

Sample usage:

 nwdata.py

Complete specification:

 nwdata.py -a -c -d -f filename -g -h -s -v --audit --csv --debug --file=filename --generate --help --show --version

 where

 -a, --audit          Audit training images and training box list.
 -c, --csv            Create a tab-separated list of training images, classes, bounding boxes and corners
 -d, --debug          Turn debug statements on
 -f, --file           Input filename (unused)
 -g, --generate       Generate cube json files from a csv list (input: cubedata.csv, output: json files in cubes)
 -h, --help           Print usage information
 -s, --show           Show bounding boxes on the images using the bounding box list
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
import json
import cv2

debug = False
generate = False
show_boxes = False
audit_images = False
csv_creation = False

training = "../training"
tcubes = training + "/cubes"
tcubestest = training + "/cubestest"
tcubedata = training + "/cubedata.csv"
tcubedatatest = training + "/cubedatatest.csv"
tboxall = training + "/training-bounding-box-all.txt"
tboxcsv = training + "/training-bounding-box-all.csv"
females_list = training + "/names/female-names.txt"
males_list = training + "/names/male-names.txt"
enbies_list = training + "/names/enby-names.txt"
predators_list = training + "/names/predator-names.txt"
resources_list = training + "/names/resource-names.txt"

res = 512
assets = "../assets"
emoticons = "tw-emoticons-%d" % res
patches = "patches"
patches_assets = assets + '/' + patches
surface_templates = "surfaces"
surface_assets = assets + '/' + surface_templates
textures = "textures"
texture_training = training + '/' + textures

# bounding_box = training + "/" + "training-bounding-box.txt"
bounding_box = training + "/" + "training-bounding-box-tmp.txt"

# Some standard texture mappings
# Standard face order is Front, Top, Back, Bottom, Left, Right
cube_texture_map_2_panes =  [0.0, 0.0, 0.5, 0.0, 0.5, 1.0, 0.0, 1.0,
                             0.5, 0.0, 1.0, 0.0, 1.0, 1.0, 0.5, 1.0,
                             0.5, 0.0, 1.0, 0.0, 1.0, 1.0, 0.5, 1.0,
                             0.5, 0.0, 1.0, 0.0, 1.0, 1.0, 0.5, 1.0,
                             0.5, 0.0, 1.0, 0.0, 1.0, 1.0, 0.5, 1.0,
                             0.5, 0.0, 1.0, 0.0, 1.0, 1.0, 0.5, 1.0]

cube_texture_map_5_panes  = [0.0, 0.0, 0.2, 0.0, 0.2, 1.0, 0.0, 1.0,
                             0.4, 0.0, 0.6, 0.0, 0.6, 1.0, 0.4, 1.0,
                             1.0, 0.0, 1.0, 1.0, 0.8, 1.0, 0.8, 0.0,
                             1.0, 0.0, 1.0, 1.0, 0.8, 1.0, 0.8, 0.0,
                             0.2, 1.0, 0.2, 0.0, 0.4, 0.0, 0.4, 1.0,
                             0.8, 0.0, 0.8, 1.0, 0.6, 1.0, 0.6, 0.0]

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
    print("Usage: nwdata.py -d -f filename -g -h -s -v --debug --file=filename --generate --help --show --version")

# Create a texture color patch - this code is found in /setup/nwu.py. Used here for convenience.
# Create an image with a solid color
# convert -size 100x100 xc:#990000 whatever.png
# convert -size 100x100 xc:rgb\(0,255,0\) whatever.png
# convert -size 100x100 xc:rgba\(255,0,0,0.4\) whatever.png
# A "patch" is a pure color res x res
# A "surface" is a greyscale res x res pattern
# A "surface_patch" is a composite (multiply operation) of the surface and the patch
# A "texture" is a n x res x res icon appended to a surface patch
def cube_color_patch(color, surface):

    red   = int(color[0] * 255.0)
    green = int(color[1] * 255.0)
    blue  = int(color[2] * 255.0)

    patch_name = patches_assets + '/patch-%03d.%03d.%03d-%dx%d.png' % (red, green, blue, res, res)
    if not os.path.exists(patch_name):
        cmd = '/usr/bin/convert -size %dx%d xc:rgb\(%d,%d,%d\) %s' % (res, res, red, green, blue, patch_name)
        if debug:
            print(cmd)
        os.system(cmd)
    if surface != "" :
        surface_name = surface_assets + '/' + surface + '.png'
        surface_patch_name = patches_assets + '/' + surface + '-patch-%03d.%03d.%03d-%dx%d.png' % (red, green, blue, res, res)
        if not os.path.exists(surface_patch_name):
            cmd = '/usr/bin/convert %s %s -compose multiply -composite %s' % (surface_name, patch_name, surface_patch_name)
            if debug:
                print(cmd)
            os.system(cmd)
        return surface_patch_name
    return patch_name

# Create a texture patch - this code is found in /setup/nwu.py. Used here for convenience.
# Create a new texture patch for a face cube
# Horizontal append. Use -append for vertical
# convert in-1.jpg in-5.jpg in-N.jpg +append out-in1-plus-in5-and-inN.jpg
def cube_color_texture(cube_emoticon, cube_player, patch_filename):
    cube_filename = assets + '/' + emoticons + '/' + cube_emoticon + '-%dx%d.png' % (res, res)
    texture_filename = texture_training + '/' + cube_emoticon + '-texture.png'
    if not os.path.exists(texture_filename):
        if cube_player == 'predator':
            # Predators get a n x res x res map
            cmd = '/usr/bin/convert %s %s %s %s %s %s +append %s' % (cube_filename, patch_filename, patch_filename, patch_filename, patch_filename, patch_filename, texture_filename)
        else:
            # All others get a 2 x res x res map
            cmd = '/usr/bin/convert %s %s +append %s' % (cube_filename, patch_filename, texture_filename)
            if debug:
                print(cmd)
            os.system(cmd)
    return texture_filename

# Show the bounding boxes on the image files
def show_box_images():

    players = ["male", "female", "enby", "predator", "resource"]
    
    with open(bounding_box, 'r') as f:
        boxes = f.read().splitlines()
        for box in boxes:
            box = box.replace("'", '"')
            json_box = json.loads(box)
            filename_base = json_box["trainee_file"]
            b = json_box["bounding_box"]
            c = json_box["corners"]
            
            for p in players:
                if p in filename_base:
                    directory = p + "s"
                    if directory == "enbys":
                        directory = "enbies"
            
            fn = training + "/trainers-jpg/" + directory + "/" + filename_base + ".jpg"

            x1 = int(b[0])
            y1 = int(b[1])
            x2 = int(b[2])
            y2 = int(b[3])

            img = cv2.imread(fn)
            color = (255, 255, 255)
            thickness = 1
            if x1 < 0 or y1 < 0 or x2 > 512 or y2 > 512:
                print("nwdata.py: Bounding box error (%3d, %3d), (%3d, %3d)" % (x1, y1, x2, y2))
            cv2.rectangle(img, (x1, y1), (x2, y2), color, thickness)

            if debug:
                # Font
                font = cv2.FONT_HERSHEY_SIMPLEX 
                # Font scale 
                font_scale = 1
                # Font color
                font_color = (0, 255, 0)
                # Font thickness
                font_thickness = 2
            
                for crnr in range(8):
                    cx = c[crnr*2]
                    cy = c[(crnr*2)+1]
                    ctext = chr(ord('0') + crnr)
                    # Using cv2.putText() method 
                    image = cv2.putText(img, ctext, (cx, cy), font, font_scale, font_color, font_thickness, cv2.LINE_AA) 

            cv2.imshow(filename_base, img)
            if cv2.waitKey(0) & 0xFF == 27:
                return
            cv2.destroyWindow(filename_base)
            
    return

# Audit files in the training directories
# Verify that each image in the trainer directories have a match in the bounding box list
def image_audit():

    media = ".png"
    images = []
    edict = {}
    with open(tboxall, 'r') as f:
        boxes = f.read().splitlines()
        for box in boxes:
            box = box.replace("'", '"')
            json_box = json.loads(box)
            filename_base = json_box["trainee_file"]
            images.append(filename_base + media)

            # training-enby-1f60a-ln-5.9509-512x512
            em = filename_base[-23:-18]
            if not em in edict:
                edict[em] = 1
            else:
                edict[em] += 1
                
    # Needs train-pngs.txt or train-jpgs.txt.
    # Try find . -name "*.jpg" -print > train-jpgs.txt in direxctory training/trainers-jpg.
    with open("train-" + media[-3:] + "s.txt", 'r') as j:
        imgs = j.read().splitlines()
        for i in imgs:
            im = i.split("/")
            fn = im[2]
            if not fn in images:
                print("# nwdata.py: %s not in box file" % fn)
                print("rm -f %s" % (im[1] + "/" + fn))
    print("nwdata.py: Found %d box dictionaries" % len(images))
    print("nwdata.py: Found %d %s images" % (len(imgs), media))
    for em in edict:
        if edict[em] != 80:
            print("nwdata.py: Image count for %s is %d" % (em, edict[em]))
    
# Create a bounding box .csv from the bounding box dictionary file
def create_csv():

    media = ".jpg"
    csv_lines = []
    with open(tboxall, 'r') as f:
        boxes = f.read().splitlines()
        for box in boxes:
            box = box.replace("'", '"')
            json_box = json.loads(box)
            filename_base = json_box["trainee_file"]
            box_xy = json_box["bounding_box"]
            corners = json_box["corners"]
            p = filename_base.split("-")
            player = p[1]
            csv = "%s\t%s\t%s\t%s" % (filename_base + media, player, box_xy, corners)
            csv_lines.append(csv)

    with open(tboxcsv, 'w') as f:
        for csvline in csv_lines:
            f.write(csvline + "\n")


# Generate cube json files from a .csv list
def cube_generate():

    if debug:
        rtxt = open("resource-check.txt", 'w')
    
    with open(tcubedata, 'r') as f:
        cubecsv = f.read().splitlines()
        for cube in cubecsv:
            c = cube.split('\t')
            
            dataset = c[0]
            cube_index = 0
            cube_player = c[1]
            cube_uuid = c[2]
            cube_emoticon = c[3]
            cube_firstname = c[4]
            cube_active = True
            cube_display = True
            cube_scale_factor = float(c[5])
            cube_type = int(c[6])
            cube_color_class = c[7]
            cube_color = [float(c[8]), float(c[9]), float(c[10]), 1.0]
            cube_material = int(c[11])
            cube_surface = c[12]
            cube_texture_index = 2

            if cube_player=='male':
                cube_texture_map = cube_texture_map_2_panes
            elif cube_player=='female':
                cube_texture_map = cube_texture_map_2_panes
            elif cube_player=='enby':
                cube_texture_map = cube_texture_map_6_panes
            elif cube_player=='predator':
                cube_texture_map = cube_texture_map_6_panes
            else:
                cube_texture_map = cube_texture_map_resource

            cube_parameters = {}
            spatial_position = [float(c[13]), float(c[14]), float(c[15])]
            spatial_rotation = [0.0, 0.0, 0.0]
            spatial_gaze = [0.0, 0.0]
            spatial_radius = float(c[16])
            spatial_destination = [0.0, 0.0, 0.0]
            spatial_velocity = 0.0
            resource_energy = float(c[17])
            texture_filename = c[18]

            patch_filename = cube_color_patch(cube_color, cube_surface)
            texture_filename = cube_color_texture(cube_emoticon, cube_player, patch_filename)
            
            cube_dict = {"dataset": dataset,
                         "cubes": [{"cube_index": cube_index, "cube_player": cube_player, "cube_uuid": cube_uuid, "cube_emoticon": cube_emoticon, "cube_firstname": cube_firstname,
                         "cube_active": cube_active, "cube_display": cube_display, "cube_scale_factor": cube_scale_factor,
                         "cube_type": cube_type, "cube_color_class": cube_color_class, "cube_color": cube_color, "cube_material": cube_material,
                         "cube_surface": cube_surface, "cube_texture_index": cube_texture_index, "cube_texture_map": cube_texture_map,
                         "cube_parameters": cube_parameters,
                         "spatial_position": spatial_position, "spatial_rotation": spatial_rotation, "spatial_gaze": spatial_gaze, "spatial_radius": spatial_radius,
                         "spatial_destination": spatial_destination, "spatial_velocity": spatial_velocity,
                         "resource_energy": resource_energy}],
                         "textures": [{"texture_index": 2, "texture_filename": texture_filename}]
            }

            cube_str = json.dumps(cube_dict)
            cube_filename = tcubes + "/training-" + cube_player + "-" + cube_emoticon + ".json"
            with open(cube_filename, 'w') as j:
                j.write(cube_str)

            if debug and cube_player == "resource":
                rtxt.write("%s, %s, %s\n" % (cube_firstname, cube_emoticon, "../" + texture_filename))
                os.system("cp %s '%s'" % ("../" + texture_filename, cube_firstname + ".png"))
                
        if debug:
            rtxt.close()

# Directory tree walk

def walk(start):

    # dirpath is a string for the path to the directory.
    # dirnames is a list of the names of the subdirectories in dirpath (excluding '.' and '..').
    # filenames is a list of the names of the non-directory files in dirpath.
    # Note that the names in the lists contain no path components.
    # To get a full path (which begins with top) to a file or directory in dirpath, do os.path.join(dirpath, name).
    
    for dirpath, dirnames, filenames in os.walk(start):
        filenames.sort()
        return filenames

# Generate a scale factor
# Calculate some cube dimensions
def cube_size():

    cube_minimum = 0.5
    cube_maximum = 1.2
    cube_dimension = random.uniform(cube_minimum, cube_maximum)
    return cube_dimension

#
# Main program starts here
#

def main():

    if generate:
        cube_generate()
        sys.exit()
        
    if show_boxes:
        show_box_images()
        sys.exit()
        
    if audit_images:
        image_audit()
        sys.exit()
        
    if csv_creation:
        create_csv()
        sys.exit()
        
    females = []
    males = []
    enbies = []
    predators = []
    resources = []
    f_index = 0
    m_index = 0
    e_index = 0
    p_index = 0
    r_index = 0
    
    try:
        with open(females_list, 'r') as f:
            females = f.read().splitlines()
        with open(males_list, 'r') as m:
            males = m.read().splitlines()
        with open(enbies_list, 'r') as e:
            enbies = e.read().splitlines()
        with open(predators_list, 'r') as p:
            predators = p.read().splitlines()
        with open(resources_list, 'r') as r:
            resources = r.read().splitlines()
    except IOError as e:
        print("nwdata.py: I/O error -- %s" % e)
        sys.exit(-1)

    # Get a list of existing cube objects in training + "/cubes"
    co = walk(tcubes)

    cubed = []
    cubecsv = []

    # Create a spreadsheet file cubedata.csv as output
    csvf = open(tcubedata, 'w')

    # Step through all .json files in training + "/cubes"
    for coj in co:
        print("nwdata.py: Extracting %s" % coj)
        with open(tcubes + "/" + coj, 'r') as c:
            cs = c.read()
        cube = json.loads(cs)

        # Make a list of dictionaries that have cube keywords
        cubed.append(cube)

        dataset = cube["dataset"]
        cubes = cube["cubes"]
        c = cubes[0]
        ci = c["cube_index"]
        cube_player = c["cube_player"]
        cube_uuid = c["cube_uuid"]
        cube_emoticon = c["cube_emoticon"]
        cube_firstname = c["cube_firstname"]
        
        # This code not required
        if False:
            # Assign a name to this cube
            if cube_player == "female":
                cube_firstname = females[f_index].split(',')[0]
                f_index += 1
            if cube_player == "male":
                cube_firstname = males[m_index].split(',')[0]
                m_index += 1
            if cube_player == "enby":
                cube_firstname = enbies[o_index]
                e_index += 1
            if cube_player == "predator":
                cube_firstname = predators[p_index]
                p_index += 1
            if cube_player == "resource":
                cube_firstname = resources[r_index]
                r_index += 1

        cube_scale_factor = c["cube_scale_factor"]
        cube_type = c["cube_type"]
        cube_color_class = c["cube_color_class"]
        cube_color = c["cube_color"]
        cube_material = c["cube_material"]
        cube_surface = c["cube_surface"]
        spatial_position = [0.0, cube_scale_factor + 0.001, 0.0]
        spatial_radius = math.sqrt(2.0 * cube_scale_factor * cube_scale_factor)
        resource_energy = c["resource_energy"]
        t = cube["textures"]
        texture_filename = t[0]["texture_filename"]

        csvf.write("%s\t%s\t%s\t%s\t%s\t%0.5f\t%d\t%s\t%0.5f\t%0.5f\t%0.5f\t%d\t%s\t%0.5f\t%0.5f\t%0.5f\t%0.5f\t%0.5f\t%s\n"
                   % (dataset,cube_player,cube_uuid,cube_emoticon,cube_firstname,cube_scale_factor,cube_type,cube_color_class,cube_color[0],cube_color[1],cube_color[2],cube_material,cube_surface,spatial_position[0],spatial_position[1],spatial_position[2],spatial_radius,resource_energy,texture_filename))

    csvf.close()

    sys.exit()

if __name__=='__main__':

    #
    # Get options and call the main program
    #                                                                                            

    try:
        options, args = getopt.getopt(sys.argv[1:], 'acdf:ghsv', ['audit', 'csv', 'debug', 'file=', 'generate', 'help', 'show', 'version'])
    except getopt.GetoptError:
        Usage()
        sys.exit(-1)

    for o, a in options:
        if o in ("-a", "--audit"):
            audit_images = True
        if o in ("-c", "--csv"):
            csv_creation = True
        if o in ("-d", "--debug"):
            debug = True
        if o in ("-f", "--file"):
            filename = a
        if o in ("-g", "--generate"):
            generate = True
        if o in ("-h", "--help"):
            Usage()
            sys.exit()
        if o in ("-s", "--show"):
            show_boxes = True
        if o in ("-v", "--version"):
            print("nwdata.py Version 1.0")
            sys.exit()
        
    main()

    sys.exit()
