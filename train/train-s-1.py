from ultralytics import YOLO

# Load a model
model = YOLO("yolo11s.pt")  # load a pretrained model (recommended for training)

# Train the model
results = model.train(data="nw.yaml", epochs=1, imgsz=512)
