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

#define UL_SHA1HANDSOFF

#अन्तर्भाव <stdio.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <stdint.h>

#अन्तर्भाव "sha1.h"

#define rol(मूल्य, bits) (((मूल्य) << (bits)) | ((मूल्य) >> (32 - (bits))))

/* blk0() and blk() perform the initial expand. */
#यदि __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
# define blk0(i) block->l[i]
#अन्यथा
# define blk0(i) (block->l[i] = (rol(block->l[i],24)&0xFF00FF00) \
    |(rol(block->l[i],8)&0x00FF00FF))
#endif

#define blk(i) (block->l[i&15] = rol(block->l[(i+13)&15]^block->l[(i+8)&15] \
    ^block->l[(i+2)&15]^block->l[i&15],1))

/* (R0+R1), R2, R3, R4 are the different operations used in SHA1 */
#define R0(v,w,x,y,z,i) z+=((w&(x^y))^y)+blk0(i)+0x5A827999+rol(v,5);w=rol(w,30);
#define R1(v,w,x,y,z,i) z+=((w&(x^y))^y)+blk(i)+0x5A827999+rol(v,5);w=rol(w,30);
#define R2(v,w,x,y,z,i) z+=(w^x^y)+blk(i)+0x6ED9EBA1+rol(v,5);w=rol(w,30);
#define R3(v,w,x,y,z,i) z+=(((w|x)&y)|(w&x))+blk(i)+0x8F1BBCDC+rol(v,5);w=rol(w,30);
#define R4(v,w,x,y,z,i) z+=(w^x^y)+blk(i)+0xCA62C1D6+rol(v,5);w=rol(w,30);

/* Hash a single 512-bit block. This is the core of the algorithm. */

शून्य ul_SHA1Transform(uint32_t state[5], ध्रुव अचिन्हित वर्ण buffer[64])
{
	uint32_t a, b, c, d, e;

	प्रकारनिर्देश सङ्घ {
		अचिन्हित वर्ण c[64];
		uint32_t l[16];
	} CHAR64LONG16;

#ifdef UL_SHA1HANDSOFF
	CHAR64LONG16 block[1];	/* use array to appear as a pointer */

	स_नकल(block, buffer, 64);
#अन्यथा
	/* The following had better never be used because it causes the
	 * pointer-to-const buffer to be cast into a pointer to non-const.
	 * And the result is written through.  I threw a "const" in, hoping
	 * this will cause a diagnostic.
	 */
	CHAR64LONG16 *block = (ध्रुव CHAR64LONG16 *)buffer;
#endif
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
#ifdef HAVE_EXPLICIT_BZERO
	explicit_bzero(&a, परिमाण(a));
	explicit_bzero(&b, परिमाण(b));
	explicit_bzero(&c, परिमाण(c));
	explicit_bzero(&d, परिमाण(d));
	explicit_bzero(&e, परिमाण(e));
#अन्यथा
	a = b = c = d = e = 0;
#endif
#ifdef UL_SHA1HANDSOFF
	स_रखो(block, '\0', परिमाण(block));
#endif
}

/* SHA1Init - Initialize new context */

शून्य ul_SHA1Init(UL_SHA1_CTX *context)
{
	/* SHA1 initialization constants */
	context->state[0] = 0x67452301;
	context->state[1] = 0xEFCDAB89;
	context->state[2] = 0x98BADCFE;
	context->state[3] = 0x10325476;
	context->state[4] = 0xC3D2E1F0;
	context->गिनती[0] = context->गिनती[1] = 0;
}

/* Run your data through this. */

शून्य ul_SHA1Update(UL_SHA1_CTX *context, ध्रुव अचिन्हित वर्ण *data, uint32_t len)
{
	uint32_t i;

	uint32_t j;

	j = context->गिनती[0];
	यदि ((context->गिनती[0] += len << 3) < j)
		context->गिनती[1]++;
	context->गिनती[1] += (len >> 29);
	j = (j >> 3) & 63;
	यदि ((j + len) > 63) {
		स_नकल(&context->buffer[j], data, (i = 64 - j));
		ul_SHA1Transform(context->state, context->buffer);
		प्रति (; i + 63 < len; i += 64) {
			ul_SHA1Transform(context->state, &data[i]);
		}
		j = 0;
	} अन्यथा
		i = 0;
	स_नकल(&context->buffer[j], &data[i], len - i);
}

/* Add padding and return the message digest. */

शून्य ul_SHA1Final(अचिन्हित वर्ण digest[20], UL_SHA1_CTX *context)
{
	अचिन्हित i;

	अचिन्हित वर्ण finalcount[8];

	अचिन्हित वर्ण c;

#यदि 0				/* untested "improvement" by DHR */
	/* Convert context->count to a sequence of bytes
	 * in finalcount.  Second element first, but
	 * big-endian order within element.
	 * But we do it all backwards.
	 */
	अचिन्हित वर्ण *fcp = &finalcount[8];

	प्रति (i = 0; i < 2; i++) {
		uint32_t t = context->गिनती[i];

		पूर्णाङ्क j;

		प्रति (j = 0; j < 4; t >>= 8, j++)
			*--fcp = (अचिन्हित वर्ण)t}
#अन्यथा
	प्रति (i = 0; i < 8; i++) {
		finalcount[i] = (अचिन्हित वर्ण)((context->गिनती[(i >= 4 ? 0 : 1)] >> ((3 - (i & 3)) * 8)) & 255);	/* Endian independent */
	}
#endif
	c = 0200;
	ul_SHA1Update(context, &c, 1);
	यावत् ((context->गिनती[0] & 504) != 448) {
		c = 0000;
		ul_SHA1Update(context, &c, 1);
	}
	ul_SHA1Update(context, finalcount, 8);	/* Should cause a SHA1Transform() */
	प्रति (i = 0; i < 20; i++) {
		digest[i] = (अचिन्हित वर्ण)
		    ((context->state[i >> 2] >> ((3 - (i & 3)) * 8)) & 255);
	}
	/* Wipe variables */
	स_रखो(context, '\0', परिमाण(*context));
	स_रखो(&finalcount, '\0', परिमाण(finalcount));
}

शून्य ul_SHA1(वर्ण *hash_out, ध्रुव वर्ण *str, अचिन्हित len)
{
	UL_SHA1_CTX ctx;
	अचिन्हित पूर्णाङ्क ii;

	ul_SHA1Init(&ctx);
	प्रति (ii = 0; ii < len; ii += 1)
		ul_SHA1Update(&ctx, (ध्रुव अचिन्हित वर्ण *)str + ii, 1);
	ul_SHA1Final((अचिन्हित वर्ण *)hash_out, &ctx);
	hash_out[20] = '\0';
}
