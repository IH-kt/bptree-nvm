import numpy as np
from matplotlib import pyplot as plt
import pandas as pd
import japanize_matplotlib
import sys
ops = ['insert', 'search', 'delete']
logsizes = ['41943328']

colorlst = ['#ffeda0', '#feb24c', '#f03b20']
fontsize = 18
labels = ['Checkpointプロセスによるログ処理の待ち時間', 'トランザクションのアボートによる消費時間', 'スレッド間同期に費やした時間']
# labels = ['Wait time for log processing', 'Wasted time by aborting transaction']

for logsize in logsizes:
    for op in ops:
        pmem_file = pd.read_csv(sys.argv[1] + '/block_csv/wait_kmeans.csv', index_col=0)
        vmem_file = pd.read_csv(sys.argv[2] + '/block_csv/wait_kmeans.csv', index_col=0)

        # print(pmem_file)
        # print(vmem_file)
        # print(pmem_file.index)

        xtick = np.array(range(len(pmem_file.index)))
        barwidth = 0.3

        cpt_bottom_p = np.zeros(len(pmem_file.index), dtype=int)
        abt_bottom_p = cpt_bottom_p + pmem_file['Checkpoint-block']
        # end_bottom_p = abt_bottom_p + pmem_file['Abort']
        htm_bottom_p = abt_bottom_p + pmem_file['Abort']
        # top_p = htm_bottom_p + pmem_file['HTM-block']
        top_p = abt_bottom_p + pmem_file['Abort']
        # print(op)
        # print(max(top_p))
        # print('cpt + end:')
        # print(pmem_file['Checkpoint-block'] + pmem_file['End'])

        cpt_bottom_v = np.zeros(len(vmem_file.index), dtype=int)
        abt_bottom_v = cpt_bottom_v + vmem_file['Checkpoint-block']
        htm_bottom_v = abt_bottom_v + vmem_file['Abort']
        # print('cpt-ratio:')
        # print(vmem_file['Checkpoint-block']/pmem_file['Checkpoint-block'])
        # print('end-ratio:')
        # print(vmem_file['End']/pmem_file['End'])

        fig = plt.figure(figsize=(7, 7))
        plt.xticks(xtick, pmem_file.index)
        plt.bar(xtick-barwidth/2, pmem_file['Checkpoint-block'], bottom=cpt_bottom_p, width=barwidth, edgecolor = 'k', color = colorlst[0], label = labels[0])
        plt.bar(xtick-barwidth/2, pmem_file['Abort']           , bottom=abt_bottom_p, width=barwidth, edgecolor = 'k', color = colorlst[1], label = labels[1])
        # plt.bar(xtick, pmem_file['End']             , bottom=end_bottom_p, width=barwidth, edgecolor = 'k', color = colorlst[2], label = labels[2])
        plt.bar(xtick-barwidth/2, pmem_file['HTM-block']       , bottom=htm_bottom_p, width=barwidth, edgecolor = 'k', color = colorlst[2], label = labels[2])

        # plt.bar(xtick, pmem_file['Checkpoint-block'], bottom=cpt_bottom_p, width=barwidth, edgecolor = 'k', color = colorlst[0], label = labels[0])
        # plt.bar(xtick, pmem_file['Abort']           , bottom=abt_bottom_p, width=barwidth, edgecolor = 'k', color = colorlst[1], label = labels[1])

        plt.bar(xtick+barwidth/2, vmem_file['Checkpoint-block'], bottom=cpt_bottom_v, width=barwidth, edgecolor = 'k', color = colorlst[0])
        plt.bar(xtick+barwidth/2, vmem_file['Abort']           , bottom=abt_bottom_v, width=barwidth, edgecolor = 'k', color = colorlst[1])
        plt.bar(xtick+barwidth/2, vmem_file['HTM-block']       , bottom=htm_bottom_v, width=barwidth, edgecolor = 'k', color = colorlst[2])

        plt.xlabel('スレッド数', fontsize=fontsize)
        # plt.xlabel('Number of Threads', fontsize=fontsize)
        plt.ylabel('消費時間 (秒/スレッド)', fontsize=fontsize)
        # plt.ylabel('Wasted Time (sec. / thread)', fontsize=fontsize)
        plt.tick_params(labelsize=fontsize)
        # plt.legend(bbox_to_anchor=(0.50, -0.20), loc='center', borderaxespad=0, ncol=2, fontsize=fontsize-4)
        plt.legend(bbox_to_anchor=(0.45, -0.30), loc='center', borderaxespad=0, fontsize=fontsize-4)
        # plt.text(0.45, max(top_p)-1, 'left: NVM, right: DRAM', fontsize=fontsize-4)
        # plt.text(1, max(top_p)-1, '左：B${^+}$-Tree${_{NH}}$，右：B${^+}$-Tree${_{NH}}$(DRAM)', fontsize=fontsize-4)
        plt.text(0.5, max(top_p)-1, '左：NVM，右：エミュレータ', fontsize=fontsize-4)
        plt.tight_layout()
        plt.savefig('wait_' + op + '_plain_logsize_' + logsize + '_presen_ja.pdf')
        plt.close()

        # fig = plt.figure()
        # plt.xticks(xtick, pmem_file.index)
        # plt.bar(xtick-barwidth/2, pmem_file['End']       , width=barwidth, edgecolor = 'k', color = colorlst[1], label = 'B${^+}$-Tree${_{NH}}$')
        # plt.bar(xtick+barwidth/2, vmem_file['End']       , width=barwidth, edgecolor = 'k', color = colorlst[2], label = 'B${^+}$-Tree${_{NH}}$(DRAM)')
        # plt.xlabel('スレッド数', fontsize=fontsize)
        # plt.ylabel('処理時間 (秒)', fontsize=fontsize)
        # plt.tick_params(labelsize=fontsize)
        # plt.legend(fontsize=fontsize-2)
        # plt.tight_layout()
        # plt.savefig('endwait_' + op + '_plain_logsize_' + logsize + '.pdf')
        # plt.close()
