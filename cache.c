#include "cache.h"
#include <stdlib.h>
#include <stdio.h>

static int _log2(unsigned int num);

int tagBits(Cache *cache, unsigned long addr) {
    // the tag bits will be the left-most bits of the 32-bit address
    return (unsigned) addr >> (cache->indexBits + cache->offsetBits);
}

int indexBits(Cache *cache, unsigned long addr) {
    //
    addr << (cache->tagBits);
    //
    return (unsigned) addr >> (cache->offsetBits+cache->indexBits);
}

int setIndexLength(Cache *cache){
    // returns number of index bits in the address
    return (unsigned) cache->indexBits;
}

int offsetLength(Cache *cache){
    //returns number of offset bits in the address
	return (unsigned) cache->offsetBits;
}

int whichSet(Cache *cache, unsigned long addr){

	int setNum=-1; 
	for (int i=0; i< cache->numSets; i++) {
		if ( tagBits(Cache *cache, unsigned long addr) == cache->tagArray[indexBits(Cache *cache, unsigned long addr)][i]) 
		{
		setNum= i;
		}
	}
	return setNum;
}

int hitWay(Cache *cache, int addr){

	if (whichSet(Cache *cache, unsigned long addr)==-1){    //if whichSet returns -1 it is a miss
		updateOnMiss(Cache *cache, unsigned addr);
		return -1;
}
	else{                                                  //else it is a hit
		updateOnHit(Cache *cache, unsigned long addr);
		return whichSet(Cache *cache, unsigned long addr)+1;    
}
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
        *(cache->lruArray + set) = malloc(setAssoc * sizeof**(cache->lruArray + set));
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
