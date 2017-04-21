#include <opencv2/opencv.hpp>
#include "pq_table.h"
#include "utils.h"


int main(){
    // (1) Make sure you've already run "demo_sift1b_train" and "demo_sift1b_encode".
    //     "codewords.txt" and "codes.bin" must be in the bin dir.

    // (2) Setup a product quantizer
    std::vector<pqtable::PQ::Array> codewords = pqtable::PQ::ReadCodewords("codewords.txt");

    // (3) Read PQ codes
    std::cout << "Start to read pq codes" << std::endl;
    pqtable::UcharVecs codes = pqtable::UcharVecs::Read("codes.bin");

    // (4) Build and write a PQTable
    std::cout << "Start to build pqtable" << std::endl;
    pqtable::PQTable pq_table(codewords, codes);
    pq_table.Write("pqtable");  // write the table into "pqtable" dir.

    return 0;
}
