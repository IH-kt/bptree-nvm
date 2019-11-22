import pandas as pd
import numpy as np
from matplotlib import pyplot as plt
import matplotlib as mpl
import sys

line_style = ['ro-', 'gv-', 'b^-', 'k*-']

result_files = sys.argv
if (len(result_files) < 4) :
    print("too few arguments")
    sys.exit()

result_df1 = pd.read_csv(result_files[1], index_col=0)
result_df2 = pd.read_csv(result_files[2], index_col=0)
result_df3 = pd.read_csv(result_files[3], index_col=0)

for i in range(3):
    p_df1 = result_df1.iloc[:, [i]]
    p_df2 = result_df2.iloc[:, [i]]
    p_df3 = result_df3.iloc[:, [i]]
    colname = p_df1.columns[0]
    p_df1.columns = [result_files[1]]
    p_df2.columns = [result_files[2]]
    p_df3.columns = [result_files[3]]
    result_df = pd.concat([p_df1, p_df2, p_df3], axis=1)
    result_df.plot.line(style=line_style)
    plt.xlabel('Number of thread')
    plt.ylabel('Elapsed time (sec.)')
    plt.xlim([1, result_df.index.max()])
    plt.ylim([0, result_df.max().max() * 1.1])
    plt.xticks(result_df.index, result_df.index)
    plt.savefig(colname + '.png')
    plt.savefig(colname + '.eps')
# result_df.plot.line(style=line_style)
# plt.xlabel('Number of thread')
# plt.ylabel('Elapsed time (sec.)')
# plt.xticks(result_df.index, result_df.index)
# plt.xscale('log')
# plt.yscale('log')
# plt.savefig('result_log.png')
# plt.savefig('result_log.eps')