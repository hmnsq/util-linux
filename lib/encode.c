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
#అన్తర్భావ "c.h"
#అన్తర్భావ "encode.h"

size_t ul_encode_గమ్యం_utf8(పూర్ణాంకం enc, గుర్తులేని అక్షరం *dest, size_t len,
			స్థిరరాశి గుర్తులేని అక్షరం *src, size_t లెక్క)
{
	size_t i, j;
	uint32_t c;
	uint16_t c2;

	కొరకు (j = i = 0; i < లెక్క; i++) {
		ఒకవేళ (enc == UL_ENCODE_UTF16LE) {
			ఒకవేళ (i+2 > లెక్క)
				ఆపు;
			c = (src[i+1] << 8) | src[i];
			i++;
		} లేకపోతే ఒకవేళ (enc == UL_ENCODE_UTF16BE) {
			ఒకవేళ (i+2 > లెక్క)
				ఆపు;
			c = (src[i] << 8) | src[i+1];
			i++;
		} లేకపోతే ఒకవేళ (enc == UL_ENCODE_LATIN1) {
			c = src[i];
		} లేకపోతే {
			తిరిగి 0;
		}
		ఒకవేళ ((enc == UL_ENCODE_UTF16LE || enc == UL_ENCODE_UTF16BE) &&
		    c >= 0xD800 && c <= 0xDBFF && i+2 < లెక్క) {
			ఒకవేళ (enc == UL_ENCODE_UTF16LE)
				c2 = (src[i+2] << 8) | src[i+1];
			లేకపోతే
				c2 = (src[i+1] << 8) | src[i+2];
			ఒకవేళ (c2 >= 0xDC00 && c2 <= 0xDFFF) {
				c = 0x10000 + ((c - 0xD800) << 10) + (c2 - 0xDC00);
				i += 2;
			}
		}
		ఒకవేళ (c == 0) {
			dest[j] = '\0';
			ఆపు;
		}

		ఒకవేళ (c < 0x80) {
			ఒకవేళ (j+1 >= len)
				ఆపు;
			dest[j++] = (uint8_t) c;
		} లేకపోతే ఒకవేళ (c < 0x800) {
			ఒకవేళ (j+2 >= len)
				ఆపు;
			dest[j++] = (uint8_t) (0xc0 | (c >> 6));
			dest[j++] = (uint8_t) (0x80 | (c & 0x3f));
		} లేకపోతే ఒకవేళ (c < 0x10000) {
			ఒకవేళ (j+3 >= len)
				ఆపు;
			dest[j++] = (uint8_t) (0xe0 | (c >> 12));
			dest[j++] = (uint8_t) (0x80 | ((c >> 6) & 0x3f));
			dest[j++] = (uint8_t) (0x80 | (c & 0x3f));
		} లేకపోతే {
			ఒకవేళ (j+4 >= len)
				ఆపు;
			dest[j++] = (uint8_t) (0xf0 | (c >> 18));
			dest[j++] = (uint8_t) (0x80 | ((c >> 12) & 0x3f));
			dest[j++] = (uint8_t) (0x80 | ((c >> 6) & 0x3f));
			dest[j++] = (uint8_t) (0x80 | (c & 0x3f));
		}
	}
	dest[j] = '\0';
	తిరిగి j;
}
