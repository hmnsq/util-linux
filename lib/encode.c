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

size_t ul_encode_to_utf8(цео enc, unsigned char *dest, size_t len,
			const unsigned char *src, size_t count)
{
	size_t i, j;
	uint32_t c;
	uint16_t c2;

	за (j = i = 0; i < count; i++) {
		ако (enc == UL_ENCODE_UTF16LE) {
			ако (i+2 > count)
				break;
			c = (src[i+1] << 8) | src[i];
			i++;
		} инако ако (enc == UL_ENCODE_UTF16BE) {
			ако (i+2 > count)
				break;
			c = (src[i] << 8) | src[i+1];
			i++;
		} инако ако (enc == UL_ENCODE_LATIN1) {
			c = src[i];
		} инако {
			врати 0;
		}
		ако ((enc == UL_ENCODE_UTF16LE || enc == UL_ENCODE_UTF16BE) &&
		    c >= 0xD800 && c <= 0xDBFF && i+2 < count) {
			ако (enc == UL_ENCODE_UTF16LE)
				c2 = (src[i+2] << 8) | src[i+1];
			инако
				c2 = (src[i+1] << 8) | src[i+2];
			ако (c2 >= 0xDC00 && c2 <= 0xDFFF) {
				c = 0x10000 + ((c - 0xD800) << 10) + (c2 - 0xDC00);
				i += 2;
			}
		}
		ако (c == 0) {
			dest[j] = '\0';
			break;
		}

		ако (c < 0x80) {
			ако (j+1 >= len)
				break;
			dest[j++] = (uint8_t) c;
		} инако ако (c < 0x800) {
			ако (j+2 >= len)
				break;
			dest[j++] = (uint8_t) (0xc0 | (c >> 6));
			dest[j++] = (uint8_t) (0x80 | (c & 0x3f));
		} инако ако (c < 0x10000) {
			ако (j+3 >= len)
				break;
			dest[j++] = (uint8_t) (0xe0 | (c >> 12));
			dest[j++] = (uint8_t) (0x80 | ((c >> 6) & 0x3f));
			dest[j++] = (uint8_t) (0x80 | (c & 0x3f));
		} инако {
			ако (j+4 >= len)
				break;
			dest[j++] = (uint8_t) (0xf0 | (c >> 18));
			dest[j++] = (uint8_t) (0x80 | ((c >> 12) & 0x3f));
			dest[j++] = (uint8_t) (0x80 | ((c >> 6) & 0x3f));
			dest[j++] = (uint8_t) (0x80 | (c & 0x3f));
		}
	}
	dest[j] = '\0';
	врати j;
}
