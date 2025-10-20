import cv2
from picamera2 import Picamera2
from ultralytics import YOLO
import math

import specs

camera = specs.Camera()
bottle = specs.Bottle()

#camera config
picam2 = Picamera2()
picam2.preview_configuration.main.size = (1280, 1280)
picam2.preview_configuration.main.format = "RGB888"
picam2.preview_configuration.align()
picam2.configure("preview")
picam2.start()

model = YOLO("yolov8_onnx_models/yolov8s.onnx")

def disp_fps():
    inference_time = results[0].speed['inference']
    fps = 1000 / inference_time  #Convert to milliseconds
    text = f'FPS: {fps:.1f}'

    font = cv2.FONT_HERSHEY_SIMPLEX
    text_size = cv2.getTextSize(text, font, 1, 2)[0]
    text_x = annotated_frame.shape[1] - text_size[0] - 10  #10 pixels from the right
    text_y = text_size[1] + 10  #10 pixels from the top
    font_scale = 1
    thick = 2

    cv2.putText(annotated_frame, text, (text_x, text_y), font, font_scale, (255, 255, 255), thick, cv2.LINE_8)

while True:

    frame = picam2.capture_array()
    results = model(frame, imgsz=224)
    annotated_frame = frame.copy() #creating copy of frame
    
    disp_fps()
        
    for box in results[0].boxes:
        obj_id = int(box.cls[0])
        obj_conf = float(box.conf[0])
        obj_name = results[0].names[obj_id]
        
        if obj_name == "bottle" and obj_conf > 0.5:
        
            box_coord = box.xyxy[0] #top left and bottom right 
            x1,y1,x2,y2 = map(int, box_coord)
            
            cv2.rectangle(annotated_frame, (x1, y1), (x2, y2), color=(0, 255, 0), thickness=2)
            label = f"{obj_name} {obj_conf:.2f}"
            cv2.putText(annotated_frame, label, (x1, y1 - 10), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0), 2, cv2.LINE_8)
            
            ###Dist calculation
            px_width = x2 - x1
            px_height = y2 - y1
            
            dist_mm = bottle.width_mm / px_width * camera.focal_len_px
            dist_m = dist_mm / 1000
            
            image_center_px = specs.IMAGE_WIDTH_PX / 2
            bottle_center_px = (x1 + x2) / 2
            offset_px = bottle_center_px - image_center_px
            angle = math.degrees(math.atan(offset_px / camera.focal_len_px))
            
            dist_txt = f"Distance: {dist_m:.2f} m"
            angle_txt = f"Angle: {angle:.2f} deg"
            
            print(dist_txt)
            print(angle_txt)
            
            
    cv2.imshow("Camera", annotated_frame) #display camera feed + bounding box
    
    
    if cv2.waitKey(1) == ord("q"):
        break

cv2.destroyAllWindows()
