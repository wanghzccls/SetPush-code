# VLDB 2023: Estimating Single-Node PageRank in $\tilde{O}\left(\min\{d_t, \sqrt{m}\}\right)$ Time



## Environment:
- Ubuntu 16.04.10
- g++ 11
- GCC 5.4.0


## Dataset:
We provide a toy dataset in the dataset directory. All datasets used in the paper are publically avaiable at: 
- http://snap.stanford.edu/data
- http://law.di.unimi.it/datasets.php



## Compile and Run the Code:
```
bash run_script.sh
```




## Parameters:  
- -d \<directory\> (e.g., dataset/youtube/) 
- -f \<filelabel\> (e.g. youtube)
- -algo \<algorithm\> (e.g., SetPush)
- -er \<the relative error parameter c\> (default 0.5)
- -qn \<# of query nodes\> (default 10)
- -alpha \<dampling factor\> (default 0.2)
- -pf \<failure probability\> (default 0.1)
- -qflag \<ID for the query set\> (-qflag 1 for using the query nodes which are uniformly sampled from the graph G, and -qflag 2 for using the query nodes which are sampled according to the degree distribution. The default value of qflag is 1)



## Instructions:
- The "datatset" directory (./dataset/) is used for storing the dataset file. 
- The "query" directory (./query/) is used for storing all the query sets. 
- The "result" directory (./result/) is used for storing all output files. 
