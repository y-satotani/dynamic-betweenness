#!/usr/bin/env python

import os
import random
import subprocess
from itertools import product

def main():
    edgelist_dir = os.path.join(
        os.path.dirname(os.path.abspath(__file__)), '..', 'data', 'artificial')
    edgelist_list = [
        os.path.join(edgelist_dir, fn)
        for fn in os.listdir(edgelist_dir) if fn.endswith('.edgelist')]
    bin_path = os.path.join(
        os.path.dirname(os.path.abspath(__file__)), 'exp-compare-performance')

    output_path = os.path.join(os.path.dirname(__file__),
                               'artificial-performance-comparison.csv')
    num_results = 100
    with open(output_path, 'w') as fp:
        fp.write('network,query,is-directed,is-weighted,is-connected,max-diff,time-proposed,time-igraph\n')
        for path, directed in product(edgelist_list, (True, False)):
            for _ in range(num_results):
                rseed = random.randint(0, 1000000)
                proc = subprocess.run(
                    [bin_path]
                    + (['-d'] if directed else [])
                    + ['-s', str(rseed), path, 'delete'],
                    capture_output=True,
                )
                stdout = proc.stdout.decode('utf-8')\
                                    .replace(edgelist_dir+'/', '')\
                                    .replace('.edgelist', '')
                fp.write(stdout)

if __name__ == '__main__':
    main()
