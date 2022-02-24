import numpy as np
from matplotlib import pyplot as plt
import matplotlib as mpl
import japanize_matplotlib
import sys
import os
import re

def main():
    plot_infos = {}
    plot_infos['bench_names'] = ['genome', 'vacation-high', 'vacation-low', 'yada', 'ssca2', 'intruder', 'kmeans-high', 'kmeans-low', 'labyrinth']
    # plot_infos['colorlst'] = ['#fecc5c', '#fd8d3c', '#f03b20', '#bd0026']
    plot_infos['colorlst'] = ['#aaaaaa', '#555555', '#000000']
    plot_infos['markerlst'] = ['*', 'v', '^', '+']
    plot_infos['font_size'] = 18
    plot_infos['plot_target_dir'] = "graphs/txsize"
    plot_infos['fn_generator'] = lambda bench_name: lambda extype: 'scaling/' + extype + '/time_' + bench_name + '.csv'
    plot_infos['linestyle'] = ['solid', 'dashed', 'dotted', 'dashdot']
    # result_file_template = ['use_mmap', 'para_cp', 'log_comp', 'para_cp_dram']
    # result_file_template = ['use_mmap', 'use_mmap_dram', 'log_comp', 'no_cp', 'no_cp_dram']
    # ledgends = ['NVM(1スレッド)', 'NVM(8スレッド)', 'NVM(ログ圧縮)', 'エミュレータ(1スレッド)']
    # ledgends = ['NVM(CP1スレッド)', 'DRAM(CP1スレッド)', 'NVM(CP8スレッド, ログ圧縮)', 'NVM(CPなし)', 'DRAM(CPなし)']
    # ledgends = ['NVM(CP1スレッド)', 'NVM(CP8スレッド, ログ圧縮)', 'NVM(CPなし)']
    data_lists = []
    for bench_name in plot_infos['bench_names']:
        data_lists.append(np.loadtxt(os.path.abspath(sys.argv[1]) + '/test_txsize/' + bench_name + '/txsize_' + bench_name + '.dat', delimiter = ','))

    fig = plt.figure(figsize=(4,4))
    ax = fig.add_subplot(1, 1, 1)
    ax.boxplot(data_lists, whis=(25, 75), labels=plot_infos['bench_names'], sym='+')
    ax.set_xticklabels(plot_infos['bench_names'], rotation=45, ha='right')
    plt.xlabel('ベンチマーク')
    plt.ylabel('トランザクションサイズ')
    plt.ylim(bottom=0)
    os.makedirs(plot_infos['plot_target_dir'], exist_ok=True)
    plt.tight_layout()
    plt.savefig(plot_infos['plot_target_dir'] + '/' + 'txsizeall.png')

main()
