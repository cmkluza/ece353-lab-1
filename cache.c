#include "cache.h"
#include <stdlib.h>
#include <stdio.h>

static int _log2(unsigned int num);

Cache *cacheAlloc(int setAssoc, int blockSize, int cacheSize) {
    Cache *cache = malloc(sizeof(Cache));
    if (!cache) {
        fprintf(stderr, "memory allocation failed - couldn't allocate cache struct, terminating program\n");
        exit(EXIT_FAILURE);
    }

    // configure cache parameters
    cache->setAssoc = setAssoc;
    cache->numBlocks = (cacheSize * 1024) / blockSize; // total cache size (bytes) / block size (bytes)
    cache->numSets = cache->numBlocks / setAssoc; // number of blocks / number of blocks per set

    cache->offsetBits = _log2((unsigned) blockSize);
    cache->indexBits = _log2((unsigned) cache->numSets);
    cache->tagBits = 32 - cache->offsetBits - cache->indexBits;
    
    // allocate an array containing each set
    cache->tagArray = malloc(cache->numSets * sizeof(unsigned int *));
    cache->lruArray = malloc(cache->numSets * sizeof(int *));
    if (!cache->tagArray || !cache->lruArray) {
        fprintf(stderr, "memory allocation failed - "
                "couldn't allocate %d sets for %s in cache, terminating program\n",
                cache->numSets, !cache->tagArray ? "tagArray" : "lruArray");
        exit(EXIT_FAILURE);
    }

    // allocate blocks for each set
    int i;
    for (i = 0; i < cache->numSets; ++i) {
        *(cache->tagArray + i) = malloc(setAssoc * sizeof(unsigned int));
        *(cache->lruArray + i) = malloc(setAssoc * sizeof(int));
        if (!*(cache->tagArray + i) || !*(cache->lruArray + i)) {
            fprintf(stderr, "memory allocation failed - "
                    "couldn't allocate %d blocks for set %d in %s, terminating program\n",
                    setAssoc, i, !*(cache->tagArray + i) ? "tagArray" : "lruArray");
            exit(EXIT_FAILURE);
        }
    }

    return cache;
}

/**
 * Psuedo-log2 method that only works for powers of 2.
 *
 * @param num the number to take log2 of
 * @return log2 of a number that is a power of 2
 */
static int _log2(unsigned int num) {
    int result = 0;
    while ((num >>= 1) > 0) ++result;
    return result;
}
