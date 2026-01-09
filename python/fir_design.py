import os
current_dir = os.path.dirname(__file__) + '/'

import numpy as np
import matplotlib.pyplot as plt

SAMPLE_RATE = 48000
TARGET_SAMPLE_RATE = 32000
CUTOFF_FREQUENCY = 14000

delay_sec = 0.001
N = int(SAMPLE_RATE * delay_sec * 2)
if N%2==0: N-=1
M = N - 1
n = np.arange(N)

wc = (CUTOFF_FREQUENCY / SAMPLE_RATE) * np.pi * 2
x = (n - M//2) * wc

sinc = np.where(x == 0, 1.0, np.sin(x) / x)

wx2 = 2 * np.pi * n / M
wx4 = wx2 * 2
wx6 = wx2 * 3

#Blackman-Harris window:
a0 = 0.4243801
a1 = 0.4973406
a2 = 0.0782793

blackman_harris = a0 - a1 * np.cos(wx2) + a2 * np.cos(wx4)
bh_impulse = sinc * blackman_harris
bh_impulse /= np.sum(bh_impulse)

#Blackman-Nuttall window:
a0 = 0.3635819
a1 = 0.4891775
a2 = 0.1365995
a3 = 0.0106411

blackman_nuttall = a0 - a1 * np.cos(wx2) + a2 * np.cos(wx4) - a3 * np.cos(wx6)
bt_impulse = sinc * blackman_nuttall
bt_impulse /= np.sum(bt_impulse)

plt.xticks([0, 0.5, 1], [0, delay_sec, delay_sec * 2])
plt.xlabel('t, seconds')

impulse_x = np.linspace(0, 1, N)
plt.plot(impulse_x, bt_impulse)
plt.plot(impulse_x, blackman_nuttall)
plt.savefig(current_dir + 'sinc.png')
plt.clf()

print('windows size:', N)

import scipy

#plot Blackman-Harris
w, h = scipy.signal.freqz(bh_impulse, 1, fs = SAMPLE_RATE)
plt.plot(w, 20 * np.log10(abs(h)), label = 'Blackman-Harris')

#plot Blackman-Nuttall
w, h = scipy.signal.freqz(bt_impulse, 1, fs = SAMPLE_RATE)
plt.plot(w, 20 * np.log10(abs(h)), label = 'Blackman-Nuttall')

#plt.xscale('log')
plt.grid(visible=True)
plt.legend()
plt.xticks([5000, 10000, CUTOFF_FREQUENCY, TARGET_SAMPLE_RATE/2, SAMPLE_RATE/2 ], rotation = 45)
plt.savefig(current_dir + 'magnitude.png')
plt.clf()

#plot frequency response
plt.plot(w, np.unwrap(np.angle(h)), label = 'Blackman-Nuttall')
plt.savefig(current_dir + 'phase.png')
