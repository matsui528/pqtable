#include "helper_sht.h"

namespace pqtable {

std::string HelperSparseHashtable::Print(const Array32 &array32)
{
    if(array32.arr == NULL){
        return "[]";
    }

    std::string s;
    s += "[";
    for(int i = 0; i < (int)array32.size() + 2; ++i){
        if(i == 0){
            s += "s:";
        }else if(i == 1){
            s += "c:";
        }
        s += std::to_string(array32.arr[i]) + ", ";
    }
    s += "]";
    return s;
}

std::string HelperSparseHashtable::Print(const BucketGroup &bucket_group)
{
    if(bucket_group.group == NULL){
        return "Empty bucket group";
    }
    std::string s;
    s += "Empty: " + std::bitset<32>(bucket_group.empty).to_string()
            + ", Group: " + HelperSparseHashtable::Print(*(bucket_group.group));
    return s;
}

std::string HelperSparseHashtable::Print(const SparseHashtable &table)
{
    BucketGroup *ptr_table = table.PtrTable();
    std::string s;
    s += "Bit: " + std::to_string(table.b) + ", BucketGroup.size: "
            + std::to_string(table.size) + "\n";

    for(UINT64 i = 0; i < table.size; ++i){
        if(ptr_table[i].group != NULL){
            s += std::to_string(i) + ": " + Print(ptr_table[i]) + "\n\n";
        }
    }
    return s;
}

std::string HelperSparseHashtable::PrintCode(const SparseHashtable &table)
{
    std::string s;
    for(UINT64 i_bucket = 0; i_bucket < table.size; ++i_bucket){
        UINT32 empty = table.PtrTable()[i_bucket].empty;
        if(empty){
            for(int si = 0; si < 32; ++si){
               int sz;
               UINT32 *vals;
               if(empty & ((UINT32)1 << si)){
                   vals = table.PtrTable()[i_bucket].query(si, &sz);
               }else{
                   continue;
               }

               UINT64 key = i_bucket;
               key <<= 5;
               key += si;
               s += std::to_string(key) + ": ";
               for(int si = 0; si < sz; ++si){
                   s += std::to_string(vals[si]) + ", ";
               }
               s += "\n";
            }
        }
    }
    return s;
}




void HelperSparseHashtable::Write(const std::string &filename, const SparseHashtable &table)
{
    BucketGroup *ptr_table = table.PtrTable();
    if(ptr_table == NULL){
        std::cerr << "Error: table is NULL in HelperShaprseHashtable::Write" << std::endl;
        assert(0);
    }


    std::ofstream out(filename, std::ios::binary);
    if(!out){
        std::cerr << "Error: cannot open file: " << filename << " in HelperShaprseHashtable::Write" << std::endl;
        exit(-1);
    }

    // write "b"
    out.write((char *) &table.b, sizeof(int));
    // write "size"
    out.write((char *) &table.size, sizeof(UINT64));

    for(UINT32 i = 0; i < table.size; ++i){  // i should be UINT64 if b >= 37 ?
        UINT32 empty = ptr_table[i].empty;
        if(empty != 0){ // data exits in ptr_table[i]
            // write "i" for specifying i-th BucketGroup
            out.write((char *) &i, sizeof(UINT32));
            // write "empty"
            out.write((char *) &empty, sizeof(UINT32));

            Array32 *ptr_array32 = ptr_table[i].group;
            for(int n = 0; n < (int) ptr_array32->size() + 2; ++n){
                int nn = n == 1 ? 0 : n;
                // write elem in array, where
                // "sz" (if n == 0 or 1), "elem" (2 <= n)
                // Note we fill sz when n = 1, which originaly is capacity
                out.write((char *) &(ptr_array32->arr[nn]), sizeof(UINT32));
            }
        }
    }
    UINT32 sentinel = table.size;
    out.write((char *) &sentinel, sizeof(UINT32));

    out.close();
}

void HelperSparseHashtable::Read(const std::string &filename, SparseHashtable *table)
{
    assert(table != NULL);
    std::ifstream in(filename, std::ios::binary);
    if(!in){
        std::cerr << "Error: cannot open file: " << filename << " in HelperSparseHashtable::Read" << std::endl;
        assert(0);
    }
    int b;
    UINT64 size;
    in.read((char *)&b, sizeof(int));
    in.read((char *)&size, sizeof(UINT64));

    //std::cout << b << ", " << size << std::endl;

    table->init(b);
    assert(table->size == size);

    while(1){
        UINT32 i; // i-th BucketGroup
        in.read((char *)&i, sizeof(UINT32));
        if(i == size){ // sentinel. Finish
            break;
        }

        UINT32 empty;
        in.read((char *)&empty, sizeof(UINT32));
        UINT32 array_size; // arr[0]
        in.read((char *)&array_size, sizeof(UINT32));

        BucketGroup *PtrBucket = &(table->PtrTable()[i]);
        PtrBucket->empty = empty;
        PtrBucket->group = new Array32();
        PtrBucket->group->init(array_size);
        PtrBucket->group->arr[0] = array_size;
        for(UINT32 n = 1; n < array_size + 2; ++n){
            in.read((char *) &(PtrBucket->group->arr[n]), sizeof(UINT32));
        }


    }



    in.close();
}


}

