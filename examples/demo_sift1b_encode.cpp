#include "pq_table.h"
#include "utils.h"


std::vector<std::vector<uchar> > EncodeParallel(const pqtable::PQ &pq, const std::vector<std::vector<float> > &vecs){
    std::vector<std::vector<uchar> > codes(vecs.size());
    #pragma omp parallel for
    for(int i = 0; i < (int) vecs.size(); ++i){
        codes[i] = pq.Encode(vecs[i]);
    }
    return codes;
}



int main(){
    // (1) Make sure you've already run "demo_sift1b_train", and "codewords.txt" is in the bin dir.

    // (2) Setup a product quantizer
    pqtable::PQ pq(pqtable::PQ::ReadCodewords("codewords.txt"));


    // (3) Setup pqcodes
    std::vector<std::vector<uchar> > codes;


    // (4) Encode in parallel
    pqtable::ItrReader reader("../../data/bigann_base.bvecs", "bvecs");
    std::vector<std::vector<float> > buff;  // Buffer
    int buff_max_size = 10000000;

    std::cout << "Start encoding" << std::endl;
    while(!reader.IsEnd()){
        buff.push_back(reader.Next());  // Read a line (a vector)
        if( (int) buff.size() == buff_max_size){  // If buff_max_size vectors are read, encode them.
            std::vector<std::vector<uchar> > buff_encoded = EncodeParallel(pq, buff); // Encode
            codes.insert(codes.end(), buff_encoded.begin(), buff_encoded.end()); // Push back
            buff.clear();
            std::cout << codes.size() << " / 1000000000 vectors are encoded in total" << std::endl;
        }
    }
    if(0 < (int) buff.size()){ // Rest
        std::vector<std::vector<uchar> > buff_encoded = EncodeParallel(pq, buff); // Encode
        codes.insert(codes.end(), buff_encoded.begin(), buff_encoded.end()); // Push back
    }


    // (5) Convert to UcharVecs, then write it
    pqtable::UcharVecs::Write("codes.bin", pqtable::UcharVecs(codes));

    return 0;
}
