#ifndef PQTABLE_CODE_TO_KEY_H
#define PQTABLE_CODE_TO_KEY_H


// Given a PQ code (vec<uchar>), compute its hash value, i.e.,
// a "uint", or several "uint"s (for multiple table case)

#include <opencv2/opencv.hpp>

namespace pqtable {

class CodeToKey
{
public:
    static void CodeToKey1(int M, const std::vector<uchar> &code, uint *key); // key should be uint key    (single var)
    static void CodeToKey2(int M, const std::vector<uchar> &code, uint *key); // key should be uint key[2] (array of uint)
    static void CodeToKey4(int M, const std::vector<uchar> &code, uint *key); // key should be uint key[4] (array of uint)
    static void CodeToKey8(int M, const std::vector<uchar> &code, uint *key); // key should be uint key[8] (array of uint)


    // Given several uchar, compute a key
    static uint Code1ToKey(const uchar &v1);
    static uint Code2ToKey(const uchar &v1, const uchar &v2);
    static uint Code4ToKey(const uchar &v1, const uchar &v2, const uchar &v3, const uchar &v4);

private:
    CodeToKey(); // prohibit default constructing

};

}

#endif // PQTABLE_CODE_TO_KEY_H
