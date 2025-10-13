from ultralytics import YOLO

model = YOLO("yolov8s.pt")

model.export(format="onnx", imgsz=224) #convert to onnx for faster CPU processing
