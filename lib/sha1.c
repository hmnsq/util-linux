/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * SHA-1 in C by Steve Reid <steve@edmweb.com>
 * 100% Public Domain
 *
 * Test Vectors (from FIPS PUB 180-1)
 * 1) "abc": A9993E36 4706816A BA3E2571 7850C26C 9CD0D89D
 * 2) "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq":  84983E44 1C3BD26E BAAE4AA1 F95129E5 E54670F1
 * 3) A million repetitions of "a":  34AA973C D4C4DAA4 F61EEB2B DBAD2731 6534016F
 */

#నిర్వచించు UL_SHA1HANDSOFF

#అన్తర్భావ <stdio.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <stdint.h>

#అన్తర్భావ "sha1.h"

#నిర్వచించు rol(విలువ, బిట్లు) (((విలువ) << (బిట్లు)) | ((విలువ) >> (32 - (బిట్లు))))

/* blk0() and blk() perform the initial expand. */
#ఒకవేళ __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
# define blk0(i) ఖండం->l[i]
#లేకపోతే
# define blk0(i) (ఖండం->l[i] = (rol(ఖండం->l[i],24)&0xFF00FF00) \
    |(rol(ఖండం->l[i],8)&0x00FF00FF))
#అయితే_ముగింపు

#నిర్వచించు blk(i) (ఖండం->l[i&15] = rol(ఖండం->l[(i+13)&15]^ఖండం->l[(i+8)&15] \
    ^ఖండం->l[(i+2)&15]^ఖండం->l[i&15],1))

/* (R0+R1), R2, R3, R4 are the different operations used in SHA1 */
#నిర్వచించు R0(v,w,x,y,z,i) z+=((w&(x^y))^y)+blk0(i)+0x5A827999+rol(v,5);w=rol(w,30);
#నిర్వచించు R1(v,w,x,y,z,i) z+=((w&(x^y))^y)+blk(i)+0x5A827999+rol(v,5);w=rol(w,30);
#నిర్వచించు R2(v,w,x,y,z,i) z+=(w^x^y)+blk(i)+0x6ED9EBA1+rol(v,5);w=rol(w,30);
#నిర్వచించు R3(v,w,x,y,z,i) z+=(((w|x)&y)|(w&x))+blk(i)+0x8F1BBCDC+rol(v,5);w=rol(w,30);
#నిర్వచించు R4(v,w,x,y,z,i) z+=(w^x^y)+blk(i)+0xCA62C1D6+rol(v,5);w=rol(w,30);

/* Hash a single 512-bit block. This is the core of the algorithm. */

శూన్య ul_SHA1Transform(uint32_t state[5], స్థిరరాశి గుర్తులేని అక్షరం బఫరు[64])
{
	uint32_t a, b, c, d, e;

	రకనిర్వచనం సంఘం {
		గుర్తులేని అక్షరం c[64];
		uint32_t l[16];
	} CHAR64LONG16;

#నిర్వచితమైతే UL_SHA1HANDSOFF
	CHAR64LONG16 ఖండం[1];	/* use array to appear as a pointer */

	జ్ఞాపకం_నకలు(ఖండం, బఫరు, 64);
#లేకపోతే
	/* The following had better never be used because it causes the
	 * pointer-to-const buffer to be cast into a pointer to non-const.
	 * And the result is written through.  I threw a "const" in, hoping
	 * this will cause a diagnostic.
	 */
	CHAR64LONG16 *ఖండం = (స్థిరరాశి CHAR64LONG16 *)బఫరు;
#అయితే_ముగింపు
	/* Copy context->state[] to working vars */
	a = state[0];
	b = state[1];
	c = state[2];
	d = state[3];
	e = state[4];
	/* 4 rounds of 20 operations each. Loop unrolled. */
	R0(a, b, c, d, e, 0);
	R0(e, a, b, c, d, 1);
	R0(d, e, a, b, c, 2);
	R0(c, d, e, a, b, 3);
	R0(b, c, d, e, a, 4);
	R0(a, b, c, d, e, 5);
	R0(e, a, b, c, d, 6);
	R0(d, e, a, b, c, 7);
	R0(c, d, e, a, b, 8);
	R0(b, c, d, e, a, 9);
	R0(a, b, c, d, e, 10);
	R0(e, a, b, c, d, 11);
	R0(d, e, a, b, c, 12);
	R0(c, d, e, a, b, 13);
	R0(b, c, d, e, a, 14);
	R0(a, b, c, d, e, 15);
	R1(e, a, b, c, d, 16);
	R1(d, e, a, b, c, 17);
	R1(c, d, e, a, b, 18);
	R1(b, c, d, e, a, 19);
	R2(a, b, c, d, e, 20);
	R2(e, a, b, c, d, 21);
	R2(d, e, a, b, c, 22);
	R2(c, d, e, a, b, 23);
	R2(b, c, d, e, a, 24);
	R2(a, b, c, d, e, 25);
	R2(e, a, b, c, d, 26);
	R2(d, e, a, b, c, 27);
	R2(c, d, e, a, b, 28);
	R2(b, c, d, e, a, 29);
	R2(a, b, c, d, e, 30);
	R2(e, a, b, c, d, 31);
	R2(d, e, a, b, c, 32);
	R2(c, d, e, a, b, 33);
	R2(b, c, d, e, a, 34);
	R2(a, b, c, d, e, 35);
	R2(e, a, b, c, d, 36);
	R2(d, e, a, b, c, 37);
	R2(c, d, e, a, b, 38);
	R2(b, c, d, e, a, 39);
	R3(a, b, c, d, e, 40);
	R3(e, a, b, c, d, 41);
	R3(d, e, a, b, c, 42);
	R3(c, d, e, a, b, 43);
	R3(b, c, d, e, a, 44);
	R3(a, b, c, d, e, 45);
	R3(e, a, b, c, d, 46);
	R3(d, e, a, b, c, 47);
	R3(c, d, e, a, b, 48);
	R3(b, c, d, e, a, 49);
	R3(a, b, c, d, e, 50);
	R3(e, a, b, c, d, 51);
	R3(d, e, a, b, c, 52);
	R3(c, d, e, a, b, 53);
	R3(b, c, d, e, a, 54);
	R3(a, b, c, d, e, 55);
	R3(e, a, b, c, d, 56);
	R3(d, e, a, b, c, 57);
	R3(c, d, e, a, b, 58);
	R3(b, c, d, e, a, 59);
	R4(a, b, c, d, e, 60);
	R4(e, a, b, c, d, 61);
	R4(d, e, a, b, c, 62);
	R4(c, d, e, a, b, 63);
	R4(b, c, d, e, a, 64);
	R4(a, b, c, d, e, 65);
	R4(e, a, b, c, d, 66);
	R4(d, e, a, b, c, 67);
	R4(c, d, e, a, b, 68);
	R4(b, c, d, e, a, 69);
	R4(a, b, c, d, e, 70);
	R4(e, a, b, c, d, 71);
	R4(d, e, a, b, c, 72);
	R4(c, d, e, a, b, 73);
	R4(b, c, d, e, a, 74);
	R4(a, b, c, d, e, 75);
	R4(e, a, b, c, d, 76);
	R4(d, e, a, b, c, 77);
	R4(c, d, e, a, b, 78);
	R4(b, c, d, e, a, 79);
	/* Add the working vars back into context.state[] */
	state[0] += a;
	state[1] += b;
	state[2] += c;
	state[3] += d;
	state[4] += e;
	/* Wipe variables */
#నిర్వచితమైతే HAVE_EXPLICIT_BZERO
	explicit_bzero(&a, పరిమాణం(a));
	explicit_bzero(&b, పరిమాణం(b));
	explicit_bzero(&c, పరిమాణం(c));
	explicit_bzero(&d, పరిమాణం(d));
	explicit_bzero(&e, పరిమాణం(e));
#లేకపోతే
	a = b = c = d = e = 0;
#అయితే_ముగింపు
#నిర్వచితమైతే UL_SHA1HANDSOFF
	memset(ఖండం, '\0', పరిమాణం(ఖండం));
#అయితే_ముగింపు
}

/* SHA1Init - Initialize new context */

శూన్య ul_SHA1Init(UL_SHA1_CTX *context)
{
	/* SHA1 initialization constants */
	context->state[0] = 0x67452301;
	context->state[1] = 0xEFCDAB89;
	context->state[2] = 0x98BADCFE;
	context->state[3] = 0x10325476;
	context->state[4] = 0xC3D2E1F0;
	context->లెక్క[0] = context->లెక్క[1] = 0;
}

/* Run your data through this. */

శూన్య ul_SHA1Update(UL_SHA1_CTX *context, స్థిరరాశి గుర్తులేని అక్షరం *దత్తాంశం, uint32_t len)
{
	uint32_t i;

	uint32_t j;

	j = context->లెక్క[0];
	ఒకవేళ ((context->లెక్క[0] += len << 3) < j)
		context->లెక్క[1]++;
	context->లెక్క[1] += (len >> 29);
	j = (j >> 3) & 63;
	ఒకవేళ ((j + len) > 63) {
		జ్ఞాపకం_నకలు(&context->బఫరు[j], దత్తాంశం, (i = 64 - j));
		ul_SHA1Transform(context->state, context->బఫరు);
		కొరకు (; i + 63 < len; i += 64) {
			ul_SHA1Transform(context->state, &దత్తాంశం[i]);
		}
		j = 0;
	} లేకపోతే
		i = 0;
	జ్ఞాపకం_నకలు(&context->బఫరు[j], &దత్తాంశం[i], len - i);
}

/* Add padding and return the message digest. */

శూన్య ul_SHA1Final(గుర్తులేని అక్షరం digest[20], UL_SHA1_CTX *context)
{
	గుర్తులేని i;

	గుర్తులేని అక్షరం finalcount[8];

	గుర్తులేని అక్షరం c;

#ఒకవేళ 0				/* untested "improvement" by DHR */
	/* Convert context->count to a sequence of bytes
	 * in finalcount.  Second element first, but
	 * big-endian order within element.
	 * But we do it all backwards.
	 */
	గుర్తులేని అక్షరం *fcp = &finalcount[8];

	కొరకు (i = 0; i < 2; i++) {
		uint32_t t = context->లెక్క[i];

		పూర్ణాంకం j;

		కొరకు (j = 0; j < 4; t >>= 8, j++)
			*--fcp = (గుర్తులేని అక్షరం)t}
#లేకపోతే
	కొరకు (i = 0; i < 8; i++) {
		finalcount[i] = (గుర్తులేని అక్షరం)((context->లెక్క[(i >= 4 ? 0 : 1)] >> ((3 - (i & 3)) * 8)) & 255);	/* Endian independent */
	}
#అయితే_ముగింపు
	c = 0200;
	ul_SHA1Update(context, &c, 1);
	అయితే ((context->లెక్క[0] & 504) != 448) {
		c = 0000;
		ul_SHA1Update(context, &c, 1);
	}
	ul_SHA1Update(context, finalcount, 8);	/* Should cause a SHA1Transform() */
	కొరకు (i = 0; i < 20; i++) {
		digest[i] = (గుర్తులేని అక్షరం)
		    ((context->state[i >> 2] >> ((3 - (i & 3)) * 8)) & 255);
	}
	/* Wipe variables */
	memset(context, '\0', పరిమాణం(*context));
	memset(&finalcount, '\0', పరిమాణం(finalcount));
}

శూన్య ul_SHA1(అక్షరం *hash_బహిర్గతం, స్థిరరాశి అక్షరం *str, గుర్తులేని len)
{
	UL_SHA1_CTX ctx;
	గుర్తులేని పూర్ణాంకం ii;

	ul_SHA1Init(&ctx);
	కొరకు (ii = 0; ii < len; ii += 1)
		ul_SHA1Update(&ctx, (స్థిరరాశి గుర్తులేని అక్షరం *)str + ii, 1);
	ul_SHA1Final((గుర్తులేని అక్షరం *)hash_బహిర్గతం, &ctx);
	hash_బహిర్గతం[20] = '\0';
}
