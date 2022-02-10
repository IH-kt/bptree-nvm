import numpy as np
from matplotlib import pyplot as plt
import pandas as pd
import japanize_matplotlib
import sys
import os
import copy

def plot_graph(plot_infos):
    result_dataframes = []
    for result_file in plot_infos['source_files']:
        df = pd.read_csv(result_file, index_col=0)
        if (hasattr(df, 'flush')):
            df['apply-log'] += df['flush']
            df = df.rename(columns={'apply-log': 'apply-log-flush'})
            del(df['flush'])
        result_dataframes.append(df)

    os.makedirs(plot_infos['plot_target_dir'], exist_ok=True)

    ax = plot_infos['ax']
    xtick = np.arange(0, len(result_dataframes[0].index))
    x_position = -1 * len(plot_infos['source_files']) / 2.0
    barwidth = 0.3
    top_of_bar = 0
    for df in result_dataframes:
        xtick_tmp = list(map(lambda x: x + barwidth*x_position, xtick))
        # for (color, label, tc) in zip(colorlst, labels, time_class):
        # ax.bar(x = xtick_tmp, height = df['Time'], bottom=0, width=barwidth, edgecolor = 'k', color = plot_infos['colorlst'][-1], align='edge')
        bar_bottom = 0
        for (color, tc, hatch) in zip(plot_infos['colorlst'], plot_infos['time_class'], plot_infos['hatches']):
            # plt.bar(xtick+barwidth*x_position, df[tc], bottom=bar_bottom, width=barwidth, edgecolor = 'k', color = color, label = label)
            ax.bar(x = xtick_tmp, height = df[tc], bottom=bar_bottom, width=barwidth, edgecolor = 'k', color = color, align='edge', hatch=hatch)
            bar_bottom += df[tc]
        top_of_bar = max([max(bar_bottom), top_of_bar])
        x_position += 1
    # ax.get_legend().set_visible(False)
    plt.title(plot_infos['title'])
    plt.xticks(xtick, result_dataframes[0].index)
    plt.ylim(top = top_of_bar * 1.1)
    plt.xlabel('ユーザスレッド数')
    # plt.xlabel('Number of Threads', fontsize=font_size)
    plt.ylabel('実行時間 (秒)')
    # plt.ylabel('Wasted Time (sec. / thread)', fontsize=font_size)
    # plt.tick_params(labelsize=font_size)
    # plt.legend(bbox_to_anchor=(0.50, -0.20), loc='center', borderaxespad=0, ncol=2, fontsize=font_size-4)

def compare_logcomp(plot_infos):
    # f = plt.figure(figsize=(10, 11))
    f = plt.figure(figsize=(7, 8))
    plt.subplots_adjust(hspace=0.40, wspace=0.4)
    plot_infos['result_suffix'] = '_cpdram'
    result_file_template = ['use_mmap', 'log_comp_lp_single', 'log_comp_lp']
    plot_infos['plot_text'] = '左：Plain-NVHTM，中：Log-Elimination (single), 右：Log-Elimination (parallel)'
    plot_infos['time_class'] = ['reserve', 'commit-finding', 'apply-log-flush']
    for (bench_name, num) in zip(plot_infos['bench_names'], range(1, len(plot_infos['bench_names']) + 1)):
        print("plot", bench_name, "@cplog")
        plot_infos['title'] = bench_name
        plot_infos['ax'] = f.add_subplot(2, 2, num)
        plot_infos['bench_name'] = bench_name
        plot_infos['source_files'] = list(map(plot_infos['fn_generator'](bench_name), result_file_template))
        plot_graph(plot_infos)
    # h, l = plot_infos['ax'].get_legend_handles_labels()
    # plt.legend(h, plot_infos['ledgends'], bbox_to_anchor=(0.40, -0.30), loc='center', borderaxespad=0, ncol=2, fontsize=plot_infos['font_size'])
    plt.text(-6.0, -0.06, plot_infos['plot_text'])
    # plt.legend(h, plot_infos['labels'], bbox_to_anchor=(0, 0), loc='center', borderaxespad=0)
    plt.legend(plot_infos['labels'], bbox_to_anchor=(-0.3, -0.4), loc='center', borderaxespad=0, ncol=3)
    plt.subplots_adjust(bottom=0.20)
    plt.savefig(plot_infos['plot_target_dir'] + '/' + 'all' + plot_infos['result_suffix'] + '.pdf')

def compare_benches(plot_infos):
    f = plt.figure(figsize=(10, 11))
    plt.subplots_adjust(hspace=0.40, wspace=0.4)
    plot_infos['result_suffix'] = '_cpbc'
    result_file_template = ['use_mmap']
    plot_infos['plot_text'] = ''
    plot_infos['time_class'] = ['reserve', 'commit-finding', 'apply-log-flush']
    for (bench_name, num) in zip(plot_infos['bench_names'], range(1, len(plot_infos['bench_names']) + 1)):
        print("plot", bench_name, "@cpbc")
        plot_infos['title'] = bench_name
        plot_infos['ax'] = f.add_subplot(2, 2, num)
        plot_infos['bench_name'] = bench_name
        plot_infos['source_files'] = list(map(plot_infos['fn_generator'](bench_name), result_file_template))
        plot_graph(plot_infos)
    # h, l = plot_infos['ax'].get_legend_handles_labels()
    # plt.legend(h, plot_infos['ledgends'], bbox_to_anchor=(0.40, -0.30), loc='center', borderaxespad=0, ncol=2, fontsize=plot_infos['font_size'])
    plt.text(-6.5, -4.0, plot_infos['plot_text'])
    # plt.legend(h, plot_infos['labels'], bbox_to_anchor=(0, 0), loc='center', borderaxespad=0)
    plt.legend(plot_infos['labels'], bbox_to_anchor=(-0.7, -0.6), loc='center', borderaxespad=0)
    plt.subplots_adjust(bottom=0.20)
    plt.savefig(plot_infos['plot_target_dir'] + '/' + 'all' + plot_infos['result_suffix'] + '.pdf')

def main():
    plot_infos = {}
    # plot_infos['bench_names'] = ['genome', 'vacation-high', 'vacation-low', 'yada', 'ssca2', 'intruder', 'kmeans-high', 'kmeans-low', 'labyrinth']
    plot_infos['bench_names'] = ['ssca2', 'intruder', 'kmeans-high', 'kmeans-low']
    # plot_infos['bench_names'] = ['intruder', 'ssca2']
    plot_infos['fn_generator'] = lambda bench_name: lambda extype: 'checkpointtime/' + extype + '/checkpointtime_' + bench_name + '.csv'
    # plot_infos['colorlst'] = ['#000000', '#999999', '#bbbbbb', '#ffffff']
    plot_infos['colorlst'] = ['#000000', '#999999', '#ffffff']

    plot_infos['plot_target_dir'] = "graphs/checkpointtime"
    plot_infos['time_class'] = ['reserve', 'commit-finding', 'apply-log-flush']
    # labels = ['Checkpointプロセスによるログ処理の待ち時間', 'トランザクションのアボートによる消費時間', 'フォールバックによる待ち時間']
    plot_infos['labels'] = ['initialization', 'log-select', 'log-write']
    # plot_infos['labels'] = ['本来の処理にかかった時間', 'Checkpointプロセスによるログ処理の待ち時間', 'トランザクションのアボートによる消費時間']
    plot_infos['font_size'] = 18
    # plot_infos['hatches'] = ['', '', '//', '']
    plot_infos['hatches'] = ['', '', '', '']
    # plot_text = '左：NVM(1スレッド), 中央：NVM(8スレッド),\n右：NVM(ログ圧縮)'
    # plot_text = '左：Plain-NVHTM, 中央：Log-compression(single),\n右：Log-compression(parallel)'
    compare_logcomp(plot_infos)
    # compare_benches(plot_infos)

main()
