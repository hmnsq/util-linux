/*
 * This code implements the MD5 message-digest algorithm.
 * The algorithm is due to Ron Rivest.  This code was
 * written by Colin Plumb in 1993, no copyright is claimed.
 * This code is in the public domain; do with it what you wish.
 *
 * Equivalent code is available from RSA Data Security, Inc.
 * This code has been tested against that, and is equivalent,
 * except that you don't need to include two pages of legalese
 * with every copy.
 *
 * To compute the message digest of a chunk of bytes, declare an
 * MD5Context structure, pass it to MD5Init, call MD5Update as
 * needed on buffers full of bytes, and then call MD5Final, which
 * will fill a supplied 16-byte array with the digest.
 */
#ضمن <string.h>		/* for memcpy() */

#ضمن "md5.h"

#إذا __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
# define byte_سرچپه(buf, len)	/* Nothing */
#وإلا
ساكن فراغ byte_سرچپه(بینښه حرف *buf, بینښه longs);

#که_نه_تعریف_شوی ASM_MD5
/*
 * Note: this code is harmless on little-endian machines.
 */
ساكن فراغ byte_سرچپه(بینښه حرف *buf, بینښه longs)
{
    uint32_t t;
    افعل {
	t = (uint32_t) ((بینښه) buf[3] << 8 | buf[2]) << 16 |
	    ((بینښه) buf[1] << 8 | buf[0]);
	*(uint32_t *) buf = t;
	buf += 4;
    } طالما (--longs);
}
#د_که_پای /* !ASM_MD5 */
#د_که_پای /* __ORDER_LITTLE_ENDIAN__ */

/*
 * Start MD5 accumulation.  Set bit count to 0 and buffer to mysterious
 * initialization constants.
 */
فراغ ul_MD5Init(بنية UL_MD5Context *ctx)
{
    ctx->buf[0] = 0x67452301;
    ctx->buf[1] = 0xefcdab89;
    ctx->buf[2] = 0x98badcfe;
    ctx->buf[3] = 0x10325476;

    ctx->بټونه[0] = 0;
    ctx->بټونه[1] = 0;
}

/*
 * Update context to reflect the concatenation of another buffer full
 * of bytes.
 */
فراغ ul_MD5Update(بنية UL_MD5Context *ctx, بینښه حرف ثابت *buf, بینښه len)
{
    uint32_t t;

    /* Update bitcount */

    t = ctx->بټونه[0];
    إذا ((ctx->بټونه[0] = t + ((uint32_t) len << 3)) < t)
	ctx->بټونه[1]++;		/* Carry from low to high */
    ctx->بټونه[1] += len >> 29;

    t = (t >> 3) & 0x3f;	/* Bytes already in shsInfo->data */

    /* Handle any leading odd-sized chunks */

    إذا (t) {
	بینښه حرف *p = (بینښه حرف *) ctx->دننه + t;

	t = 64 - t;
	إذا (len < t) {
	    حافظه_نقل(p, buf, len);
	    أرجع;
	}
	حافظه_نقل(p, buf, t);
	byte_سرچپه(ctx->دننه, 16);
	ul_MD5Transform(ctx->buf, (uint32_t *) ctx->دننه);
	buf += t;
	len -= t;
    }
    /* Process data in 64-byte chunks */

    طالما (len >= 64) {
	حافظه_نقل(ctx->دننه, buf, 64);
	byte_سرچپه(ctx->دننه, 16);
	ul_MD5Transform(ctx->buf, (uint32_t *) ctx->دننه);
	buf += 64;
	len -= 64;
    }

    /* Handle any remaining bytes of data. */

    حافظه_نقل(ctx->دننه, buf, len);
}

/*
 * Final wrapup - pad to 64-byte boundary with the bit pattern
 * 1 0* (64-bit count of bits processed, MSB-first)
 */
فراغ ul_MD5Final(بینښه حرف digest[UL_MD5_اوږدتیا], بنية UL_MD5Context *ctx)
{
    بینښه شمېر;
    بینښه حرف *p;

    /* Compute number of bytes mod 64 */
    شمېر = (ctx->بټونه[0] >> 3) & 0x3F;

    /* Set the first char of padding to 0x80.  This is safe since there is
       always at least one byte free */
    p = ctx->دننه + شمېر;
    *p++ = 0x80;

    /* Bytes of padding needed to make 64 bytes */
    شمېر = 64 - 1 - شمېر;

    /* Pad out to 56 mod 64 */
    إذا (شمېر < 8) {
	/* Two lots of padding:  Pad the first block to 64 bytes */
	memset(p, 0, شمېر);
	byte_سرچپه(ctx->دننه, 16);
	ul_MD5Transform(ctx->buf, (uint32_t *) ctx->دننه);

	/* Now fill the next block with 56 bytes */
	memset(ctx->دننه, 0, 56);
    } وإلا {
	/* Pad block to 56 bytes */
	memset(p, 0, شمېر - 8);
    }
    byte_سرچپه(ctx->دننه, 14);

    /* Append length in bits and transform.
     * Use memcpy to avoid aliasing problems.  On most systems,
     * this will be optimized away to the same code.
     */
    حافظه_نقل(&ctx->دننه[14 * حجم(uint32_t)], &ctx->بټونه[0], 4);
    حافظه_نقل(&ctx->دننه[15 * حجم(uint32_t)], &ctx->بټونه[1], 4);

    ul_MD5Transform(ctx->buf, (uint32_t *) ctx->دننه);
    byte_سرچپه((بینښه حرف *) ctx->buf, 4);
    حافظه_نقل(digest, ctx->buf, UL_MD5_اوږدتیا);
    memset(ctx, 0, حجم(*ctx));	/* In case it's sensitive */
}

#که_نه_تعریف_شوی ASM_MD5

/* The four core functions - F1 is optimized somewhat */

/* #define F1(x, y, z) (x & y | ~x & z) */
#تعریف_کړه F1(x, y, z) (z ^ (x & (y ^ z)))
#تعریف_کړه F2(x, y, z) F1(z, x, y)
#تعریف_کړه F3(x, y, z) (x ^ y ^ z)
#تعریف_کړه F4(x, y, z) (y ^ (x | ~z))

/* This is the central step in the MD5 algorithm. */
#تعریف_کړه MD5_ګام(f, w, x, y, z, معلومات, s) \
	( w += f(x, y, z) + معلومات,  w = w<<s | w>>(32-s),  w += x )

/*
 * The core of the MD5 algorithm, this alters an existing MD5 hash to
 * reflect the addition of 16 longwords of new data.  MD5Update blocks
 * the data and converts bytes into longwords for this routine.
 */
فراغ ul_MD5Transform(uint32_t buf[4], uint32_t ثابت دننه[16])
{
    سجل uint32_t a, b, c, d;

    a = buf[0];
    b = buf[1];
    c = buf[2];
    d = buf[3];

    MD5_ګام(F1, a, b, c, d, دننه[0] + 0xd76aa478, 7);
    MD5_ګام(F1, d, a, b, c, دننه[1] + 0xe8c7b756, 12);
    MD5_ګام(F1, c, d, a, b, دننه[2] + 0x242070db, 17);
    MD5_ګام(F1, b, c, d, a, دننه[3] + 0xc1bdceee, 22);
    MD5_ګام(F1, a, b, c, d, دننه[4] + 0xf57c0faf, 7);
    MD5_ګام(F1, d, a, b, c, دننه[5] + 0x4787c62a, 12);
    MD5_ګام(F1, c, d, a, b, دننه[6] + 0xa8304613, 17);
    MD5_ګام(F1, b, c, d, a, دننه[7] + 0xfd469501, 22);
    MD5_ګام(F1, a, b, c, d, دننه[8] + 0x698098d8, 7);
    MD5_ګام(F1, d, a, b, c, دننه[9] + 0x8b44f7af, 12);
    MD5_ګام(F1, c, d, a, b, دننه[10] + 0xffff5bb1, 17);
    MD5_ګام(F1, b, c, d, a, دننه[11] + 0x895cd7be, 22);
    MD5_ګام(F1, a, b, c, d, دننه[12] + 0x6b901122, 7);
    MD5_ګام(F1, d, a, b, c, دننه[13] + 0xfd987193, 12);
    MD5_ګام(F1, c, d, a, b, دننه[14] + 0xa679438e, 17);
    MD5_ګام(F1, b, c, d, a, دننه[15] + 0x49b40821, 22);

    MD5_ګام(F2, a, b, c, d, دننه[1] + 0xf61e2562, 5);
    MD5_ګام(F2, d, a, b, c, دننه[6] + 0xc040b340, 9);
    MD5_ګام(F2, c, d, a, b, دننه[11] + 0x265e5a51, 14);
    MD5_ګام(F2, b, c, d, a, دننه[0] + 0xe9b6c7aa, 20);
    MD5_ګام(F2, a, b, c, d, دننه[5] + 0xd62f105d, 5);
    MD5_ګام(F2, d, a, b, c, دننه[10] + 0x02441453, 9);
    MD5_ګام(F2, c, d, a, b, دننه[15] + 0xd8a1e681, 14);
    MD5_ګام(F2, b, c, d, a, دننه[4] + 0xe7d3fbc8, 20);
    MD5_ګام(F2, a, b, c, d, دننه[9] + 0x21e1cde6, 5);
    MD5_ګام(F2, d, a, b, c, دننه[14] + 0xc33707d6, 9);
    MD5_ګام(F2, c, d, a, b, دننه[3] + 0xf4d50d87, 14);
    MD5_ګام(F2, b, c, d, a, دننه[8] + 0x455a14ed, 20);
    MD5_ګام(F2, a, b, c, d, دننه[13] + 0xa9e3e905, 5);
    MD5_ګام(F2, d, a, b, c, دننه[2] + 0xfcefa3f8, 9);
    MD5_ګام(F2, c, d, a, b, دننه[7] + 0x676f02d9, 14);
    MD5_ګام(F2, b, c, d, a, دننه[12] + 0x8d2a4c8a, 20);

    MD5_ګام(F3, a, b, c, d, دننه[5] + 0xfffa3942, 4);
    MD5_ګام(F3, d, a, b, c, دننه[8] + 0x8771f681, 11);
    MD5_ګام(F3, c, d, a, b, دننه[11] + 0x6d9d6122, 16);
    MD5_ګام(F3, b, c, d, a, دننه[14] + 0xfde5380c, 23);
    MD5_ګام(F3, a, b, c, d, دننه[1] + 0xa4beea44, 4);
    MD5_ګام(F3, d, a, b, c, دننه[4] + 0x4bdecfa9, 11);
    MD5_ګام(F3, c, d, a, b, دننه[7] + 0xf6bb4b60, 16);
    MD5_ګام(F3, b, c, d, a, دننه[10] + 0xbebfbc70, 23);
    MD5_ګام(F3, a, b, c, d, دننه[13] + 0x289b7ec6, 4);
    MD5_ګام(F3, d, a, b, c, دننه[0] + 0xeaa127fa, 11);
    MD5_ګام(F3, c, d, a, b, دننه[3] + 0xd4ef3085, 16);
    MD5_ګام(F3, b, c, d, a, دننه[6] + 0x04881d05, 23);
    MD5_ګام(F3, a, b, c, d, دننه[9] + 0xd9d4d039, 4);
    MD5_ګام(F3, d, a, b, c, دننه[12] + 0xe6db99e5, 11);
    MD5_ګام(F3, c, d, a, b, دننه[15] + 0x1fa27cf8, 16);
    MD5_ګام(F3, b, c, d, a, دننه[2] + 0xc4ac5665, 23);

    MD5_ګام(F4, a, b, c, d, دننه[0] + 0xf4292244, 6);
    MD5_ګام(F4, d, a, b, c, دننه[7] + 0x432aff97, 10);
    MD5_ګام(F4, c, d, a, b, دننه[14] + 0xab9423a7, 15);
    MD5_ګام(F4, b, c, d, a, دننه[5] + 0xfc93a039, 21);
    MD5_ګام(F4, a, b, c, d, دننه[12] + 0x655b59c3, 6);
    MD5_ګام(F4, d, a, b, c, دننه[3] + 0x8f0ccc92, 10);
    MD5_ګام(F4, c, d, a, b, دننه[10] + 0xffeff47d, 15);
    MD5_ګام(F4, b, c, d, a, دننه[1] + 0x85845dd1, 21);
    MD5_ګام(F4, a, b, c, d, دننه[8] + 0x6fa87e4f, 6);
    MD5_ګام(F4, d, a, b, c, دننه[15] + 0xfe2ce6e0, 10);
    MD5_ګام(F4, c, d, a, b, دننه[6] + 0xa3014314, 15);
    MD5_ګام(F4, b, c, d, a, دننه[13] + 0x4e0811a1, 21);
    MD5_ګام(F4, a, b, c, d, دننه[4] + 0xf7537e82, 6);
    MD5_ګام(F4, d, a, b, c, دننه[11] + 0xbd3af235, 10);
    MD5_ګام(F4, c, d, a, b, دننه[2] + 0x2ad7d2bb, 15);
    MD5_ګام(F4, b, c, d, a, دننه[9] + 0xeb86d391, 21);

    buf[0] += a;
    buf[1] += b;
    buf[2] += c;
    buf[3] += d;
}

#د_که_پای

