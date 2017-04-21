/* dynamic array of 32-bit integers
 * arr[0]   : array size
 * arr[1]   : array capacity
 * arr[2..] : array content */

#ifndef ARRAY32_H__
#define ARRAY32_H__

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "types.h"

class Array32 {

 private:
    static double ARRAY_RESIZE_FACTOR;
    static double ARRAY_RESIZE_ADD_FACTOR;

 public:

    static void set_array_resize_factor(double arf);

    // -- Added by matsui --
    // arr[0] : size
    // arr[1] : capacity
    // arr[2] ~ : data
    UINT32 *arr;

    Array32();

    ~Array32();

    void cleanup();

    void push(UINT32 data);

    void insert(UINT32 index, UINT32 data);

    UINT32* data();

    UINT32 size() const; // "const" is added by matsui

    UINT32 capacity();

    Array32& operator= (const Array32&);	

    void print();

    void init(int size);

    // added by matsui
    static double *PtrArrayResizeFactor(){return &ARRAY_RESIZE_FACTOR;}
};

#endif
