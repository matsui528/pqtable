#include <opencv2/opencv.hpp>
#include "pq_table.h"
#include "utils.h"


int main(){
    // (1) Make sure you have already downloaded sift1b data in data/ by scripts/download_siftsmall.sh

    // (2) Read vectors
    int top_n = 10000000; // Use top 10M vectors
    std::vector<std::vector<float> > learns = pqtable::ReadTopN("../../data/bigann_learn.bvecs", "bvecs", top_n);

    // (3) Train a product quantizer
    int M = 4;
    std::cout << "=== Train a product quantizer ===" << std::endl;
    std::vector<pqtable::PQ::Array> codewords = pqtable::PQ::Learn(learns, M);
    pqtable::PQ::WriteCodewords("codewords.txt", codewords);

    return 0;
}
