# Computer vision for Robot (Jake)

### Brief Overview

This project implements a voice-activated computer vision pipeline that lets a swerve-drive robot “see” its environment, lock onto a specific object (a water bottle), and send angle/distance commands to a motion controller for pickup.

The system is built around two main “brains”:

Brain 1 (Perception + High-Level Control):
- NVIDIA Jetson Orin Nano or Raspberry Pi 5 with a Hailo AI Accelerator and a Pi Camera Module 3 for running object detection (YOLOv8 on COCO) and geometric distance/angle calculations.

Brain 2 (Low-Level Control):
- An Arduino Mega 2560 that receives commands over UART and directly controls the drive and arm hardware.

A Raspberry Pi 4 sits alongside this as a dedicated ASR (Automatic Speech Recognition) node

For vision, the system uses a pre-trained YOLOv8 model on the COCO dataset to detect common household objects (e.g., bottles). For initial testing, the team focused on a standard water bottle. To improve frame rate on edge hardware, the default PyTorch YOLO model is converted to ONNX, which produces smoother video (from ~3–5 FPS up to roughly 2x more frames that) and more consistent detections.

The depth/angle estimation uses basic geometry and similar triangles: with known bottle dimensions, camera focal length, and the YOLO bounding box size in pixels, the system estimates how far away the bottle is and at what angle it lies relative to the camera. Those values are then sent via UART to the Arduino, which drives the robot into the correct position to pick up the bottle.

# ASR for Robot (Andres)
- Leverages Open AI's Whisper model to transcribe audio into text, which is then parsed to extract command data. 
- Utilizing subprocess in Main.py to run a user interface to run commands by pressing "enter" or exiting with "q" or "Q"
- Transcription formatted as (angle, distance)

# How To Set up SSH for ASR
(<> means not literal, enter the respective thing based on the argument)

1. Enable SSH on the pi
1a.  sudo raspi-config
1b.  Enable ssh
1c. Set new hostname (change the raspberrypi default, will prompt you to reboot after)

2. SSH
2a. Get worker IP "hostname -I"
2b. On master device terminal run [ssh (user)@(IP)]
2c. Enter the respective user’s password

3. Passwordless SSH
3a. Generate SSH key on master device
- ssh-keygen -t ed25519
- When asked for location → enter (default location)
- When asked for passphrase → enter twice (no passphrase)
3b. Copy your key to the pi (via master device)
- ssh-copy-id <user>@<pi’s_hostname>.local
3c. SSH Shortcut
- nano ~/.ssh/config 
- Then type the following:
Host pi4
    HostName pi4.local
    User amartinezhall
    IdentityFile ~/.ssh/id_ed25519

From here, you should be able to run ssh <hostname>
This process assumes a changing IP address, so don’t worry about that.
