#ifndef BUCKET_GROUP_H__
#define BUCKET_GROUP_H__

#include <stdio.h>
#include <math.h>
#include "types.h"
#include "array32.h"
//#include "bitarray.h"

// added by matsui, which is originally placed in bitops.h
#define popcnt __builtin_popcount

class BucketGroup {

 public:
    
    UINT32 empty;

    Array32 *group;

    BucketGroup();

    ~BucketGroup();

    void insert(int subindex, UINT32 data);

    UINT32* query(int subindex, int *size);

};

#endif
