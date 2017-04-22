#include "pq_table.h"
#include "utils.h"


int main(){
    // (1) Make sure you have already downloaded siftsmall data in data/ by scripts/download_siftsmall.sh

    // (2) Read vectors
    std::vector<std::vector<float> > queries = pqtable::ReadTopN("../../data/siftsmall/siftsmall_query.fvecs", "fvecs");  // Because top_n is not set, read all vectors
    std::vector<std::vector<float> > bases = pqtable::ReadTopN("../../data/siftsmall/siftsmall_base.fvecs", "fvecs");
    std::vector<std::vector<float> > learns = pqtable::ReadTopN("../../data/siftsmall/siftsmall_learn.fvecs", "fvecs");


    // (3) Train a product quantizer
    int M = 4;
    std::cout << "=== Train a product quantizer ===" << std::endl;
    pqtable::PQ pq(pqtable::PQ::Learn(learns, M));


    // (4) Encode vectors to PQ-codes
    std::cout << "=== Encode vectors into PQ codes ===" << std::endl;
    pqtable::UcharVecs codes = pq.Encode(bases);


    // (5) Build a PQTable
    std::cout << "=== Build PQTable ===" << std::endl;
    pqtable::PQTable tbl(pq.GetCodewords(), codes);


    // (6) Do search
    std::cout << "=== Do search ===" << std::endl;
    double t0 = pqtable::Elapsed();
    for(int q = 0; q < (int) queries.size(); ++q){
        std::pair<int, float> result = tbl.Query(queries[q]);  // result = (nearest_id, its_dist)
        std::cout << q << "th query: nearest_id=" << result.first << ", dist=" << result.second << std::endl;
    }
    std::cout << (pqtable::Elapsed() - t0) / queries.size() * 1000 << " [msec/query]" << std::endl;


    return 0;
}
