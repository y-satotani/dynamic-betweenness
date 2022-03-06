#!/usr/bin/env python

import os
import random
import networkx as nx
from multiprocessing import Pool

def create_single_network(args):
    topo, n, k, seed = args
    E = create_seed_network(args)

    for weighted in (True, False):
        if weighted:
            weights = [
                random.randint(1, 5) for _ in range(len(E))]
        else:
            weights = [None] * len(E)

        path = make_network_path(topo, n, k, weighted, seed)
        contents = '\n'.join(
            f'{u} {v} {w}' if w else f'{u} {v}' for (u, v), w in zip(E, weights)
        )
        with open(path, 'w') as fp:
            print(contents, file=fp)

def create_seed_network(args):
    topo, n, k, seed = args
    def shuffle(e):
        f = list(e)
        random.shuffle(f)
        return f
    random.seed(seed)

    # generate strongly connected graph
    if(topo == 'RRG'):
        G = nx.random_regular_graph(k, n)
    elif(topo == 'BA'):
        G = nx.barabasi_albert_graph(n, k//2)

    E = list(shuffle(e) for e in G.edges())
    return E

def make_network_path(topo, n, k, weighted, seed):
    here = os.path.dirname(os.path.abspath(__file__))
    w_label = "weighted" if weighted else "unweighted"
    return os.path.join(here, f'{topo}-{n}-{k}-{w_label}-{seed:04d}.edgelist')

def main():
    # build networks
    args = [
        (topo, n, k, seed)
        for seed in range(1, 51)
        for topo in ('RRG', 'BA')
        for n in range(100, 1001, 100)
        for k in (4, 8)
        if n > k
    ]

    p = Pool()
    list(p.map(create_single_network, args))

if __name__ == '__main__':
    main()
