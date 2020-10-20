import pandas as pd
import numpy as np
from matplotlib import pyplot as plt
import matplotlib as mpl
import japanize_matplotlib
import sys
import os
import re

def plot_graph(plot_infos):
    source_dataframes = []
    for source_file in plot_infos['source_files']:
        source_dataframes.append(pd.read_csv(source_file, index_col=0))

    os.makedirs(plot_infos['plot_target_dir'], exist_ok=True)

    ax = plt.axes()
    for (df, color, marker) in zip(source_dataframes, plot_infos['colorlst'], plot_infos['markerlst']):
        df.plot(ax=ax, color=color, marker=marker, figsize=(8,6))
    plt.xlabel('スレッド数', fontsize=plot_infos['font_size'])
    # plt.xlabel('Number of Threads', fontsize=font_size)
    plt.ylabel('実行時間 (秒)', fontsize=plot_infos['font_size'])
    # plt.ylabel('Execution Time (sec.)', fontsize=font_size)
    # plt.xlim([1, result_df.index.max()])
    # plt.ylim([0, 7])
    plt.ylim(bottom = 0)
    # plt.xticks(result_df.index, result_df.index)
    plt.legend(plot_infos['ledgends'], bbox_to_anchor=(0.40, -0.30), loc='center', borderaxespad=0, ncol=2, fontsize=plot_infos['font_size'])
    plt.tick_params(labelsize=plot_infos['font_size'])
    # plt.title('スレッド数による実行時間の変化 (' + ops[i] + ')', fontsize=font_size)
    plt.tight_layout()
    # plt.savefig(log_size_str + '/' + colname + '.png')
    plt.savefig(plot_infos['plot_target_dir'] + '/' + plot_infos['bench_name'] + plot_infos['result_suffix'] + '.pdf')
    plt.close()

def compare_dram(plot_infos):
    plot_infos['result_suffix'] = '_cpdram'
    result_file_template = ['use_mmap', 'use_mmap_dram']
    plot_infos['ledgends'] = ['Log-on-NVM', 'Log-on-DRAM']
    for bench_name in plot_infos['bench_names']:
        plot_infos['bench_name'] = bench_name
        plot_infos['source_files'] = list(map(plot_infos['fn_generator'](bench_name), result_file_template))
        plot_graph(plot_infos)

def compare_log_compression(plot_infos):
    plot_infos['result_suffix'] = '_cplogcmp'
    result_file_template = ['use_mmap', 'log_comp'] # TODO: ログ圧縮 + CP1スレッドが必要になる
    plot_infos['ledgends'] = ['Plain-NVHTM', 'Log-compression']
    for bench_name in plot_infos['bench_names']:
        plot_infos['source_files'] = list(map(plot_infos['fn_generator'](bench_name), result_file_template))
        plot_graph(plot_infos)

def main():
    plot_infos = {}
    plot_infos['bench_names'] = ['genome', 'intruder', 'kmeans-high', 'kmeans-low', 'labyrinth', 'ssca2', 'vacation-high', 'vacation-low', 'yada']
    plot_infos['colorlst'] = ['#fecc5c', '#fd8d3c', '#f03b20', '#bd0026']
    plot_infos['markerlst'] = ['o', 'v', '^', 's']
    plot_infos['font_size'] = 18
    plot_infos['plot_target_dir'] = "graphs/scaling"
    plot_infos['fn_generator'] = lambda bench_name: lambda extype: 'scaling/' + extype + '/time_' + bench_name + '.csv'
    # result_file_template = ['use_mmap', 'para_cp', 'log_comp', 'para_cp_dram']
    # result_file_template = ['use_mmap', 'use_mmap_dram', 'log_comp', 'no_cp', 'no_cp_dram']
    # ledgends = ['NVM(1スレッド)', 'NVM(8スレッド)', 'NVM(ログ圧縮)', 'エミュレータ(1スレッド)']
    # ledgends = ['NVM(CP1スレッド)', 'DRAM(CP1スレッド)', 'NVM(CP8スレッド, ログ圧縮)', 'NVM(CPなし)', 'DRAM(CPなし)']
    # ledgends = ['NVM(CP1スレッド)', 'NVM(CP8スレッド, ログ圧縮)', 'NVM(CPなし)']
    compare_dram(plot_infos)
    compare_log_compression(plot_infos)

main()
