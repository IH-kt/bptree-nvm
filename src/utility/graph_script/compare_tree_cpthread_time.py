import pandas as pd
import numpy as np
from matplotlib import pyplot as plt
import matplotlib as mpl
import japanize_matplotlib
import sys
import os
import re

colorlst = ['#fecc5c', '#fd8d3c', '#f03b20', '#bd0026']
markerlst = ['o', 'v', '^', 's']

font_size = 18
line_style = ['ro-', 'gv-', 'b^-', 'k*-']
ops = ['挿入', '検索', '削除']

def plot_graph(log_size_str, result_file1, result_file2, result_file3, result_file4, cols, memtype):
    result_df1 = pd.read_csv(result_file1 + '/result.csv', index_col=0)
    result_df2 = pd.read_csv(result_file2 + '/result.csv', index_col=0)
    result_df3 = pd.read_csv(result_file3 + '/result.csv', index_col=0)
    result_df4 = pd.read_csv(result_file4 + '/result.csv', index_col=0)

    os.makedirs(log_size_str, exist_ok=True)

    for i in range(3):
        p_df1 = result_df1.iloc[:, [i]]
        p_df2 = result_df2.iloc[:, [i]]
        p_df3 = result_df3.iloc[:, [i]]
        p_df4 = result_df4.iloc[:, [i]]
        colname = p_df1.columns[0]
        # p_df1.columns = [result_file1.split('../' + memtype + '/elapsed_time/')[1]]
        # p_df2.columns = [result_file2.split('../' + 'vmem' + '/elapsed_time/')[1]]
        # p_df3.columns = [result_file3.split('../' + memtype + '/elapsed_time/')[1]]
        # p_df4.columns = [result_file4.split('../' + memtype + '/elapsed_time/')[1]]
        result_df = pd.concat([p_df1, p_df2, p_df3, p_df4], axis=1)
        result_df.columns = ['col1', 'col2', 'col3', 'col4']
        # result_df = p_df1
        # result_df = pd.concat([p_df1, p_df3, p_df4], axis=1)
        # ax = result_df.plot.line(style=line_style)
        ax = plt.axes()
        for (col, color, marker) in zip(result_df, colorlst, markerlst):
            tmp = eval('result_df.' + col)
            tmp.plot(ax=ax, color=color, marker=marker, figsize=(6,6))
        # plt.xlabel('スレッド数', fontsize=font_size)
        plt.xlabel('ユーザスレッド数', fontsize=font_size)
        # plt.ylabel('実行時間 (秒)', fontsize=font_size)
        plt.ylabel('実行時間 (秒)', fontsize=font_size)
        plt.xlim([1, result_df.index.max()])
        # plt.ylim([0, 7])
        plt.ylim(bottom = 0)
        plt.xticks(result_df.index, result_df.index)
        plt.legend(cols, bbox_to_anchor=(0.40, -0.30), loc='center', borderaxespad=0, ncol=2, fontsize=font_size)
        plt.tick_params(labelsize=font_size)
        # plt.title('スレッド数による実行時間の変化 (' + ops[i] + ')', fontsize=font_size)
        plt.tight_layout()
        # plt.savefig(log_size_str + '/' + colname + '.png')
        plt.savefig(log_size_str + '/' + colname.split('_concurrent.exe')[0] + '_presen.pdf')
        plt.close()

        result_df = pd.concat([p_df1, p_df2, p_df3, p_df4], axis=1)
        result_df.columns = [1, 2, 4, 8]
        result_df.index = ['col1', 'col2', 'col3', 'col4']
        result_df = result_df.T
        ax = plt.axes()
        for (col, color, marker) in zip(result_df, colorlst, markerlst):
            tmp = eval('result_df.' + col)
            tmp.plot(ax=ax, color=color, marker=marker, figsize=(6,6))
        # plt.xlabel('スレッド数', fontsize=font_size)
        plt.xlabel('Checkpointプロセスのスレッド数', fontsize=font_size)
        # plt.ylabel('実行時間 (秒)', fontsize=font_size)
        plt.ylabel('実行時間 (秒)', fontsize=font_size)
        plt.xlim([1, result_df.index.max()])
        # plt.ylim([0, 7])
        plt.ylim(bottom = 0)
        plt.xticks(result_df.index, result_df.index)
        plt.legend(cols, bbox_to_anchor=(0.40, -0.30), loc='center', borderaxespad=0, ncol=2, fontsize=font_size)
        plt.tick_params(labelsize=font_size)
        # plt.title('スレッド数による実行時間の変化 (' + ops[i] + ')', fontsize=font_size)
        plt.tight_layout()
        # plt.savefig(log_size_str + '/' + colname + '.png')
        plt.savefig(log_size_str + '/' + colname.split('_concurrent.exe')[0] + '_cp_presen.pdf')
        plt.close()

        # result_df.plot.line(style=line_style)
        # plt.xlabel('スレッド数')
        # plt.ylabel('実行時間 (秒)')
        # plt.xticks(result_df.index, result_df.index)
        # plt.xscale('log')
        # plt.yscale('log')
        # plt.title('スレッド数による実行時間の変化（' + ops[i] + '）')
        # plt.savefig(log_size_str + '/' + colname + '.result_log.png')
        # plt.savefig(log_size_str + '/' + colname + '.result_log.eps')

# result_files = sys.argv
# if (len(result_files) < 4) :
#     print("too few arguments")
#     sys.exit()

for memtype in ['pmem']: # ['vmem']: # ['pmem', 'vmem']:
    result_file_dir1 = '../' + memtype + '/para_cp/bptree_nvhtm_0/logsz_41943328/cpthr_1'
    result_file_dir2 = '../' + memtype + '/para_cp/bptree_nvhtm_0/logsz_41943328/cpthr_2'
    result_file_dir3 = '../' + memtype + '/para_cp/bptree_nvhtm_0/logsz_41943328/cpthr_4'
    result_file_dir4 = '../' + memtype + '/para_cp/bptree_nvhtm_0/logsz_41943328/cpthr_8'

    plot_graph('scaling/' + memtype, result_file_dir1, result_file_dir2, result_file_dir3, result_file_dir4, [r'1スレッド', r"2スレッド", r"4スレッド", "8スレッド"], memtype)
plt.close('all')
