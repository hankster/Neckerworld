#! /usr/bin/python3
"""
nwu.py -- Python utilities for nw.py and other setup programs

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

debug = False

# Resolution for patches, surfaces and textures
res = 512

# Location of all our graphical assets
assets = "../assets"
training = "../training"
emoticons = "tw-emoticons-%d" % res
patches = "patches"
patches_assets = assets + '/' + patches
surface_templates = "surfaces"
surface_assets = assets + '/' + surface_templates
textures = "textures"
texture_assets = assets + '/' + textures
cubedata = training + '/cubedata.csv'

# List of locations [ (x, y, z, r) , ... ]
locations = []

# Lists of males, females, enbies
males = []
males_used = []
males_file = assets + "/males.txt"
females = []
females_used = []
females_file = assets + "/females.txt"
enbies = []
enbies_used = []
enbies_file = assets + "/enbies.txt"

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

# Necker World Characters - Players in this universe
# Blue - male
# White - female
# Purple - enby
# Red - predator
# Green - resource
nw_players = ['male', 'female', 'enby', 'predator', 'resource']
nw_players_colors = {'male':'b', 'female':'w', 'enby':'p', 'predator':'r', 'resource':'g'}
nw_characters = ['b', 'w', 'p', 'g','r']
nw_enby_emoticons = ['1f60a', '1f61a', '1f633']
nw_enby_textures = ['texture-1f60a-3072x512.png','texture-1f61a-3072x512.png','texture-1f633-3072x512.png']
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
light_ambient = [0.65, 0.65, 0.65]
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

# Create a new cube
def new_cube(cube_index, ground_scale_factor, number_ground_textures):

    cube_uuid = str(uuid.uuid4())
    cube_player = random.choices(nw_players, weights=nw_distribution, k=1)[0]
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
        cube_emoticon = random.choice(nw_enby_emoticons)
        cube_firstname = "Hunter"
        cube_surface = "rainbow"
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
    cube_active = True
    cube_display = True
    cube_scale_factor = cube_size()
    cube_type = 3
    cube_color_class = nw_players_colors[cube_player]
    cube_color = cube_colors(cube_color_class)
    cube_material = 0
    cube_texture_index = number_ground_textures + cube_index
    cube_parameters = {}

    spatial_position = cube_placement(cube_scale_factor, ground_scale_factor)
    spatial_rotation = cube_rotation()
    spatial_radius = math.sqrt(2.0 * cube_scale_factor * cube_scale_factor)
    spatial_gaze = [0.0, 0.0]
    spatial_destination = [0.0, 0.0, 0.0]
    spatial_velocity = 0.0

    if cube_player == 'resource':
        resource_energy = 2000.0
    else:
        resource_energy = 100.0
            
    cube = {"cube_index": cube_index, "cube_player": cube_player, "cube_uuid": cube_uuid, "cube_emoticon": cube_emoticon, "cube_firstname": cube_firstname,
            "cube_active": cube_active, "cube_display": cube_display, "cube_scale_factor": cube_scale_factor,
            "cube_type": cube_type, "cube_color_class": cube_color_class, "cube_color": cube_color, "cube_material": cube_material,
            "cube_surface": cube_surface, "cube_texture_index": cube_texture_index, "cube_texture_map": cube_texture_map,
            "cube_parameters": cube_parameters,
            "spatial_position": spatial_position, "spatial_rotation": spatial_rotation, "spatial_gaze": spatial_gaze, "spatial_radius": spatial_radius,
            "spatial_destination": spatial_destination, "spatial_velocity": spatial_velocity,
            "resource_energy": resource_energy
    }

    return cube

# Setup textures
def new_texture(texture_index, number_ground_textures, cubeList):

    # This is a ground plane texture for testing only
    if texture_index == 0:
        texture_filename = assets + "/Grey-100.png"
        texture = {"texture_index": texture_index, "texture_filename": texture_filename}
        return texture
    
    # This is our main reference ground plane
    if texture_index == 1:
        texture_filename = assets + "/texref-%dx%d.png" % (res, res)
        texture = {"texture_index": texture_index, "texture_filename": texture_filename}
        return texture

    # These are textures for all the individual cube players
    t = texture_index - number_ground_textures
    cube_player = cubeList[t]['cube_player']
    cube_color = cubeList[t]['cube_color']
    cube_emoticon = cubeList[t]['cube_emoticon'] 
    cube_surface = cubeList[t]['cube_surface']
    if cube_player == "enby" :
        texture_filename = assets + "/texture-" + cube_emoticon + "-3072x512.png"
    else :
        cube_color_filename = cube_color_patch(cube_color, cube_surface)
        texture_filename = cube_color_texture(cube_emoticon, cube_player, cube_color_filename)
    texture = {"texture_index": texture_index, "texture_filename": texture_filename}

    return texture

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
        print("nwu.py: Unable to read males file %s - %s" % (males_file, e))
        sys.exit()

# Get a unique male from the list
def get_male():
    while len(males_used) < len(males):
        male = random.choice(males)
        if male not in males_used:
            males_used.append(male)
            return male[0:-4]
    return random.choice(males)[:-4]

# Read the list of possible females into a list
def females_list():
    global females
    try:
        with open(females_file) as f:
            females = f.read().splitlines()
    except IOError as e:
        print("nwu.py: Unable to read females file %s - %s" % (females_file, e))
        sys.exit()

# Get a unique female from the list
def get_female():
    while len(females_used) < len(females):
        female = random.choice(females)
        if female not in females_used:
            females_used.append(female)
            return female[0:-4]
    return random.choice(females)[:-4]

# Read the list of possible enbies into a list
def enbies_list():
    global enbies
    try:
        with open(enbies_file) as f:
            enbies = f.read().splitlines()
    except IOError as e:
        print("nwu.py: Unable to read enbies file %s - %s" % (enbies_file, e))
        sys.exit()

# Get a unique enby from the list
def get_enby():
    while len(enbies_used) < len(enbies):
        enby = random.choice(enbies)
        if enby not in enbies_used:
            enbies_used.append(enby)
            return enby[0:-4]
    return random.choice(enbies)[:-4]

# Read the list of possible predators into a list
def predators_list():
    global predators
    try:
        with open(predators_file) as f:
            predators = f.read().splitlines()
    except IOError as e:
        print("nwu.py: Unable to read predators file %s - %s" % (predators_file, e))
        sys.exit()

# Get a unique predator from the list
def get_predator():
    while len(predators_used) < len(predators):
        predator = random.choice(predators)
        if predator not in predators_used:
            predators_used.append(predator)
            return predator[0:-4]
    return random.choice(predators)[:-4]

# Read the list of possible resources into a list
def resources_list():
    global resources
    try:
        with open(resources_file) as f:
            resources = f.read().splitlines()
    except IOError as e:
        print("nwu.py: Unable to read resources file %s - %s" % (resources_file, e))
        sys.exit()

# Get a unique resource from the list
def get_resource():
    while len(resources_used) < len(resources):
        resource = random.choice(resources)
        if resource not in resources_used:
            resources_used.append(resource)
            return resource[0:-4]
    return random.choice(resources)[:-4]

# Read the list of possible surfaces into a list
def surfaces_list():
    global surfaces
    try:
        with open(surfaces_file) as f:
            surfaces = f.read().splitlines()
    except IOError as e:
        print("nwu.py: Unable to read surfaces file %s - %s" % (surfaces_file, e))
        sys.exit()

# Get a unique surface from the list
def get_surface():
    while len(surfaces_used) < len(surfaces):
        surface = random.choice(surfaces)
        if surface not in surfaces_used:
            surfaces_used.append(surface)
            return surface[0:-4]
    return random.choice(surfaces)[:-4]

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
                print("nwu.py: Conflict in location - dr %4.2f lr+pr % 4.2f" % (dr, lr+pr))
                break

        if not conflict:
            locations.append((px, py, pz, pr))
            return [px, py, pz]
        placement_attempts -= 1

    print("nwu.py: Unable to position cube without conflict")
    return [0.0, py, 0.0]

# Assign cube rotation
def cube_rotation():

    rotation_x = 0.0
    rotation_y = random.uniform(0.0, 2*math.pi)
    rotation_z = 0.0
    return [rotation_x, rotation_y, rotation_z]

# Setup our window
def setup_window(dataset, background_color):
    window_title = "Neckerworld" + " - " + dataset
    main_window_width = 1280
    main_window_height = 720
    main_window_channels = 4
    window_background_color = background_color
    window = {"window_title": window_title, "main_window_width": main_window_width, "main_window_height": main_window_height, "main_window_channels": main_window_channels, "window_background_color": window_background_color} 
    return window

# Setup the camera position and target
def setup_camera(camera_position, camera_target, camera_up):
    camera = {"camera_position": camera_position, "camera_target": camera_target, "camera_up": camera_up}
    return camera

# Setup the grounds
def setup_grounds(ng, uuid, gsf, ground_texture_map_reference, ground_texture_map_background, camera_position, camera_target, camera_up):
    groundList = []
    ground_index = 0
    ground_uuid = uuid
    ground_scale_factor_0 = gsf
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
    ground = {"ground_index": ground_index, "ground_uuid": ground_uuid, "ground_scale_factor": ground_scale_factor_0, "ground_type": ground_type, "ground_color": ground_color, "ground_material": ground_material, "ground_texture_index": ground_texture_index, "ground_texture_map": ground_texture_map, "ground_spatial_position": ground_spatial_position, "ground_spatial_rotation": ground_spatial_rotation, "ground_view_position": ground_view_position, "ground_view_target": ground_view_target, "ground_view_up": ground_view_up}
    groundList.append(ground)

    ground_index = 1
    ground_uuid = uuid
    # Check if this is correct. It's not used.
    ground_scale_factor_1 = gsf+10.0
    ground_type = 2
    ground_color = color_green
    ground_material = 0
    ground_texture_index = 2
    ground_texture_map = ground_texture_map_background
    ground_spatial_position = [0.0, 0.0, -40.0]
    ground_spatial_rotation = [1.5708, 0.0, 0.0]
    ground_view_position = camera_position
    ground_view_target = camera_target
    ground_view_up = camera_up
    z_offsset = -ground_scale_factor_1
    ground = {"ground_index": ground_index, "ground_uuid": ground_uuid, "ground_scale_factor": ground_scale_factor_1, "ground_type": ground_type, "ground_color": ground_color, "ground_material": ground_material, "ground_texture_index": ground_texture_index, "ground_texture_map": ground_texture_map, "ground_spatial_position": ground_spatial_position, "ground_spatial_rotation": ground_spatial_rotation, "ground_view_position": ground_view_position, "ground_view_target": ground_view_target, "ground_view_up": ground_view_up}

    if ng == 1:
        return groundList
    else:
        groundList.append(ground)
        return groundList

# Setup cubes
def setup_cubes(nw_player_count, ground_scale_factor, number_ground_textures):
    cubeList = []
    for cube_index in range(nw_player_count):
        cube = new_cube(cube_index, ground_scale_factor, number_ground_textures)
        cubeList.append(cube)
    return cubeList

# Load all cube descriptors from cubedata.csv
def new_cube_from_list_initialize():

    cube_database = {'male':[], 'female':[], 'enby':[], 'predator':[], 'resource':[]}
    with open(cubedata, 'r') as f:
        cd = f.readlines()
    for c in cd:
        c = c.strip()
        clist = c.split('\t')
        cube_database[clist[1]].append(clist)

    return cube_database

# Lookup cube description from cubedata.csv
def new_cube_from_list(cube_database, cube_index, cube_player, cube_player_index, cube_x, cube_z, cube_angle, ground_scale_factor, number_ground_textures):

    # dataset,cube_player,cube_uuid,cube_emoticon,cube_firstname,cube_scale_factor,cube_type,cube_color_class,cube_color[0],cube_color[1],cube_color[2],cube_material,cube_surface,spatial_position[0],spatial_position[1],spatial_position[2],spatial_radius,resource_energy,texture_filename
    # female-1f31d,female,9c220cfb-e22e-40ac-bfc4-09fd40bd5fa8,1f31d,Mary,0.53965,3,w,0.63137,0.63137,0.63137,0,,0.00000,0.54065,0.00000,0.76318,100.00000,training/textures/1f31d-texture.png

    maps = {'male': cube_texture_map_2_panes, 'female': cube_texture_map_2_panes, 'enby': cube_texture_map_6_panes, 'predator': cube_texture_map_6_panes, 'resource': cube_texture_map_resource}

    cd = cube_database[cube_player][cube_player_index]
    cube_player = cd[1]
    cube_uuid = cd[2]
    cube_emoticon = cd[3]
    cube_firstname = cd[4]
    cube_texture_map = maps[cube_player]
    cube_active = True
    cube_display = True
    cube_scale_factor = float(cd[5])
    cube_type = int(cd[6])
    cube_color_class = cd[7]
    cube_color = [float(cd[8]), float(cd[9]), float(cd[10]), 1.0]
    cube_material = int(cd[11])
    cube_surface = cd[12]
    cube_texture_index = number_ground_textures + cube_index
    cube_parameters = {}

    spatial_position = [cube_x, float(cd[14]), cube_z] 
    spatial_rotation = [0.0, cube_angle, 0.0]
    spatial_radius = float(cd[16])
    spatial_gaze = [0.0, 0.0]
    spatial_destination = [0.0, 0.0, 0.0]
    spatial_velocity = 0.0
    resource_energy = float(cd[17])
            
    cube = {"cube_index": cube_index, "cube_player": cube_player, "cube_uuid": cube_uuid, "cube_emoticon": cube_emoticon, "cube_firstname": cube_firstname,
            "cube_active": cube_active, "cube_display": cube_display, "cube_scale_factor": cube_scale_factor,
            "cube_type": cube_type, "cube_color_class": cube_color_class, "cube_color": cube_color, "cube_material": cube_material,
            "cube_surface": cube_surface, "cube_texture_index": cube_texture_index, "cube_texture_map": cube_texture_map,
            "cube_parameters": cube_parameters,
            "spatial_position": spatial_position, "spatial_rotation": spatial_rotation, "spatial_gaze": spatial_gaze, "spatial_radius": spatial_radius,
            "spatial_destination": spatial_destination, "spatial_velocity": spatial_velocity,
            "resource_energy": resource_energy
    }

    return cube

# Setup cubes from a .csv file
def setup_cubelist(cubelist, ground_scale_factor, number_ground_textures):

    # Read contens of cubedat.csv
    cube_database = new_cube_from_list_initialize()

    cubeList = []

    with open(cubelist, 'r') as f:
        c_lines = f.readlines()

        if debug:
            print(c_lines)

        for c in c_lines:
            if "Player" in c:
                continue
            c = c.strip()
            cube_parameters = c.split('\t')
            cube_index = int(cube_parameters[0])
            cube_player = cube_parameters[1]
            cube_player_index = int(cube_parameters[2])
            cube_x = float(cube_parameters[3])
            cube_z = float(cube_parameters[4])
            cube_angle = float(cube_parameters[5])
            print("nwu.py: Creating cube %d which is a %s using list item %d at (%0.2f, %0.2f) facing %0.2f" % (cube_index, cube_player, cube_player_index, cube_x, cube_z, cube_angle))
            cube = new_cube_from_list(cube_database, cube_index, cube_player, cube_player_index, cube_x, cube_z, cube_angle, ground_scale_factor, number_ground_textures)
            cubeList.append(cube)

    return cubeList

# Setup lights
def setup_lights(ambient):
    lightList = []
    light_index = 0
    light_position = light0
    light_intensity = light_100
    light_ambient = light_025
    light_diffuse = light_075
    light_specular = light_000
    light_constant = 1.0
    light_linear = 0.045
    light_quadratic = 0.0075
    light = {"light_index": light_index, "light_position": light_position, "light_intensity": light_intensity, "light_ambient": ambient, "light_diffuse": light_diffuse, "light_specular": light_specular, "light_constant": light_constant, "light_linear": light_linear, "light_quadratic": light_quadratic}
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
    light = {"light_index": light_index, "light_position": light_position, "light_intensity": light_intensity, "light_ambient": ambient, "light_diffuse": light_diffuse, "light_specular": light_specular, "light_constant": light_constant, "light_linear": light_linear, "light_quadratic": light_quadratic}
    lightList.append(light)
 
    return lightList

# Setup materials
def setup_materials():
    materialList = []
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

    return materialList

# Setup textxures
def setup_textures(nw_player_count, number_ground_textures, cubeList):
    textureList = []
    for texture_index in range(nw_player_count+2):
        texture = new_texture(texture_index, number_ground_textures, cubeList)
        textureList.append(texture)
    return textureList

# Create a new JSON data file
def create_JSON():

    # Create the output JSON filename based on the input .CSV name.
    if filename != "":
        jsonName = filename
    else:
        jsonName = "nw.json"
        
    # Initialize JSON object
    data = {"dataset":dataset}

    if debug :
        print(json.dumps(data, indent=4, sort_keys=True))

    # Write the new JSON file
    try:
        with open(jsonName, 'w') as f:
            json.dump(data, f)
    except (IOError, ValueError) as e:
        print("nwu.py: error -- %s" % (e.args[0]))
        sys.exit(1)

