import statistics as st
import numpy as np
from matplotlib import pyplot as plt
import sys
# import decimal
import os
import subprocess
import csv
import pandas as pd
exefiles = ["insert_concurrent.exe"]
exp_loop_times = range(3)
warmup_num = 500000 # 元からある要素の数
trial_num = 500000 # 検索・追加する回数
thread_num = [17] # スレッド数
# vrampath = "/home/iiboshi/dramdir"
pmempath = "/mnt/nvmm/iiboshi/data"
pmemlogpath = "/mnt/nvmm/iiboshi/log"
# linestyles = ["ro-", "b.-", "gs-", "k+-", "y^-", "c*-", "m1-", "kD-", "kx-", "k3-"]

def exp():
    try:
        cmd = ['./' + exefiles[0], str(warmup_num), str(trial_num), str(warmup_num + trial_num), str(thread_num[0]), pmempath, pmemlogpath]
        print(cmd)
        result = subprocess.run(cmd, stdout = subprocess.PIPE, stderr = None).stdout.decode("utf8")
    except NameError as err:
        print("NameError: {0}".format(err))
    except:
        print("execution error.", sys.exc_info());
    return result

for fn in exefiles:
    if not os.path.exists(fn):
        print(fn + " not exists.")
        sys.exit()

results = exp()

print(results)
np.save('result_raw.npy', results)
results_dataframe = pd.DataFrame(results, index=exefiles, columns=thread_num).T
results_dataframe.to_csv('result.csv')
