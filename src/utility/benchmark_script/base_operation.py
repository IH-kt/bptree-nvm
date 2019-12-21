import statistics as st
import numpy as np
from matplotlib import pyplot as plt
import sys
# import decimal
import os
import subprocess
import csv
import pandas as pd
exefiles = ["insert_concurrent.exe", "search_concurrent.exe", "delete_concurrent.exe"]
exp_loop_times = range(5)
warmup_num = int(sys.argv[1]) # 元からある要素の数
trial_num = int(sys.argv[2]) # 検索・追加する回数
thread_num = eval(sys.argv[3]) # スレッド数
pmempath = sys.argv[4]
pmemlogpath = sys.argv[5]
# linestyles = ["ro-", "b.-", "gs-", "k+-", "y^-", "c*-", "m1-", "kD-", "kx-", "k3-"]

def exp_loop(filename, mode, mempath):
    result_array = [];
    print("executing: " + filename)
    try:
        for i in thread_num:
            print("thread_num: " + str(i))
            inner_result_array = [];
            cmd = ['./' + filename, str(warmup_num), str(trial_num), str(warmup_num + trial_num), str(i), pmempath, pmemlogpath]
            print(cmd)
            for j in exp_loop_times:
                print("trial " + str(j+1))
                spres = subprocess.run(cmd, stdout = subprocess.PIPE, stderr = subprocess.PIPE)
                print(spres.stdout)
                print(spres.stderr)
                inner_result_array.append(float(spres.stdout.decode("utf8")));
                with open(filename + ".thr." + str(i) + ".trial." + str(j+1) + ".dmp", mode='w', encoding="utf-8") as f:
                    f.write(spres.stderr.decode("utf8"))
            result_array.append(inner_result_array);
    except NameError as err:
        print("NameError: {0}".format(err))
    except subprocess.CalledProcessError as err:
        print("CalledProcessError:")
        print(err.stdout)
        print(err.stderr)
    except ValueError as err:
        print(e)
        print(spres.stdout)
        print(spres.stderr)
    except Exception as e:
        print("execution error.", sys.exc_info())
        print(e)
    return result_array

for fn in exefiles:
    if not os.path.exists(fn):
        print(fn + " not exists.")
        sys.exit()

results = []
for fn in exefiles:
    results.append(exp_loop(fn, "i", ""))

np.save('result_raw.npy', results)
results_dataframe = pd.DataFrame(np.median(results, axis=2), index=exefiles, columns=thread_num).T
results_dataframe.to_csv('result.csv')
