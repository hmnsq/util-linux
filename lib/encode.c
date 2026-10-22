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
#include "c.h"
#include "encode.h"

size_t ul_encode_to_utf8(số_nguyên enc, unsigned char *dest, size_t len,
			const unsigned char *src, size_t count)
{
	size_t i, j;
	uint32_t c;
	uint16_t c2;

	cho (j = i = 0; i < count; i++) {
		nếu (enc == UL_ENCODE_UTF16LE) {
			nếu (i+2 > count)
				break;
			c = (src[i+1] << 8) | src[i];
			i++;
		} không_thì nếu (enc == UL_ENCODE_UTF16BE) {
			nếu (i+2 > count)
				break;
			c = (src[i] << 8) | src[i+1];
			i++;
		} không_thì nếu (enc == UL_ENCODE_LATIN1) {
			c = src[i];
		} không_thì {
			trả_về 0;
		}
		nếu ((enc == UL_ENCODE_UTF16LE || enc == UL_ENCODE_UTF16BE) &&
		    c >= 0xD800 && c <= 0xDBFF && i+2 < count) {
			nếu (enc == UL_ENCODE_UTF16LE)
				c2 = (src[i+2] << 8) | src[i+1];
			không_thì
				c2 = (src[i+1] << 8) | src[i+2];
			nếu (c2 >= 0xDC00 && c2 <= 0xDFFF) {
				c = 0x10000 + ((c - 0xD800) << 10) + (c2 - 0xDC00);
				i += 2;
			}
		}
		nếu (c == 0) {
			dest[j] = '\0';
			break;
		}

		nếu (c < 0x80) {
			nếu (j+1 >= len)
				break;
			dest[j++] = (uint8_t) c;
		} không_thì nếu (c < 0x800) {
			nếu (j+2 >= len)
				break;
			dest[j++] = (uint8_t) (0xc0 | (c >> 6));
			dest[j++] = (uint8_t) (0x80 | (c & 0x3f));
		} không_thì nếu (c < 0x10000) {
			nếu (j+3 >= len)
				break;
			dest[j++] = (uint8_t) (0xe0 | (c >> 12));
			dest[j++] = (uint8_t) (0x80 | ((c >> 6) & 0x3f));
			dest[j++] = (uint8_t) (0x80 | (c & 0x3f));
		} không_thì {
			nếu (j+4 >= len)
				break;
			dest[j++] = (uint8_t) (0xf0 | (c >> 18));
			dest[j++] = (uint8_t) (0x80 | ((c >> 12) & 0x3f));
			dest[j++] = (uint8_t) (0x80 | ((c >> 6) & 0x3f));
			dest[j++] = (uint8_t) (0x80 | (c & 0x3f));
		}
	}
	dest[j] = '\0';
	trả_về j;
}
