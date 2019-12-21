import pandas as pd
from matplotlib import pyplot as plt
import matplotlib as mpl
import japanize_matplotlib

line_style = ['ro-', 'gv-', 'b^-', 'k*-']

type_list = ['plain', 'db']
op_list = ['insert', 'delete', 'search']
op_list_j = ['挿入', '削除', '検索']
thr_list = [1, 2, 4, 8, 16] # TODO
log_list = [33056, 65824, 131360, 262432, 524576, 1048864] # TODO

bar_width=0.2

for nvhtm_type in type_list:
    for i in range(0, len(op_list)):
        for thr in thr_list:
            wait_csv = pd.read_csv('wait_' + op_list[i] + '_' + nvhtm_type + '.thr.' + str(thr) + '.csv', index_col=0);
            max_val = max(wait_csv['logsize'])
            print(max_val)
            ax = wait_csv.plot(kind='bar', ylim=[0, max_val*1.05], stacked=True)
            plt.xlabel('ログ容量')
            plt.ylabel('スレッドあたりの待ち時間（s）')
            plt.title('ログ容量数別の待ち時間の変化（' + op_list_j[i] + '，スレッド数' + str(thr) + '）')
            #plt.savefig('abort_' + nvhtm_type + '_' + op_list[i] + '_' + str(logsz) + '.png')
            plt.savefig('wait_' + nvhtm_type + '_' + op_list[i] + '.thr.' + str(thr) + '.eps')
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
