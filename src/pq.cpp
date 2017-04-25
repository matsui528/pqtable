#include "pq.h"


namespace pqtable {

UcharVecs::UcharVecs(const std::vector<std::vector<uchar> > &vec)
{
    assert(!vec.empty());
    Resize((int) vec.size(), (int) vec[0].size());
    for(int n = 0; n < m_N; ++n){
        SetVec(n, vec[n]);
    }
}

void UcharVecs::Resize(int N, int D)
{
    m_N = N;
    m_D = D;
    //m_data.clear(); // values are remained
    m_data.resize( (unsigned long long) m_N * m_D /*, 0*/);
}

const uchar &UcharVecs::GetVal(int n, int d) const {
    //assert(0 <= n && n < m_N && 0 <= d && d < m_D);
    return m_data[ (unsigned long long) n * m_D + d];
}



std::vector<uchar> UcharVecs::GetVec(int n) const {
    assert(0 <= n && n < m_N);
    return std::vector<uchar>(m_data.begin() + (unsigned long long) n * m_D,
                              m_data.begin() + (unsigned long long) (n + 1) * m_D);
}


void UcharVecs::SetVal(int n, int d, uchar val){
    assert(0 <= n && n < m_N && 0 <= d && d < m_D);
    m_data[ (unsigned long long) n * m_D + d] = val;
}

void UcharVecs::SetVec(int n, const std::vector<uchar> &vec)
{
    assert(0 <= n && n < m_N);
    assert( (int) vec.size() == m_D);
    for(int d = 0; d < m_D; ++d){
        SetVal(n, d, vec[d]);
    }
}

void UcharVecs::Write(std::string path, const UcharVecs &vecs)
{
    std::ofstream ofs(path, std::ios::binary);
    if(!ofs.is_open()){
        std::cerr << "Error: cannot open " << path << std::ends;
        assert(0);
    }

    // Write (1) N, (2) D, and (3) data
    int N = vecs.Size();
    int D = vecs.Dim();
    ofs.write( (char *) &N, sizeof(int));
    ofs.write( (char *) &D, sizeof(int));
    for(int n = 0; n < N; ++n){
        std::vector<uchar> vec = vecs.GetVec(n);
        ofs.write( (char *) vec.data(), sizeof(uchar) * D);
    }

}

void UcharVecs::Read(std::string path, UcharVecs *vecs, int top_n)
{
    assert(vecs != NULL);
    std::ifstream ifs(path, std::ios::binary);
    if(!ifs.is_open()){
        std::cerr << "Error: cannot open " << path << std::ends;
        assert(0);
    }

    // Read (1) N, (2) D, and (3) data
    int N, D;
    ifs.read( (char *) &N, sizeof(int));
    ifs.read( (char *) &D, sizeof(int));

    if(top_n == -1){
        top_n = N;
    }
    assert(0 < top_n && top_n <= N);

    vecs->Resize(top_n, D);

    for(int n = 0; n < top_n; ++n){
        std::vector<uchar> buf(D);
        ifs.read( (char *) buf.data(), sizeof(uchar) * D);
        vecs->SetVec(n, buf);
    }
}

UcharVecs UcharVecs::Read(std::string path, int top_n)
{
    UcharVecs codes;
    Read(path, &codes, top_n);
    return codes;
}

PQ::PQ(const std::vector<PQ::Array> &codewords){
    m_M = (int) codewords.size();
    m_Ks = (int) codewords[0].size();
    m_Ds = (int) codewords[0][0].size();
    m_codewords = codewords;
}

std::vector<PQ::Array> PQ::Learn(const std::vector<std::vector<float> > &vecs, int M, int Ks){
    cv::Mat vecs_cvmat = ArrayToMat(vecs); // Convert to cv::Mat
    return Learn(vecs_cvmat, M, Ks);
}

std::vector<PQ::Array> PQ::Learn(const cv::Mat &vecs_cvmat, int M, int Ks)
{
    assert(Ks < vecs_cvmat.rows); // #vecs must be larger than Ks
    int D = vecs_cvmat.cols;
    assert(D % M == 0);
    int Ds = D / M;

    std::vector<Array> codewords(M, Array(Ks, std::vector<float>(Ds, 0.0)));

    // Do learn
    for(int m = 0; m < M; ++m){ // for each subspace
        std::cout << "learning m: " << m << " / " << M << std::endl;

        // Focus sub space
        cv::Mat svec = vecs_cvmat(cv::Range::all(),
                                  cv::Range(Ds * m, Ds * (m + 1)));

        // Do k-means
        cv::Mat label;
        cv::Mat center;
        cv::kmeans(svec, Ks, label,
                   cv::TermCriteria(cv::TermCriteria::EPS+cv::TermCriteria::MAX_ITER, 1000, 1),
                   3, cv::KMEANS_PP_CENTERS, center);

        // Record
        codewords[m] = MatToArray(center);
    }
    return codewords;
}


std::vector<uchar> PQ::Encode(const std::vector<float> &vec) const
{
    assert((int) vec.size() == m_Ds * m_M);

    std::vector<uchar> code(m_M);

    for(int m = 0; m < m_M; ++m){
        float min_dist = FLT_MAX;
        int min_ks = -1;

        // find the nearest codeword
        for(int ks = 0; ks < m_Ks; ++ks){
            float dist = 0;
            for(int ds = 0; ds < m_Ds; ++ds){
                float diff = vec[m * m_Ds + ds] - m_codewords[m][ks][ds];
                dist += diff * diff;
            }
            if(dist < min_dist){
                min_dist = dist;
                min_ks = ks;
            }
        }
        assert(min_ks != -1);
        code[m] = (uchar) min_ks;
    }
    return code;
}


UcharVecs PQ::Encode(const std::vector<std::vector<float> > &vecs) const
{
    UcharVecs codes((int) vecs.size(), m_M);
    for(int n = 0; n < (int) vecs.size(); ++n){
        codes.SetVec(n, Encode(vecs[n]));
    }
    return codes;
}

std::vector<float> PQ::Decode(const std::vector<uchar> &code) const
{
    assert((int) code.size() == m_M);
    std::vector<float> v(m_Ds * m_M);

    for(int m = 0; m < m_M; ++m){
        int ks = code[m];
        for(int ds = 0; ds < m_Ds; ++ds){
            v[m * m_Ds + ds] = m_codewords[m][ks][ds];
        }
    }
    return v;
}

std::vector<std::vector<float> > PQ::Decode(const UcharVecs &codes) const
{
    std::vector<std::vector<float> > vecs(codes.Size());
    for(int i = 0; i < (int) codes.Size(); ++i){
        vecs[i] = Decode(codes.GetVec(i));
    }
    return vecs;

}

PQ::Array PQ::DTable(const std::vector<float> &query) const
{
    assert((int) query.size() == m_Ds * m_M);

    Array dtable(m_M, std::vector<float>(m_Ks)); // m_M * m_Ks

    for(int m = 0; m < m_M; ++m){
        for(int ks = 0; ks < m_Ks; ++ks){
            float dist = 0;
            // squared L2 dist
            for(int ds = 0; ds < m_Ds; ++ds){
                float diff = query[m * m_Ds + ds] - m_codewords[m][ks][ds];
                dist += diff * diff;
            }
            dtable[m][ks] = dist;
        }
    }
    return dtable;
}

float PQ::AD(const PQ::Array &dtable, const std::vector<uchar> &code) const
{
    //assert( (int) code.size() == m_M);
    float dist = 0;
    for(int m = 0; m < m_M; ++m){
        uchar ks = code[m];
        dist += dtable[m][ks];
    }
    return dist;
}


std::vector<float> PQ::AD(const PQ::Array &dtable, const UcharVecs &codes) const
{
    //assert((int) dtable.size() == m_M);
    //assert((int) dtable[0].size() == m_Ks);

    std::vector<float> dists(codes.Size());

    for(int n = 0; n < (int) codes.Size(); ++n){
        float dist = 0;
        for(int m = 0; m < m_M; ++m){
            uchar ks = codes.GetVal(n, m);
            dist += dtable[m][ks];
        }
        dists[n] = dist;
    }
    return dists;
}

float PQ::AD(const PQ::Array &dtable, const UcharVecs &codes, int n) const
{
    float dist = 0;
    for(int m = 0; m < m_M; ++m){
        //uchar ks = code[m];
        uchar ks = *(codes.RawDataPtr() + (unsigned long long) n * m_M + m);
        dist += dtable[m][ks];
    }
    return dist;
}



std::vector<std::pair<int, float> > PQ::Sort(const std::vector<float> &dists, int top_k)
{
    assert(top_k == -1 || (0 <= top_k && top_k < (int) dists.size())); // if top_k == -1, sort all
    if(top_k == -1){
        top_k = (int) dists.size();
    }

    std::vector<std::pair<int, float> > sorted_dists(dists.size());

    for(int i = 0; i < (int) dists.size(); ++i){
        sorted_dists[i] = std::pair<int, float>(i, dists[i]); // set (id, dist)
    }

    // sort top_k. This takes O(nlogk)
    std::partial_sort(sorted_dists.begin(), sorted_dists.begin() + top_k, sorted_dists.end(),
                      [](const std::pair<int, float> &a, const std::pair<int, float> &b){return a.second < b.second;});

    sorted_dists.resize(top_k);
    sorted_dists.shrink_to_fit();

    return sorted_dists;
}

void PQ::WriteCodewords(std::string file_path, const std::vector<PQ::Array> &codewords)
{
    std::ofstream ofs(file_path);
    assert(ofs.is_open());
    int M = (int) codewords.size();
    int Ks = (int) codewords[0].size();
    int Ds = (int) codewords[0][0].size();

    ofs << M << "," << Ks << "," << Ds << std::endl;

    for(int m = 0; m < M; ++m){
        ofs << m << ":\n"; // write m for each sub space
        for(int ks = 0; ks < Ks; ++ks){
            for(int ds = 0; ds < Ds; ++ds){
                ofs << codewords[m][ks][ds] << ",";
            }
            ofs << "\n";
        }
    }
}

std::vector<PQ::Array> PQ::ReadCodewords(std::string file_path)
{
    std::ifstream ifs(file_path);
    assert(ifs.is_open());
    int M, Ks, Ds;
    char c1, c2; // dummy
    int v; // dummy

    ifs >> M >> c1 >> Ks >> c2 >> Ds; // Read the first line. c1 = c2 = ","

    std::vector<Array> codewords(M, Array(Ks, std::vector<float>(Ds, 0.0)));
    for(int m = 0; m < M; ++m){
        ifs >> v >> c1; // c1 = ":"
        assert(v == m);
        for(int ks = 0; ks < Ks; ++ks){
            for(int ds = 0; ds < Ds; ++ds){
                ifs >> codewords[m][ks][ds] >> c1;
            }
        }
    }
    return codewords;
}

cv::Mat PQ::ArrayToMat(const PQ::Array &array)
{
    assert( (int) array.size() > 0);
    assert( (int) array[0].size() > 0);
    cv::Mat mat( (int) array.size(), (int) array[0].size(), CV_32FC1);
    for(int n = 0; n < (int) array.size(); ++n){
        for(int i = 0; i < (int) array[0].size(); ++i){
            mat.at<float>(n, i) = array[n][i];
        }
    }
    return mat;
}

PQ::Array PQ::MatToArray(const cv::Mat &mat)
{
    assert(mat.type() == CV_32FC1);
    assert(mat.cols > 0);
    assert(mat.rows > 0);
    Array array(mat.rows, std::vector<float>(mat.cols));
    for(int n = 0; n < mat.rows; ++n){
        for(int i = 0; i < mat.cols; ++i){
            array[n][i] = mat.at<float>(n, i);
        }
    }
    return array;
}


}
