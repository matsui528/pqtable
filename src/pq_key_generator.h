#ifndef PQTABLE_PQ_KEY_GENERATOR_H
#define PQTABLE_PQ_KEY_GENERATOR_H

// Given an original vector and codewords,
// PQKeyGenerator compute candidates of PQ codes (PQKey), by which
// the hash-table is fetched.
//
// This class is explained in Alg. 3 in Y. Matsui et al.,
// "PQTable: Non-exhaustive Fast Search for
// Product-quantized Codes using Hash Tables", arXiv 2017

#include <opencv2/opencv.hpp>
#include "pq.h"
#include "code_to_key.h"
#include <unordered_set>


namespace pqtable {

struct PQKey{
    PQKey() {}
    PQKey(uint key_, float dist_) : key(std::move(key_)), dist(std::move(dist_)) {}
    uint key;
    float dist;
};



class PQKeyGenerator
{
public:
    PQKeyGenerator(const std::vector<float> &vec,
                   const std::vector<PQ::Array> &codewords);

    void NextKey(PQKey *pq_key);


private:
    PQKeyGenerator();

    int m_M;
    int m_Ks;
    int m_Ds;

    // --- DistKsId (element of m_sortedDTable) -----
    struct DistKsId{
        DistKsId() : dist(-1), ks(0), sorted_id(0) {}
        DistKsId(float dist_, int ks_) : dist(dist_), ks(ks_), sorted_id(0) {}
        float dist;
        uchar ks;
        uchar sorted_id; // After sorted, its id
    };
    std::vector<std::vector<DistKsId> > m_sortedDTable; // [m][sorted_id], where sorted_id is 0 - m_Ks


    // ---- Cand (element of priority queue. Cand contaisn M DistKsId.) ---
    struct Cand{
        Cand() {}
        Cand(const std::vector<DistKsId> &dist_ks_ids_);
        void UpdateDist(); // from dist_ks_ids, compute dist

        void ToUInt(uint *key) const;
        void ToPQKey(PQKey *pq_key) const;

        std::vector<DistKsId> dist_ks_ids;
        float dist; // sum of dists in dist_ks_ids

    };
    std::vector<Cand> NextCands(const Cand &cand); // Expand cand, then create M new_cand


    // -- PriorityQueue for cands ---
    class PriorityQueue{
    public:
        //PriorityQueue(){m_identities.init(64);}
        void Push(const Cand &cand);
        void Pop(Cand *cand_dist_min);

    private:
        std::vector<Cand> m_cands;
        std::unordered_set<uint> m_identities; // used for checking duplicate ids
    };

    PriorityQueue m_candidate;
};

}

#endif // PQTABLE_PQ_KEY_GENERATOR_H
