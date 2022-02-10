import numpy as np
from matplotlib import pyplot as plt
import pandas as pd
import japanize_matplotlib
import sys
ops = ['insert', 'search', 'delete']
logsizes = ['41943328']

colorlst = ['#ffffb2', '#fecc5c', '#fd8d3c', '#e31a1c']
fontsize = 18
labels = ['ライブラリ', 'ログの適用範囲の決定', 'ログの適用', '永続化']

for logsize in logsizes:
    for op in ops:
        pmem_file = pd.read_csv(sys.argv[1] + '/checkpoint_' + op + '_plain.logsize.' + logsize + '.csv', index_col=0)
        vmem_file = pd.read_csv(sys.argv[2] + '/checkpoint_' + op + '_plain.logsize.' + logsize + '.csv', index_col=0)

        # print(pmem_file)
        # print(vmem_file)
        # print(pmem_file.index)

        xtick = np.array(range(len(pmem_file.index)))
        barwidth = 0.3

        rsv_bottom_p = np.zeros(len(pmem_file.index), dtype=int)
        cfd_bottom_p = rsv_bottom_p + pmem_file['reserve']
        alg_bottom_p = cfd_bottom_p + pmem_file['commit-finding']
        fls_bottom_p = alg_bottom_p + pmem_file['apply-log']
        top_p = fls_bottom_p + pmem_file['flush']

        rsv_bottom_v = np.zeros(len(vmem_file.index), dtype=int)
        cfd_bottom_v = rsv_bottom_v + vmem_file['reserve']
        alg_bottom_v = cfd_bottom_v + vmem_file['commit-finding']
        fls_bottom_v = alg_bottom_v + vmem_file['apply-log']

        fig = plt.figure(figsize=(7, 6))
        plt.xticks(xtick, pmem_file.index)
        plt.bar(xtick-barwidth/2, pmem_file['reserve']       , bottom=rsv_bottom_p, width=barwidth, edgecolor = 'k', color = colorlst[0], label = labels[0])
        plt.bar(xtick-barwidth/2, pmem_file['commit-finding'], bottom=cfd_bottom_p, width=barwidth, edgecolor = 'k', color = colorlst[1], label = labels[1])
        plt.bar(xtick-barwidth/2, pmem_file['apply-log']     , bottom=alg_bottom_p, width=barwidth, edgecolor = 'k', color = colorlst[2], label = labels[2])
        plt.bar(xtick-barwidth/2, pmem_file['flush']         , bottom=fls_bottom_p, width=barwidth, edgecolor = 'k', color = colorlst[3], label = labels[3])

        plt.bar(xtick+barwidth/2, vmem_file['reserve']       , bottom=rsv_bottom_v, width=barwidth, edgecolor = 'k', color = colorlst[0])
        plt.bar(xtick+barwidth/2, vmem_file['commit-finding'], bottom=cfd_bottom_v, width=barwidth, edgecolor = 'k', color = colorlst[1])
        plt.bar(xtick+barwidth/2, vmem_file['apply-log']     , bottom=alg_bottom_v, width=barwidth, edgecolor = 'k', color = colorlst[2])
        plt.bar(xtick+barwidth/2, vmem_file['flush']         , bottom=fls_bottom_v, width=barwidth, edgecolor = 'k', color = colorlst[3])

        plt.xlabel('スレッド数', fontsize=fontsize)
        plt.ylabel('Checkpointプロセスの実行時間内訳 (秒)', fontsize=fontsize)
        plt.tick_params(labelsize=fontsize)
        plt.legend(bbox_to_anchor=(0.50, -0.20), loc='center', borderaxespad=0, ncol=2, fontsize=fontsize-4)
        plt.text(1.75, max(top_p)-1, '左：不揮発性メモリ，右：DRAM', fontsize=fontsize-4)
        # plt.text(1, max(top_p)-max(top_p)/10, '左：B${^+}$-Tree${_{NH}}$，右：B${^+}$-Tree${_{NH}}$(DRAM)', fontsize=fontsize-4)
        plt.tight_layout()
        plt.savefig('checkpoint_' + op + '_plain_logsize_' + logsize + '.pdf')
        plt.close()

        # fig, ax = plt.subplots()
        # plt.xticks(xtick, pmem_file.index)
        # plt.bar(xtick-barwidth/2, pmem_file['End']       , width=barwidth, edgecolor = 'k', color = colorlst[1], label = 'B${^+}$-Tree${_{NH}}$')
        # plt.bar(xtick+barwidth/2, vmem_file['End']       , width=barwidth, edgecolor = 'k', color = colorlst[2], label = 'B${^+}$-Tree${_{NH}}$(DRAM)')
        # plt.xlabel('スレッド数', fontsize=fontsize)
        # plt.ylabel('処理時間 (秒)', fontsize=fontsize)
        # ax.yaxis.offsetText.set_fontsize(18)
        # ax.ticklabel_format(style="sci", axis="y", scilimits=(0,0))
        # plt.tick_params(labelsize=fontsize)
        # plt.legend(fontsize=fontsize-2)
        # plt.tight_layout()
        # plt.savefig('endwait_' + op + '_plain_logsize_' + logsize + '.pdf')
        # plt.close()
