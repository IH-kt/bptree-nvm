import pandas as pd
import numpy as np
from matplotlib import pyplot as plt
import matplotlib as mpl
import japanize_matplotlib
import sys
import os
import re

def plot_graph(target_dir, result_dfs, labels, op, opname, colorlst, markerlst, font_size):

    os.makedirs(target_dir, exist_ok=True)

    ax = plt.axes()
    for (df, color, marker) in zip(result_dfs, colorlst, markerlst):
        df.plot(ax=ax, color=color, marker=marker, figsize=(6,6))
    plt.xlabel('スレッド数', fontsize=font_size)
    # plt.xlabel('Number of Threads', fontsize=font_size)
    plt.ylabel('実行時間 (秒)', fontsize=font_size)
    # plt.ylabel('Execution Time (sec.)', fontsize=font_size)
    plt.xlim([1, result_dfs[0].index.max()])
    # plt.ylim([0, 7])
    plt.ylim(bottom = 0)
    # plt.xticks(result_df.index, result_df.index)
    plt.legend(labels)
    # plt.legend(labels, bbox_to_anchor=(0.40, -0.30), loc='center', borderaxespad=0, ncol=2, fontsize=font_size)
    plt.tick_params(labelsize=font_size)
    # plt.title('スレッド数による実行時間の変化 (' + ops[i] + ')', fontsize=font_size)
    plt.tight_layout()
    # plt.savefig(log_size_str + '/' + colname + '.png')
    plt.savefig(target_dir + '/more_' + op + '.pdf')
    plt.close()

def switch_cpthread(tree_type):
    if tree_type in {'use_pmem', 'use_pmem_dram_log'}:
        return 1
    else:
        return 8

def read_csvs(result_file_dirs, op):
    dataframes = []
    for result_file_dir in result_file_dirs:
        df_tmp = pd.read_csv(result_file_dir + '/' + op + '.csv', usecols=[2,3], index_col=0)
        dataframes.append(df_tmp)
    return dataframes

def main():
    # colorlst = ['#fecc5c', '#fd8d3c', '#f03b20', '#bd0026']
    # colorlst = ['#feedde', '#fdbe85', '#fd8d3c', '#e6550d', '#a63603']
    colorlst = ['#ffffcc', '#ffeda0', '#fed976', '#feb24c', '#fd8d3c', '#fc4e2a', '#e31a1c', '#b10026']
    markerlst = ['o', 'v', '^', 's', '*', '>', '<', 's']
    font_size = 18
    line_style = ['ro-', 'gv-', 'b^-', 'k*-']
    ops = ['insert', 'search', 'delete', 'mixed']
    opnames = ['挿入', '検索', '削除', '混合']
    # tree_types = ['use_mmap', 'parallel_cp', 'log_compression', 'parallel_cp_dram_log']
    # tree_names = ['NVM(1スレッド)', 'NVM(8スレッド)', 'NVM(ログ圧縮+8スレッド)', 'DRAM(8スレッド)']
    tree_types = [
            #'use_mmap',
            #'parallel_cp',
            # 'log_compression',
            # 'parallel_cp_dram_log',
            # 'optimized_commit',
            # 'no_cp',
            # 'no_cp_dram',
            # 'no_cp_optc'
            'more_loop_use_mmap',
            'more_loop_parallel_cp',
            'more_loop_log_compression'
            ]
    tree_names = [
            'NVM+CP1スレッド',
            'NVM+CP8スレッド',
            'NVM+ログ圧縮+CP8スレッド'
            #'DRAM+CP8スレッド',
            #'NVM+ログ圧縮+ログflushなし+CP8スレッド',
            #'NVM+CPなし',
            #'DRAM+CPなし',
            #'NVM+ログflushなし+CPなし'
            ]
    # tree_types = ['log_compression', 'parallel_cp_dram_log']
    # tree_names = ['NVM(ログ圧縮+8スレッド)', 'DRAM(8スレッド)']

    if (len(sys.argv) < 2) :
        print("too few arguments")
        sys.exit()
    root_dir = sys.argv[1]
    result_file_dirs = list(map(lambda x: root_dir + '/' + x, tree_types))

    for (op, opname) in zip(ops, opnames):
        dataframes = read_csvs(result_file_dirs, op)
        plot_graph('scaling/', dataframes, tree_names, op, opname, colorlst, markerlst, font_size)

main()
