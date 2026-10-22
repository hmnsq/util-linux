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
#ضمن "c.h"
#ضمن "encode.h"

size_t ul_encode_to_utf8(صحيح enc, unsigned حرف *dest, size_t len,
			ثابت unsigned حرف *src, size_t count)
{
	size_t i, j;
	uint32_t c;
	uint16_t c2;

	עבור (j = i = 0; i < count; i++) {
		אם (enc == UL_ENCODE_UTF16LE) {
			אם (i+2 > count)
				اكسر;
			c = (src[i+1] << 8) | src[i];
			i++;
		} אחרת אם (enc == UL_ENCODE_UTF16BE) {
			אם (i+2 > count)
				اكسر;
			c = (src[i] << 8) | src[i+1];
			i++;
		} אחרת אם (enc == UL_ENCODE_LATIN1) {
			c = src[i];
		} אחרת {
			החזר 0;
		}
		אם ((enc == UL_ENCODE_UTF16LE || enc == UL_ENCODE_UTF16BE) &&
		    c >= 0xD800 && c <= 0xDBFF && i+2 < count) {
			אם (enc == UL_ENCODE_UTF16LE)
				c2 = (src[i+2] << 8) | src[i+1];
			אחרת
				c2 = (src[i+1] << 8) | src[i+2];
			אם (c2 >= 0xDC00 && c2 <= 0xDFFF) {
				c = 0x10000 + ((c - 0xD800) << 10) + (c2 - 0xDC00);
				i += 2;
			}
		}
		אם (c == 0) {
			dest[j] = '\0';
			اكسر;
		}

		אם (c < 0x80) {
			אם (j+1 >= len)
				اكسر;
			dest[j++] = (uint8_t) c;
		} אחרת אם (c < 0x800) {
			אם (j+2 >= len)
				اكسر;
			dest[j++] = (uint8_t) (0xc0 | (c >> 6));
			dest[j++] = (uint8_t) (0x80 | (c & 0x3f));
		} אחרת אם (c < 0x10000) {
			אם (j+3 >= len)
				اكسر;
			dest[j++] = (uint8_t) (0xe0 | (c >> 12));
			dest[j++] = (uint8_t) (0x80 | ((c >> 6) & 0x3f));
			dest[j++] = (uint8_t) (0x80 | (c & 0x3f));
		} אחרת {
			אם (j+4 >= len)
				اكسر;
			dest[j++] = (uint8_t) (0xf0 | (c >> 18));
			dest[j++] = (uint8_t) (0x80 | ((c >> 12) & 0x3f));
			dest[j++] = (uint8_t) (0x80 | ((c >> 6) & 0x3f));
			dest[j++] = (uint8_t) (0x80 | (c & 0x3f));
		}
	}
	dest[j] = '\0';
	החזר j;
}
