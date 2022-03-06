# dynamic-betweenness
Source code for the paper "An algorithm for updating betweenness centrality scores of all vertices in a graph upon deletion of a single edge."

### Instructions for reproducing the results

- install requirements and build dependent library
- build `dybc` library and executable
- prepare synthetic/real networks
- run the executable and collect the results

#### Install requirements and build dependent library

- install `scons >= 4.2.0`, `python >= 3.6`, `networkx >= 2.0`
- clone official `igraph` using git, git-checkout to `0.9.6`, and install igraph manually (this step is necessary as the source depends on igraph's internal header files)

#### Build `dybc` library and executable

- run scons as following at `dynamic-betweenness/src`
```
scons IGRAPH_LOCATION=<the root directory of igraph source>
```
- the executables are installed to `dynamic-betweenness/src/release`

#### Prepare synthetic/real networks

- run following script at `dynamic-betweenness/src/data/artificial` to prepare synthesic networks
```
python make-artificial-networks.py
```
- to prepare real networks,
  1. download `edgelists.tar.gz` from [Google Drive](https://drive.google.com/file/d/11-CXQJ2e-ZlxFDTcG4fvCr4twPVL1psa/view?usp=sharing)
  2. move it to `dynamic-betweenness/src/data/real`
  3. untar `edgelists.tar.gz`


#### Run the executable and collect the results

- set current working directory to `dynamic-betweenness/src/release`
- run `python run-artificial-experiments.py` to conduct the experiment of synthesic networks
  - `artificial-performance-comparison.csv` is created
- run `python run-real-experiments.py` to conduct the experiment of real networks
  - `real-performance-comparison.csv` is created
- the two csv files have the following columns
  1. `network`: the name of the network
    - for synthesic networks, the value is formatted in `topology-n-k-weighted-seed`
  2. `query`: `delete`
  3. `is-directed`: whether the network is directed
  4. `is-weighted`: whether the network is weighted
  5. `is-connected`: whether the network is connected after deletion/insertion
  6. `max-diff`: the maximum difference between the BC value using proposed algorithm and the BC value using igraph
  7. `time-proposed`: the execution time to calculate BC values using the proposed algorithm (in seconds)
  8. `time-igraph`: the execution time for calculating BC values in the igraph program (in seconds)

