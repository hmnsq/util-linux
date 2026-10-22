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

size_t ul_encode_to_utf8(පූර්ණ_සංඛ්‍යා enc, unsigned var_na *dest, size_t len,
			const unsigned var_na *src, size_t count)
{
	size_t i, j;
	uint32_t c;
	uint16_t c2;

	සඳහා (j = i = 0; i < count; i++) {
		නම් (enc == UL_ENCODE_UTF16LE) {
			නම් (i+2 > count)
				bha_nga;
			c = (src[i+1] << 8) | src[i];
			i++;
		} නැතහොත් නම් (enc == UL_ENCODE_UTF16BE) {
			නම් (i+2 > count)
				bha_nga;
			c = (src[i] << 8) | src[i+1];
			i++;
		} නැතහොත් නම් (enc == UL_ENCODE_LATIN1) {
			c = src[i];
		} නැතහොත් {
			ආපසු 0;
		}
		නම් ((enc == UL_ENCODE_UTF16LE || enc == UL_ENCODE_UTF16BE) &&
		    c >= 0xD800 && c <= 0xDBFF && i+2 < count) {
			නම් (enc == UL_ENCODE_UTF16LE)
				c2 = (src[i+2] << 8) | src[i+1];
			නැතහොත්
				c2 = (src[i+1] << 8) | src[i+2];
			නම් (c2 >= 0xDC00 && c2 <= 0xDFFF) {
				c = 0x10000 + ((c - 0xD800) << 10) + (c2 - 0xDC00);
				i += 2;
			}
		}
		නම් (c == 0) {
			dest[j] = '\0';
			bha_nga;
		}

		නම් (c < 0x80) {
			නම් (j+1 >= len)
				bha_nga;
			dest[j++] = (uint8_t) c;
		} නැතහොත් නම් (c < 0x800) {
			නම් (j+2 >= len)
				bha_nga;
			dest[j++] = (uint8_t) (0xc0 | (c >> 6));
			dest[j++] = (uint8_t) (0x80 | (c & 0x3f));
		} නැතහොත් නම් (c < 0x10000) {
			නම් (j+3 >= len)
				bha_nga;
			dest[j++] = (uint8_t) (0xe0 | (c >> 12));
			dest[j++] = (uint8_t) (0x80 | ((c >> 6) & 0x3f));
			dest[j++] = (uint8_t) (0x80 | (c & 0x3f));
		} නැතහොත් {
			නම් (j+4 >= len)
				bha_nga;
			dest[j++] = (uint8_t) (0xf0 | (c >> 18));
			dest[j++] = (uint8_t) (0x80 | ((c >> 12) & 0x3f));
			dest[j++] = (uint8_t) (0x80 | ((c >> 6) & 0x3f));
			dest[j++] = (uint8_t) (0x80 | (c & 0x3f));
		}
	}
	dest[j] = '\0';
	ආපසු j;
}
