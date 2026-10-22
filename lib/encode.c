/*
 * Based on code from libblkid,
 *
 * Copyright (C) 2008 Kay Sievers <kay.sievers@vrfy.org>
 * Copyright (C) 2009 Karel Zak <kzak@redhat.com>
 * Copyright (C) 2020 Pali Rohár <pali.rohar@gmail.com>
 *
 * This file may be redistributed under the terms of the
 * GNU Lesser General Public License.
 */
#அந்தர்பாவ "c.h"
#அந்தர்பாவ "encode.h"

size_t ul_encode_to_utf8(முழு enc, unsigned எழுத்து *dest, size_t len,
			மாறிலி unsigned எழுத்து *src, size_t மொத்தம்)
{
	size_t i, j;
	uint32_t c;
	uint16_t c2;

	க்கு (j = i = 0; i < மொத்தம்; i++) {
		எனில் (enc == UL_ENCODE_UTF16LE) {
			எனில் (i+2 > மொத்தம்)
				நிறுத்து;
			c = (src[i+1] << 8) | src[i];
			i++;
		} இல்லையெனில் எனில் (enc == UL_ENCODE_UTF16BE) {
			எனில் (i+2 > மொத்தம்)
				நிறுத்து;
			c = (src[i] << 8) | src[i+1];
			i++;
		} இல்லையெனில் எனில் (enc == UL_ENCODE_LATIN1) {
			c = src[i];
		} இல்லையெனில் {
			திரும்பு 0;
		}
		எனில் ((enc == UL_ENCODE_UTF16LE || enc == UL_ENCODE_UTF16BE) &&
		    c >= 0xD800 && c <= 0xDBFF && i+2 < மொத்தம்) {
			எனில் (enc == UL_ENCODE_UTF16LE)
				c2 = (src[i+2] << 8) | src[i+1];
			இல்லையெனில்
				c2 = (src[i+1] << 8) | src[i+2];
			எனில் (c2 >= 0xDC00 && c2 <= 0xDFFF) {
				c = 0x10000 + ((c - 0xD800) << 10) + (c2 - 0xDC00);
				i += 2;
			}
		}
		எனில் (c == 0) {
			dest[j] = '\0';
			நிறுத்து;
		}

		எனில் (c < 0x80) {
			எனில் (j+1 >= len)
				நிறுத்து;
			dest[j++] = (uint8_t) c;
		} இல்லையெனில் எனில் (c < 0x800) {
			எனில் (j+2 >= len)
				நிறுத்து;
			dest[j++] = (uint8_t) (0xc0 | (c >> 6));
			dest[j++] = (uint8_t) (0x80 | (c & 0x3f));
		} இல்லையெனில் எனில் (c < 0x10000) {
			எனில் (j+3 >= len)
				நிறுத்து;
			dest[j++] = (uint8_t) (0xe0 | (c >> 12));
			dest[j++] = (uint8_t) (0x80 | ((c >> 6) & 0x3f));
			dest[j++] = (uint8_t) (0x80 | (c & 0x3f));
		} இல்லையெனில் {
			எனில் (j+4 >= len)
				நிறுத்து;
			dest[j++] = (uint8_t) (0xf0 | (c >> 18));
			dest[j++] = (uint8_t) (0x80 | ((c >> 12) & 0x3f));
			dest[j++] = (uint8_t) (0x80 | ((c >> 6) & 0x3f));
			dest[j++] = (uint8_t) (0x80 | (c & 0x3f));
		}
	}
	dest[j] = '\0';
	திரும்பு j;
}
