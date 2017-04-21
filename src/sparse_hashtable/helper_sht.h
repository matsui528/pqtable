// Helper functions for sparse hash table
// The codes in this direcoty are from https://github.com/norouzi/mih:
// "Fast Exact Search in Hamming Space with Multi-Index Hashing,
// M. Norouzi, A. Punjani, D. J. Fleet, IEEE TPAMI 2014"

#ifndef PQTABLE_HELPER_SHT_H
#define PQTABLE_HELPER_SHT_H

#include <iostream>
#include <bitset> // for visualizing bits in Print()
#include <fstream> // for IO
#include <cassert>
#include "sparse_hashtable.h"

namespace pqtable {

class HelperSparseHashtable{
public:
    static std::string Print(const Array32 &array32);
    static std::string Print(const BucketGroup &bucket_group);
    static std::string Print(const SparseHashtable &table);

    static std::string PrintCode(const SparseHashtable &table);

    static void Write(const std::string &filename, const SparseHashtable &table);
    static void Read(const std::string &filename, SparseHashtable *table);



};

}

#endif // PQTABLE_HELPER_SHT_H
