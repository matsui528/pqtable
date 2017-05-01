#include "pq_table.h"

namespace pqtable {

PQSingleTable::PQSingleTable(const std::vector<PQ::Array> &codewords, const UcharVecs &pq_codes) :
    m_PQ(codewords){
    assert(pq_codes.Dim() == m_PQ.GetM());

    if(4 < m_PQ.GetM()){
        std::cerr << "Error: Currently, only M<=4 is supported for single table" << std::endl;
        assert(0);
    }

    m_sHashTable.init(8 * m_PQ.GetM());
    for(int n = 0; n < pq_codes.Size(); ++n){
        uint key;
        CodeToKey::CodeToKey1(m_PQ.GetM(), pq_codes.GetVec(n), &key);
        m_sHashTable.insert(key, (uint) n);
    }
}

PQSingleTable::PQSingleTable(std::string dir_path) :
    m_PQ(PQ::ReadCodewords(dir_path + "/codeword.txt")){
    assert(dir_path.substr((int) dir_path.size() - 1) != "/"); // dir_path must be "som_dir". Not "some_dir/"
    HelperSparseHashtable::Read(dir_path + "/table.bin", &m_sHashTable); // Read hash table
}

std::pair<int, float> PQSingleTable::Query(const std::vector<float> &query) {
    PQKeyGenerator key_gen(query, m_PQ.GetCodewords());
    PQKey pqkey;

    while(1){
        key_gen.NextKey(&pqkey);
        int sz;
        uint *result = m_sHashTable.query(pqkey.key, &sz);
        if(result != NULL){
            return std::pair<int, float>(result[0], pqkey.dist); // return the first item
        }
    }
}

std::vector<std::pair<int, float> > PQSingleTable::Query(const std::vector<float> &query, int top_k) {
    assert(0 < top_k);

    // If top_k = 1, use a top-1 version
    if(top_k == 1){
        std::vector<std::pair<int, float> > score;
        score.push_back(Query(query));
        return score;
    }

    std::vector<std::pair<int, float> > found_scores;
    PQKeyGenerator key_gen(query, m_PQ.GetCodewords());

    PQKey pqkey;
    while(1){
        key_gen.NextKey(&pqkey);
        int sz;
        uint *result = m_sHashTable.query(pqkey.key, &sz);
        if(result != NULL){ // found items
            for(int i = 0; i < sz; ++i){
                found_scores.push_back(std::pair<int, float>(result[i], pqkey.dist));
            }
        }
        if(top_k <= (int) found_scores.size()){
            found_scores.resize(top_k);
            found_scores.shrink_to_fit();
            return found_scores;
        }
    }
}

void PQSingleTable::Write(std::string dir_path){
    assert(dir_path.substr((int) dir_path.size() - 1) != "/"); // dir_path must be "some_dir". Not "some_dir/"

    // This part can be re-written by boost::filesystem.
    std::string cmd = "mkdir -p " + dir_path;
    assert(!system(cmd.c_str())); // Create a directory where table files are written

    // Write "T"
    std::ofstream ofs(dir_path + "/T.txt");
    assert(ofs.is_open());
    ofs << 1;

    // Write codewords
    PQ::WriteCodewords(dir_path + "/codeword.txt", m_PQ.GetCodewords());

    // Write table
    HelperSparseHashtable::Write(dir_path + "/table.bin", m_sHashTable);
}





PQMultiTable::PQMultiTable(const std::vector<PQ::Array> &codewords, const UcharVecs &pq_codes, int T)
    : m_PQ(codewords)
{
    assert(1 < T && m_PQ.GetM() % T == 0);
    m_T = T;

    // Divide codeword into codewords_each
    DivideCodewords(codewords, m_T, &m_codewordsEach);

    // Setup hashtables
    int each_M = m_PQ.GetM() / m_T;
    m_sHashTableEach.resize(m_T);

    for(int t = 0; t < m_T; ++t){
        m_sHashTableEach[t].init(8 * each_M);
    }

    for(int n = 0; n < pq_codes.Size(); ++n){
        std::vector<uchar> code = pq_codes.GetVec(n);
        for(int t = 0; t < m_T; ++t){
            uint key;
            CodeToKey::CodeToKey1(each_M,
                                  std::vector<uchar>(code.begin() + each_M * t, code.begin() + each_M * (t + 1)),
                                  &key);
            m_sHashTableEach[t].insert(key, (uint) n);
        }
    }

    // Store original codes
    m_codes = pq_codes;
}

PQMultiTable::PQMultiTable(std::string dir_path) :  // Read from saved files (a dir contaings files)
    m_PQ(PQ::ReadCodewords(dir_path + "/codeword.txt"))
{
    // Read T
    std::ifstream ifs(dir_path + "/T.txt");
    assert(ifs.is_open());
    ifs >> m_T;

    // Set codesords each
    DivideCodewords(m_PQ.GetCodewords(), m_T, &m_codewordsEach);

    // Read tables
    m_sHashTableEach.resize(m_T);
    for(int t = 0;t < m_T; ++t){
        HelperSparseHashtable::Read(dir_path + "/table" + std::to_string(t) + ".bin", &(m_sHashTableEach[t]));
    }

    // Set pqcode
    UcharVecs::Read(dir_path + "/pqcode.bin", &m_codes);

}

std::pair<int, float> PQMultiTable::Query(const std::vector<float> &query) // fot top-1
{
    assert( (int) query.size() % m_T == 0);
    std::unordered_map<uint, int> count;


    PQ::Array dtable = m_PQ.DTable(query);

    // Setup key generator
    std::vector<PQKeyGenerator> key_gens;
    int each_D = (int) query.size() / m_T;
    for(int t = 0; t < m_T; ++t){
        key_gens.push_back(PQKeyGenerator(std::vector<float>(query.begin() + each_D * t,
                                                             query.begin() + each_D * (t + 1)),
                                          m_codewordsEach[t]));
    }


    std::vector<std::pair<int, float> > candidates;
    PQKey pqkey;
    while(1){
        // For each table, compute nearest ones
        for(int t = 0; t < m_T; ++t){
            key_gens[t].NextKey(&pqkey);
            int sz;
            uint *result = m_sHashTableEach[t].query(pqkey.key, &sz);
            if(result != NULL){ // found!
                for(int i = 0; i < sz; ++i){
                    uint id = result[i];
                    int c = ++count[id];

                    if(c == 1){ // if this is the first insert
                        candidates.emplace_back(id, m_PQ.AD(dtable, m_codes, id)); // Compute AD and store
                    }
                    if(c == m_T){ // m_T th times checked
                        float min_dist = FLT_MAX;
                        int min_i = -1;
                        for(const auto &cand : candidates){ // Evaluate candidates in count
                            if(cand.second < min_dist){
                                min_dist = cand.second;
                                min_i = cand.first;
                            }
                        }
                        assert(min_i != -1);
                        return std::pair<int, float>(min_i, min_dist);
                    }
                }
            }
        }

    }
}

std::vector<std::pair<int, float> > PQMultiTable::Query(const std::vector<float> &query, int top_k) // fot top-k
{
    assert(0 < top_k);
    assert( (int) query.size() % m_T == 0);

    // If top_k = 1, use a top-1 version
    if(top_k == 1){
        std::vector<std::pair<int, float> > score;
        score.push_back(Query(query));
        return score;
    }

    std::unordered_map<uint, int> count;

    PQ::Array dtable = m_PQ.DTable(query);

    // Setup key generator
    std::vector<PQKeyGenerator> key_gens;
    int each_D = (int) query.size() / m_T;
    for(int t = 0; t < m_T; ++t){
        key_gens.push_back(PQKeyGenerator(std::vector<float>(query.begin() + each_D * t,
                                                             query.begin() + each_D * (t + 1)),
                                          m_codewordsEach[t]));
    }


    std::vector<std::pair<int, float> > candidates;

    PQKey pqkey;
    while(1){
        // For each table, compute nearest ones
        for(int t = 0; t < m_T; ++t){
            key_gens[t].NextKey(&pqkey);
            int sz;
            uint *result = m_sHashTableEach[t].query(pqkey.key, &sz);
            if(result != NULL){ // found!
                for(int i = 0; i < sz; ++i){
                    uint id = result[i];
                    int c = ++count[id];

                    if(c == 1){ // if this is the first insert
                        candidates.emplace_back(id, m_PQ.AD(dtable, m_codes, id)); // Compute AD and store
                    }
                    if(c == m_T){ // m_T th times checked
                        float dist_min = m_PQ.AD(dtable, m_codes, id); // this computation is redundant.. Set bound

                        // From candidates, find element whose dist is less than dist_min
                        auto pos = std::partition(candidates.begin(), candidates.end(),
                                                  [&](const std::pair<int, float> &p){return p.second <= dist_min;});
                        int k_less_than_dist_min = std::distance(candidates.begin(), pos);

                        // If enough number of elemets have dist less than dist_min,
                        if(top_k <= k_less_than_dist_min){
                            candidates.resize(k_less_than_dist_min);
                            std::partial_sort(candidates.begin(), candidates.begin() + top_k, candidates.end(),
                                              [](const std::pair<int, float> &p1, const std::pair<int, float> &p2){return p1.second < p2.second;});
                            candidates.resize(top_k);
                            candidates.shrink_to_fit();
                            return candidates;
                        }
                    }
                }
            }
        }
    }
}



void PQMultiTable::Write(std::string dir_path){
    assert(dir_path.substr((int) dir_path.size() - 1) != "/"); // dir_path must be "some_dir". Not "some_dir/"

    // This part can be re-written by boost::filesystem.
    std::string cmd = "mkdir -p " + dir_path;
    assert(!system(cmd.c_str())); // Create a directory where table files are written

    // Write codewords;
    PQ::WriteCodewords(dir_path + "/codeword.txt", m_PQ.GetCodewords());

    // Write "T"
    std::ofstream ofs(dir_path + "/T.txt");
    assert(ofs.is_open());
    ofs << m_T;

    // Write tables
    for(int t = 0; t < m_T; ++t){
        HelperSparseHashtable::Write(dir_path + "/table" + std::to_string(t) + ".bin", m_sHashTableEach[t]);
    }

    // Write pq codes
    UcharVecs::Write(dir_path + "/pqcode.bin", m_codes);
}

void PQMultiTable::DivideCodewords(const std::vector<PQ::Array> &codewords, int T, std::vector<std::vector<PQ::Array> > *codewords_each){
    assert(codewords_each != NULL);
    int M = (int) codewords.size();
    // Divide codeword into codewords_each
    codewords_each->resize(T);
    int each_M = M / T;

    for(int t = 0; t < T; ++t){
        (*codewords_each)[t].assign(codewords.begin() + each_M * t,
                                    codewords.begin() + each_M * (t + 1));
    }
}

PQTable::PQTable(const std::vector<PQ::Array> &codewords, const UcharVecs &pq_codes, int T)
{
    // If T == -1, then the best T is automatically selected
    if(T == -1){
        T = PQMultiTable::OptimalT( (int) codewords.size() * 8, pq_codes.Size());
    }

    if(T == 1){
        m_table = (I_PQTable *) new PQSingleTable(codewords, pq_codes);
    }else if(1 < T){
        m_table = (I_PQTable *) new PQMultiTable(codewords, pq_codes, T);
    }else{
        std::cerr << "Error: strange T: " << T << " in PQTable construction" << std::endl;
    }
}

PQTable::PQTable(std::string dir_path){
    // Read T
    std::ifstream ifs(dir_path + "/T.txt");
    assert(ifs.is_open());
    int T;
    ifs >> T;

    if(T == 1){
        m_table = (I_PQTable *) new PQSingleTable(dir_path);
    }else if(1 < T){
        m_table = (I_PQTable *) new PQMultiTable(dir_path);
    }else{
        std::cerr << "Error: strange T: " << T << " in PQTable construction" << std::endl;
    }
}

PQTable::~PQTable(){
    delete m_table;
}

std::pair<int, float> PQTable::Query(const std::vector<float> &query){
    return m_table->Query(query);
}

std::vector<std::pair<int, float> > PQTable::Query(const std::vector<float> &query, int top_k) {
    return m_table->Query(query, top_k);
}

void PQTable::Write(std::string dir_path) {
    m_table->Write(dir_path);
}


}
