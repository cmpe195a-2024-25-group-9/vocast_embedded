import sounddevice as sd
import socket
import numpy as np

# Audio settings
SAMPLE_RATE = 48000  # Match ESP32
CHUNK_SIZE = 256     # Mono samples per packet
UDP_IP = "10.0.0.12"  # ESP32 IP address
UDP_PORT = 12345

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

def callback(indata, frames, time, status):
    if status:
        print(status)

    # `indata` shape: (frames, channels)
    mono = indata[:, 0]  # Assume 1st channel
    mono_int16 = (mono * 32767).astype(np.int16)

    # Now expand mono to stereo
    stereo = np.empty((mono_int16.shape[0], 2), dtype=np.int16)
    stereo[:, 0] = mono_int16  # Left
    stereo[:, 1] = mono_int16  # Right

    data_bytes = stereo.tobytes()

    # Send it
    sock.sendto(data_bytes, (UDP_IP, UDP_PORT))

# Open input stream
with sd.InputStream(samplerate=SAMPLE_RATE, channels=1, dtype='float32',
                    blocksize=CHUNK_SIZE, callback=callback):
    print("Streaming mic audio as stereo via UDP...")
    while True:
        pass  # Keep the program running
