import subprocess
import time
import sys

while True:
    user_input = input("Press Enter to run ASR on pi4 (or 'q' to quit): ").strip()
    if user_input.lower() == 'q':
        break

    ssh_command = "ssh pi4 'cd ~/whisper-project/voice_control && ./asr'"

    proc = subprocess.Popen(
        ssh_command,
        shell=True,
        stdout=subprocess.PIPE,  
        stderr=subprocess.DEVNULL,  # ignore debug/timing stderr (required for 100% accuracy)
        text=True,
        bufsize=1
    )

    print("\nRecording for 15s")

    # Force visible countdown (one line per second)
    for i in range(15, 0, -1):
        print(f"{i:2}")
        sys.stdout.flush()
        time.sleep(1)

    print("Standby while audio processes...\n", flush=True)

    last_line = ""
    for line in proc.stdout:
        print(line, end="")
        if line.strip():
            last_line = line.strip()

    proc.wait()
