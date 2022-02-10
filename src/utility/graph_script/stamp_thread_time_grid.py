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
    max_val = 0
    for source_file in plot_infos['source_files']:
        source_dataframes.append(pd.read_csv(source_file, index_col=0))
    source_dataframes = list(map(lambda df: df / source_dataframes[0]['time'][1] * 100, source_dataframes))
    for source_file in source_dataframes:
        df_max = max(source_file['time'])
        if (max_val < df_max):
            max_val = df_max

    os.makedirs(plot_infos['plot_target_dir'], exist_ok=True)

    ax = plot_infos['ax']
    for (df, color, marker, linestyle) in zip(source_dataframes, plot_infos['colorlst'], plot_infos['markerlst'], plot_infos['linestyle']):
        df.plot(ax=ax, color=color, linestyle=linestyle, marker=marker, markeredgecolor='black', figsize=(8,6))
    ax.get_legend().set_visible(False)
    plt.title(plot_infos['title'])
    # plt.xlabel('スレッド数', fontsize=plot_infos['font_size'])
    plt.xlabel('ユーザスレッド数')
    # plt.xlabel('Number of Threads', fontsize=font_size)
    # plt.ylabel('実行時間 (秒)', fontsize=plot_infos['font_size'])
    plt.ylabel('実行時間 (%)')
    # plt.ylabel('Execution Time (sec.)', fontsize=font_size)
    # plt.xlim([1, result_df.index.max()])
    # plt.ylim([0, 7])
    plt.ylim(bottom = 0)
    plt.ylim(top = max_val * 1.1)
    # plt.xticks(result_df.index, result_df.index)
    # plt.tick_params(labelsize=plot_infos['font_size'])
    # plt.title('スレッド数による実行時間の変化 (' + ops[i] + ')', fontsize=font_size)
    plt.tight_layout()
    # plt.savefig(log_size_str + '/' + colname + '.png')

def compare_dram(plot_infos):
    f = plt.figure(figsize=(32, 34))
    plot_infos['result_suffix'] = '_cpdram'
    result_file_template = ['use_mmap', 'use_mmap_dram']
    plot_infos['ledgends'] = ['Log-on-NVM', 'Log-on-DRAM']
    for (bench_name, num) in zip(plot_infos['bench_names'], range(1, len(plot_infos['bench_names']) + 1)):
        print("plot", bench_name, "@cpdram")
        plot_infos['title'] = bench_name
        plot_infos['ax'] = f.add_subplot(3, 3, num)
        plot_infos['bench_name'] = bench_name
        plot_infos['source_files'] = list(map(plot_infos['fn_generator'](bench_name), result_file_template))
        plot_graph(plot_infos)
    h, l = plot_infos['ax'].get_legend_handles_labels()
    # plt.legend(h, plot_infos['ledgends'], bbox_to_anchor=(0.40, -0.30), loc='center', borderaxespad=0, ncol=2, fontsize=plot_infos['font_size'])
    plt.legend(h, plot_infos['ledgends'], bbox_to_anchor=(-1.00, -0.50), loc='center', borderaxespad=0, ncol=2)
    plt.subplots_adjust(bottom=0.15)
    plt.savefig(plot_infos['plot_target_dir'] + '/' + 'all' + plot_infos['result_suffix'] + '.pdf')
    plt.close()

def compare_log_compression(plot_infos):
    f = plt.figure(figsize=(32, 34))
    plot_infos['result_suffix'] = '_cplogcmp'
    result_file_template = ['use_mmap', 'log_comp_lp_single', 'log_comp_lp']
    plot_infos['ledgends'] = ['Plain-NVHTM', 'Log-Elimination (single)', 'Log-Elimination (parallel)']
    for (bench_name, num) in zip(plot_infos['bench_names'], range(1, len(plot_infos['bench_names']) + 1)):
        print("plot", bench_name, "@cplog")
        plot_infos['title'] = bench_name
        plot_infos['ax'] = f.add_subplot(3, 3, num)
        plot_infos['bench_name'] = bench_name
        plot_infos['source_files'] = list(map(plot_infos['fn_generator'](bench_name), result_file_template))
        plot_graph(plot_infos)
    h, l = plot_infos['ax'].get_legend_handles_labels()
    # plt.legend(h, plot_infos['ledgends'], bbox_to_anchor=(0.40, -0.30), loc='center', borderaxespad=0, ncol=2, fontsize=plot_infos['font_size'])
    plt.legend(h, plot_infos['ledgends'], bbox_to_anchor=(-1.00, -0.70), loc='center', borderaxespad=0, ncol=2)
    plt.subplots_adjust(bottom=0.20)
    plt.savefig(plot_infos['plot_target_dir'] + '/' + 'all' + plot_infos['result_suffix'] + '.pdf')

def breakdown_log_compression(plot_infos):
    f = plt.figure(figsize=(32, 34))
    plot_infos['result_suffix'] = '_cplogbrk'
    result_file_template = ['use_mmap', 'log_comp_lp_offloadonly', 'log_comp_lp']
    plot_infos['ledgends'] = ['Plain-NVHTM', 'Offloading', 'Log-compression']
    for (bench_name, num) in zip(plot_infos['bench_names'], range(1, len(plot_infos['bench_names']) + 1)):
        print("plot", bench_name, "@brklog")
        plot_infos['title'] = bench_name
        plot_infos['ax'] = f.add_subplot(3, 3, num)
        plot_infos['bench_name'] = bench_name
        plot_infos['source_files'] = list(map(plot_infos['fn_generator'](bench_name), result_file_template))
        plot_graph(plot_infos)
    h, l = plot_infos['ax'].get_legend_handles_labels()
    # plt.legend(h, plot_infos['ledgends'], bbox_to_anchor=(0.40, -0.30), loc='center', borderaxespad=0, ncol=2, fontsize=plot_infos['font_size'])
    plt.legend(h, plot_infos['ledgends'], bbox_to_anchor=(-1.00, -0.70), loc='center', borderaxespad=0, ncol=2)
    plt.subplots_adjust(bottom=0.20)
    plt.savefig(plot_infos['plot_target_dir'] + '/' + 'all' + plot_infos['result_suffix'] + '.pdf')

def main():
    plot_infos = {}
    plot_infos['bench_names'] = ['genome', 'vacation-high', 'vacation-low', 'yada', 'ssca2', 'intruder', 'kmeans-high', 'kmeans-low', 'labyrinth']
    # plot_infos['colorlst'] = ['#fecc5c', '#fd8d3c', '#f03b20', '#bd0026']
    plot_infos['colorlst'] = ['#aaaaaa', '#555555', '#000000']
    plot_infos['markerlst'] = ['*', 'v', '^', '+']
    plot_infos['font_size'] = 18
    plot_infos['plot_target_dir'] = "graphs/scaling"
    plot_infos['fn_generator'] = lambda bench_name: lambda extype: 'scaling/' + extype + '/time_' + bench_name + '.csv'
    plot_infos['linestyle'] = ['solid', 'dashed', 'dotted', 'dashdot']
    # result_file_template = ['use_mmap', 'para_cp', 'log_comp', 'para_cp_dram']
    # result_file_template = ['use_mmap', 'use_mmap_dram', 'log_comp', 'no_cp', 'no_cp_dram']
    # ledgends = ['NVM(1スレッド)', 'NVM(8スレッド)', 'NVM(ログ圧縮)', 'エミュレータ(1スレッド)']
    # ledgends = ['NVM(CP1スレッド)', 'DRAM(CP1スレッド)', 'NVM(CP8スレッド, ログ圧縮)', 'NVM(CPなし)', 'DRAM(CPなし)']
    # ledgends = ['NVM(CP1スレッド)', 'NVM(CP8スレッド, ログ圧縮)', 'NVM(CPなし)']
    compare_dram(plot_infos)
    compare_log_compression(plot_infos)
    # breakdown_log_compression(plot_infos)

main()
