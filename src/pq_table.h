#ifndef PQTABLE_PQ_TABLE_H
#define PQTABLE_PQ_TABLE_H


// PQTable
// Y. Matsui, T. Yamasaki, and K. Aizawa, "PQTable: Non-exhaustive Fast Search for
// Product-quantized Codes using Hash Tables", arXiv 2017
//
// Usage:
//   vector<vector<float>> train_vecs = /* set vec */
//   vector<vector<float>> base_vecs = /* set vec */
//   vector<vector<float>> query_vecs = /* set vec */
//
//   int M = 4; /* The number of sub-vector */
//   pqtable::PQ pq(pqtable::PQ::Learn(train_vecs, M));   // Train product quantizer
//   pqtable::UcharVecs codes = pq.Encode(base_vecs);     // Encode vectors
//
//   /* PQTable is instantiated with the codewords and the PQ-codes. */
//   /* The best "T" is selected automaticallly. If T=1, the sinle-PQTable is
//   /* created. Otherwise, multi-PQTable is created.
//   pqtable::PQTable tbl(pq.GetCodewords(), codes);
//
//   /* Neareset neighbor search for 0-th query vector. */
//   /* score.first is the nearest-id. score.second is the distance between */
//   /* the 0-th query and the nearest-id-th base vec. */
//   pair<int, float> score = tbl.Query(query_vecs[0]);
//
//   /* Or, you can do top-k search */
//   /* scores[0] is the nearest result, and scores[1] is the second nearest result. */
//   int top_k = 3;
//   vector<pair<int, float>> scores = tbl.Query(query_vecs[0], top_k);

#include <opencv2/opencv.hpp>
#include <unordered_map>

#include "pq.h"
#include "code_to_key.h"
#include "pq_key_generator.h"
#include "sparse_hashtable/sparse_hashtable.h"
#include "sparse_hashtable/helper_sht.h"


namespace pqtable {

class I_PQTable // interface. abstract basic class.
{
public:
    virtual ~I_PQTable() {}

    virtual std::pair<int, float> Query(const std::vector<float> &query) = 0;   // for top-1 search
    virtual std::vector<std::pair<int, float> > Query(const std::vector<float> &query, int top_k) = 0;  // for top-k search
    virtual void Write(std::string dir_path) = 0;
};



class PQSingleTable: I_PQTable
{
public:
    PQSingleTable(const std::vector<PQ::Array> &codewords,
            const UcharVecs &pq_codes);
    PQSingleTable(std::string dir_path); // Read from saved files (a dir contains files)

    // Querying function.
    std::pair<int, float> Query(const std::vector<float> &query); // fot top-1
    std::vector<std::pair<int, float> > Query(const std::vector<float> &query, int top_k); // for top-k        

    // IO
    void Write(std::string dir_path);

private:
    PQSingleTable();

    PQ m_PQ;

    // Table
    SparseHashtable m_sHashTable;
};


class PQMultiTable: I_PQTable
{
public:
    PQMultiTable(const std::vector<PQ::Array> &codewords,
                 const UcharVecs &pq_codes,
                 int T);
    PQMultiTable(std::string dir_path);

    std::pair<int, float> Query(const std::vector<float> &query);
    std::vector<std::pair<int, float> > Query(const std::vector<float> &query, int top_k);

    // IO
    void Write(std::string dir_path);

    static int OptimalT(int B, int N) {
        return std::pow(2, std::round(std::log2(B / std::log2(N))));
    }
private:
    PQMultiTable();        

    int m_T;
    std::vector<std::vector<PQ::Array> > m_codewordsEach; // [t][m][ks][ds]
    std::vector<SparseHashtable> m_sHashTableEach; // [t]

    PQ m_PQ;
    UcharVecs m_codes; // PQ code itself

    // Helper function. Divide codewords into codewords_each
    void DivideCodewords(const std::vector<PQ::Array> &codewords,
                         int T,
                         std::vector<std::vector<PQ::Array> > *codewords_each);
};




// Proxy class. This class automatically select the best table from singletable of multitable
class PQTable{
public:
    PQTable(const std::vector<PQ::Array> &codewords,
            const UcharVecs &pq_codes,
            int T = -1); // If T == -1, then the best T is automatically selected

    PQTable(std::string dir_path); // Read from the saved dir

    ~PQTable();


    std::pair<int, float> Query(const std::vector<float> &query);
    std::vector<std::pair<int, float> > Query(const std::vector<float> &query, int top_k);

    // IO
    void Write(std::string dir_path);

private:
    PQTable(); // Default construct is prohibited
    PQTable(const PQTable &);     // In current implementation, copy is prohibited
    const PQTable &operator =(const PQTable &);      // In current implementation, copy is prohibited

    I_PQTable *m_table;
};

}

#endif // PQTABLE_PQ_TABLE_H
