#! /usr/bin/bash

#PYTORCH_CUDA_ALLOC_CONF=expandable_segments:True
#export PYTORCH_CUDA_ALLOC_CONF

# Nano
# python3 train.py --img 512 --epochs 3 --data nw.yaml --weights yolov5n.pt

# Small
# python3 train.py --img 512 --epochs 3 --data nw.yaml --weights yolov5s.pt

# Medium
# python3 train.py --img 512 --epochs 3 --data nw.yaml --weights yolov5m.pt

# Large
python3 train.py --img 512 --epochs 3 --data nw.yaml --weights yolov5l.pt

# XLarge
#python3 train.py --img 512 --epochs 3 --data nw.yaml --weights yolov5x.pt
