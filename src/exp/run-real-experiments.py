#!/usr/bin/env python

import os
import random
import subprocess
import networkx as nx
from itertools import chain, product
from multiprocessing import Pool

import time

def main():
    # build networks
    network_path = os.path.join(os.path.dirname(__file__), '..', 'data', 'real')
    edgelist_path = os.path.join(network_path, 'edgelists')
    network_list_path = os.path.join(network_path, 'networks.list')
    assert os.path.exists(network_list_path)

    output_path = os.path.join(os.path.dirname(__file__),
                               'real-performance-comparison.csv')
    cache_path = os.path.join(
        os.path.dirname(os.path.abspath(__file__)), 'exp-calculate-cache')
    bin_path = os.path.join(
        os.path.dirname(os.path.abspath(__file__)), 'exp-compare-performance')

    with open(network_list_path, 'r') as fp:
        network_list = [
            l.split() for l in fp.read().splitlines() if not l.startswith('#')]

    num_count = 100

    ofp = open(output_path, 'w')
    ofp.write('network,query,is-directed,is-weighted,is-connected,max-diff,time-proposed,time-igraph\n')

    for net, order, size, direction, weight in network_list:
        edgelist = os.path.join(edgelist_path, f'{net}.edgelist')
        diflag = '-d' if direction == 'directed' else ''
        if not os.path.exists(edgelist):
            print(f'Network {edgelist} not found.')
            print('Hint: download requires. run data/real/download.py.')
            continue

        print(f'Building cache for {net} (order: {order}, size: {size}, diflag: {diflag})')
        dybccache = f'{net}.dybccache'
        subprocess.run(
            [cache_path]
            + ([diflag] if diflag else [])
            + [edgelist, dybccache]
        )
        print(f'Finished building cache ({os.path.getsize(dybccache)} bytes)')

        print(f'Running experiment for {net}')
        for _ in range(num_count):
            rseed = random.randint(0, 1000000)
            proc = subprocess.run(
                [bin_path, '--seed', str(rseed)]
                + ([diflag] if diflag else [])
                + ['-c', dybccache, edgelist, 'delete'],
                capture_output=True
            )
            stdout = proc.stdout.decode('utf-8')\
                                    .replace(edgelist_path+'/', '')\
                                    .replace('.edgelist', '')
            ofp.write(stdout)

        print(f'Finish running experiment for {net}')
        os.remove(dybccache)

if __name__ == '__main__':
    main()
