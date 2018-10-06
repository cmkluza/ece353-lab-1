#include "cache.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void cacheTest(int argc, char *argv[]);
void tagBitsTest(int argc, char *argv[]);
void indexBitsTest(int argc, char *argv[]);
void whichSetTest(int argc, char *argv[]);
void hitWayTest(int argc, char *argv[]);

void _print_bits(unsigned long long num);

int main(int argc, char *argv[]) {
    if (argc < 2 || *argv[1] != '-') {
        printf("usage: ./test -<mode> [parameters]\n");
        return 0;
    }

    switch (*(argv[1] + 1)) {
        case 'c':
            cacheTest(argc, argv);
            break;

        case 't':
            tagBitsTest(argc, argv);
            break;

        case 'i':
            indexBitsTest(argc, argv);
            break;

        case 's':
            whichSetTest(argc, argv);
            break;

        case 'h':
            hitWayTest(argc, argv);
            break;

        default:
            printf("usage: ./test -<mode> [parameters]\n");
    }

    return 0;
}

void tagBitsTest(int argc, char *argv[]) {
    if (argc != 6) {
        printf("usage: ./test -t set_associativity block_size_b cache_size_kb hex_addr\n");
        return;
    }

    long setAssoc = strtol(argv[2], NULL, 10);
    long blockSize = strtol(argv[3], NULL, 10);
    long cacheSize = strtol(argv[4], NULL, 10);
    unsigned long inputNum = (unsigned long) strtol(argv[5], NULL, 16);

    Cache *cache = cacheAlloc((int) setAssoc, (int) blockSize, (int) cacheSize);

    printf("Input num:\n");
    printf("\tHex: %lx\n", inputNum);
    printf("\tBinary: ");
    _print_bits((unsigned) inputNum);

    printf("\nTag bits:\n");
    unsigned tag = (unsigned) tagBits(cache, inputNum);
    printf("\tHex: %x\n", tag);
    printf("\tBinary: ");
    _print_bits(tag);

    cacheFree(cache);
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

    if (argc != 6 || strcmp(argv[5], "out") != 0) return;

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
    printf("================ PRINTING TAGS ================\n");
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

    // print out the cache lru
    printf("================ PRINTING LRU ================\n");
    printf("SET\t\t");
    for (i = 0; i < cache->setAssoc; ++i) printf("BLOCK %d\t\t", i);
    printf("\n");
    for (set = 0; set < cache->numSets; ++set) {
        printf("Set %d", set);
        if (set < 1000) printf("\t");
        // loop through each block in each set
        for (block = 0; block < cache->setAssoc; ++block) {
            printf("\t%d\t", cache->lruArray[set][block]);
        }
        printf("\n");
    }

    cacheFree(cache);
}

void indexBitsTest(int argc, char *argv[]) {
    if (argc != 6) {
        printf("usage: ./test -i set_associativity block_size_b cache_size_kb hex_addr\n");
        return;
    }

    long setAssoc = strtol(argv[2], NULL, 10);
    long blockSize = strtol(argv[3], NULL, 10);
    long cacheSize = strtol(argv[4], NULL, 10);
    printf("input str: %s\n", argv[5]);
    unsigned long inputNum = (unsigned long) strtol(argv[5], NULL, 16);

    Cache *cache = cacheAlloc((int) setAssoc, (int) blockSize, (int) cacheSize);

    printf("Cache config: setAssoc = %ld, blockSize = %ld, cacheSize = %ld\n", setAssoc, blockSize, cacheSize);
    printf("\tnumBlocks = %d, numSets = %d\n", cache->numBlocks, cache->numSets);
    printf("\toffsetBits = %d, indexBits = %d, tagBits = %d\n", cache->offsetBits, cache->indexBits, cache->tagBits);


    printf("Input num:\n");
    printf("\tHex: %lx\n", inputNum);
    printf("\tBinary: ");
    _print_bits(inputNum);

    printf("\nIndex bits:\n");
    unsigned index = (unsigned) indexBits(cache, inputNum);
    printf("\tHex: %x\n", index);
    printf("\tBinary: ");
    _print_bits(index);
    printf("\n\tDecimal: %d", index);

    cacheFree(cache);
}

void whichSetTest(int argc, char *argv[]) {
    if (argc != 6) {
        printf("usage: ./test -s set_associativity block_size_b cache_size_kb hex_addr\n");
        return;
    }

    long setAssoc = strtol(argv[2], NULL, 10);
    long blockSize = strtol(argv[3], NULL, 10);
    long cacheSize = strtol(argv[4], NULL, 10);
    unsigned long inputNum = (unsigned long) strtol(argv[5], NULL, 16);

    Cache *cache = cacheAlloc((int) setAssoc, (int) blockSize, (int) cacheSize);

    printf("Cache config: setAssoc = %ld, blockSize = %ld, cacheSize = %ld\n", setAssoc, blockSize, cacheSize);
    printf("\tnumBlocks = %d, numSets = %d\n", cache->numBlocks, cache->numSets);
    printf("\toffsetBits = %d, indexBits = %d, tagBits = %d\n", cache->offsetBits, cache->indexBits, cache->tagBits);


    printf("Input address:\n");
    printf("\tAddress: %lx\n", inputNum);

    printf("Set: %d\n", whichSet(cache, inputNum));

    cacheFree(cache);
}

void hitWayTest(int argc, char *argv[]) {
    if (argc != 6) {
        printf("usage: ./test -h set_associativity block_size_b cache_size_kb hex_addr\n");
        return;
    }

    long setAssoc = strtol(argv[2], NULL, 10);
    long blockSize = strtol(argv[3], NULL, 10);
    long cacheSize = strtol(argv[4], NULL, 10);
    unsigned long inputNum = (unsigned long) strtol(argv[5], NULL, 16);

    Cache *cache = cacheAlloc((int) setAssoc, (int) blockSize, (int) cacheSize);

    printf("Cache config: setAssoc = %ld, blockSize = %ld, cacheSize = %ld\n", setAssoc, blockSize, cacheSize);
    printf("\tnumBlocks = %d, numSets = %d\n", cache->numBlocks, cache->numSets);
    printf("\toffsetBits = %d, indexBits = %d, tagBits = %d\n", cache->offsetBits, cache->indexBits, cache->tagBits);


    printf("Input address:\n");
    printf("\tAddress: %lx\n", inputNum);

    printf("Hit?: %d\n", hitWay(cache, inputNum));
    printf("middle\n");
    printf("Again?: %d\n", hitWay(cache, inputNum));


    cacheFree(cache);
}

void _print_bits(unsigned long long num) {
    // hard-coded to print 32-bit values
    int i;
    for (i = 31; i >= 0; --i) {
        printf("%u", (unsigned int) ((num >> i) & 1));
    }
}