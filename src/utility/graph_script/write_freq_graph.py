import pandas as pd
from matplotlib import pyplot as pl
from matplotlib import ticker as tck
import sys

worker_timestamps = []
checkpoint_timestamps = []
for line in sys.stdin:
    if (line[0] == 'w'):
        worker_timestamps.append(float(line[1:]))
    else:
        checkpoint_timestamps.append(float(line[1:]))
worker_diff = []
worker_freq = []
worker_passed_time = []
start = worker_timestamps[0]
prev = worker_timestamps[0]
for ts in worker_timestamps[1:]:
    worker_diff.append(ts - prev)
    worker_freq.append(256 * 1024 / (ts - prev))
    prev = ts
    worker_passed_time.append(ts - start)

# print("timestamps =", timestamps)
# print("diff =", diff)
# print("freq =", diff)
# print(passed_time)

worker_freq_index = list(range(1, len(worker_timestamps)))
worker_major_fmt = list(map(str, worker_passed_time))
worker_major_fmt.insert(0, "")

freq_df = pd.DataFrame(worker_freq, index=worker_passed_time, columns=['sec'])
# freq_df = pd.DataFrame(freq, columns=['sec'])
ax = freq_df.plot()
ax.set_xlabel('passed time')
ax.set_ylabel('write amount (byte/sec)')
# ax.ticklabel_format(style='plain', axis='x')
pl.ylim([0, freq_df.max().max() * 1.1])
pl.savefig('worker_write_freq.png')
pl.savefig('worker_write_freq.eps')

if (len(checkpoint_timestamps) != 0):
    checkpoint_diff = []
    checkpoint_freq = []
    checkpoint_passed_time = []
    for ts in checkpoint_timestamps[1:]:
        checkpoint_diff.append(ts - prev)
        checkpoint_freq.append(256 * 1024 / (ts - prev))
        prev = ts
        checkpoint_passed_time.append(ts - start)

    checkpoint_freq_index = list(range(1, len(checkpoint_timestamps)))
    checkpoint_major_fmt = list(map(str, worker_passed_time))
    checkpoint_major_fmt.insert(0, "")

    freq_df = pd.DataFrame(checkpoint_freq, index=checkpoint_passed_time, columns=['sec'])
    # freq_df = pd.DataFrame(freq, columns=['sec'])
    ax = freq_df.plot()
    ax.set_xlabel('passed time')
    ax.set_ylabel('write amount (byte/sec)')
    # ax.ticklabel_format(style='plain', axis='x')
    pl.ylim([0, freq_df.max().max() * 1.1])
    pl.savefig('checkpoint_write_freq.png')
    pl.savefig('checkpoint_write_freq.eps')
plt.close('all')
