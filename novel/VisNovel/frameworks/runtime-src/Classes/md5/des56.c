
/*
 * Fast implementation of the DES, as described in the Federal Register,
 * Vol. 40, No. 52, p. 12134, March 17, 1975.
 *
 * Stuart Levy, Minnesota Supercomputer Center, April 1988.
 * Currently (2007) slevy@ncsa.uiuc.edu
 * NCSA, University of Illinois Urbana-Champaign
 *
 * Calling sequence:
 *
 * typedef unsigned long keysched[32];
 *
 * fsetkey(key, keysched)	/ * Converts a DES key to a "key schedule" * /
 *	unsigned char	key[8];
 *	keysched	*ks;
 *
 * fencrypt(block, decrypt, keysched)	/ * En/decrypts one 64-bit block * /
 *	unsigned char	block[8];	/ * data, en/decrypted in place * /
 *	int		decrypt;	/ * 0=>encrypt, 1=>decrypt * /
 *	keysched	*ks;		/ * key schedule, as set by fsetkey * /
 *
 * Key and data block representation:
 * The 56-bit key (bits 1..64 including "parity" bits 8, 16, 24, ..., 64)
 * and the 64-bit data block (bits 1..64)
 * are each stored in arrays of 8 bytes.
 * Following the NBS numbering, the MSB has the bit number 1, so
 *  key[0] = 128*bit1 + 64*bit2 + ... + 1*bit8, ... through
 *  key[7] = 128*bit57 + 64*bit58 + ... + 1*bit64.
 * In the key, "parity" bits are not checked; their values are ignored.
 *
*/

/*
===============================================================================
License

des56.c is licensed under the terms of the MIT license reproduced below.
This means that des56.c is free software and can be used for both academic
and commercial purposes at absolutely no cost.
===============================================================================
Copyright (C) 1988 Stuart Levy

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
 */


#ifdef __cplusplus
extern "C" {
#endif    
#include "des56.h"


/*
 * Key schedule generation.
 * We begin by pointlessly permuting the 56 useful key bits into
 * two groups of 28 bits called C and D.
 * bK_C and bK_D are indexed by C and D bit numbers, respectively,
 * and give the key bit number (1..64) which should initialize that C/D bit.
 * This is the "permuted choice 1" table.
 */

static tiny bK_C[28] = {
	57, 49, 41, 33, 25, 17,  9,
	 1, 58, 50, 42, 34, 26, 18,
	10,  2, 59, 51, 43, 35, 27,
	19, 11,  3, 60, 52, 44, 36,
};
static tiny bK_D[28] = {
	63, 55, 47, 39, 31, 23, 15,
	 7, 62, 54, 46, 38, 30, 22,
	14,  6, 61, 53, 45, 37, 29,
	21, 13,  5, 28, 20, 12, 4,
};

/*
 * For speed, we invert these, building tables to map groups of
 * key bits into the corresponding C and D bits.
 * We represent C and D each as 28 contiguous bits right-justified in a
 * word, padded on the left with zeros.
 * If key byte `i' is said to contain bits Ki,0 (MSB) Ki,1 ... Ki,7 (LSB)
 * then
 *	wC_K4[i][Ki,0 Ki,1 Ki,2 Ki,3] gives the C bits for Ki,0..3,
 *	wD_K4[i][Ki,0 Ki,1 Ki,2 Ki,3] the corresponding D bits,
 *	wC_K3[i][Ki,4 Ki,5 Ki,6] the C bits for Ki,4..6,
 * and	wD_K3[i][Ki,4 Ki,5 Ki,6] the D bits for Ki,4..6.
 * Ki,7 is ignored since it is the nominal parity bit.
 * We could just use a single table for [i][Ki,0 .. Ki,6] but that
 * would take a lot of storage for such a rarely-used function.
 */

static	word32 wC_K4[8][16], wC_K3[8][8];
static	word32 wD_K4[8][16], wD_K3[8][8];

/*
 * Successive Ci and Di for the sixteen steps in the key schedule are
 * created by independent 28-bit left circular shifts on C and D.
 * The shift count varies with the step number.
 */
static tiny preshift[16] = {
	1, 1, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 1,
};

/*
 * Each step in the key schedule is generated by selecting 48 bits
 * (8 groups of 6 bits) from the appropriately shifted Ci and Di.
 * bCD_KS, indexed by the key schedule bit number, gives the bit number
 * in CD (CD1 = MSB of C, CD28 = LSB of C, CD29 = MSB of D, CD56 = LSB of D)
 * which determines that bit of the key schedule.
 * Note that only C bits (1..28) appear in the first (upper) 24 bits of
 * the key schedule, and D bits (29..56) in the second (lower) 24 bits.
 * This is the "permuted-choice-2" table.
 */

static tiny bCD_KS[48] = {
	14, 17, 11, 24,  1,  5,
	3,  28, 15,  6, 21, 10,
	23, 19, 12,  4, 26,  8,
	16,  7, 27, 20, 13,  2,
	41, 52, 31, 37, 47, 55,
	30, 40, 51, 45, 33, 48,
	44, 49, 39, 56, 34, 53,
	46, 42, 50, 36, 29, 32,
};

/*
 * We invert bCD_KS into a pair of tables which map groups of 4
 * C or D bits into corresponding key schedule bits.
 * We represent each step of the key schedule as 8 groups of 8 bits,
 * with the 6 real bits right-justified in each 8-bit group.
 * hKS_C4[i][C4i+1 .. C4i+4] gives the bits in the high order (first four)
 * key schedule "bytes" which correspond to C bits 4i+1 .. 4i+4.
 * lKS_D4[i][D4i+1 .. D4i+4] gives the appropriate bits in the latter (last 4)
 * key schedule bytes, from the corresponding D bits.
 */

static word32 hKS_C4[7][16];
static word32 lKS_D4[7][16];

/*
 * Encryption/decryption.
 * Before beginning, and after ending, we perform another useless permutation
 * on the bits in the data block.
 *
 * The initial permutation and its inverse, final permutation
 * are too simple to need a table for.	If we break the input I1 .. I64 into
 * 8-bit chunks I0,0 I0,1 ... I0,7 I1,0 I1,1 ... I7,7
 * then the initial permutation sets LR as follows:
 * L = I7,1 I6,1 I5,1 ... I0,1	I7,3 I6,3 ... I0,3  I7,5 ... I0,5  I7,7 ... I0,7
 * and
 * R = I7,0 I6,0 I5,0 ... I0,0	I7,2 I6,2 ... I0,2  I7,4 ... I0,4  I7,6 ... I0,6
 *
 * If we number the bits in the final LR similarly,
 * L = L0,0 L0,1 ... L3,7  R = R0,0 R0,1 ... R3,7
 * then the output is
 * O = R0,7 L0,7 R1,7 L1,7 ... R3,7 L3,7 R0,6 L0,6 ... L3,6 R0,5 ... R3,0 L3,0
 *
 * To speed I => LR shuffling we use an array of 32-bit values indexed by
 * 8-bit input bytes.
 * wL_I8[ 0 I0,1 0 I0,3 0 I0,5 0 I0,7 ] = the corresponding L bits.
 * Other R and L bits are derived from wL_I8 by shifting.
 *
 * To speed LR => O shuffling, an array of 32-bit values indexed by 4-bit lumps:
 * wO_L4[ L0,4 L0,5 L0,6 L0,7 ] = the corresponding high-order 32 O bits.
 */

static word32 wL_I8[0x55 + 1];
static word32 wO_L4[16];

/*
 * Core of encryption/decryption.
 * In each key schedule stage, we:
 *	take 8 overlapping groups of 6 bits each from R
 *	   (the NBS tabulates the bit selections in the E table,
 *	    but it's so simple we just use shifting to get the right bits)
 *	XOR each group with the corresponding bits from the key schedule
 *	Use the resulting 6 bits as an index into the appropriate S table
 *	   (there are 8 such tables, one per group of 6 bits)
 *	Each S entry yields 4 bits.
 *	The 8 groups of 4 bits are catenated into a 32-bit value.
 *	Those 32 bits are permuted according to the P table.
 *	Finally the permuted 32-bit value is XORed with L and becomes
 *	the R value for the next stage, while the previous R becomes the new L.
 *
 * Here, we merge the P permutation with the S tables by making the
 * S entries be 32-bit masks, already suitably permuted.
 * Also, the bits in each six-bit group must be permuted before use as
 * an index into the NBS-tabulated S tables.
 * We rearrange entries in wPS so that natural bit order can be used.
 */

static word32 wPS[8][64];

static tiny P[32] = {
	16,  7, 20, 21,
	29, 12, 28, 17,
	 1, 15, 23, 26,
	 5, 18, 31, 10,
	 2,  8, 24, 14,
	32, 27,  3,  9,
	19, 13, 30,  6,
	22, 11,  4, 25,
};

static tiny S[8][64] = {
     {
	14, 4,13, 1, 2,15,11, 8, 3,10, 6,12, 5, 9, 0, 7,
	 0,15, 7, 4,14, 2,13, 1,10, 6,12,11, 9, 5, 3, 8,
	 4, 1,14, 8,13, 6, 2,11,15,12, 9, 7, 3,10, 5, 0,
	15,12, 8, 2, 4, 9, 1, 7, 5,11, 3,14,10, 0, 6,13,
     },

     {
	15, 1, 8,14, 6,11, 3, 4, 9, 7, 2,13,12, 0, 5,10,
	 3,13, 4, 7,15, 2, 8,14,12, 0, 1,10, 6, 9,11, 5,
	 0,14, 7,11,10, 4,13, 1, 5, 8,12, 6, 9, 3, 2,15,
	13, 8,10, 1, 3,15, 4, 2,11, 6, 7,12, 0, 5,14, 9,
     },

     {
	10, 0, 9,14, 6, 3,15, 5, 1,13,12, 7,11, 4, 2, 8,
	13, 7, 0, 9, 3, 4, 6,10, 2, 8, 5,14,12,11,15, 1,
	13, 6, 4, 9, 8,15, 3, 0,11, 1, 2,12, 5,10,14, 7,
	 1,10,13, 0, 6, 9, 8, 7, 4,15,14, 3,11, 5, 2,12,
     },

     {
	 7,13,14, 3, 0, 6, 9,10, 1, 2, 8, 5,11,12, 4,15,
	13, 8,11, 5, 6,15, 0, 3, 4, 7, 2,12, 1,10,14, 9,
	10, 6, 9, 0,12,11, 7,13,15, 1, 3,14, 5, 2, 8, 4,
	 3,15, 0, 6,10, 1,13, 8, 9, 4, 5,11,12, 7, 2,14,
     },

     {
	 2,12, 4, 1, 7,10,11, 6, 8, 5, 3,15,13, 0,14, 9,
	14,11, 2,12, 4, 7,13, 1, 5, 0,15,10, 3, 9, 8, 6,
	 4, 2, 1,11,10,13, 7, 8,15, 9,12, 5, 6, 3, 0,14,
	11, 8,12, 7, 1,14, 2,13, 6,15, 0, 9,10, 4, 5, 3,
     },

     {
	12, 1,10,15, 9, 2, 6, 8, 0,13, 3, 4,14, 7, 5,11,
	10,15, 4, 2, 7,12, 9, 5, 6, 1,13,14, 0,11, 3, 8,
	 9,14,15, 5, 2, 8,12, 3, 7, 0, 4,10, 1,13,11, 6,
	 4, 3, 2,12, 9, 5,15,10,11,14, 1, 7, 6, 0, 8,13,
     },

     {
	 4,11, 2,14,15, 0, 8,13, 3,12, 9, 7, 5,10, 6, 1,
	13, 0,11, 7, 4, 9, 1,10,14, 3, 5,12, 2,15, 8, 6,
	 1, 4,11,13,12, 3, 7,14,10,15, 6, 8, 0, 5, 9, 2,
	 6,11,13, 8, 1, 4,10, 7, 9, 5, 0,15,14, 2, 3,12,
     },

     {
	13, 2, 8, 4, 6,15,11, 1,10, 9, 3,14, 5, 0,12, 7,
	 1,15,13, 8,10, 3, 7, 4,12, 5, 6,11, 0,14, 9, 2,
	 7,11, 4, 1, 9,12,14, 2, 0, 6,10,13,15, 3, 5, 8,
	 2, 1,14, 7, 4,10, 8,13,15,12, 9, 0, 3, 5, 6,11,
     },
};

static void buildtables( void )
{
	register int i, j;
	register word32 v;
	word32 wC_K[64], wD_K[64];
	word32 hKS_C[28], lKS_D[28];
	int Smap[64];
	word32 wP[32];

#if USG
#  define	ZERO(array)	memset((char *)(array), '\0', sizeof(array))
#else
# if BSD
#  define	ZERO(array)	bzero((char *)(array), sizeof(array))
# else 
#  define	ZERO(array)	{ register word32 *p = (word32 *)(array); \
				  i = sizeof(array) / sizeof(*p); \
				  do { *p++ = 0; } while(--i > 0); \
				}
# endif 
#endif 


	/* Invert permuted-choice-1 (key => C,D) */

	ZERO(wC_K);
	ZERO(wD_K);
	v = 1;
	for(j = 28; --j >= 0; ) {
		wC_K[ bK_C[j] - 1 ] = wD_K[ bK_D[j] - 1 ] = v;
		v += v; 	/* (i.e. v <<= 1) */
	}

	for(i = 0; i < 64; i++) {
	    int t = 8 >> (i & 3);
	    for(j = 0; j < 16; j++) {
		if(j & t) {
		    wC_K4[i >> 3][j] |= wC_K[i];
		    wD_K4[i >> 3][j] |= wD_K[i];
		    if(j < 8) {
			wC_K3[i >> 3][j] |= wC_K[i + 3];
			wD_K3[i >> 3][j] |= wD_K[i + 3];
		    }
		}
	    }
	    /* Generate the sequence 0,1,2,3, 8,9,10,11, ..., 56,57,58,59. */
	    if(t == 1) i += 4;
	}

	/* Invert permuted-choice-2 */

	ZERO(hKS_C);
	ZERO(lKS_D);
	v = 1;
	for(i = 24; (i -= 6) >= 0; ) {
	    j = i+5;
	    do {
		hKS_C[ bCD_KS[j] - 1 ] = lKS_D[ bCD_KS[j+24] - 28 - 1 ] = v;
		v += v; 	/* Like v <<= 1 but may be faster */
	    } while(--j >= i);
	    v <<= 2;		/* Keep byte aligned */
	}

	for(i = 0; i < 28; i++) {
	    v = 8 >> (i & 3);
	    for(j = 0; j < 16; j++) {
		if(j & v) {
		    hKS_C4[i >> 2][j] |= hKS_C[i];
		    lKS_D4[i >> 2][j] |= lKS_D[i];
		}
	    }
	}

	/* Initial permutation */

	for(i = 0; i <= 0x55; i++) {
	    v = 0;
	    if(i & 64) v =  (word32) 1 << 24;
	    if(i & 16) v |= (word32) 1 << 16;
	    if(i & 4)  v |= (word32) 1 << 8;
	    if(i & 1)  v |= 1;
	    wL_I8[i] = v;
	}

	/* Final permutation */

	for(i = 0; i < 16; i++) {
	    v = 0;
	    if(i & 1) v = (word32) 1 << 24;
	    if(i & 2) v |= (word32) 1 << 16;
	    if(i & 4) v |= (word32) 1 << 8;
	    if(i & 8) v |= (word32) 1;
	    wO_L4[i] = v;
	}

	/* Funny bit rearrangement on second index into S tables */

	for(i = 0; i < 64; i++) {
		Smap[i] = (i & 0x20) | (i & 1) << 4 | (i & 0x1e) >> 1;
	}

	/* Invert permutation P into mask indexed by R bit number */

	v = 1;
	for(i = 32; --i >= 0; ) {
		wP[ P[i] - 1 ] = v;
		v += v;
	}

	/* Build bit-mask versions of S tables, indexed in natural bit order */

	for(i = 0; i < 8; i++) {
	    for(j = 0; j < 64; j++) {
		int k, t;

		t = S[i][ Smap[j] ];
		for(k = 0; k < 4; k++) {
		    if(t & 8)
			wPS[i][j] |= wP[4*i + k];
		    t += t;
		}
	    }
	}
}


void fsetkey(char key[8], keysched *ks)
{
	register int i;
	register word32 C, D;
	static int built = 0;

	if(!built) {
		buildtables();
		built = 1;
	}

	C = D = 0;
	for(i = 0; i < 8; i++) {
		register int v;

		v = key[i] >> 1;	/* Discard "parity" bit */
		C |= wC_K4[i][(v>>3) & 15] | wC_K3[i][v & 7];
		D |= wD_K4[i][(v>>3) & 15] | wD_K3[i][v & 7];
	}

	/*
	 * C and D now hold the suitably right-justified
	 * 28 permuted key bits each.
	 */
	for(i = 0; i < 16; i++) {
#ifdef CRAY
#define choice2(x, v)  x[6][v&15] | x[5][(v>>4)&15] | x[4][(v>>8)&15] | \
		    x[3][(v>>12)&15] | x[2][(v>>16)&15] | x[1][(v>>20)&15] | \
		    x[0][(v>>24)&15]
#else
		register word32 *ap;

#  define choice2(x, v)  ( \
		    ap = &(x)[0][0], \
		    ap[16*6 + (v&15)] | \
		    ap[16*5 + ((v>>4)&15)]  | ap[16*4 + ((v>>8)&15)]  | \
		    ap[16*3 + ((v>>12)&15)] | ap[16*2 + ((v>>16)&15)] | \
		    ap[16*1 + ((v>>20)&15)] | ap[16*0 + ((v>>24)&15)] )
#endif 


		/* 28-bit left circular shift */
		C <<= preshift[i];
		C = ((C >> 28) & 3) | (C & (((word32)1<<28) - 1));
		ks->KS[i].h = choice2(hKS_C4, C);

		D <<= preshift[i];
		D = ((D >> 28) & 3) | (D & (((word32)1<<28) - 1));
		ks->KS[i].l = choice2(lKS_D4, D);
	}
}

void
fencrypt(char block[8], int decrypt, keysched *ks)
{
	int i;
	register word32 L, R;
	#ifdef __cplusplus
	register struct keysched::keystage *ksp;
	#else
	register struct keystage *ksp;
	#endif    
	register word32 *ap;

	/* Initial permutation */

	L = R = 0;
	i = 7;
	ap = wL_I8;
	do {
		register int v;

		v = block[i];	/* Could optimize according to ENDIAN */
		L = ap[v & 0x55] | (L << 1);
		R = ap[(v >> 1) & 0x55] | (R << 1);
	} while(--i >= 0);

	if(decrypt) {
		ksp = &ks->KS[15];
	} else {
		ksp = &ks->KS[0];
	}

#ifdef CRAY
#  define PS(i,j)	wPS[i][j]
#else 
#  define PS(i,j)	ap[64*(i) + (j)]
	ap = &wPS[0][0];
#endif 

	i = 16;
	do {
		register word32 k, tR;

		tR = (R >> 15) | (R << 17);

		k = ksp->h;
		L ^= PS(0, ((tR >> 12) ^ (k >> 24)) & 63)
		   | PS(1, ((tR >> 8) ^ (k >> 16)) & 63)
		   | PS(2, ((tR >> 4) ^ (k >> 8)) & 63)
		   | PS(3, (tR ^ k) & 63);

		k = ksp->l;
		L ^= PS(4, ((R >> 11) ^ (k >> 24)) & 63)
		   | PS(5, ((R >> 7) ^ (k >> 16)) & 63)
		   | PS(6, ((R >> 3) ^ (k >> 8)) & 63)
		   | PS(7, ((tR >> 16) ^ k) & 63);

		tR = L;
		L = R;
		R = tR;


		if(decrypt)
			ksp--;
		else
			ksp++;
	} while(--i > 0);
	{
		register word32 t;

#ifdef CRAY
# define FP(k)	(wO_L4[ (L >> (k)) & 15 ] << 1 | wO_L4[ (R >> (k)) & 15 ])
#else 
# define FP(k)	(ap[ (L >> (k)) & 15 ] << 1 | ap[ (R >> (k)) & 15 ])

		ap = wO_L4;
#endif 

		t = FP(0) | (FP(8) | (FP(16) | (FP(24) << 2)) << 2) << 2;
		R = FP(4) | (FP(12) | (FP(20) | (FP(28) << 2)) << 2) << 2;
		L = t;
	}
	{
		register word32 t;
		register char *bp;

		bp = &block[7];
		t = R;
		*bp = t & 255;
		*--bp = (t >>= 8) & 255;
		*--bp = (t >>= 8) & 255;
		*--bp = (t >> 8) & 255;
		t = L;
		*--bp = t & 255;
		*--bp = (t >>= 8) & 255;
		*--bp = (t >>= 8) & 255;
		*--bp = (t >> 8) & 255;
	}
}

#ifdef __cplusplus
}
#endif   