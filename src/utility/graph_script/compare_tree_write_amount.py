import pandas as pd
import numpy as np
from matplotlib import pyplot as plt
import matplotlib as mpl
import japanize_matplotlib

colorlst = ['#fecc5c', '#fd8d3c', '#f03b20', '#bd0026']

# type_list = ['bptree_nvhtm_0', 'bptree_nvhtm_1']
type_list = ['bptree_nvhtm_0', 'fptree_concurrent_0']
# type_list = ['bptree_nvhtm_0']
# type_list = ['bptree_nvhtm_1']
op_list = ['insert', 'delete', 'search']
op_list_j = ['挿入', '削除', '検索']
thr_list = [1, 2, 4, 8, 16] # TODO
# log_list = [33056, 65824, 131360, 262432, 524576, 1048864] # TODO
# log_list = [2621728, 5243168, 10486048, 20971808, 41943328, 83886368] # TODO
# log_list = [10486048, 20971808, 41943328, 83886368] # TODO
log_list = [41943328] # TODO
mem_list = ['pmem']
font_size = 18
# labels = ['Worker', 'Checkpoint', 'Filtered']
# labels = ['Writes to log', 'Log reflection', 'Optimized']
labels = ['ログへの書き込み', 'ログの反映', '最適化による減少']

for i in range(0, len(op_list)):
    csvs = []
    csv = pd.read_csv('write_amount/' + 'pmem' + '/' + type_list[0] + '/write_amount_' + op_list[i] + '.csv', index_col=0, usecols=[2,3,4,5])
    csv['worker'] /= 1000000000
    csv['checkpoint'] /= 1000000000
    print(csv)
    csvs.append(csv)
    csv = pd.read_csv('write_amount/' + 'pmem' + '/' + type_list[1] + '/write_amount_' + op_list[i] + '.csv', index_col=0, usecols=[2,3,4])
    csv['worker'] /= 1000000000
    csv['checkpoint'] /= 1000000000
    print(csv)
    csvs.append(csv)
    cp_bottoms = []
    for csv in csvs:
        cp_bottoms.append(csv['worker'])
    xtick = np.array(range(len(csvs[0].index)))
    barwidth = 0.3
    # if i == 0:
    #     plt.ylim(top=2200000)
    # else:
    #     plt.ylim(top=1500000)
    fig = plt.figure(figsize=(7, 7))
    plt.xticks(xtick, csvs[0].index)

    plt.bar(xtick-barwidth/2, csvs[0]['worker'], width=barwidth, edgecolor = 'k', color = colorlst[0], label = labels[0])
    plt.bar(xtick-barwidth/2, csvs[0]['checkpoint'], width=barwidth, edgecolor = 'k', color = colorlst[1], label = labels[1], bottom=cp_bottoms[0])
    csvs[0]['nofil'] /= 1000000000
    csvs[0]['nofil'] -= csvs[0]['checkpoint']
    plt.bar(xtick-barwidth/2, csvs[0]['nofil'], width=barwidth, edgecolor = 'k', color = colorlst[2], label = labels[2], bottom=cp_bottoms[0]+csvs[0]['checkpoint'])

    plt.bar(xtick+barwidth/2, csvs[1]['worker'], width=barwidth, edgecolor = 'k', color = colorlst[0])
    plt.bar(xtick+barwidth/2, csvs[1]['checkpoint'], width=barwidth, edgecolor = 'k', color = colorlst[1], bottom=cp_bottoms[1])

    plt.xlabel('スレッド数', fontsize=font_size)
    # plt.xlabel('Number of Threads', fontsize=font_size)
    plt.ylabel('書き込み総量（GB）', fontsize=font_size)
    # plt.ylabel('Write Amount（GB）', fontsize=font_size)
    plt.tick_params(labelsize=font_size)
    plt.xticks(rotation=0)
    plt.legend(labels, fontsize=font_size, bbox_to_anchor=(0.40, -0.30), loc='center', borderaxespad=0, ncol=2)
    plt.ylim(top=1.3*max(csvs[0]['worker'] + csvs[0]['nofil']))
    plt.text(1.5, 1.2*max(csvs[0]['worker'] + csvs[0]['nofil']), '左：NV-HTM，右：FPTree', fontsize=font_size-4)
    # plt.text(1.5, 1.2*max(csvs[0]['worker'] + csvs[0]['nofil']), 'left: NV-HTM，right: FPTree', fontsize=font_size-4)
    plt.tight_layout()
    # plt.title('スレッド数によるアボート回数の変化（' + op_list_j[i] + '）')
    #plt.savefig('abort_' + nvhtm_type + '_' + op_list[i] + '_' + str(logsz) + '.png')
    plt.savefig('write_amount/write_amount_' + op_list[i] + '_presen_ja.pdf')
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
