#include "pq_table.h"
#include "utils.h"


int main(){
    // (1) Make sure you have already downloaded sift1b data in data/ by scripts/download_siftsmall.sh

    // (2) Read vectors
    int top_n = 10000000; // Use top 10M vectors
    std::vector<std::vector<float> > learns = pqtable::ReadTopN("../../data/bigann_learn.bvecs", "bvecs", top_n);

    // (3) Train a product quantizer
    int M = 4;  // You can change this to 8. M=4: a PQ-code is 32-bit.  M=8: a PQ-code is 64-bit.
    std::cout << "=== Train a product quantizer ===" << std::endl;
    std::vector<pqtable::PQ::Array> codewords = pqtable::PQ::Learn(learns, M);
    pqtable::PQ::WriteCodewords("codewords.txt", codewords);

    return 0;
}
