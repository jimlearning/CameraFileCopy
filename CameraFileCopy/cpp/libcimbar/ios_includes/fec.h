/**
 * iOS platform compatibility header for fec.h
 * 
 * This is a stub implementation for iOS compatibility, providing empty implementations
 * of functions from the libfec library, which is not available on iOS.
 */

#ifndef FEC_H_iOS_COMPAT
#define FEC_H_iOS_COMPAT

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

// Viterbi codec types
typedef void *void_t;

// Function stubs for K=7 rate=1/2 convolutional encoder
void *create_viterbi27(int);
int init_viterbi27(void *, int);
int update_viterbi27_blk(void *, unsigned char *, int);
int chainback_viterbi27(void *, unsigned char *, int, int);
void delete_viterbi27(void *);

// Function stubs for K=9 rate=1/2 convolutional encoder
void *create_viterbi29(int);
int init_viterbi29(void *, int);
int update_viterbi29_blk(void *, unsigned char *, int);
int chainback_viterbi29(void *, unsigned char *, int, int);
void delete_viterbi29(void *);

// Function stubs for K=9 rate=1/3 convolutional encoder
void *create_viterbi39(int);
int init_viterbi39(void *, int);
int update_viterbi39_blk(void *, unsigned char *, int);
int chainback_viterbi39(void *, unsigned char *, int, int);
void delete_viterbi39(void *);

// Function stubs for K=15 rate=1/6 convolutional encoder
void *create_viterbi615(int);
int init_viterbi615(void *, int);
int update_viterbi615_blk(void *, unsigned char *, int);
int chainback_viterbi615(void *, unsigned char *, int, int);
void delete_viterbi615(void *);

// Reed-Solomon codec stubs
typedef struct {
    int nroots;
    unsigned int fcr;
    unsigned int prim;
    unsigned int iprim;
    unsigned char *genpoly;
    int symsize;
    int blocks;
} RS;

void free_rs_char(void *);
void *init_rs_char(int, int, int, int, int);
int decode_rs_char(void *, unsigned char *, int *, int, int);
int encode_rs_char(void *, unsigned char *, unsigned char *);

#ifdef __cplusplus
}
#endif

#endif // FEC_H_iOS_COMPAT
