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
mem_list = ['pmem', 'vmem']

for memtype in mem_list:
    for nvhtm_type in type_list:
        for i in range(0, len(op_list)):
            ab_csv = pd.read_csv('abort/' + memtype + '/abort_' + op_list[i] + '_' + nvhtm_type + '.csv', index_col=1, usecols=[1,2,3,4,5,6]);
            max_val = max(ab_csv['abort'])
            print(max_val)
            others = ab_csv['abort'] - ab_csv['conflict'] - ab_csv['capacity'] - ab_csv['explicit']
            others.name = 'other'
            abort_df = pd.concat([ab_csv, others], axis=1)
            for logsz in log_list:
                tmp_df = abort_df[abort_df['logsize'] == logsz]
                del tmp_df['abort']
                del tmp_df['logsize']
                ax = tmp_df.plot(kind='bar', stacked=True, ylim=[0, max_val*1.05])
                plt.xlabel('スレッド数')
                plt.ylabel('アボート回数')
                plt.title('スレッド数によるアボート回数の変化（' + op_list_j[i] + '）')
                #plt.savefig('abort_' + nvhtm_type + '_' + op_list[i] + '_' + str(logsz) + '.png')
                plt.savefig('abort/' + memtype + '/abort_' + nvhtm_type + '_' + op_list[i] + '_' + str(logsz) + '.eps')
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
