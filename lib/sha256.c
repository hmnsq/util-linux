/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.
 *
 * public domain sha256 crypt implementation
 *
 * original sha crypt design: http://people.redhat.com/drepper/SHA-crypt.txt
 * in this implementation at least 32bit int is assumed,
 * key length is limited, the $5$ prefix is mandatory, '\n' and ':' is rejected
 * in the salt and rounds= setting must contain a valid iteration count,
 * on error "*" is returned.
 */
#समावेश <ctype.h>
#समावेश <stdlib.h>
#समावेश <stdio.h>
#समावेश <string.h>
#समावेश <stdint.h>

#समावेश "sha256.h"

/* public domain sha256 implementation based on fips180-3 */

काष्ठा sha256 {
	uint64_t len;    /* processed message length */
	uint32_t h[8];   /* hash state */
	uint8_t buf[64]; /* message block buffer */
};

अटल uint32_t ror(uint32_t n, पूर्णांक k) { वापस (n >> k) | (n << (32-k)); }
#घोषणा Ch(x,y,z)  (z ^ (x & (y ^ z)))
#घोषणा Maj(x,y,z) ((x & y) | (z & (x | y)))
#घोषणा S0(x)      (ror(x,2) ^ ror(x,13) ^ ror(x,22))
#घोषणा S1(x)      (ror(x,6) ^ ror(x,11) ^ ror(x,25))
#घोषणा R0(x)      (ror(x,7) ^ ror(x,18) ^ (x>>3))
#घोषणा R1(x)      (ror(x,17) ^ ror(x,19) ^ (x>>10))

अटल स्थिर uint32_t K[64] = {
0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

अटल व्योम processblock(काष्ठा sha256 *s, स्थिर uint8_t *buf)
{
	uint32_t W[64], t1, t2, a, b, c, d, e, f, g, h;
	पूर्णांक i;

	क्रम (i = 0; i < 16; i++) {
		W[i] = (uint32_t)buf[4*i]<<24;
		W[i] |= (uint32_t)buf[4*i+1]<<16;
		W[i] |= (uint32_t)buf[4*i+2]<<8;
		W[i] |= buf[4*i+3];
	}
	क्रम (; i < 64; i++)
		W[i] = R1(W[i-2]) + W[i-7] + R0(W[i-15]) + W[i-16];
	a = s->h[0];
	b = s->h[1];
	c = s->h[2];
	d = s->h[3];
	e = s->h[4];
	f = s->h[5];
	g = s->h[6];
	h = s->h[7];
	क्रम (i = 0; i < 64; i++) {
		t1 = h + S1(e) + Ch(e,f,g) + K[i] + W[i];
		t2 = S0(a) + Maj(a,b,c);
		h = g;
		g = f;
		f = e;
		e = d + t1;
		d = c;
		c = b;
		b = a;
		a = t1 + t2;
	}
	s->h[0] += a;
	s->h[1] += b;
	s->h[2] += c;
	s->h[3] += d;
	s->h[4] += e;
	s->h[5] += f;
	s->h[6] += g;
	s->h[7] += h;
}

अटल व्योम pad(काष्ठा sha256 *s)
{
	अचिन्हित r = s->len % 64;

	s->buf[r++] = 0x80;
	अगर (r > 56) {
		स_रखो(s->buf + r, 0, 64 - r);
		r = 0;
		processblock(s, s->buf);
	}
	स_रखो(s->buf + r, 0, 56 - r);
	s->len *= 8;
	s->buf[56] = s->len >> 56;
	s->buf[57] = s->len >> 48;
	s->buf[58] = s->len >> 40;
	s->buf[59] = s->len >> 32;
	s->buf[60] = s->len >> 24;
	s->buf[61] = s->len >> 16;
	s->buf[62] = s->len >> 8;
	s->buf[63] = s->len;
	processblock(s, s->buf);
}

अटल व्योम sha256_init(काष्ठा sha256 *s)
{
	s->len = 0;
	s->h[0] = 0x6a09e667;
	s->h[1] = 0xbb67ae85;
	s->h[2] = 0x3c6ef372;
	s->h[3] = 0xa54ff53a;
	s->h[4] = 0x510e527f;
	s->h[5] = 0x9b05688c;
	s->h[6] = 0x1f83d9ab;
	s->h[7] = 0x5be0cd19;
}

अटल व्योम sha256_योग(काष्ठा sha256 *s, uint8_t *md)
{
	पूर्णांक i;

	pad(s);
	क्रम (i = 0; i < 8; i++) {
		md[4*i] = s->h[i] >> 24;
		md[4*i+1] = s->h[i] >> 16;
		md[4*i+2] = s->h[i] >> 8;
		md[4*i+3] = s->h[i];
	}
}

अटल व्योम sha256_update(काष्ठा sha256 *s, स्थिर व्योम *m, अचिन्हित दीर्घ len)
{
	स्थिर uint8_t *p = m;
	अचिन्हित r = s->len % 64;

	s->len += len;
	अगर (r) {
		अगर (len < 64 - r) {
			स_नकल(s->buf + r, p, len);
			वापस;
		}
		स_नकल(s->buf + r, p, 64 - r);
		len -= 64 - r;
		p += 64 - r;
		processblock(s, s->buf);
	}
	क्रम (; len >= 64; len -= 64, p += 64)
		processblock(s, p);
	स_नकल(s->buf, p, len);
}

व्योम ul_SHA256(अचिन्हित अक्षर hash_out[UL_SHA256_लंबाई], स्थिर अचिन्हित अक्षर *str, माप_प्रकार len)
{
	काष्ठा sha256 state = {};
	sha256_init(&state);
	sha256_update(&state, str, len);
	sha256_योग(&state, hash_out);
}
