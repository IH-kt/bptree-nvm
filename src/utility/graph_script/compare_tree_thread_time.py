import pandas as pd
import numpy as np
from matplotlib import pyplot as plt
import matplotlib as mpl
import sys
import os
import re

def get_logsz_dir(path):
    files = os.listdir(path)
    logsz_dirs = []
    for name in files:
        if (os.path.isdir(os.path.join(path, name)) and re.match(r'logsz_.\d+', name)):
            logsz_dirs.append(name)
    return sorted(logsz_dirs, key=lambda s: int(s.split('logsz_')[1]))

line_style = ['ro-', 'gv-', 'b^-', 'k*-']

def plot_graph(log_size_str, result_file1, result_file2, result_file3):
    result_df1 = pd.read_csv(result_file1 + '/result.csv', index_col=0)
    result_df2 = pd.read_csv(result_file2 + '/result.csv', index_col=0)
    result_df3 = pd.read_csv(result_file3 + '/result.csv', index_col=0)

    os.makedirs(log_size_str)

    for i in range(3):
        p_df1 = result_df1.iloc[:, [i]]
        p_df2 = result_df2.iloc[:, [i]]
        p_df3 = result_df3.iloc[:, [i]]
        colname = p_df1.columns[0]
        p_df1.columns = [result_file1.split('../elapsed_time/')[1]]
        p_df2.columns = [result_file2.split('../elapsed_time/')[1]]
        p_df3.columns = [result_file3.split('../elapsed_time/')[1]]
        result_df = pd.concat([p_df1, p_df2, p_df3], axis=1)
        result_df.plot.line(style=line_style)
        plt.xlabel('Number of thread')
        plt.ylabel('Elapsed time (sec.)')
        plt.xlim([1, result_df.index.max()])
        plt.ylim([0, result_df.max().max() * 1.1])
        plt.xticks(result_df.index, result_df.index)
        plt.savefig(log_size_str + '/' + colname + '.png')
        plt.savefig(log_size_str + '/' + colname + '.eps')

        result_df.plot.line(style=line_style)
        plt.xlabel('Number of thread')
        plt.ylabel('Elapsed time (sec.)')
        plt.xticks(result_df.index, result_df.index)
        plt.xscale('log')
        plt.yscale('log')
        plt.savefig(log_size_str + '/result_log.png')
        plt.savefig(log_size_str + '/result_log.eps')

# result_files = sys.argv
# if (len(result_files) < 4) :
#     print("too few arguments")
#     sys.exit()

result_file_dirs_1 = get_logsz_dir('../elapsed_time/bptree_nvhtm_0/')
result_file_dirs_2 = get_logsz_dir('../elapsed_time/bptree_nvhtm_1/')
result_file_dirs_3 = get_logsz_dir('../elapsed_time/fptree_concurrent_0/')

for i in range(len(result_file_dirs_1)):
    result_file_dir1 = '../elapsed_time/bptree_nvhtm_0/' + result_file_dirs_1[i]
    result_file_dir2 = '../elapsed_time/bptree_nvhtm_1/' + result_file_dirs_2[i]
    result_file_dir3 = '../elapsed_time/fptree_concurrent_0/' + result_file_dirs_3[0]
    plot_graph(result_file_dirs_1[i], result_file_dir1, result_file_dir2, result_file_dir3)
