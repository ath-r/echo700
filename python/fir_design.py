import os
current_dir = os.path.dirname(__file__) + '/'

import numpy as np
import matplotlib.pyplot as plt

SAMPLE_RATE = 48000
TARGET_SAMPLE_RATE = 32000
CUTOFF_FREQUENCY = 14000

delay_sec = 0.001
window_size = int(SAMPLE_RATE * delay_sec * 2)
x = np.linspace(-delay_sec, delay_sec, window_size) * CUTOFF_FREQUENCY * 2 * np.pi
fir_impulse_response = np.sin(x)/(x) * np.blackman(window_size)

plt.xticks([0, 0.5, 1], [0, delay_sec, delay_sec * 2])
plt.xlabel('t, seconds')
plt.plot(np.linspace(0, 1, window_size), fir_impulse_response)
plt.savefig(current_dir + 'sinc.png')
plt.clf()

print('windows size:', window_size)

import scipy

w, h = scipy.signal.freqz(fir_impulse_response, 1, fs = SAMPLE_RATE)

plt.plot(w, 20 * np.log10(abs(h)))
#plt.xscale('log')
plt.grid(visible=True)
plt.xticks([5000, 10000, CUTOFF_FREQUENCY, TARGET_SAMPLE_RATE/2, SAMPLE_RATE/2 ], rotation = 45)
plt.savefig(current_dir + 'reponse.png')
plt.clf()