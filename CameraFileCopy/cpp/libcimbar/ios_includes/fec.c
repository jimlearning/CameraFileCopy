/**
 * iOS platform compatibility implementation for fec.h
 * 
 * This file provides stub implementations for iOS compatibility of functions
 * from the libfec library, which is not available on iOS.
 */

#include "fec.h"
#include <stdio.h>

// K=7 rate=1/2 convolutional encoder stubs
void *create_viterbi27(int len) {
    printf("WARNING: Using stub implementation of create_viterbi27 on iOS\n");
    return malloc(1); // Return dummy pointer
}

int init_viterbi27(void *vp, int starting_state) {
    return 0;
}

int update_viterbi27_blk(void *vp, unsigned char *syms, int nbits) {
    return 0;
}

int chainback_viterbi27(void *vp, unsigned char *data, int nbits, int endstate) {
    return 0;
}

void delete_viterbi27(void *vp) {
    if (vp) free(vp);
}

// K=9 rate=1/2 convolutional encoder stubs
void *create_viterbi29(int len) {
    return malloc(1);
}

int init_viterbi29(void *vp, int starting_state) {
    return 0;
}

int update_viterbi29_blk(void *vp, unsigned char *syms, int nbits) {
    return 0;
}

int chainback_viterbi29(void *vp, unsigned char *data, int nbits, int endstate) {
    return 0;
}

void delete_viterbi29(void *vp) {
    if (vp) free(vp);
}

// K=9 rate=1/3 convolutional encoder stubs
void *create_viterbi39(int len) {
    return malloc(1);
}

int init_viterbi39(void *vp, int starting_state) {
    return 0;
}

int update_viterbi39_blk(void *vp, unsigned char *syms, int nbits) {
    return 0;
}

int chainback_viterbi39(void *vp, unsigned char *data, int nbits, int endstate) {
    return 0;
}

void delete_viterbi39(void *vp) {
    if (vp) free(vp);
}

// K=15 rate=1/6 convolutional encoder stubs
void *create_viterbi615(int len) {
    return malloc(1);
}

int init_viterbi615(void *vp, int starting_state) {
    return 0;
}

int update_viterbi615_blk(void *vp, unsigned char *syms, int nbits) {
    return 0;
}

int chainback_viterbi615(void *vp, unsigned char *data, int nbits, int endstate) {
    return 0;
}

void delete_viterbi615(void *vp) {
    if (vp) free(vp);
}

// Reed-Solomon codec stubs
void free_rs_char(void *rs) {
    if (rs) free(rs);
}

void *init_rs_char(int symsize, int gfpoly, int fcr, int prim, int nroots) {
    RS *rs = (RS*)malloc(sizeof(RS));
    if (rs) {
        rs->nroots = nroots;
        rs->fcr = fcr;
        rs->prim = prim;
        rs->iprim = 0;
        rs->genpoly = NULL;
        rs->symsize = symsize;
        rs->blocks = 0;
    }
    return rs;
}

int decode_rs_char(void *rs, unsigned char *data, int *eras_pos, int no_eras, int pad) {
    return 0;
}

int encode_rs_char(void *rs, unsigned char *data, unsigned char *parity) {
    return 0;
}
