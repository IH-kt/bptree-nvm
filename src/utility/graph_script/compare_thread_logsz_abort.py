import pandas as pd
from matplotlib import pyplot as plt
import matplotlib as mpl
import japanize_matplotlib

line_style = ['ro-', 'gv-', 'b^-', 'k*-']

type_list = ['fptree_concurrent_', 'bptree_concurrent_']
op_list = ['insert', 'delete', 'search']
op_list_j = ['挿入', '削除', '検索']
thr_list = [1, 2, 4, 8, 16] # TODO
log_list = [65824] # TODO
memtypes = ['pmem', 'vmem']

for mem in memtypes:
    for tree_type in type_list:
        if mem == 'vmem' and tree_type == 'bptree_concurrent_':
            continue
        for i in range(0, len(op_list)):
            ab_csv = pd.read_csv('abort/' + mem + '/' + tree_type + '/abort_' + op_list[i] + '.csv', index_col=0);
            others = ab_csv['abort'] - ab_csv['conflict'] - ab_csv['capacity'] - ab_csv['explicit']
            others.name = 'other'
            abort_df = pd.concat([ab_csv, others], axis=1)
            del abort_df['abort']
            ax = abort_df.plot(kind='bar', stacked=True)
            plt.xlabel('スレッド数')
            plt.ylabel('アボート回数')
            plt.title('スレッド数によるアボート回数の変化（' + op_list_j[i] + '）')
                #plt.savefig('abort_' + nvhtm_type + '_' + op_list[i] + '_' + str(logsz) + '.png')
            plt.savefig('abort/' + mem + '/' + tree_type + '/abort_' + op_list[i] + '.eps')
    plt.close('all')
    
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
