#! /usr/bin/python3
"""
prettyjson.py -- A Python program

Sample usage:

 prettyjson.py

Complete specification:

 prettyjson.py -d -f jsonfile -h -i indent -v --debug --file=jsonfile --help --indent=indent --version jsonfile

 where

 -d, --debug          Turn debug statements on
 -f, --file           Input JSON filename to print
 -h, --help           Print usage information
 -i, --indent         Amount to indent
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
import json

debug = False
filename = ""
indent_amount = 4

def Usage():
    print("Usage: prettyjson.py -d -f filename -h -i indent -v --debug --file=filename --help --indent=indent --version jsonfile")

#
# Main program starts here
#

def main():
    
    try:
        with open(filename, 'r') as json_file:
            json_object = json.load(json_file)
            print(json.dumps(json_object, indent=indent_amount))

    except IOError as e:
        print("prettyjson.py: I/O error -- %s" % e)
        sys.exit(-1)

if __name__=='__main__':

    #
    # Get options and call the main program
    #                                                                                            

    try:
        options, args = getopt.getopt(sys.argv[1:], 'df:hi:v', ['debug', 'file=', 'help', 'indent=', 'version'])
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
        if o in ("-i", "--indent"):
            indent_amount = int(a)
        if o in ("-v", "--version"):
            print("prettyjson.py Version 1.0")
            sys.exit()

    # The JSON file can be specified either with -f, --file or as an additional argument
    if len(args) > 0:
        filename = args[0]

    if filename == "":
        print("prettyjson.py: A JSON file has not been specified.")
        Usage()
        sys.exit(-1)
        
    main()

    sys.exit()
