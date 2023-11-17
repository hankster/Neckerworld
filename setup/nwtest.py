#! /usr/bin/python3
"""
nw.py -- A Python program to create JSON files for cube.cpp

Sample usage:

 nw.py

Complete specification:

 nw.py -d -f filename -h -v --debug --file=filename --help --version

 where

 -d, --debug          Turn debug statements on
 -f, --file           Input filename
 -h, --help           Print usage information
 -s, --set            Dataset
 -v, --version        Report program version

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

debug = False
filename = ""
dataset = "NW Standard"

# Resolution for patches, surfaces and textures
res = 512

# Location of all our graphical assets
assets = "../assets"
emoticons = "tw-emoticons-%d" % res
patches = "patches"
patches_assets = assets + '/' + patches
surface_templates = "surfaces"
surface_assets = assets + '/' + surface_templates
textures = "textures"
texture_assets = assets + '/' + textures

# Number of grounds
grounds_used = 2

# List of locations [ (x, y, z, r) , ... ]
locations = []

# Lists of males, females
males = []
males_used = []
males_file = assets + "/males.txt"
females = []
females_used = []
females_file = assets + "/females.txt"

# Lists of predators
predators = []
predators_used = []
predators_file = assets + "/predators.txt"

# Lists of resources
resources = []
resources_used = []
resources_file = assets + "/resources.txt"

# Lists of surfaces
surfaces = []
surfaces_used = []
surfaces_file = assets + "/surfaces.txt"

# These are  the top-level lists in the JSON dataset.
cubeList = []
groundList = []
lightList = []
materialList = []
textureList = []
wireList = []

# Necker World Characters - Players in this universe
# Blue -  male
# White - female
# Yellow - enby
# Red -   predator
# Green - resource
nw_players = ['male', 'female', 'enby', 'predator', 'resource']
nw_players_colors = {'male':'b', 'female':'w', 'enby':'p', 'predator':'r', 'resource':'g'}
nw_characters = ['b', 'w', 'p', 'g','r']
nw_player_count = 5
# Weights in this list determines the distribution
# [male, female, enby, resource, predator]
nw_distribution = [5, 4, 1, 3, 2]

# Colors
color_red = [1.0, 0.0, 0.0, 1.0]
color_green = [0.0, 1.0, 0.0, 1.0]
color_blue = [0.0, 0.0, 1.0, 1.0]
color_yellow = [1.0, 1.0, 0.0, 1.0]
color_purple = [1.0, 0.0, 1.0, 1.0]
color_black = [0.0, 0.0, 0.0, 1.0]
color_white = [1.0, 1.0, 1.0, 1.0]
color_grey_025 = [0.25, 0.25, 0.25, 1.0]
color_grey_050 = [0.50, 0.50, 0.50, 1.0]
color_grey_075 = [0.75, 0.75, 0.75, 1.0]

# Greenish background color
color_green_mud = [0.25490, 0.313725, 0.282352, 1.0]

# Light colors and intensities
light0 = [5.0, 10.0, 5.0]
light1 = [-5.0, 10.0, 5.0]
light_color_orange = [0.98, 0.725, 0.278]
light_color_white = [1.0, 1.0, 1.0]
light_000 = [0.0, 0.0, 0.0]
light_025 = [0.25, 0.25, 0.25]
light_050 = [0.5, 0.5, 0.5]
light_075 = [0.75, 0.75, 0.75]
light_100 = [1.0, 1.0, 1.0]
light_ambient = [0.3, 0.3, 0.3]
light_diffuse = [0.8, 0.8, 0.8]
light_specular = [0.2, 0.2, 0.2]

# Some standard materials
material_plain_ambient = [1.0, 1.0, 1.0]
material_plain_diffuse = [1.0, 1.0, 1.0]
material_plain_specular = [0.0, 0.0, 0.0]
material_plain_shininess = 0.0
material_highlight_ambient = [0.0, 0.0, 0.0]
material_highlight_diffuse = [0.8, 0.8, 0.8]
material_highlight_specular = [0.5, 0.5, 0.5]
material_highlight_shininess = 0.25

# A greenish groundplane
material_green = [5.0/255.0, 159.0/255.0, 84.0/255.0]

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

ground_texture_map_reference = [0.0, 0.0, 20.0, 0.0, 20.0, 20.0, 0.0, 20.0]
ground_texture_map_reference_wide = [0.0, 0.0, 100.0, 0.0, 100.0, 100.0, 0.0, 100.0]
ground_texture_map_background = [0.875, 1.0, 0.125, 1.0, 0.125, 0.0, 0.875, 0.0]

def Usage():
    print("Usage: nw.py -d -f filename -h -v --debug --file=filename --help --version")

standard_test = ['male', 'female', 'enby', 'predator', 'resource']
standard_uuids = ['1354b75f-9ca5-4da5-80b2-8f0056f9e08e',
                  'a1a98c6d-e123-4c7e-8cd0-9dabfdb60e22',
                  'd08f9dc2-8c83-4fba-b4ab-af1d1ae49ba6',
                  'b5a49a67-60cf-4b10-b136-04f28d986368',
                  'cd9d80e9-40d7-49f6-b92b-ac41b4161fff']
standard_emoticons = ['1f609', '1f62f', '1f633', '1f63d', '1f370']
standard_surfaces = ['', '', '', 'surface-8-snakeskin-512x512', '']
standard_scale_factors = [1.1660442497464159,
                          0.8232279164407639,
                          0.7604819221086675,
                          1.1587331717166975,
                          0.8258545191808149]
standard_colors = [[0.12941176470588237, 0.1411764705882353, 0.7411764705882353, 1.0],
                   [0.7098039215686275, 0.7098039215686275, 0.7098039215686275, 1.0],
                   [0.6823529411764706, 0.16862745098039217, 0.9607843137254902, 1.0],
                   [0.8980392156862745, 0.1843137254901961, 0.26666666666666666, 1.0],
                   [0.058823529411764705, 1.0, 0.1411764705882353, 1.0]]
standard_spatial_positions = [[3.6059312918693376, 1.1760442497464159, -2.5714391267882064],
                              [0.9866604474932856, 0.833227916440764, 3.62627407669257],
                              [5.661916028190252, 0.7704819221086675, 6.960729040553753],
                              [-5.869175668233945, 1.1687331717166975, -1.3895444125031053],
                              [-4.595876890857919, 0.8358545191808149, 7.691108589757597]]
standard_spatial_rotations = [[0.0, 0.2064446700508486, 0.0],
                              [0.0, 5.375748231828279, 0.0],
                              [0.0, -math.pi/4.0, 0.0],
                              [0.0, 0.7880859605606255, 0.0],
                              [0.0, 5.70378068520789, 0.0]]

    # Create a new cube
def new_cube(cube_index, ground_scale_factor):

    global cubeList
    global standard_test
    global standard_uuids
    global standard_emoticons
    global standard_surfaces
    global standard_scale_factors
    global standard_colors
    global standard_spatial_positions
    global standard_spatial_rotations

    cube_player = standard_test[cube_index]
    
    cube_uuid = str(uuid.uuid4())
    # standard_uuids.append(cube_uuid)
    cube_uuid = standard_uuids[cube_index]

    if cube_player=='male':
        cube_emoticon = get_male()
        cube_firstname = "Jack"
        cube_surface = ""
        cube_texture_map = cube_texture_map_2_panes
    elif cube_player=='female':
        cube_emoticon = get_female()
        cube_firstname = "Jill"
        cube_surface = ""
        cube_texture_map = cube_texture_map_2_panes
    elif cube_player=='enby':
        cube_emoticon = "1f633"
        cube_firstname = "Hunter"
        cube_surface = ""
        cube_texture_map = cube_texture_map_6_panes
    elif cube_player=='predator':
        cube_emoticon = get_predator()
        cube_firstname = "Predator"
        cube_surface = get_surface()
        cube_texture_map = cube_texture_map_6_panes
    else:
        cube_emoticon = get_resource()
        cube_firstname = "Food"
        cube_surface = ""
        cube_texture_map = cube_texture_map_resource
    # standard_emoticons.append(cube_emoticon)
    cube_emoticon = standard_emoticons[cube_index]
    # standard_surfaces.append(cube_surface)
    cube_surface = standard_surfaces[cube_index]
    cube_active = True
    cube_display = True
    cube_scale_factor = cube_size()
    # standard_scale_factors.append(cube_scale_factor)
    cube_scale_factor = standard_scale_factors[cube_index]
    cube_type = 3
    cube_color_class = nw_players_colors[cube_player]
    cube_color = cube_colors(cube_color_class)
    # standard_colors.append(cube_color)
    cube_color = standard_colors[cube_index]
    cube_material = 0
    cube_texture_index = grounds_used + cube_index
    cube_parameters = {}

    spatial_position = cube_placement(cube_scale_factor, ground_scale_factor)
    # standard_spatial_positions.append(spatial_position)
    spatial_position = standard_spatial_positions[cube_index]
    spatial_rotation = cube_rotation()
    # standard_spatial_rotations.append(spatial_rotation)
    spatial_rotation = standard_spatial_rotations[cube_index]
    spatial_radius = math.sqrt(2.0 * cube_scale_factor * cube_scale_factor)
    spatial_gaze = [0.0, 0.0]
    spatial_destination = [0.0, 0.0, 0.0]
    spatial_velocity = 0.0

    if cube_player == 'resource':
        resource_energy = 2000.0
    else:
        resource_energy = 100.0
            
    cube = {"cube_index": cube_index, "cube_player": cube_player, "cube_uuid": cube_uuid, "cube_emoticon": cube_emoticon, "cube_firstname": cube_firstname, "cube_active": cube_active, "cube_display": cube_display, "cube_scale_factor": cube_scale_factor,
            "cube_type": cube_type, "cube_color_class": cube_color_class, "cube_color": cube_color, "cube_material": cube_material,
            "cube_surface": cube_surface, "cube_texture_index": cube_texture_index, "cube_texture_map": cube_texture_map,
            "cube_parameters": cube_parameters,
            "spatial_position": spatial_position, "spatial_rotation": spatial_rotation, "spatial_gaze": spatial_gaze, "spatial_radius": spatial_radius,
            "spatial_destination": spatial_destination, "spatial_velocity": spatial_velocity,
            "resource_energy": resource_energy
    }

    cubeList.append(cube)

    return

# Setup textures
def new_texture(texture_index):

    global cubeList
    global textureList

    # This is a ground plane texture for testing only
    if texture_index == 0:
        texture_filename = assets + "/Grey-100.png"
        texture = {"texture_index": texture_index, "texture_filename": texture_filename}
        textureList.append(texture)
        return

    # This is our main reference ground plane
    if texture_index == 1:
        texture_filename = assets + "/texref-%dx%d.png" % (res, res)
        texture = {"texture_index": texture_index, "texture_filename": texture_filename}
        textureList.append(texture)
        return

    # These are textures for all the individual cube players
    t = texture_index - 2
    cube_player = cubeList[t]['cube_player']
    cube_color = cubeList[t]['cube_color']
    cube_emoticon = cubeList[t]['cube_emoticon'] 
    cube_surface = cubeList[t]['cube_surface']
    if cube_player == "enby" :
        texture_filename = assets + "/texture-1f633-3072x512.png"
    else :
        cube_color_filename = cube_color_patch(cube_color, cube_surface)
        texture_filename = cube_color_texture(cube_emoticon, cube_player, cube_color_filename)
    texture = {"texture_index": texture_index, "texture_filename": texture_filename}
    textureList.append(texture)
    return

# Setup colors for our cubes
def cube_colors(c):

    intensity_low = 96
    intensity_high = 255-intensity_low
    rand_r = random.randint(0, intensity_low)
    rand_g = random.randint(0, intensity_low)
    rand_b = random.randint(0, intensity_low)
    rand_w = random.randint(0, intensity_low)

    if c == 'r':
        red = intensity_high + rand_r
        green = rand_g
        blue = rand_b
    elif c == 'g':
        red = rand_r
        green = intensity_high + rand_g
        blue = rand_b
    elif c == 'b':
        red = rand_r
        green = rand_g
        blue = intensity_high + rand_b
    elif c == 'p':
        red = intensity_high + rand_r
        green = rand_g
        blue = intensity_high + rand_b
    elif c == 'y':
        red = intensity_high + rand_r
        green = intensity_high + rand_g
        blue = rand_b
    elif c == 'w':
        red = intensity_high + rand_w
        green = intensity_high + rand_w
        blue = intensity_high + rand_w
    else:
        red = 0
        green = 0
        blue = 0
    
    return [float(red)/255.0, float(green)/255.0, float(blue)/255.0, 1.0]

# Read the list of possible males into a list
def males_list():
    global males
    try:
        with open(males_file) as f:
            males = f.read().splitlines()
    except IOError as e:
        print("nw.py: Unable to read males file %s - %s" % (males_file, e))
        sys.exit()

# Get a unique male from the list
def get_male():
    noface = True
    while noface:
        male = random.choice(males)
        if male not in males_used:
            males_used.append(male)
            return male[0:-4]
    return "nomale"

# Read the list of possible females into a list
def females_list():
    global females
    try:
        with open(females_file) as f:
            females = f.read().splitlines()
    except IOError as e:
        print("nw.py: Unable to read females file %s - %s" % (females_file, e))
        sys.exit()

# Get a unique female from the list
def get_female():
    noface = True
    while noface:
        female = random.choice(females)
        if female not in females_used:
            females_used.append(female)
            return female[0:-4]
    return "nofemale"

# Read the list of possible predators into a list
def predators_list():
    global predators
    try:
        with open(predators_file) as f:
            predators = f.read().splitlines()
    except IOError as e:
        print("nw.py: Unable to read predators file %s - %s" % (predators_file, e))
        sys.exit()

# Get a unique predator from the list
def get_predator():
    nopredator = True
    while nopredator:
        predator = random.choice(predators)
        if predator not in predators_used:
            predators_used.append(predator)
            return predator[0:-4]
    return "nopredator"

# Read the list of possible resources into a list
def resources_list():
    global resources
    try:
        with open(resources_file) as f:
            resources = f.read().splitlines()
    except IOError as e:
        print("nw.py: Unable to read resources file %s - %s" % (resources_file, e))
        sys.exit()

# Get a unique resource from the list
def get_resource():
    noresource = True
    while noresource:
        resource = random.choice(resources)
        if resource not in resources_used:
            resources_used.append(resource)
            return resource[0:-4]
    return "noresource"

# Read the list of possible surfaces into a list
def surfaces_list():
    global surfaces
    try:
        with open(surfaces_file) as f:
            surfaces = f.read().splitlines()
    except IOError as e:
        print("nw.py: Unable to read surfaces file %s - %s" % (surfaces_file, e))
        sys.exit()

# Get a unique surface from the list
def get_surface():
    nosurface = True
    while nosurface:
        surface = random.choice(surfaces)
        if surface not in surfaces_used:
            surfaces_used.append(surface)
            return surface[0:-4]
    return "nosurface"

# Create a texture color patch
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
    cmd = '/usr/bin/convert -size %dx%d xc:rgb\(%d,%d,%d\) %s' % (res, res, red, green, blue, patch_name)
    print(cmd)
    os.system(cmd)
    if surface != "" :
        surface_name = surface_assets + '/' + surface + '.png'
        surface_patch_name = patches_assets + '/' + surface + '-patch-%03d.%03d.%03d-%dx%d.png' % (red, green, blue, res, res)
        cmd = '/usr/bin/convert %s %s -compose multiply -composite %s' % (surface_name, patch_name, surface_patch_name)
        print(cmd)
        os.system(cmd)
        return surface_patch_name
    return patch_name

# Create a new texture patch for a face cube
# Horizontal append. Use -append for vertical
# convert in-1.jpg in-5.jpg in-N.jpg +append out-in1-plus-in5-and-inN.jpg
def cube_color_texture(cube_emoticon, cube_player, patch_filename):
    cube_filename = assets + '/' + emoticons + '/' + cube_emoticon + '-%dx%d.png' % (res, res)
    texture_filename = texture_assets + '/' + cube_emoticon + '-texture.png'
    if cube_player == 'predator':
        # Predators get a n x res x res map
        cmd = '/usr/bin/convert %s %s %s %s %s %s +append %s' % (cube_filename, patch_filename, patch_filename, patch_filename, patch_filename, patch_filename, texture_filename)
    else:
        # All others get a 2 x res x res map
        cmd = '/usr/bin/convert %s %s +append %s' % (cube_filename, patch_filename, texture_filename)
    print(cmd)
    os.system(cmd)
    return texture_filename

# Calculate some cube dimensions
def cube_size():

    cube_minimum = 0.5
    cube_maximum = 1.2
    cube_dimension = random.uniform(cube_minimum, cube_maximum)
    return cube_dimension

# Assign a cube location
def cube_location(cube_scale_factor, ground_scale_factor):

    dimension_minimum = -0.9 * ground_scale_factor
    dimension_maximum =  0.9 * ground_scale_factor
    x = random.uniform(dimension_minimum, dimension_maximum)
    y = cube_scale_factor + 0.01
    z = random.uniform(dimension_minimum, dimension_maximum)
    r = math.sqrt(2 * cube_scale_factor * cube_scale_factor)
    
    return [x, y, z], r

# Check for location conflict and return cube position        
def cube_placement(cube_scale_factor, ground_scale_factor):

    # Try to place a cube without conflict (maximum number of tries)
    placement_attempts = 30
    while placement_attempts > 0:
        
        # Get a trial location for the cube and its radius
        position, radius = cube_location(cube_scale_factor, ground_scale_factor)
        px = position[0]
        py = position[1]
        pz = position[2]
        pr = radius

        # Assume there is not a location conflict
        conflict = False

        # Test if the distance to any existing cube is too short
        for l in locations:
            lx = l[0]
            ly = l[1]
            lz = l[2]
            lr = l[3]
            dx = lx - px
            dz = lz - pz
            dr = math.sqrt(dx*dx + dz*dz)
            if dr < lr+pr:
                conflict = True
                print("nw.py: Conflict in location - dr %4.2f lr+pr % 4.2f" % (dr, lr+pr))
                break

        if not conflict:
            locations.append((px, py, pz, pr))
            return [px, py, pz]
        placement_attempts -= 1

    print("nw.py: Unable to position cube without conflict")
    return [0.0, py, 0.0]

# Assign cube rotation
def cube_rotation():
    
    rotation_x = 0.0
    rotation_y = random.uniform(0.0, 2*math.pi)
    rotation_z = 0.0
    return [rotation_x, rotation_y, rotation_z]

# Create a new JSON data file
def create_JSON():

    global cubeList
    global groundList
    global lightList
    global materialList
    global textureList
    global wireList

    # Create the output JSON filename based on the input .CSV name.
    if filename != "":
        jsonName = filename
    else:
        jsonName = "nwtest.json"
        
    # Initialize JSON object
    data = {"dataset":dataset}

    # These are  the top-level lists in the dataset.
    cubeList = []
    groundList = []
    lightList = []
    materialList = []
    textureList = []
    wireList = []

    # Setup our window
    window_title = "Necker World" + " - " + dataset
    main_window_width = 1280
    main_window_height = 720
    main_window_channels = 4
    window_background_color = color_green_mud
    window = {"window_title": window_title, "main_window_width": main_window_width, "main_window_height": main_window_height, "main_window_channels": main_window_channels, "window_background_color": window_background_color} 
    data["window"] = window

    # Setup the camera position and target
    camera_position = [0.0, 5.0, 14.5]
    camera_target = [0.0, 0.0, 0.0]
    camera_up = [0.0, 1.0, 0.0]
    camera = {"camera_position": camera_position, "camera_target": camera_target, "camera_up": camera_up}
    data["camera"] = camera

    # Setup the grounds
    ground_index = 0
    ground_scale_factor_0 = 10.0
    ground_type = 3
    ground_color = color_green
    ground_material = 1
    ground_texture_index = 1
    ground_texture_map = ground_texture_map_reference
    ground_spatial_position = [0.0, 0.0, 0.0]
    ground_spatial_rotation = [0.0, 0.0, 0.0]
    ground_view_position = camera_position
    ground_view_target = camera_target
    ground_view_up = camera_up
    ground = {"ground_index": ground_index, "ground_scale_factor": ground_scale_factor_0, "ground_type": ground_type, "ground_color": ground_color, "ground_material": ground_material, "ground_texture_index": ground_texture_index, "ground_texture_map": ground_texture_map, "ground_spatial_position": ground_spatial_position, "ground_spatial_rotation": ground_spatial_rotation, "ground_view_position": ground_view_position, "ground_view_target": ground_view_target, "ground_view_up": ground_view_up}
    groundList.append(ground)

    data["grounds"] = groundList

    # Setup cubes
    for cube_index in range(nw_player_count):
        new_cube(cube_index, ground_scale_factor_0)
    data["cubes"] = cubeList

    if debug:
        print(standard_test)
        print(standard_uuids)
        print(standard_emoticons)
        print(standard_surfaces)
        print(standard_scale_factors)
        print(standard_colors)
        print(standard_spatial_positions)
        print(standard_spatial_rotations)

    # Setup lights
    light_index = 0
    light_position = light0
    light_intensity = light_100
    light_ambient = light_025
    light_diffuse = light_075
    light_specular = light_000
    light_constant = 1.0
    light_linear = 0.045
    light_quadratic = 0.0075
    light = {"light_index": light_index, "light_position": light_position, "light_intensity": light_intensity, "light_ambient": light_ambient, "light_diffuse": light_diffuse, "light_specular": light_specular, "light_constant": light_constant, "light_linear": light_linear, "light_quadratic": light_quadratic}
    lightList.append(light)

    light_index = 1
    light_position = light1
    light_intensity = light_100
    light_ambient = light_025
    light_diffuse = light_075
    light_specular = light_000
    light_constant = 1.0
    light_linear = 0.045
    light_quadratic = 0.0075
    light = {"light_index": light_index, "light_position": light_position, "light_intensity": light_intensity, "light_ambient": light_ambient, "light_diffuse": light_diffuse, "light_specular": light_specular, "light_constant": light_constant, "light_linear": light_linear, "light_quadratic": light_quadratic}
    lightList.append(light)
 
    data["lights"] = lightList

    # Setup materials
    material_index = 0
    material_ambient = material_plain_ambient
    material_diffuse = material_plain_diffuse
    material_specular = material_plain_specular
    material_shininess = material_plain_shininess
    material = {"material_index": material_index, "material_ambient": material_ambient, "material_diffuse": material_diffuse, "material_specular": material_specular, "material_shininess": material_shininess}
    materialList.append(material)

    material_index = 1
    material_ambient = material_plain_ambient
    material_diffuse = material_green
    material_specular = material_plain_specular
    material_shininess = material_plain_shininess
    material = {"material_index": material_index, "material_ambient": material_ambient, "material_diffuse": material_diffuse, "material_specular": material_specular, "material_shininess": material_shininess}
    materialList.append(material)

    data["materials"] = materialList

    # Setup textures

    for texture_index in range(nw_player_count+2):
        new_texture(texture_index)

    data["textures"] = textureList

    if debug :
        print(window)
        print(camera)
        print(ground)
        print(cubeList)
        print(groundList)
        print(lightList)
        print(materialList)
        print(textureList)
        print(wireList)

    if debug :
        print(json.dumps(data, indent=4, sort_keys=True))

    # Write the new JSON file
    try:
        with open(jsonName, 'w') as f:
            json.dump(data, f)
    except (IOError, ValueError) as e:
        print("nw.py: error -- %s" % (e.args[0]))
        sys.exit(1)

#
# Main program starts here
#

def main():
    
    print("nw.py -- Create dataset %s for the cube program" % dataset)

    # Read in lists of males, females, predators, resources and surfaces
    males_list()
    females_list()
    predators_list()
    resources_list()
    surfaces_list()

    # Now start creating our big JSON file
    create_JSON()

    return

if __name__=='__main__':

    #
    # Get options and call the main program
    #                                                                                            

    try:
        options, args = getopt.getopt(sys.argv[1:], 'df:hs:v', ['debug', 'file=', 'help', 'set=', 'version'])
    except getopt.GetoptError:
        Usage()
        sys.exit(2)

    for o, a in options:
        if o in ("-d", "--debug"):
            debug = True
        if o in ("-f", "--file"):
            filename = a
        if o in ("-h", "--help"):
            Usage()
            sys.exit()
        if o in ("-d", "--set"):
            dataset = a
        if o in ("-v", "--version"):
            print("nw.py Version 1.0")
            sys.exit()
        
    main()
    sys.exit()
