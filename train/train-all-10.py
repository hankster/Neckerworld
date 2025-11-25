# A Python script to train Neckerworld models of different sizes.

import time
from ultralytics import YOLO

# Load a model
# model = YOLO("yolo11x.pt")  # load a pretrained model (recommended for training)

# Train the model
# results = model.train(data="nw.yaml", epochs=10, imgsz=512)

# Let system power down and cool down
# time.sleep(30.0*60.0)

# Load a model
# model = YOLO("yolo11l.pt")  # load a pretrained model (recommended for training)

# Train the model
# results = model.train(data="nw.yaml", epochs=10, imgsz=512)

# Let system power down and cool down
# time.sleep(30.0*60.0)

# Load a model
# model = YOLO("yolo11m.pt")  # load a pretrained model (recommended for training)

# Train the model
# results = model.train(data="nw.yaml", epochs=10, imgsz=512)

# Let system power down and cool down
# time.sleep(30.0*60.0)

# Load a model
# model = YOLO("yolo11s.pt")  # load a pretrained model (recommended for training)

# Train the model
# results = model.train(data="nw.yaml", epochs=10, imgsz=512)

# Load a model
model = YOLO("yolo11n.pt")  # load a pretrained model (recommended for training)

# Train the model
results = model.train(data="nw.yaml", epochs=10, imgsz=512)

# Look for results here
ls -al runs/detect/*/weights
