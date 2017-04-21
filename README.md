# PQTable

**Product Quantization Table (PQTable)** is a fast nearest neighbor search method for product-quantized codes via hash-tables. PQTable achieves one of the fastest search performances on a single CPU to date (2017) with significantly efficient memory usage (0.059 ms per query over
10^9 data points with just 5.5 GB memory consumption).

References:

- Project page: [https://www.hal.t.u-tokyo.ac.jp/~matsui/project/pqtable/pqtable.html](https://www.hal.t.u-tokyo.ac.jp/~matsui/project/pqtable/pqtable.html)

- Y. Matsui, T. Yamasaki, and K. Aizawa, "**PQTable: Non-exhaustive Fast Search for Product-quantized Codes using Hash Tables**", arXiv 2017. [[paper](XXX)] (extended version of ICCV paper)

- Y. Matsui, T. Yamasaki, and K. Aizawa, "**PQTable: Fast Exact Asymmetric Distance Neighbor Search for Product Quantization using Hash Tables**", ICCV 2015. [[paper](https://www.hal.t.u-tokyo.ac.jp/~matsui/project/pqtable/doc/iccv2015.pdf)][[supplementary](https://www.hal.t.u-tokyo.ac.jp/~matsui/project/pqtable/doc/supplementary.pdf)]


## Building

Requisites:

* C++11

* OpenCV 3.X

* (optional, but strongly recommended) [TCMalloc](http://goog-perftools.sourceforge.net/doc/tcmalloc.html). On ubuntu, you can install TCMalloc by `sudo apt-get install google-perftools`. Then link it via the "-ltcmalloc" linker flag. This makes the search 2x times faster.

Build as usual with CMake:
```
$ mkdir build && cd build && cmake ..
$ make 
```

## Testing
### Demo using the siftsmall dataset
You can try a small demo using the siftsmall data. This does not take time.
First, please download the siftsmall vectors on `data/`.
```
$ bash scripts/download_siftsmall.sh 
```
Go to the bin directory, and run the demo
```
$ cd build/bin
$ ./demo_siftsmall
```
The program trains a product-quantizer, encodes vectors, builds PQTable, and runs the search. The final results will be somthing like:
```
...
93th query: nearest_id=6950, dist=49826.5
94th query: nearest_id=4053, dist=59649.2
95th query: nearest_id=7271, dist=76137.4
96th query: nearest_id=7512, dist=71723.1
97th query: nearest_id=7927, dist=68105
98th query: nearest_id=6289, dist=15346.3
99th query: nearest_id=8082, dist=54443.8
0.113859 [msec/query]
```
Note that these results (the nearest_ids and the distances) might be slightly different from yours because the training step includes a random process.



### Demo using the sift1b dataset
You can try a large-scale demo using the sift1b data. This demo reproduces the results of Table 3 in the paper.
Let's download the sift1b vectors on `data/`.
```
$ bash scripts/download_sift1b.sh 
```
This would take several hours. Since the data is large, ~200 GB disk space is required.
Next, go to the bin dir, and run the code for training a product quantizer.
```
$ cd build/bin
$ ./demo_sift1b_train
```
This also takes several hours. If you do not care about the quality of the product quantizer and just try the demo, please change `top_n` in the `demo_sift1b_train.cpp` to the small number such as 10000, and compile it again.
After finishing the training, the trained file `codewords.txt` is created on the `build/bin`. 
Next, encode input vectors into PQ codes.
```
$ ./demo_sift1b_encode
```
This would take one or two hours. You can get `codes.bin`, which is a set of PQ codes of base vectors in the sift1b data. It takes 4 GB (if M=4).
Then, build a PQTable using `codes.bin`.
```
$ ./demo_sift1b_build_table
```
A directory `pqtable` will be created, which contains the codewords and the table itself. Using these files, you can run the search.
```
$ ./demo_sift1b_search
```
You will have a result like this:
```
top_k: 1
0.0579275 [msec/query] 
```
The is the runtime per query for the sift1b data. You will also have `score.txt`, which contains the searched IDs. You can check a recall rate using an evaluation script.
```
$ cd ../..
$ python scripts/eval.py build/bin/score.txt data/gnd/idx_1000M.ivecs
```
This scripts evaluates the search result using the groundtruth annotation.
The reuslt will be:
```
Recall@1: 0.002
```
Note that you can see any top-k results by passing the argment in the search function, e.g.,
```
$ ./demo_sift1b_search 100   # This creates top-100 results on score.txt
$ cd ../..
$ python scripts/eval.py build/bin/score.txt data/gnd/idx_1000M.ivecs
```
Then you will see Recall@1, 2, 5, ..., 100.





 