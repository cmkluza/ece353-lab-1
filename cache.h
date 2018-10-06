/**
 * ECE 353 - Computer Systems Lab
 * Lab I, Cache Simulator
 *
 * cache.h - outlines the interface necessary to simulate
 * the CPU cache.
 *
 * @authors Laura Deburgo, Cameron Kluza, Kyle McWherter
 */

#ifndef CACHE_H
#define CACHE_H

/////////////////////////////
// Structs, enums, typdefs //
/////////////////////////////

/** 
 * A collection of cache configuration info and arrays storing relevant cache data.
 */
typedef struct {
    int numSets; /**< number of sets within the cache */
    int numBlocks; /**< number of blocks within the cache */
    int setAssoc; /**< set associativity of the cache */

    int offsetBits; /**< number of offset bits in address */
    int indexBits; /**< number of index bits in address */
    int tagBits; /**< number of tag bits in address */

    /**
     * Array of block tags. Subscripted as `[set][block]`.
     */
    unsigned int **tagArray;
    /**
     * Array of block LRU statuses. Subscripted as `[set][block]`.
     */
    int **lruArray;
} Cache;

///////////////////////////////////////
// Functions specified in assignment //
///////////////////////////////////////

/**
 * Outputs the cache set in which the address falls.
 *
 * @param cache a pointer to a configured cache 
 * @param addr the address to check
 * @return a number [0,numSets) that indicates which set the address falls in
 */
int whichSet(Cache *cache, unsigned long addr);

/**
 * Outputs the number of bits in the set index field of the address.
 *
 * @param cache a pointer to a configured cache
 * @return the number of bits in the set index field for the given cache
 */
int setIndexLength(Cache *cache);

/**
 * Outputs the number of bits in the line offset field of the address.
 *
 * @param cache a pointer to a configured cache
 * @return the number of bits in the line offset field for the given cache
 */
int offsetLength(Cache *cache);

/**
 * Outputs the tag bits associated with the address.
 *
 * @param cache a pointer to a configured cache
 * @param addr the address to get the tag bits from
 * @return the tag bits associated with the address for the given cache
 */
unsigned tagBits(Cache *cache, unsigned long addr);

/**
 * If there is a hit, this outputs the cache way in which the accessed line can be found; it
 * returns -1 if there is a cache miss.
 *
 * @param cache a pointer to a configured cache
 * @param addr the address to attempt to pull from the cache
 * @return the set in which the accessed block can be found or -1 for a miss
 */
int hitWay(Cache *cache, unsigned long addr);

/**
 * Updates the `tagArray` and `lruArray` upon a hit. This function is only called on a cache
 * hit.
 *
 * @param cache a pointer to a configured cache
 * @param addr the address where the hit occurred
 */
void updateOnHit(Cache *cache, unsigned long addr);

/**
 * Updates the `tagArray` and `lruArray` upon a miss. This function is only called on a cache
 * miss.
 *
 * @param cache a pointer to a configured cache
 * @param addr the address where the miss occurred
 */
void updateOnMiss(Cache *cache, unsigned long addr);

/**
 * Allocates and configures a cache struct given the cache parameters.
 *
 * @param setAssoc the set associativity for the cache
 * @param blockSize the block size for the cache (bytes)
 * @param cacheSize the total cache size (kilobytes)
 * @return a pointer to a cache configured for the given parameters
 */
Cache *cacheAlloc(int setAssoc, int blockSize, int cacheSize);

/**
 * Frees space allocated for a cache.
 *
 * @param cache the cache to be allocated
 */
 void cacheFree(Cache *cache);

 /**
  * Returns the index of the given address.
  *
  * @param cache a pointer to a configured cache
  * @param addr the address to extract index bits from
  * @return the index for this address
  */
int indexBits(Cache *cache, unsigned long addr);

#endif
