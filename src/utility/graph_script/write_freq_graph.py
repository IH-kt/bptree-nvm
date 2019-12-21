import pandas as pd
from matplotlib import pyplot as pl
from matplotlib import ticker as tck
import japanize_matplotlib
import sys
from statistics import mean, median,variance,stdev

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
    tsdiff = ts - prev
    if (tsdiff == 0):
        tsdiff =  0.000000001
    worker_diff.append(tsdiff)
    worker_freq.append(256 * 1024 / tsdiff)
    prev = ts
    worker_passed_time.append(ts - start)

# print("timestamps =", timestamps)
# print("diff =", diff)
# print("freq =", diff)
# print(passed_time)

if (len(worker_timestamps) > 1):
    worker_freq_index = list(range(1, len(worker_timestamps)))
    worker_major_fmt = list(map(str, worker_passed_time))
    worker_major_fmt.insert(0, "")

    freq_df = pd.DataFrame(worker_freq, index=worker_passed_time, columns=['sec'])
    # freq_df = pd.DataFrame(freq, columns=['sec'])
    ax = freq_df.plot(legend=False)
    ax.set_xlabel('経過時間 (秒)')
    ax.set_ylabel('書き込み頻度 (バイト/秒)')
    pl.title('Workerプロセスの書き込み頻度')
    ax.ticklabel_format(style="sci", axis="y", scilimits=(0,0))
    # ax.ticklabel_format(style='plain', axis='x')
    pl.ylim([0, freq_df.max().max() * 1.1])
    pl.savefig('worker_write_freq.png')
    pl.savefig('worker_write_freq.eps')

if (len(checkpoint_timestamps) > 1):
    start = checkpoint_timestamps[0]
    prev = checkpoint_timestamps[0]
    checkpoint_diff = []
    checkpoint_freq = []
    checkpoint_passed_time = []
    for ts in checkpoint_timestamps[1:]:
        tsdiff = ts - prev
        if (tsdiff == 0):
            tsdiff =  0.000000001
        checkpoint_diff.append(tsdiff)
        checkpoint_freq.append(256 * 1024 / (tsdiff))
        prev = ts
        checkpoint_passed_time.append(ts - start)
    print("mean of freq = " + str(mean(checkpoint_freq)))

    checkpoint_freq_index = list(range(1, len(checkpoint_timestamps)))
    checkpoint_major_fmt = list(map(str, worker_passed_time))
    checkpoint_major_fmt.insert(0, "")

    freq_df = pd.DataFrame(checkpoint_freq, index=checkpoint_passed_time, columns=['sec'])
    # freq_df = pd.DataFrame(freq, columns=['sec'])
    ax = freq_df.plot(legend=False)
    ax.set_xlabel('経過時間 (秒)', fontsize=18)
    ax.set_ylabel('書き込み頻度 (バイト/秒)', fontsize=18)
    pl.title('Checkpointプロセスの書き込み頻度', fontsize=18)
    ax.yaxis.offsetText.set_fontsize(18)
    ax.ticklabel_format(axis="", scilimits=(0,0))
    ax.tick_params(labelsize=16)
    # ax.ticklabel_format(style='plain', axis='x')
    pl.ylim([0, freq_df.max().max() * 1.1])
    # pl.ylim([0, 100000000])
    pl.tight_layout()
    pl.savefig('checkpoint_write_freq.png')
    pl.savefig('checkpoint_write_freq.eps')
    pl.close('all')
