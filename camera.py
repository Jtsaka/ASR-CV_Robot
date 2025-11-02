import cv2
import specs
import math
from picamera2 import Picamera2
from ultralytics import YOLO

class CVProcessing:
    def __init__(self):
        #data classes  
        self.camera = specs.Camera()
        self.bottle = specs.Bottle()

        #camera config
        self.picam2 = Picamera2()
        self.picam2.preview_configuration.main.size = (1280, 1280)
        self.picam2.preview_configuration.main.format = "RGB888"
        self.picam2.preview_configuration.align()
        self.picam2.configure("preview")
        self.picam2.start()

        self.model = YOLO("yolov8_onnx_models/yolov8s.onnx")

    def disp_fps(self, results, annotated_frame):
        try:
            inference_time = results[0].speed['inference']
            if inference_time <= 0: 
                return
            fps = 1000 / inference_time  #Convert to milliseconds
            text = f'FPS: {fps:.1f}'    

            #Define font and position
            font = cv2.FONT_HERSHEY_SIMPLEX
            text_size = cv2.getTextSize(text, font, 1, 2)[0]
            text_x = annotated_frame.shape[1] - text_size[0] - 10  #10 pixels from the right
            text_y = text_size[1] + 10  #10 pixels from the top
            font_scale = 1
            thick = 2

            #Draw the text on the annotated frame
            cv2.putText(annotated_frame, text, (text_x, text_y), font, font_scale, (255, 255, 255), thick, cv2.LINE_8)
        except (KeyError, IndexError):
            pass #skip if speed data isn't available

    def dist_calc(self, obj_px_width) -> float:
        if obj_px_width <= 0: 
            return 0
        
        dist_mm = (self.bottle.width_mm * self.camera.focal_len_px) / obj_px_width
        dist_m = dist_mm / 1000
        
        return dist_m

    def ang_calc(self, box_x1, box_x2) -> float:
        bottle_center_x_px = (box_x1 + box_x2) / 2
        frame_center_x_px = specs.IMAGE_WIDTH_PX / 2
        offset_px = bottle_center_x_px - frame_center_x_px

        angle_radians = math.atan(offset_px / self.camera.focal_len_px)
        angle_degrees = math.degrees(angle_radians)

        return angle_degrees

    def process_frame(self):

        frame = self.picam2.capture_array()
        results = self.model(frame, imgsz=224)
        annotated_frame = frame.copy() #creating copy of frame
        
        self.disp_fps(results, annotated_frame)

        bottle_data = None
            
        for box in results[0].boxes:
            obj_id = int(box.cls[0])
            obj_conf = float(box.conf[0])
            obj_name = results[0].names[obj_id]
            
            if obj_name == "bottle" and obj_conf > 0.6:
            
                box_coord = box.xyxy[0] #top left and bottom right 
                x1,y1,x2,y2 = map(int, box_coord)
                
                cv2.rectangle(annotated_frame, (x1, y1), (x2, y2), color=(0, 255, 0), thickness=2)
                label = f"{obj_name} {obj_conf:.2f}"
                cv2.putText(annotated_frame, label, (x1, y1 - 10), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0), 2, cv2.LINE_8)
                
                ###Dist calculation
                px_width = x2 - x1
                px_height = y2 - y1

                dist = self.dist_calc(px_width)
                # print(f"Distance: {dist:.2f} m")

                ###Ang calculation
                ang = self.ang_calc(x1, x2)
                # print(f"Angle: {ang:.2f} deg")

                bottle_data = {"distance": dist, "angle": ang}

                dist_txt = f"Dist: {dist:.2f} m"
                ang_txt = f"Angle: {ang:.1f} deg"
                cv2.putText(annotated_frame, dist_txt, (x1, y2 + 20), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0), 2)
                cv2.putText(annotated_frame, ang_txt, (x1, y2 + 45), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 255), 2)

                break
                
        return annotated_frame, bottle_data
        
        
    def stop(self):
        self.picam2.stop()
        print("Camera Stopped")
