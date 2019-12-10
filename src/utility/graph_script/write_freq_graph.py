import pandas as pd
from matplotlib import pyplot as pl
from matplotlib import ticker as tck
import sys

timestamps = []
for line in sys.stdin:
    timestamps.append(float(line))
diff = []
freq = []
passed_time = []
start = timestamps[0]
prev = timestamps[0]
for ts in timestamps[1:]:
    diff.append(ts - prev)
    freq.append(256 * 1024 / (ts - prev))
    prev = ts
    passed_time.append(ts - start)

# print("timestamps =", timestamps)
# print("diff =", diff)
# print("freq =", diff)
# print(passed_time)

freq_index = list(range(1, len(timestamps)))
major_fmt = list(map(str, passed_time))
major_fmt.insert(0, "")

freq_df = pd.DataFrame(freq, index=passed_time, columns=['sec'])
# freq_df = pd.DataFrame(freq, columns=['sec'])
ax = freq_df.plot()
ax.set_xlabel('passed time')
ax.set_ylabel('write amount (byte/sec)')
# ax.ticklabel_format(style='plain', axis='x')
pl.ylim([0, freq_df.max().max() * 1.1])
pl.savefig('write_freq.png')
pl.savefig('write_freq.eps')
