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
#شامل <string.h>		/* for memcpy() */

#شامل "md5.h"

#اگر __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
# define byte_الٹ(buf, len)	/* Nothing */
#ورنہ
ساکن خالی byte_الٹ(غیرعلامتی حرف *buf, غیرعلامتی longs);

#اگر_غیر_تعریف_شدہ ASM_MD5
/*
 * Note: this code is harmless on little-endian machines.
 */
ساکن خالی byte_الٹ(غیرعلامتی حرف *buf, غیرعلامتی longs)
{
    uint32_t t;
    کرو {
	t = (uint32_t) ((غیرعلامتی) buf[3] << 8 | buf[2]) << 16 |
	    ((غیرعلامتی) buf[1] << 8 | buf[0]);
	*(uint32_t *) buf = t;
	buf += 4;
    } جب (--longs);
}
#اگر_ختم /* !ASM_MD5 */
#اگر_ختم /* __ORDER_LITTLE_ENDIAN__ */

/*
 * Start MD5 accumulation.  Set bit count to 0 and buffer to mysterious
 * initialization constants.
 */
خالی ul_MD5Init(ساخت UL_MD5Context *ctx)
{
    ctx->buf[0] = 0x67452301;
    ctx->buf[1] = 0xefcdab89;
    ctx->buf[2] = 0x98badcfe;
    ctx->buf[3] = 0x10325476;

    ctx->بٹ[0] = 0;
    ctx->بٹ[1] = 0;
}

/*
 * Update context to reflect the concatenation of another buffer full
 * of bytes.
 */
خالی ul_MD5Update(ساخت UL_MD5Context *ctx, غیرعلامتی حرف ثابت *buf, غیرعلامتی len)
{
    uint32_t t;

    /* Update bitcount */

    t = ctx->بٹ[0];
    اگر ((ctx->بٹ[0] = t + ((uint32_t) len << 3)) < t)
	ctx->بٹ[1]++;		/* Carry from low to high */
    ctx->بٹ[1] += len >> 29;

    t = (t >> 3) & 0x3f;	/* Bytes already in shsInfo->data */

    /* Handle any leading odd-sized chunks */

    اگر (t) {
	غیرعلامتی حرف *p = (غیرعلامتی حرف *) ctx->اندر + t;

	t = 64 - t;
	اگر (len < t) {
	    حافظہ_نقل(p, buf, len);
	    واپس;
	}
	حافظہ_نقل(p, buf, t);
	byte_الٹ(ctx->اندر, 16);
	ul_MD5Transform(ctx->buf, (uint32_t *) ctx->اندر);
	buf += t;
	len -= t;
    }
    /* Process data in 64-byte chunks */

    جب (len >= 64) {
	حافظہ_نقل(ctx->اندر, buf, 64);
	byte_الٹ(ctx->اندر, 16);
	ul_MD5Transform(ctx->buf, (uint32_t *) ctx->اندر);
	buf += 64;
	len -= 64;
    }

    /* Handle any remaining bytes of data. */

    حافظہ_نقل(ctx->اندر, buf, len);
}

/*
 * Final wrapup - pad to 64-byte boundary with the bit pattern
 * 1 0* (64-bit count of bits processed, MSB-first)
 */
خالی ul_MD5Final(غیرعلامتی حرف digest[UL_MD5_لمبائی], ساخت UL_MD5Context *ctx)
{
    غیرعلامتی گنتی;
    غیرعلامتی حرف *p;

    /* Compute number of bytes mod 64 */
    گنتی = (ctx->بٹ[0] >> 3) & 0x3F;

    /* Set the first char of padding to 0x80.  This is safe since there is
       always at least one byte free */
    p = ctx->اندر + گنتی;
    *p++ = 0x80;

    /* Bytes of padding needed to make 64 bytes */
    گنتی = 64 - 1 - گنتی;

    /* Pad out to 56 mod 64 */
    اگر (گنتی < 8) {
	/* Two lots of padding:  Pad the first block to 64 bytes */
	memset(p, 0, گنتی);
	byte_الٹ(ctx->اندر, 16);
	ul_MD5Transform(ctx->buf, (uint32_t *) ctx->اندر);

	/* Now fill the next block with 56 bytes */
	memset(ctx->اندر, 0, 56);
    } ورنہ {
	/* Pad block to 56 bytes */
	memset(p, 0, گنتی - 8);
    }
    byte_الٹ(ctx->اندر, 14);

    /* Append length in bits and transform.
     * Use memcpy to avoid aliasing problems.  On most systems,
     * this will be optimized away to the same code.
     */
    حافظہ_نقل(&ctx->اندر[14 * حجم(uint32_t)], &ctx->بٹ[0], 4);
    حافظہ_نقل(&ctx->اندر[15 * حجم(uint32_t)], &ctx->بٹ[1], 4);

    ul_MD5Transform(ctx->buf, (uint32_t *) ctx->اندر);
    byte_الٹ((غیرعلامتی حرف *) ctx->buf, 4);
    حافظہ_نقل(digest, ctx->buf, UL_MD5_لمبائی);
    memset(ctx, 0, حجم(*ctx));	/* In case it's sensitive */
}

#اگر_غیر_تعریف_شدہ ASM_MD5

/* The four core functions - F1 is optimized somewhat */

/* #define F1(x, y, z) (x & y | ~x & z) */
#تعریف_کرو F1(x, y, z) (z ^ (x & (y ^ z)))
#تعریف_کرو F2(x, y, z) F1(z, x, y)
#تعریف_کرو F3(x, y, z) (x ^ y ^ z)
#تعریف_کرو F4(x, y, z) (y ^ (x | ~z))

/* This is the central step in the MD5 algorithm. */
#تعریف_کرو MD5_قدم(f, w, x, y, z, ڈیٹا, s) \
	( w += f(x, y, z) + ڈیٹا,  w = w<<s | w>>(32-s),  w += x )

/*
 * The core of the MD5 algorithm, this alters an existing MD5 hash to
 * reflect the addition of 16 longwords of new data.  MD5Update blocks
 * the data and converts bytes into longwords for this routine.
 */
خالی ul_MD5Transform(uint32_t buf[4], uint32_t ثابت اندر[16])
{
    اندراج uint32_t a, b, c, d;

    a = buf[0];
    b = buf[1];
    c = buf[2];
    d = buf[3];

    MD5_قدم(F1, a, b, c, d, اندر[0] + 0xd76aa478, 7);
    MD5_قدم(F1, d, a, b, c, اندر[1] + 0xe8c7b756, 12);
    MD5_قدم(F1, c, d, a, b, اندر[2] + 0x242070db, 17);
    MD5_قدم(F1, b, c, d, a, اندر[3] + 0xc1bdceee, 22);
    MD5_قدم(F1, a, b, c, d, اندر[4] + 0xf57c0faf, 7);
    MD5_قدم(F1, d, a, b, c, اندر[5] + 0x4787c62a, 12);
    MD5_قدم(F1, c, d, a, b, اندر[6] + 0xa8304613, 17);
    MD5_قدم(F1, b, c, d, a, اندر[7] + 0xfd469501, 22);
    MD5_قدم(F1, a, b, c, d, اندر[8] + 0x698098d8, 7);
    MD5_قدم(F1, d, a, b, c, اندر[9] + 0x8b44f7af, 12);
    MD5_قدم(F1, c, d, a, b, اندر[10] + 0xffff5bb1, 17);
    MD5_قدم(F1, b, c, d, a, اندر[11] + 0x895cd7be, 22);
    MD5_قدم(F1, a, b, c, d, اندر[12] + 0x6b901122, 7);
    MD5_قدم(F1, d, a, b, c, اندر[13] + 0xfd987193, 12);
    MD5_قدم(F1, c, d, a, b, اندر[14] + 0xa679438e, 17);
    MD5_قدم(F1, b, c, d, a, اندر[15] + 0x49b40821, 22);

    MD5_قدم(F2, a, b, c, d, اندر[1] + 0xf61e2562, 5);
    MD5_قدم(F2, d, a, b, c, اندر[6] + 0xc040b340, 9);
    MD5_قدم(F2, c, d, a, b, اندر[11] + 0x265e5a51, 14);
    MD5_قدم(F2, b, c, d, a, اندر[0] + 0xe9b6c7aa, 20);
    MD5_قدم(F2, a, b, c, d, اندر[5] + 0xd62f105d, 5);
    MD5_قدم(F2, d, a, b, c, اندر[10] + 0x02441453, 9);
    MD5_قدم(F2, c, d, a, b, اندر[15] + 0xd8a1e681, 14);
    MD5_قدم(F2, b, c, d, a, اندر[4] + 0xe7d3fbc8, 20);
    MD5_قدم(F2, a, b, c, d, اندر[9] + 0x21e1cde6, 5);
    MD5_قدم(F2, d, a, b, c, اندر[14] + 0xc33707d6, 9);
    MD5_قدم(F2, c, d, a, b, اندر[3] + 0xf4d50d87, 14);
    MD5_قدم(F2, b, c, d, a, اندر[8] + 0x455a14ed, 20);
    MD5_قدم(F2, a, b, c, d, اندر[13] + 0xa9e3e905, 5);
    MD5_قدم(F2, d, a, b, c, اندر[2] + 0xfcefa3f8, 9);
    MD5_قدم(F2, c, d, a, b, اندر[7] + 0x676f02d9, 14);
    MD5_قدم(F2, b, c, d, a, اندر[12] + 0x8d2a4c8a, 20);

    MD5_قدم(F3, a, b, c, d, اندر[5] + 0xfffa3942, 4);
    MD5_قدم(F3, d, a, b, c, اندر[8] + 0x8771f681, 11);
    MD5_قدم(F3, c, d, a, b, اندر[11] + 0x6d9d6122, 16);
    MD5_قدم(F3, b, c, d, a, اندر[14] + 0xfde5380c, 23);
    MD5_قدم(F3, a, b, c, d, اندر[1] + 0xa4beea44, 4);
    MD5_قدم(F3, d, a, b, c, اندر[4] + 0x4bdecfa9, 11);
    MD5_قدم(F3, c, d, a, b, اندر[7] + 0xf6bb4b60, 16);
    MD5_قدم(F3, b, c, d, a, اندر[10] + 0xbebfbc70, 23);
    MD5_قدم(F3, a, b, c, d, اندر[13] + 0x289b7ec6, 4);
    MD5_قدم(F3, d, a, b, c, اندر[0] + 0xeaa127fa, 11);
    MD5_قدم(F3, c, d, a, b, اندر[3] + 0xd4ef3085, 16);
    MD5_قدم(F3, b, c, d, a, اندر[6] + 0x04881d05, 23);
    MD5_قدم(F3, a, b, c, d, اندر[9] + 0xd9d4d039, 4);
    MD5_قدم(F3, d, a, b, c, اندر[12] + 0xe6db99e5, 11);
    MD5_قدم(F3, c, d, a, b, اندر[15] + 0x1fa27cf8, 16);
    MD5_قدم(F3, b, c, d, a, اندر[2] + 0xc4ac5665, 23);

    MD5_قدم(F4, a, b, c, d, اندر[0] + 0xf4292244, 6);
    MD5_قدم(F4, d, a, b, c, اندر[7] + 0x432aff97, 10);
    MD5_قدم(F4, c, d, a, b, اندر[14] + 0xab9423a7, 15);
    MD5_قدم(F4, b, c, d, a, اندر[5] + 0xfc93a039, 21);
    MD5_قدم(F4, a, b, c, d, اندر[12] + 0x655b59c3, 6);
    MD5_قدم(F4, d, a, b, c, اندر[3] + 0x8f0ccc92, 10);
    MD5_قدم(F4, c, d, a, b, اندر[10] + 0xffeff47d, 15);
    MD5_قدم(F4, b, c, d, a, اندر[1] + 0x85845dd1, 21);
    MD5_قدم(F4, a, b, c, d, اندر[8] + 0x6fa87e4f, 6);
    MD5_قدم(F4, d, a, b, c, اندر[15] + 0xfe2ce6e0, 10);
    MD5_قدم(F4, c, d, a, b, اندر[6] + 0xa3014314, 15);
    MD5_قدم(F4, b, c, d, a, اندر[13] + 0x4e0811a1, 21);
    MD5_قدم(F4, a, b, c, d, اندر[4] + 0xf7537e82, 6);
    MD5_قدم(F4, d, a, b, c, اندر[11] + 0xbd3af235, 10);
    MD5_قدم(F4, c, d, a, b, اندر[2] + 0x2ad7d2bb, 15);
    MD5_قدم(F4, b, c, d, a, اندر[9] + 0xeb86d391, 21);

    buf[0] += a;
    buf[1] += b;
    buf[2] += c;
    buf[3] += d;
}

#اگر_ختم

