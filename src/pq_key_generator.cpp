#include "pq_key_generator.h"

namespace pqtable {

PQKeyGenerator::PQKeyGenerator(const std::vector<float> &vec, const std::vector<PQ::Array> &codewords)
{
    m_M = (int) codewords.size();
    if(4 < m_M){
        std::cerr << "Error: Currently, M<=4 is supported. M: " << m_M << std::endl;
        exit(1);
    }
    m_Ks = (int) codewords[0].size();
    m_Ds = (int) codewords[0][0].size();

    // ----- Setup sortedDTable ----
    m_sortedDTable = std::vector<std::vector<DistKsId> >(m_M, std::vector<DistKsId>(m_Ks, DistKsId(0, 0)));

    // Compute a distance from a query to each centroid
    for(int m = 0; m < m_M; ++m){
        for(int ks = 0; ks < m_Ks; ++ks){
            m_sortedDTable[m][ks].dist = 0.0;
            m_sortedDTable[m][ks].ks = (uchar) ks;
            for(int ds = 0; ds < m_Ds; ++ds){
                float diff = vec[m * m_Ds + ds] - codewords[m][ks][ds];
                m_sortedDTable[m][ks].dist += diff * diff;
            }
        }
        // Sort for each sub space
        std::sort(m_sortedDTable[m].begin(), m_sortedDTable[m].end(),
                  [](const DistKsId &a1, const DistKsId &a2){return a1.dist < a2.dist;});

        // Then, simply record its ids
        for(int sorted_id = 0; sorted_id < m_Ks; ++sorted_id){
            m_sortedDTable[m][sorted_id].sorted_id = (uchar) sorted_id;
        }
    }

    // Insert the first elem
    std::vector<PQKeyGenerator::DistKsId> nearest;
    for(int m = 0; m < m_M; ++m){
        nearest.push_back(m_sortedDTable[m][0]);
    }
    m_candidate.Push(Cand(nearest));
}


void PQKeyGenerator::NextKey(PQKey *pq_key) {
    Cand cand;
    m_candidate.Pop(&cand); // Pop the next-nearest code

    for(const auto &next_cand : NextCands(cand)){
        m_candidate.Push(next_cand);
    }

    cand.ToPQKey(pq_key);
}



std::vector<PQKeyGenerator::Cand> PQKeyGenerator::NextCands(const PQKeyGenerator::Cand &cand){
    std::vector<Cand> next_cands(m_M);

    for(int m = 0; m < m_M; ++m){
        next_cands[m].dist_ks_ids = cand.dist_ks_ids;
        uchar sorted_id = next_cands[m].dist_ks_ids[m].sorted_id;
        if(sorted_id == 255){ // Special case. if sorted_id reached the final ks, then simply return the same thing
            next_cands[m].UpdateDist();
        }else{
            next_cands[m].dist_ks_ids[m] = m_sortedDTable[m][sorted_id + 1]; // increment
            next_cands[m].UpdateDist();
        }
    }

    return next_cands;
}



PQKeyGenerator::Cand::Cand(const std::vector<PQKeyGenerator::DistKsId> &dist_ks_ids_) : dist_ks_ids(dist_ks_ids_){
    UpdateDist();
}

void PQKeyGenerator::Cand::UpdateDist(){
    dist = 0;
    for(const auto &dist_ks_id : dist_ks_ids){
        dist += dist_ks_id.dist;
    }
}

void PQKeyGenerator::Cand::ToUInt(uint *key) const
{
    int M = (int) dist_ks_ids.size();
    if(M == 4){
        *key = CodeToKey::Code4ToKey(dist_ks_ids[0].ks, dist_ks_ids[1].ks, dist_ks_ids[2].ks, dist_ks_ids[3].ks);
    }else if(M == 2){
        *key = CodeToKey::Code2ToKey(dist_ks_ids[0].ks, dist_ks_ids[1].ks);
    }else if(M == 1){
        *key = CodeToKey::Code1ToKey(dist_ks_ids[0].ks);
    }else{
        std::cerr << "Error: strange m in CandToPQKey. M: " << M << std::endl;
        exit(1);
    }
}

void PQKeyGenerator::Cand::ToPQKey(PQKey *pq_key) const
{
    ToUInt(&(pq_key->key));
    pq_key->dist = dist;
}


void PQKeyGenerator::PriorityQueue::Push(const PQKeyGenerator::Cand &cand){
    uint identity;
    cand.ToUInt(&identity);

    if(m_identities.find(identity) == m_identities.end()){ // does not contain
        m_identities.insert(identity);
        m_cands.push_back(cand);
        std::push_heap(m_cands.begin(), m_cands.end(),
                       [](const Cand &c1, const Cand &c2){return c1.dist > c2.dist;});
    }
}

void PQKeyGenerator::PriorityQueue::Pop(Cand *cand_dist_min) {
    std::pop_heap(m_cands.begin(), m_cands.end(),
                  [](const Cand &c1, const Cand &c2){return c1.dist > c2.dist;});
    *cand_dist_min = m_cands.back();
    m_cands.pop_back();
}

}
