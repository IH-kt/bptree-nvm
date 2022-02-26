import pandas as pd
import numpy as np
from matplotlib import pyplot as plt
import matplotlib as mpl
import japanize_matplotlib
import sys
import os
import re
from matplotlib.ticker import ScalarFormatter

def plot_graph(plot_infos):
    # source_dataframes = []
    max_val = max(plot_infos['plot_data'])
    # print(plot_infos['plot_data'])

    # for source_file in plot_infos['source_files']:
    #     source_dataframes.append(pd.read_csv(source_file, index_col=0))
    #     df_max = max(source_dataframes[-1]['time'])
    #     if (max_val < df_max):
    #         max_val = df_max

    os.makedirs(plot_infos['plot_target_dir'], exist_ok=True)

    ax = plot_infos['plot_data'].plot.bar(color=plot_infos['colorlst'][0], edgecolor='black', figsize=(4,4))
    ax.set_xticklabels(plot_infos['bench_names'], rotation=45, ha='right')
    ax.yaxis.set_major_formatter(ScalarFormatter(useMathText=True))
    ax.ticklabel_format(style="sci",  axis="y",scilimits=(0,0))
    # for (df, color, marker, linestyle) in zip(source_dataframes, plot_infos['colorlst'], plot_infos['markerlst'], plot_infos['linestyle']):
    #     df.plot(ax=ax, color=color, linestyle=linestyle, marker=marker, markeredgecolor='black', figsize=(8,6))
    # ax.get_legend().set_visible(False)
    # plt.title(plot_infos['title'])
    # plt.xlabel('スレッド数', fontsize=plot_infos['font_size'])
    plt.xlabel('ベンチマーク')
    # plt.xlabel('Number of Threads', fontsize=font_size)
    # plt.ylabel('実行時間 (秒)', fontsize=plot_infos['font_size'])
    plt.ylabel(plot_infos['ylabel'])
    # plt.ylabel('Execution Time (sec.)', fontsize=font_size)
    # plt.xlim([1, result_df.index.max()])
    # plt.ylim([0, 7])
    plt.ylim(bottom = 0)
    # plt.ylim(top = max_val * 1.1)
    # plt.ylim(top = 10000000)
    # plt.ylim(top = 1)

    # plt.xticks(result_df.index, result_df.index)
    # plt.tick_params(labelsize=plot_infos['font_size'])
    # plt.title('スレッド数による実行時間の変化 (' + ops[i] + ')', fontsize=font_size)
    plt.tight_layout()
    # plt.savefig(log_size_str + '/' + colname + '.png')

def write_freq(plot_infos):
    plot_infos['result_suffix'] = '_writefreq'
    result_file_template = ['use_mmap']
    # data_list = []
    #     csvfile = pd.read_csv(plot_infos['abort_fn_generator'](bench_name), index_col=0)
    #     data_list.append(csvfile['abortps'][8])
    plot_infos['plot_data'] = pd.read_csv(plot_infos['writeratio_source'], index_col=0).reindex(index=plot_infos['bench_names'])['writefreq']
    plot_infos['ylabel'] = '書き込み頻度（回/秒）'
    plot_graph(plot_infos)
    # h, l = plot_infos['ax'].get_legend_handles_labels()
    # plt.legend(h, plot_infos['ledgends'], bbox_to_anchor=(0.40, -0.30), loc='center', borderaxespad=0, ncol=2, fontsize=plot_infos['font_size'])
    # plt.legend(h, plot_infos['ledgends'], bbox_to_anchor=(-1.00, -0.50), loc='center', borderaxespad=0, ncol=2)
    # plt.subplots_adjust(bottom=0.15)
    # plt.savefig(plot_infos['plot_target_dir'] + '/' + 'all' + plot_infos['result_suffix'] + '.pdf')
    # plt.close()
    plt.savefig(plot_infos['plot_target_dir'] + '/' + 'all' + plot_infos['result_suffix'] + '.pdf')
    plt.close()

def tx_ratio(plot_infos):
    plot_infos['result_suffix'] = '_txratio'
    result_file_template = ['use_mmap']
    # data_list = []
    #     csvfile = pd.read_csv(plot_infos['abort_fn_generator'](bench_name), index_col=0)
    #     data_list.append(csvfile['abortps'][8])
    plot_infos['plot_data'] = pd.read_csv(plot_infos['writeratio_source'], index_col=0).reindex(index=plot_infos['bench_names'])['txratio']
    plot_infos['ylabel'] = 'トランザクション実行割合'
    plot_graph(plot_infos)
    # h, l = plot_infos['ax'].get_legend_handles_labels()
    # plt.legend(h, plot_infos['ledgends'], bbox_to_anchor=(0.40, -0.30), loc='center', borderaxespad=0, ncol=2, fontsize=plot_infos['font_size'])
    # plt.legend(h, plot_infos['ledgends'], bbox_to_anchor=(-1.00, -0.50), loc='center', borderaxespad=0, ncol=2)
    # plt.subplots_adjust(bottom=0.15)
    # plt.savefig(plot_infos['plot_target_dir'] + '/' + 'all' + plot_infos['result_suffix'] + '.pdf')
    # plt.close()
    plt.savefig(plot_infos['plot_target_dir'] + '/' + 'all' + plot_infos['result_suffix'] + '.pdf')
    plt.close()

def abort_freq(plot_infos):
    plot_infos['result_suffix'] = '_abortfreq'
    result_file_template = ['use_mmap']
    # plot_infos['ledgends'] = ['NVHTM']
    data_list = []
    for (bench_name, num) in zip(plot_infos['bench_names'], range(1, len(plot_infos['bench_names']) + 1)):
        # print("plot", bench_name, "@cplog")
        # plot_infos['title'] = bench_name
        # plot_infos['ax'] = f.add_subplot(3, 3, num)
        # plot_infos['bench_name'] = bench_name
        csvfile = pd.read_csv(plot_infos['abort_fn_generator'](bench_name), index_col=0)
        data_list.append(csvfile['abortps'][8])
    plot_infos['plot_data'] = pd.DataFrame([data_list], index = ['abortps'], columns = plot_infos['bench_names']).T
    plot_infos['ylabel'] = 'アボート頻度（回/秒）'
    plot_graph(plot_infos)
    # h, l = plot_infos['ax'].get_legend_handles_labels()
    # plt.legend(h, plot_infos['ledgends'], bbox_to_anchor=(0.40, -0.30), loc='center', borderaxespad=0, ncol=2, fontsize=plot_infos['font_size'])
    # plt.legend(h, plot_infos['ledgends'], bbox_to_anchor=(-1.00, -0.70), loc='center', borderaxespad=0, ncol=2)
    # plt.subplots_adjust(bottom=0.20)
    plt.savefig(plot_infos['plot_target_dir'] + '/' + 'all' + plot_infos['result_suffix'] + '.pdf')
    plt.close()

def main():
    plot_infos = {}
    plot_infos['bench_names'] = ['genome', 'vacation-high', 'vacation-low', 'yada', 'ssca2', 'intruder', 'kmeans-high', 'kmeans-low', 'labyrinth']
    # plot_infos['colorlst'] = ['#fecc5c', '#fd8d3c', '#f03b20', '#bd0026']
    plot_infos['colorlst'] = ['#aaaaaa', '#555555', '#000000']
    plot_infos['font_size'] = 18
    plot_infos['plot_target_dir'] = "graphs/stats"
    plot_infos['writeratio_source'] = 'bench_stats/use_mmap/stats.csv'
    plot_infos['abort_fn_generator'] = lambda bench_name: 'aborts/use_mmap/abort_' + bench_name + '.csv'
    # result_file_template = ['use_mmap', 'para_cp', 'log_comp', 'para_cp_dram']
    # result_file_template = ['use_mmap', 'use_mmap_dram', 'log_comp', 'no_cp', 'no_cp_dram']
    # ledgends = ['NVM(1スレッド)', 'NVM(8スレッド)', 'NVM(ログ圧縮)', 'エミュレータ(1スレッド)']
    # ledgends = ['NVM(CP1スレッド)', 'DRAM(CP1スレッド)', 'NVM(CP8スレッド, ログ圧縮)', 'NVM(CPなし)', 'DRAM(CPなし)']
    # ledgends = ['NVM(CP1スレッド)', 'NVM(CP8スレッド, ログ圧縮)', 'NVM(CPなし)']
    # tx_ratio(plot_infos)
    write_freq(plot_infos)
    abort_freq(plot_infos)

main()
