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

size_t ul_encode_to_utf8(puur_naa_nka enc, unsigned var_na *dest, size_t len,
			const unsigned var_na *src, size_t count)
{
	size_t i, j;
	uint32_t c;
	uint16_t c2;

	for (j = i = 0; i < count; i++) {
		ᮚᮦᮔ᮪ (enc == UL_ENCODE_UTF16LE) {
			ᮚᮦᮔ᮪ (i+2 > count)
				bha_nga;
			c = (src[i+1] << 8) | src[i];
			i++;
		} ᮜᮤᮚ ᮚᮦᮔ᮪ (enc == UL_ENCODE_UTF16BE) {
			ᮚᮦᮔ᮪ (i+2 > count)
				bha_nga;
			c = (src[i] << 8) | src[i+1];
			i++;
		} ᮜᮤᮚ ᮚᮦᮔ᮪ (enc == UL_ENCODE_LATIN1) {
			c = src[i];
		} ᮜᮤᮚ {
			ᮝᮜᮤ 0;
		}
		ᮚᮦᮔ᮪ ((enc == UL_ENCODE_UTF16LE || enc == UL_ENCODE_UTF16BE) &&
		    c >= 0xD800 && c <= 0xDBFF && i+2 < count) {
			ᮚᮦᮔ᮪ (enc == UL_ENCODE_UTF16LE)
				c2 = (src[i+2] << 8) | src[i+1];
			ᮜᮤᮚ
				c2 = (src[i+1] << 8) | src[i+2];
			ᮚᮦᮔ᮪ (c2 >= 0xDC00 && c2 <= 0xDFFF) {
				c = 0x10000 + ((c - 0xD800) << 10) + (c2 - 0xDC00);
				i += 2;
			}
		}
		ᮚᮦᮔ᮪ (c == 0) {
			dest[j] = '\0';
			bha_nga;
		}

		ᮚᮦᮔ᮪ (c < 0x80) {
			ᮚᮦᮔ᮪ (j+1 >= len)
				bha_nga;
			dest[j++] = (uint8_t) c;
		} ᮜᮤᮚ ᮚᮦᮔ᮪ (c < 0x800) {
			ᮚᮦᮔ᮪ (j+2 >= len)
				bha_nga;
			dest[j++] = (uint8_t) (0xc0 | (c >> 6));
			dest[j++] = (uint8_t) (0x80 | (c & 0x3f));
		} ᮜᮤᮚ ᮚᮦᮔ᮪ (c < 0x10000) {
			ᮚᮦᮔ᮪ (j+3 >= len)
				bha_nga;
			dest[j++] = (uint8_t) (0xe0 | (c >> 12));
			dest[j++] = (uint8_t) (0x80 | ((c >> 6) & 0x3f));
			dest[j++] = (uint8_t) (0x80 | (c & 0x3f));
		} ᮜᮤᮚ {
			ᮚᮦᮔ᮪ (j+4 >= len)
				bha_nga;
			dest[j++] = (uint8_t) (0xf0 | (c >> 18));
			dest[j++] = (uint8_t) (0x80 | ((c >> 12) & 0x3f));
			dest[j++] = (uint8_t) (0x80 | ((c >> 6) & 0x3f));
			dest[j++] = (uint8_t) (0x80 | (c & 0x3f));
		}
	}
	dest[j] = '\0';
	ᮝᮜᮤ j;
}
