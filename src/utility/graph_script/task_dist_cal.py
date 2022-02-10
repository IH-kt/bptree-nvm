import sys
import re
import numpy as np
from scipy.stats.mstats import gmean

if (len(sys.argv) < 2):
    print('too few arguments:', sys.argv)
    sys.exit()

source_file = sys.argv[1]
print('processing:', source_file)
with open(source_file) as f:
    source_lines = f.readlines()

task_lines = [float(re.split('TASK DISTRIBUTION: ', line)[1]) for line in source_lines if 'TASK' in line]

print('gmean:', gmean(task_lines))
