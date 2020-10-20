import pandas as pd
from matplotlib import pyplot as plt
import matplotlib as mpl
import japanize_matplotlib
import sys

def plot_graph(target_dir, data, op, labels, colorlst, font_size):
    max_val = max(data['abort'])
    others = data['abort'] - data['conflict'] - data['capacity'] - data['explicit']
    others.name = 'other'
    abort_df = pd.concat([data, others], axis=1)
    del abort_df['abort']
    abort_df['conflict'] /= abort_df.index
    abort_df['capacity'] /= abort_df.index
    abort_df['explicit'] /= abort_df.index
    abort_df['other']    /= abort_df.index
    ax = abort_df.plot(kind='bar', stacked=True, color=colorlst, edgecolor='k')

    ax.yaxis.offsetText.set_fontsize(font_size)
    ax.ticklabel_format(style="sci", axis="y", scilimits=(0,0))

    plt.xlabel('スレッド数', fontsize=font_size)
    plt.ylabel('アボート回数/スレッド', fontsize=font_size)
    plt.tick_params(labelsize=font_size)
    plt.xticks(rotation=0)
    plt.ylim(top=10000000)
    plt.legend(labels, fontsize=font_size)
    plt.tight_layout()
    print(target_dir + '/abort_' + op + '.pdf')
    plt.savefig(target_dir + '/abort_' + op + '.pdf')
    plt.close()

#         ind = np.arange(len(thr_list))
#         for thr in thr_list:
#             thr_filter = abort_df['thread'] == thr
#             i = 0
#             for logsz in log_list:
#                 plt.bar(ind+i*bar_width, abort_df['conflict'], width=bar_width, color='r', label='conflict')
#                 btm = abort_df['conflict'].values
#                 plt.bar(ind+i*bar_width, abort_df['capacity'], width=bar_width, bottom=btm, color='r', label='capacity')
    # ab_csv.plot(kind='bar', stacked=True)
    # plt.show()
def main():
    colorlst = ['#fecc5c', '#fd8d3c', '#f03b20', '#bd0026']
    font_size = 18
    ops = ['insert', 'mixed', 'delete', 'search']
    opnames = ['挿入', '削除', '検索']
    # tree_types = ['use_mmap', 'parallel_cp', 'log_compression', 'parallel_cp_dram_log', 'optimized_commit']
    # tree_types = ['use_mmap', 'no_cp', 'no_cp_dram', 'no_cp_optc', 'ex_small_leaf_use_mmap']
    tree_types = ['use_mmap', 'ex_small_leaf_use_mmap']
    tree_names = ['NVM+CP1スレッド', 'NVM+CP8スレッド', 'NVM+ログ圧縮+CP8スレッド', 'DRAM+CP8スレッド', 'NVM+ログ圧縮+ログflushなし+CP8スレッド']
    labels = ['競合', '書き込み上限超過', 'ログ容量不足', 'その他']

    if (len(sys.argv) < 2) :
        print("too few arguments")
        sys.exit()
    root_dir = sys.argv[1]

    for tree_type in tree_types:
        for op in ops:
            # dataframes = read_csvs(result_file_dirs, op)
            print(tree_type)
            dataframe = pd.read_csv(root_dir + '/abort/' + tree_type + '/abort_' + op + '.csv', index_col=0)
            plot_graph('abort/' + tree_type, dataframe, op, labels, colorlst, font_size)

main()
