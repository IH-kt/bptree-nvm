import pandas as pd
import numpy as np
from matplotlib import pyplot as plt
import matplotlib as mpl
import japanize_matplotlib
import sys
import os
import re

def plot_graph(result_files, plot_dir, ledgends, colorlst, markerlst, font_size):
    result_dataframes = []
    for result_file in result_files:
        result_dataframes.append(pd.read_csv(result_file, index_col=0))

    os.makedirs(plot_dir, exist_ok=True)

    for i in range(3):
        ax = plt.axes()
        for (df, color, marker) in zip(result_dataframes, colorlst, markerlst):
            df.plot(ax=ax, color=color, marker=marker, figsize=(8,6))
        plt.xlabel('スレッド数', fontsize=font_size)
        # plt.xlabel('Number of Threads', fontsize=font_size)
        plt.ylabel('実行時間 (秒)', fontsize=font_size)
        # plt.ylabel('Execution Time (sec.)', fontsize=font_size)
        # plt.xlim([1, result_df.index.max()])
        # plt.ylim([0, 7])
        plt.ylim(bottom = 0)
        # plt.xticks(result_df.index, result_df.index)
        plt.legend(ledgends, bbox_to_anchor=(0.40, -0.30), loc='center', borderaxespad=0, ncol=2, fontsize=font_size)
        plt.tick_params(labelsize=font_size)
        # plt.title('スレッド数による実行時間の変化 (' + ops[i] + ')', fontsize=font_size)
        plt.tight_layout()
        # plt.savefig(log_size_str + '/' + colname + '.png')
        plt.savefig(plot_dir + '/threads_ja.pdf')
        plt.close()


def main():
    colorlst = ['#fecc5c', '#fd8d3c', '#f03b20', '#bd0026']
    markerlst = ['o', 'v', '^', 's']
    font_size = 18
    ledgends = ['NVM(1スレッド)', 'NVM(8スレッド)', 'NVM(ログ圧縮)', 'エミュレータ(1スレッド)']
    result_files = ['result_kmeans-high_test_REDO-TS-FORK/thread_csv/time_kmeans.csv', 'result_kmeans-high_test_REDO-TS-FORK_cp8/thread_csv/time_kmeans.csv', 'result_kmeans-high_test_REDO-TS-FORK_logcmp/thread_csv/time_kmeans.csv', 'result_kmeans-high_test_REDO-TS-FORK_emulator/thread_csv/time_kmeans.csv']
    plot_target_dir = "graphs"
    plot_graph(result_files, plot_target_dir, ledgends, colorlst, markerlst, font_size)

main()
