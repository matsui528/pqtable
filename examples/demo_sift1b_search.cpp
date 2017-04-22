#include "pq_table.h"
#include "utils.h"


int main(int argc, char *argv []){
    int top_k;
    assert(argc == 1 || argc == 2);
    if(argc == 1){
        top_k = 1;
    }else{
        top_k = atoi(argv[1]);
    }
    std::cout << "top_k: " << top_k << std::endl;


    // (1) Make sure you've already run "demo_sift1b_train", "demo_sift1b_encode",
    //     "demo_sift1b_build_table". The "pqtable" dir must be in the bin dir.

    // (2) Read query vectors
    std::vector<std::vector<float> > queries = pqtable::ReadTopN("../../data/bigann_query.bvecs", "bvecs");

    // (3) Read the PQTable
    pqtable::PQTable table("pqtable");

    // (4) Search

    // ranked_scores[q][k] : top-k th result of the q-th query.
    std::vector<std::vector<std::pair<int, float> > >
            ranked_scores(queries.size(), std::vector<std::pair<int, float> >(top_k));

    double t0 = pqtable::Elapsed();
    for(int q = 0; q < (int) queries.size(); ++q){
        ranked_scores[q] = table.Query(queries[q], top_k);
    }
    std::cout << (pqtable::Elapsed() - t0) / queries.size() * 1000 << " [msec/query] " << std::endl;

    // (5) Write scores
    pqtable::WriteScores("score.txt", ranked_scores);


    return 0;
}
