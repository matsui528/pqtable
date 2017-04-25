// The codes in this direcoty are from https://github.com/norouzi/mih:
// "Fast Exact Search in Hamming Space with Multi-Index Hashing,
// M. Norouzi, A. Punjani, D. J. Fleet, IEEE TPAMI 2014"


#include <inttypes.h>

#ifndef TYPES_H__
#define TYPES_H__

// Types
typedef unsigned long UINT64;
typedef unsigned int UINT32;
typedef unsigned short UINT16;
typedef unsigned char UINT8;

/* typedef uint64_t UINT64; */
/* typedef uint32_t UINT32; */
/* typedef uint16_t UINT16; */
/* typedef uint8_t UINT8; */

// Constants
#define UINT64_1 ((UINT64)0x01)
#define UINT32_1 ((UINT32)0x01)

#endif
