#include "code_to_key.h"

namespace pqtable {

void CodeToKey::CodeToKey1(int M, const std::vector<uchar> &code, uint *key)
{
    if(M == 1){
        *key = Code1ToKey(code[0]);
    }else if(M == 2){
        *key = Code2ToKey(code[0], code[1]);
    }else if(M == 4){
        *key = Code4ToKey(code[0], code[1], code[2], code[3]);
    }else{
        std::cerr << "Error M must be 1, 2, or 4, in CodeToKey::CodeToKey1. M: " << M << std::endl;
        exit(1);
    }
}

void CodeToKey::CodeToKey2(int M, const std::vector<uchar> &code, uint *key)
{
    if(M == 2){
        key[0] = Code1ToKey(code[0]);
        key[1] = Code1ToKey(code[1]);
    }else if(M == 4){
        key[0] = Code2ToKey(code[0], code[1]);
        key[1] = Code2ToKey(code[2], code[3]);
    }else if(M == 8){
        key[0] = Code4ToKey(code[0], code[1], code[2], code[3]);
        key[1] = Code4ToKey(code[4], code[5], code[6], code[7]);
    }else{
        std::cerr << "Error M must be 2, 4, or 8 in CodeToKey::CodeToKey2. M: " << M << std::endl;
        exit(1);
    }
}



void CodeToKey::CodeToKey4(int M, const std::vector<uchar> &code, uint *key)
{
    if(M == 4){
        key[0] = Code1ToKey(code[0]);
        key[1] = Code1ToKey(code[1]);
        key[2] = Code1ToKey(code[2]);
        key[3] = Code1ToKey(code[3]);
    }else if(M == 8){
        key[0] = Code2ToKey(code[0], code[1]);
        key[1] = Code2ToKey(code[2], code[3]);
        key[2] = Code2ToKey(code[4], code[5]);
        key[3] = Code2ToKey(code[6], code[7]);
    }else if(M == 16){
        key[0] = Code4ToKey(code[0], code[1], code[2], code[3]);
        key[1] = Code4ToKey(code[4], code[5], code[6], code[7]);
        key[2] = Code4ToKey(code[8], code[9], code[10], code[11]);
        key[3] = Code4ToKey(code[12], code[13], code[14], code[15]);
    }else{
        std::cerr << "Error M must be 4, 8, or 16 in CodeToKey::CodeToKey4. M: " << M << std::endl;
        exit(1);
    }
}

void CodeToKey::CodeToKey8(int M, const std::vector<uchar> &code, uint *key)
{
    if(M == 8){
        key[0] = Code1ToKey(code[0]);
        key[1] = Code1ToKey(code[1]);
        key[2] = Code1ToKey(code[2]);
        key[3] = Code1ToKey(code[3]);
        key[4] = Code1ToKey(code[4]);
        key[5] = Code1ToKey(code[5]);
        key[6] = Code1ToKey(code[6]);
        key[7] = Code1ToKey(code[7]);
    }else if(M == 16){
        key[0] = Code2ToKey(code[0], code[1]);
        key[1] = Code2ToKey(code[2], code[3]);
        key[2] = Code2ToKey(code[4], code[5]);
        key[3] = Code2ToKey(code[6], code[7]);
        key[4] = Code2ToKey(code[8], code[9]);
        key[5] = Code2ToKey(code[10], code[11]);
        key[6] = Code2ToKey(code[12], code[13]);
        key[7] = Code2ToKey(code[14], code[15]);
    }else{
        std::cerr << "Error M must be 8 or 16 in CodeToKey::CodeToKey8. M: " << M << std::endl;
        exit(1);
    }
}

uint CodeToKey::Code1ToKey(const uchar &v1)
{
    return (uint) v1;

}

uint CodeToKey::Code2ToKey(const uchar &v1, const uchar &v2)
{
    return (uint) 256 * v1 + v2;
}

uint CodeToKey::Code4ToKey(const uchar &v1, const uchar &v2, const uchar &v3, const uchar &v4)
{
    return (uint) 16777216 * v1 + 65536 * v2 + 256 * v3 + v4;
}


}

