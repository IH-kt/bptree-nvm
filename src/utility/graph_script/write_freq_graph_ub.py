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

diff_index = list(range(1, len(timestamps)))
major_fmt = list(map(str, diff_index))
major_fmt.insert(0, "")

diff_df = pd.DataFrame(diff, index=diff_index, columns=['sec'])
q_80 = diff_df.quantile(q=[0.80]).iat[0, 0]
ax = diff_df.plot()
ax.set_xlabel('number of times')
ax.set_ylabel('second')
ax.ticklabel_format(style='plain', axis='x')
ax.set_ylim([0, 0.1])
pl.savefig('write_freq_ub.png')
pl.savefig('write_freq_ub.eps')
