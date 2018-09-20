#include "cache.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void cacheTest(int argc, char *argv[]);

int main(int argc, char *argv[]) {
    if (argc < 2 || *argv[1] != '-') {
        printf("usage: ./test -<mode> [parameters]\n");
        return 0;
    }

    switch (*(argv[1] + 1)) {
        case 'c':
            cacheTest(argc, argv);
            break;
        
        default:
            printf("usage: ./test -<mode> [parameters]\n");
    }
    
    return 0;
}

void cacheTest(int argc, char *argv[]) {
    if (argc != 5 && argc != 6) {
        printf("usage: ./test -c set_associativity block_size_b cache_size_kb [out]\n");
        return;
    }

    long setAssoc = strtol(argv[2], NULL, 10);
    long blockSize = strtol(argv[3], NULL, 10);
    long cacheSize = strtol(argv[4], NULL, 10);

    Cache *cache = cacheAlloc((int) setAssoc, (int) blockSize, (int) cacheSize);

    printf("Cache allocated:\n\t"
           "Number of offset bits: %d\n\t"
           "Number of index bits: %d\n\t"
           "Number of tag bits: %d\n\n\t"
           "Number of sets: %d\n\t"
           "Number of blocks: %d\n\t"
           "Number of blocks/set: %d\n",
           cache->offsetBits, cache->indexBits, cache->tagBits, 
           cache->numSets, cache->numBlocks, cache->setAssoc);

    if (argc != 6 || strcmp(argv[5],"out") != 0) return;

    int set, block;
    srand((1 << 3) + (1 << 6));
    // loop through each set
    for (set = 0; set < cache->numSets; ++set) {
        // loop through each block in each set
        for (block = 0; block < cache->setAssoc; ++block) {
            cache->tagArray[set][block] = (unsigned) rand();
        }
    }

    // print out the cache tags
    printf("SET\t\t");
    int i;
    for (i = 0; i < cache->setAssoc; ++i) printf("BLOCK %d\t\t\t", i);
    printf("\n");
    for (set = 0; set < cache->numSets; ++set) {
        printf("Set %d", set);
        if (set < 1000) printf("\t");
        // loop through each block in each set
        for (block = 0; block < cache->setAssoc; ++block) {
            printf("\t%8x\t", cache->tagArray[set][block]);
        }
        printf("\n");
    }
}

