import numpy as np
from matplotlib import pyplot as plt
import pandas as pd
import japanize_matplotlib
import sys
import os

def commit_plot(target_dir, data, op, labels, colorlst, font_size):
    xtick = np.array(range(len(data.index)))
    barwidth = 0.3

    clwb_bottom = np.zeros(len(data.index), dtype=int)
    wait_bottom = clwb_bottom + data['clwb']
    fence_bottom = wait_bottom + data['otherwait']

    fig = plt.figure(figsize=(5, 5))
    plt.xticks(xtick, data.index)

    plt.bar(xtick, data['clwb'], bottom=clwb_bottom, width=barwidth, edgecolor = 'k', color = colorlst[0], label = labels[0])
    plt.bar(xtick, data['otherwait'], bottom=wait_bottom, width=barwidth, edgecolor = 'k', color = colorlst[1], label = labels[1])
    plt.bar(xtick, data['fence'], bottom=fence_bottom, width=barwidth, edgecolor = 'k', color = colorlst[2], label = labels[2])

    plt.xlabel('スレッド数', fontsize=font_size)
    # plt.xlabel('Number of Threads', fontsize=fontsize)
    plt.ylabel('commit時間内訳(秒/スレッド)', fontsize=font_size-1)
    plt.ylim(top=4)
    # plt.ylabel('Wasted Time (sec. / thread)', fontsize=fontsize)
    plt.tick_params(labelsize=font_size)
    # plt.legend(bbox_to_anchor=(0.50, -0.20), loc='center', borderaxespad=0, ncol=2, fontsize=fontsize-4)
    plt.legend(bbox_to_anchor=(0.45, -0.30), loc='center', borderaxespad=0, fontsize=font_size-4, ncol=2)
    # plt.text(0.45, max(top_p)-1, 'left: NVM, right: DRAM', fontsize=fontsize-4)
    # plt.text(1, max(top_p)-1, '左：B${^+}$-Tree${_{NH}}$，右：B${^+}$-Tree${_{NH}}$(DRAM)', fontsize=fontsize-4)
    # plt.text(0.5, max(top_p)-1, '左：NV-HTM，右：DRAM利用', fontsize=fontsize-4)
    plt.tight_layout()
    plt.savefig(target_dir + '/commit_' + op + '.pdf')
    plt.close()

def plot_graph(target_dir, data, op, labels, colorlst, font_size):
    os.makedirs(target_dir, exist_ok=True)
    commit_plot(target_dir, data, op, labels, colorlst, font_size)


def main():
    colorlst = ['#fecc5c', '#fd8d3c', '#f03b20', '#bd0026']
    # colorlst = ['#feedde', '#fdbe85', '#fd8d3c', '#e6550d', '#a63603']
    font_size = 18
    ops = ['insert', 'search', 'delete', 'mixed']
    opnames = ['挿入', '検索', '削除', '混合']
    # tree_types = ['parallel_cp', 'log_compression', 'parallel_cp_dram_log', 'optimized_commit']
    # tree_types = ['no_cp', 'no_cp_dram', 'no_cp_optc']
    # tree_types = ['cp_initialize_only', 'cp_nothing', 'cp_nowrite']
    tree_types = ['cp_noflush']
    # tree_types = ['optimized_commit', 'no_cp_optc']
    # tree_types = ['use_mmap', 'parallel_cp', 'log_compression', 'parallel_cp_dram_log', 'optimized_commit', 'more_loop_compression']
    # tree_names = ['NVM+CP1スレッド', 'NVM+CP8スレッド', 'NVM+ログ圧縮+CP8スレッド', 'DRAM+CP8スレッド', 'NVM+ログ圧縮+ログflushなし+CP8スレッド', 'NVM+ログflushなし+CPなし']
    labels = ['CLWBの発行', '他スレッド待ち', 'SFENCE']
    # labels = ['Wait time for log processing', 'Wasted time by aborting transaction']
    # tree_types = ['log_compression', 'parallel_cp_dram_log']
    # tree_names = ['NVM(ログ圧縮+8スレッド)', 'DRAM(8スレッド)']

    if (len(sys.argv) < 2) :
        print("too few arguments")
        sys.exit()
    root_dir = sys.argv[1]
    # result_file_dirs = list(map(lambda x: root_dir + '/waittime/' + x, tree_types))

    for tree_type in tree_types:
        for op in ops:
            # dataframes = read_csvs(result_file_dirs, op)
            dataframe = pd.read_csv(root_dir + '/committime/' + tree_type + '/commit_' + op + '.csv', index_col=0)
            plot_graph('committime/' + tree_type, dataframe, op, labels, colorlst, font_size)

main()
