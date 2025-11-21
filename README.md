# Computer vision for Robot
- Robot vision leverages YOLOv8 model using the COCO dataset (script written as a .py file)
- Utilizing CV allows the robot to make more informed and complex movments, with the goal of the robot chassis aligning itself to an object and translating to its position.

# ASR for Robot
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
