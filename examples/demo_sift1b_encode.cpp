#include "pq_table.h"
#include "utils.h"

// Encode vecs in parallel
std::vector<std::vector<uchar> > EncodeParallel(const pqtable::PQ &pq, const std::vector<std::vector<float> > &vecs){
    std::vector<std::vector<uchar> > codes(vecs.size());
    #pragma omp parallel for
    for(int i = 0; i < (int) vecs.size(); ++i){
        codes[i] = pq.Encode(vecs[i]);
    }
    return codes;
}

// Assign "added" to "base[start_id] - base[start_id + added.Size()"]
void Assign(pqtable::UcharVecs &base, int start_id, const pqtable::UcharVecs &added){
    assert(start_id + added.Size() <= base.Size());
    for(int i = 0; i < added.Size(); ++i){
        base.SetVec(start_id + i, added.GetVec(i));
    }
}


int main(){
    // (1) Make sure you've already run "demo_sift1b_train", and "codewords.txt" is in the bin dir.

    // (2) Setup a product quantizer
    pqtable::PQ pq(pqtable::PQ::ReadCodewords("codewords.txt"));


    // (3) Setup pqcodes
    int N = 1000000000;
    pqtable::UcharVecs codes(N, pq.GetM());  // a PQ-code is M-dim vec<uchar>.


    // (4) Encode in parallel.
    //     The codes below are the same as
    //         vector<vector<float>> bases = pqtable::ReadTopN("../../data/bigann_base.bvecs", "bvecs");
    //         pqtable::UcharVecs codes = pq.Encode(bases);
    //     However, this naive encoding is slow and memory-inefficient.
    //     So we repeat the following three steps: (1) reading N / 100 vectors in a buffer,
    //     (2) encoding the buffer in parallel, and (3) refresh the buffer.
    //     Since the buffer is refreshed everytime, the memory consumption is just the buffer.

    pqtable::ItrReader reader("../../data/bigann_base.bvecs", "bvecs");
    std::vector<std::vector<float> > buff;  // Buffer
    int buff_max_size = N / 100;
    int id_encoded = 0;

    std::cout << "Start encoding" << std::endl;
    while(!reader.IsEnd()){
        buff.push_back(reader.Next());  // Read a line (a vector) into the buffer
        if( (int) buff.size() == buff_max_size){  // (1) If buff_max_size vectors are read,
            Assign(codes, id_encoded, EncodeParallel(pq, buff)); // (2) Encode the buffer, and assign to the codes
            id_encoded += (int) buff.size();  // update id
            buff.clear(); // (3) refresh
            std::cout << id_encoded << " / " << N << " vectors are encoded in total" << std::endl;
        }
    }
    if(0 < (int) buff.size()){ // Rest
        Assign(codes, id_encoded, EncodeParallel(pq, buff)); // Encode buff, and assign to codes
    }


    // (5) Write codes
    pqtable::UcharVecs::Write("codes.bin", codes);

    return 0;
}
