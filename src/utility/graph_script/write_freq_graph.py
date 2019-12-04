import pandas as pd
from matplotlib import pyplot as pl
from matplotlib import ticker as tck
import sys

timestamps = []
for line in sys.stdin:
    timestamps.append(float(line))
diff = []
prev = timestamps[0]
for ts in timestamps[1:]:
    diff.append(ts - prev)
    prev = ts

print("timestamps =", timestamps)
print("diff =", diff)

diff_index = list(range(1, len(timestamps)))
major_fmt = list(map(str, diff_index))
major_fmt.insert(0, "")

diff_df = pd.DataFrame(diff, index=diff_index, columns=['sec'])
ax = diff_df.plot()
ax.xaxis.set_major_locator(tck.MultipleLocator(1.0))
ax.xaxis.set_major_formatter(tck.FixedFormatter(major_fmt))
ax.set_xlabel('number of times')
ax.set_ylabel('second')
pl.savefig('write_freq.png')
pl.savefig('write_freq.eps')
