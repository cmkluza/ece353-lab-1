#include "cache.h"
#include <stdlib.h>
#include <stdio.h>

static int _log2(unsigned int num);

unsigned tagBits(Cache *cache, unsigned long addr) {
    // the tag bits will be the left-most bits of the 32-bit address
    return (unsigned) addr >> (cache->indexBits + cache->offsetBits);
}

int indexBits(Cache *cache, unsigned long addr) {
    //shift out tagbits
    unsigned long addr1 = addr << (cache->tagBits); 
    //then shift out offset bits
    return addr1 >> (cache->offsetBits + cache->tagBits);
}

int setIndexLength(Cache *cache) {
    // returns number of index bits in the address
    return (unsigned) cache->indexBits;
}

int offsetLength(Cache *cache) {
    //returns number of offset bits in the address
    return (unsigned) cache->offsetBits;
}

int whichSet(Cache *cache, unsigned long addr) {

    return (int) indexBits(cache, addr);
}

int hitWay(Cache *cache, unsigned long addr) {
    int i, hitBlock = -1;
    for(i=0;i<cache->setAssoc;i++){
        if (cache->tagArray[whichSet(cache, addr)][i] == tagBits(cache, addr)) {    //if whichSet returns -1 it is a miss
            // if we match the tag, update the block where we matched
            hitBlock = i;
        }
    }

    if (hitBlock == -1) { // if block didn't change, it's a miss
        updateOnMiss(cache, addr);
    } else { // otherwise hit
        updateOnHit(cache, addr);
    }

    return hitBlock;
}

void updateOnHit(Cache *cache, unsigned long addr) {
    int i;
    for (i = 0; i < cache->numSets; i++) {                            //go into each set
        int j;
        for (j = 0; j < cache->setAssoc; j++) {                  //then each block
            if (cache->lruArray[i][j] == -1) {                 //if hasn't been used
            }
            if (tagBits(cache, addr)==cache->tagArray[i][j]) {
                cache->lruArray[i][j] = 0;
                //if its the place of the hit (reset LRU)
            } else {
                cache->lruArray[i][j]++;
                //if else then increase its age
            }
        }
    }
}

void updateOnMiss(Cache *cache, unsigned long addr) {
    // find a location to store the new tag
    int lruBlock = 0, block;
    int set = whichSet(cache, addr);
    for (block = 0; block < cache->setAssoc; ++block) {
        // stop immediately if we find an unused location
        if (cache->lruArray[set][block] == -1) {
            lruBlock = block;
            break;
        }

        // otherwise, find the largest (least recently used) value
        if (cache->lruArray[set][block] > cache->lruArray[set][lruBlock]) {
            lruBlock = block;
        }
    }

    // increment every other valid block
    for (block = 0; block < cache->setAssoc; ++block) {
        if (cache->lruArray[set][block] != -1) {
            ++(cache->lruArray[set][block]);
        }
    }

    cache->lruArray[set][lruBlock] = 0;
    cache->tagArray[set][lruBlock] = tagBits(cache, addr);
}

Cache *cacheAlloc(int setAssoc, int blockSize, int cacheSize) {
    Cache *cache = malloc(sizeof *cache);
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
    cache->tagArray = malloc(cache->numSets * sizeof *cache->tagArray);
    cache->lruArray = malloc(cache->numSets * sizeof *cache->lruArray);
    if (!cache->tagArray || !cache->lruArray) {
        fprintf(stderr, "memory allocation failed - "
                        "couldn't allocate %d sets for %s in cache, terminating program\n",
                cache->numSets, !cache->tagArray ? "tagArray" : "lruArray");
        exit(EXIT_FAILURE);
    }

    // allocate blocks for each set
    int set, block;
    for (set = 0; set < cache->numSets; ++set) {
        *(cache->tagArray + set) = malloc(setAssoc * sizeof **(cache->tagArray + set));
        *(cache->lruArray + set) = malloc(setAssoc * sizeof **(cache->lruArray + set));
        if (!*(cache->tagArray + set) || !*(cache->lruArray + set)) {
            fprintf(stderr, "memory allocation failed - "
                            "couldn't allocate %d blocks for set %d in %s, terminating program\n",
                    setAssoc, set, !*(cache->tagArray + set) ? "tagArray" : "lruArray");
            exit(EXIT_FAILURE);
        }
        // initialize each value in lruArray to "invalid" (01)
        for (block = 0; block < setAssoc; ++block) cache->lruArray[set][block] = -1;
    }

    return cache;
}

void cacheFree(Cache *cache) {
    // free each set in memory
    int set;
    for (set = 0; set < cache->numSets; ++set) {
        free(cache->tagArray[set]);
        free(cache->lruArray[set]);
    }
    // free the arrays
    free(cache->tagArray);
    free(cache->lruArray);
    // free the cache itself
    free(cache);
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
