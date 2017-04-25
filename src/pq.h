#ifndef PQTABLE_PQ_H
#define PQTABLE_PQ_H

// H. Jegou, et al. "Product Quantization for Nearest Neighbor Search" TPAMI 2011
// The authors provide MATLAB codes:
// http://people.rennes.inria.fr/Herve.Jegou/projects/ann.html

#include <opencv2/opencv.hpp>
#include <fstream> // for IO

namespace pqtable {

// Data structure to handle a billion-scale data
// UcharVecs is something like vec<vec<uchar>>.
// When constructing (or Resize(), or Read()), this class malloc a long array (vec<uchar>),
// so this class achieves almost ideal memory consumption.
// In PQ, UcharVecs is used as follows:
//     vector<vector<float>> vecs = /* set data */ ;
//     PQ pq = /* setup a product quantizer */
//     UcharVecs code(N, M);   // N: #vectors. M: #subspace
//     for(int n = 0; n < N; ++n){
//       UcharVecs[n].SetVec(n, pq.Encode(vecs[n]));
//     }
// Or, the direct interfacface is also provided.
//     UcharVecs code = pq.Encode(vecs)
// Or, you can convert from vec<vec<uchar>>
//     vector<vector<uchar>> some_codes = /* set codes */
//     UcharVecs code(some_codes)
// IO interfaces are:
//     UcharVecs::Write("code.bin", code);  // write
//     UcharVecs code_read = UcharVecs::Read("code.bin"); // read
// The size of "code.bin" is the ideal size + 8 bytes (we record N and D),
// e.g., if N=10^9 and D=4, then code.bin will be 4,000,000,008 bytes.

class UcharVecs{
public:
    UcharVecs() : m_N(0), m_D(0) {}   // Space is not allocated. You need to call Resize first
    UcharVecs(int N, int D) { Resize(N, D); }  // Space is allocated
    UcharVecs(const std::vector<std::vector<uchar> > &vec);  // vec<vec<uchar>> -> UcharVecs

    void Resize(int N, int D); // After resized, the old values are remained

    // Getter
    const uchar &GetVal(int n, int d) const;     // n-th vec, d-th dim
    std::vector<uchar> GetVec(int n) const;     // n-th vec

    // Setter
    void SetVal(int n, int d, uchar val);
    void SetVec(int n, const std::vector<uchar> &vec);

    // IO
    static void Write(std::string path, const UcharVecs &vecs);
    static void Read(std::string path, UcharVecs *vecs, int top_n = -1); // Read top_n codes. if top_n==-1, read all
    static UcharVecs Read(std::string path, int top_n = -1); // wrapper.

    // Be careful
    const uchar *RawDataPtr() const {return m_data.data();}

    int Size() const {return m_N;}
    int Dim() const {return m_D;}

private:
    int m_N;
    int m_D;
    std::vector<uchar> m_data; // a long array

};


// Product quantization class
//
// How to construct:
// (1) Learn
//   std::vector<PQ::Array> codewords = PQ::Learn(vecs, 4);
//   PQ::WriteCodewords("cw.txt", codewords); // Optionally, you can save the codewords
//   PQ pq(codewords);
//   /* then you can use pq */
//
//   ... or, you can learn directly
//   PQ pq(PQ::Learn(vecs, 4));
//   /* then you can use pq */
//
// (2) Read from saved codewords
//   std::vector<PQ::Array> codewords = PQ::ReadCodewords("cw.txt")
//   PQ pq(codewords);
//   /* then you can use pq */
//
//   ... or, you can read from saved codewords directly
//   PQ pq(PQ::ReadCodewords("hoge.txt"))
//   /* then you can use pq */
//
// (3) Create codewords using different ways
//   std::vector<PQ::Array> codewords = some_func();
//   PQ pq(codewords);
//   /* then you can use pq */

class PQ
{
public:
    // for convinience, rename vec<vec<float>> as Array
    typedef std::vector<std::vector<float> > Array;

    // codewords[m][ks][ds] : m-th subspace, ks-th codeword, ds-th element
    PQ(const std::vector<Array> &codewords);

    static std::vector<Array> Learn(const std::vector<std::vector<float> > &vecs,
                                    int M, int Ks = 256);
    static std::vector<Array> Learn(const cv::Mat &vecs_cvmat, // a vec per row
                                    int M, int Ks = 256);

    // Give a vector, encode it
    std::vector<uchar> Encode(const std::vector<float> &vec) const;
    UcharVecs Encode(const std::vector<std::vector<float> > &vecs) const;  // Encode several vectors at once

    // Given a PQ code, decode it
    std::vector<float> Decode(const std::vector<uchar> &code) const;
    std::vector<std::vector<float> > Decode(const UcharVecs &codes) const;  // Decode several codes at once

    // dtable[m][ks] : the distance between m-th subspace, ks-th codeword
    Array DTable(const std::vector<float> &query) const;

    // Give a DTable and PQ codes, compute an asymmetric distance
    float AD(const Array &dtable, const std::vector<uchar> &code) const;
    std::vector<float> AD(const Array &dtable,
                          const UcharVecs &codes) const;
    float AD(const Array &dtable, const UcharVecs &codes, int n) const; // Fast impl

    // Just sort results
    static std::vector<std::pair<int, float> > Sort(const std::vector<float> &dists, int top_k = -1); // if top_k == -1, sort all

    // Getter
    const int &GetM() const {return m_M;}
    const int &GetDs() const {return m_Ds;}
    const int &GetKs() const {return m_Ks;}
    const std::vector<Array> &GetCodewords() const {return m_codewords;}

    // IO funcs
    static void WriteCodewords(std::string file_path, const std::vector<Array> &codewords);
    static std::vector<Array> ReadCodewords(std::string file_path);


    // Utility functions. Array <-> cv::Mat
    // array[n][i] <-> mat.at<float>(n, i)
    static cv::Mat ArrayToMat(const Array &array);
    static Array MatToArray(const cv::Mat &mat);

private:
    //int m_D;   // e.g., m_D = 128 (SIFT)
    int m_M;   // e.g., m_M = 4 (a SIFT feature is divided into 4 parts)
    int m_Ds;  // e.g., m_Ds = 32 (m_Ds = m_D / m_M)
    int m_Ks;  // e..g, m_Ks = 256

    std::vector<Array> m_codewords; // [ns][ks][ds]


};



}

#endif // PQTABLE_PQ_H


