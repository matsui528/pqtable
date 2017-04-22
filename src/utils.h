#ifndef PQTABLE_UTILS_H
#define PQTABLE_UTILS_H

#include <opencv2/opencv.hpp>
#include <fstream>
#include <time.h>


namespace pqtable {

// Iterative reader class for reading .bvecs or .fvecs files.
// The next vector (std::vector<float>) is read by Next() function.
//
// Usage:
//   ItrReader reader("data.fvecs", "fvecs");
//   while(!reader.IsEnd()){
//     std::vector<float> vec = reader.Next();
//     /* some stuff for vec */
//   }
//
// Optional wrapper interface:
//   int top_n = 100;
//   std::vector<std::vector<float> > vecs = ReadTopN("data.fvecs", "fvecs", top_n);

// Interface (abstract basic class) of iterative reader
class I_ItrReader{
public:
    virtual ~I_ItrReader() {}
    virtual bool IsEnd() = 0;
    virtual std::vector<float> Next() = 0;
};

// Iterative reader for fvec file
class FvecsItrReader : I_ItrReader{
public:
    FvecsItrReader(std::string filename);
    bool IsEnd();
    std::vector<float> Next();
private:
    FvecsItrReader(); // prohibit default construct
    std::ifstream ifs;
    std::vector<float> vec; // store the next vec
    bool eof_flag;
};

// Iterative reader for bvec file
class BvecsItrReader : I_ItrReader{
public:
    BvecsItrReader(std::string filename);
    bool IsEnd();
    std::vector<float> Next(); // Read bvec, but return vec<float>
private:
    BvecsItrReader(); // prohibit default construct
    std::ifstream ifs;
    std::vector<float> vec; // store the next vec
    bool eof_flag;
};

// Proxy class
class ItrReader{
public:
    // ext must be "fvecs" or "bvecs"
    ItrReader(std::string filename, std::string ext);
    ~ItrReader();

    bool IsEnd();
    std::vector<float> Next();

private:
    ItrReader();
    I_ItrReader *m_reader;
};

// Wrapper. Read top-N vectors
// If top_n = -1, then read all vectors
std::vector<std::vector<float> > ReadTopN(std::string filename, std::string ext, int top_n = -1);






// Timer function
// Usage:
//   double t0 = Elapsed()
//   /* do something */
//   std::cout << Elapsed() - t0 << " [sec]" << std::endl;

double Elapsed();




// Output scores
// scores[q][k]: k-th score of q-th query, where each score is pair<int, float>.
// score[q][k].first: id,   score[q][k].second: distance
void WriteScores(std::string path,
                 const std::vector<std::vector<std::pair<int, float> > > &scores);


}

#endif // PQTABLE_UTILS_H
