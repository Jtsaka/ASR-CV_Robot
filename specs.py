###Raspberry Pi Cam Module 3 Specs
from dataclasses import dataclass
import math

IMAGE_WIDTH_PX = 1280

@dataclass
class Bottle:
    """Data model for a bottle using standardized measurements."""
    width_mm: float = 70.0 #mm

@dataclass
class Camera:
    """
    Data model for a Pi Cam Module 3 

    References: https://datasheets.raspberrypi.com/camera/camera-module-3-product-brief.pdf
    - 
    """
    focal_len_mm: float = 4.74 #in mm
    diag_fov: int = 70 #deg
    horiz_fov: int = 66 #deg
    vert_fov: int = 41 #deg
    
    focal_len_px: float = (IMAGE_WIDTH_PX / 2) / math.tan(math.radians(horiz_fov) / 2)
    

