import numpy as np
import soundfile as sf

# Load samples from text file (one sample per line)
samples = np.loadtxt("tuningfork.txt")

# Save as 16-bit WAV
sf.write("sine.wav", samples, 44100, subtype="PCM_16")
