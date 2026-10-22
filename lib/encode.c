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

	لكل (j = i = 0; i < count; i++) {
		ܐܢ (enc == UL_ENCODE_UTF16LE) {
			ܐܢ (i+2 > count)
				اكسر;
			c = (src[i+1] << 8) | src[i];
			i++;
		} ܐܚܪܢܐ ܐܢ (enc == UL_ENCODE_UTF16BE) {
			ܐܢ (i+2 > count)
				اكسر;
			c = (src[i] << 8) | src[i+1];
			i++;
		} ܐܚܪܢܐ ܐܢ (enc == UL_ENCODE_LATIN1) {
			c = src[i];
		} ܐܚܪܢܐ {
			ܦܢܐ 0;
		}
		ܐܢ ((enc == UL_ENCODE_UTF16LE || enc == UL_ENCODE_UTF16BE) &&
		    c >= 0xD800 && c <= 0xDBFF && i+2 < count) {
			ܐܢ (enc == UL_ENCODE_UTF16LE)
				c2 = (src[i+2] << 8) | src[i+1];
			ܐܚܪܢܐ
				c2 = (src[i+1] << 8) | src[i+2];
			ܐܢ (c2 >= 0xDC00 && c2 <= 0xDFFF) {
				c = 0x10000 + ((c - 0xD800) << 10) + (c2 - 0xDC00);
				i += 2;
			}
		}
		ܐܢ (c == 0) {
			dest[j] = '\0';
			اكسر;
		}

		ܐܢ (c < 0x80) {
			ܐܢ (j+1 >= len)
				اكسر;
			dest[j++] = (uint8_t) c;
		} ܐܚܪܢܐ ܐܢ (c < 0x800) {
			ܐܢ (j+2 >= len)
				اكسر;
			dest[j++] = (uint8_t) (0xc0 | (c >> 6));
			dest[j++] = (uint8_t) (0x80 | (c & 0x3f));
		} ܐܚܪܢܐ ܐܢ (c < 0x10000) {
			ܐܢ (j+3 >= len)
				اكسر;
			dest[j++] = (uint8_t) (0xe0 | (c >> 12));
			dest[j++] = (uint8_t) (0x80 | ((c >> 6) & 0x3f));
			dest[j++] = (uint8_t) (0x80 | (c & 0x3f));
		} ܐܚܪܢܐ {
			ܐܢ (j+4 >= len)
				اكسر;
			dest[j++] = (uint8_t) (0xf0 | (c >> 18));
			dest[j++] = (uint8_t) (0x80 | ((c >> 12) & 0x3f));
			dest[j++] = (uint8_t) (0x80 | ((c >> 6) & 0x3f));
			dest[j++] = (uint8_t) (0x80 | (c & 0x3f));
		}
	}
	dest[j] = '\0';
	ܦܢܐ j;
}
