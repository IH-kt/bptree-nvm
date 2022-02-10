import numpy as np
from matplotlib import pyplot as plt
import pandas as pd
import japanize_matplotlib
import sys
ops = ['insert', 'search', 'delete']
logsizes = ['41943328']

colorlst = ['#ffffb2', '#fed976', '#feb24c', '#fd8d3c', '#f03b20', '#bd0026']
fontsize = 18
labels = ['初期化', 'ログエントリの選択', 'ログの読み出し', 'ライブラリの使用', 'ログの反映', '永続化']
# labels = ['Initialization', 'Selection of log entry', 'Reflection of log', 'Persisting']

for logsize in logsizes:
    for op in ops:
        pmem_file = pd.read_csv(sys.argv[1] + '/checkpoint_' + op + '_plain.logsize.' + logsize + '.csv', index_col=0)
        # vmem_file = pd.read_csv(sys.argv[2] + '/checkpoint_' + op + '_plain.logsize.' + logsize + '.csv', index_col=0)

        # print(pmem_file)
        # print(vmem_file)
        # print(pmem_file.index)

        xtick = np.array(range(len(pmem_file.index)))
        barwidth = 0.3

        rsv_bottom_p = np.zeros(len(pmem_file.index), dtype=int)
        cfd_bottom_p = rsv_bottom_p + pmem_file['reserve']
        rlg_bottom_p = cfd_bottom_p + pmem_file['commit-finding']
        lib_bottom_p = rlg_bottom_p + pmem_file['log-read']
        apl_bottom_p = lib_bottom_p + pmem_file['library']
        fls_bottom_p = apl_bottom_p + pmem_file['apply']
        top_p = fls_bottom_p + pmem_file['flush']

        # rsv_bottom_v = np.zeros(len(vmem_file.index), dtype=int)
        # cfd_bottom_v = rsv_bottom_v + vmem_file['reserve']
        # alg_bottom_v = cfd_bottom_v + vmem_file['commit-finding']
        # fls_bottom_v = alg_bottom_v + vmem_file['apply-log']

        fig = plt.figure(figsize=(5, 5))
        plt.xticks(xtick, pmem_file.index)
        plt.bar(xtick, pmem_file['reserve']       , bottom=rsv_bottom_p, width=barwidth, edgecolor = 'k', color = colorlst[0], label = labels[0])
        plt.bar(xtick, pmem_file['commit-finding'], bottom=cfd_bottom_p, width=barwidth, edgecolor = 'k', color = colorlst[1], label = labels[1])
        plt.bar(xtick, pmem_file['log-read'], bottom=rlg_bottom_p, width=barwidth, edgecolor = 'k', color = colorlst[2], label = labels[2])
        plt.bar(xtick, pmem_file['library'], bottom=lib_bottom_p, width=barwidth, edgecolor = 'k', color = colorlst[3], label = labels[3])
        plt.bar(xtick, pmem_file['apply'], bottom=apl_bottom_p, width=barwidth, edgecolor = 'k', color = colorlst[4], label = labels[4])
        plt.bar(xtick, pmem_file['flush'], bottom=fls_bottom_p, width=barwidth, edgecolor = 'k', color = colorlst[5], label = labels[5])
        # plt.bar(xtick-barwidth/2, pmem_file['flush']         , bottom=fls_bottom_p, width=barwidth, edgecolor = 'k', color = colorlst[3], label = labels[3])

        # plt.bar(xtick+barwidth/2, vmem_file['reserve']       , bottom=rsv_bottom_v, width=barwidth, edgecolor = 'k', color = colorlst[0])
        # plt.bar(xtick+barwidth/2, vmem_file['commit-finding'], bottom=cfd_bottom_v, width=barwidth, edgecolor = 'k', color = colorlst[1])
        # plt.bar(xtick+barwidth/2, vmem_file['apply-log']     , bottom=alg_bottom_v, width=barwidth, edgecolor = 'k', color = colorlst[2])
        # plt.bar(xtick+barwidth/2, vmem_file['flush']         , bottom=fls_bottom_v, width=barwidth, edgecolor = 'k', color = colorlst[3])

        plt.xlabel('ユーザスレッド数', fontsize=fontsize)
        # plt.xlabel('Number of threads', fontsize=fontsize)
        plt.ylabel('実行時間 (秒)', fontsize=fontsize)
        # plt.ylabel('Execution time (sec.)', fontsize=fontsize)
        plt.tick_params(labelsize=fontsize)
        plt.legend(bbox_to_anchor=(0.50, -0.35), loc='center', borderaxespad=0, ncol=2, fontsize=fontsize-4)
        # plt.text(0.5, max(top_p)-0.2, '左：NV-HTM，右：DRAM利用', fontsize=fontsize-4)
        # plt.text(0.00, max(top_p)-0.2, 'left: NVM, right: DRAM', fontsize=fontsize-4)
        # plt.text(1, max(top_p)-max(top_p)/10, '左：B${^+}$-Tree${_{NH}}$，右：B${^+}$-Tree${_{NH}}$(DRAM)', fontsize=fontsize-4)
        plt.ylim(top=15)
        plt.tight_layout()
        plt.savefig('checkpoint_' + op + '_plain_logsize_' + logsize + '_presen_ja.pdf')
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
