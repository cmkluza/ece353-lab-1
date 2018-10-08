/**
 * ECE 353 - Computer Systems Lab
 * Lab I, Cache Simulator
 *
 * cache_sim.c - reads a trace of addresses from a file and
 * simulates accessing them in a CPU cache of the
 * specified configuration.
 *
 * @authors Laura DeBurgo, Cameron Kluza, Kyle McWherter
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

long hits, misses;

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
unsigned long whichSet(Cache *cache, unsigned long addr);

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
unsigned long indexBits(Cache *cache, unsigned long addr);

/**
 * Psuedo-log2 method that only works for powers of 2.
 *
 * @param num the number to take log2 of
 * @return log2 of a number that is a power of 2
 */
static int _log2(unsigned int num);

int main(int argc, char *argv[]) {
    if (argc != 5) {
        printf("Usage: ./cache-sim set_associativity block_size_b cache_size_kb tracefile\n");
        return 0;
    }

    // get parameters from command line
    long setAssoc = strtol(argv[1], NULL, 10);
    long blockSize = strtol(argv[2], NULL, 10);
    long cacheSize = strtol(argv[3], NULL, 10);
    FILE *fp = fopen(argv[4], "r"); // open trace file for reading

    // check that the file was opened
    if (!fp) {
        fprintf(stderr, "[ERROR] unable to open file: %s\n", argv[4]);
        return 1;
    }

    // allocate the cache
    Cache *cache = cacheAlloc(setAssoc, blockSize, cacheSize);

    printf("allocated a cache with %x offset bits, %x index bits, %x tag bits\n", cache->offsetBits, cache->indexBits, cache->tagBits);

    // buffer to read trace into
    char address[10]; // address is 8 hex chars at most
    unsigned long addr; // current address

    // process each line in the file
    while (fgets(address, 10, fp)) {
        // process the hex address into a number
        addr = (unsigned long) strtol(address, NULL, 16);
        // see if we've hit
        if (hitWay(cache, addr) != -1) {
            ++hits;
        } else {
            ++misses;
        }
    }

    // check for irregular stopping
    if (!feof(fp)) {
        fprintf(stderr, "[ERROR] unexpected error reading addresses: %s\n",
                strerror(errno));
    }

    // output necessary data
    printf("%s %ld %ld %ld %f\n",
           argv[4], cacheSize, setAssoc, blockSize,
           ((double) misses) / (hits + misses));

    printf("misses: %ld, hits: %ld, total: %ld\n", misses, hits, misses + hits);

    cacheFree(cache);

    return 0;
}

unsigned tagBits(Cache *cache, unsigned long addr) {
    // the tag bits will be the left-most bits of the 32-bit address
    return (unsigned) addr >> (cache->indexBits + cache->offsetBits);
}

unsigned long indexBits(Cache *cache, unsigned long addr) {
    addr >>= cache->offsetBits;
    addr &= ((1 << cache->indexBits) - 1);
    return addr;
}

int setIndexLength(Cache *cache) {
    // returns number of index bits in the address
    return (unsigned) cache->indexBits;
}

int offsetLength(Cache *cache) {
    //returns number of offset bits in the address
    return (unsigned) cache->offsetBits;
}

unsigned long whichSet(Cache *cache, unsigned long addr) {
    return indexBits(cache, addr);
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

static int _log2(unsigned int num) {
    int result = 0;
    while ((num >>= 1) > 0) ++result;
    return result;
}
