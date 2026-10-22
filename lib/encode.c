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
#شامل "c.h"
#شامل "encode.h"

size_t ul_encode_مقصد_utf8(صحیح enc, غیرعلامتی حرف *dest, size_t len,
			ثابت غیرعلامتی حرف *src, size_t گنتی)
{
	size_t i, j;
	uint32_t c;
	uint16_t c2;

	برائے (j = i = 0; i < گنتی; i++) {
		اگر (enc == UL_ENCODE_UTF16LE) {
			اگر (i+2 > گنتی)
				توڑو;
			c = (src[i+1] << 8) | src[i];
			i++;
		} ورنہ اگر (enc == UL_ENCODE_UTF16BE) {
			اگر (i+2 > گنتی)
				توڑو;
			c = (src[i] << 8) | src[i+1];
			i++;
		} ورنہ اگر (enc == UL_ENCODE_LATIN1) {
			c = src[i];
		} ورنہ {
			واپس 0;
		}
		اگر ((enc == UL_ENCODE_UTF16LE || enc == UL_ENCODE_UTF16BE) &&
		    c >= 0xD800 && c <= 0xDBFF && i+2 < گنتی) {
			اگر (enc == UL_ENCODE_UTF16LE)
				c2 = (src[i+2] << 8) | src[i+1];
			ورنہ
				c2 = (src[i+1] << 8) | src[i+2];
			اگر (c2 >= 0xDC00 && c2 <= 0xDFFF) {
				c = 0x10000 + ((c - 0xD800) << 10) + (c2 - 0xDC00);
				i += 2;
			}
		}
		اگر (c == 0) {
			dest[j] = '\0';
			توڑو;
		}

		اگر (c < 0x80) {
			اگر (j+1 >= len)
				توڑو;
			dest[j++] = (uint8_t) c;
		} ورنہ اگر (c < 0x800) {
			اگر (j+2 >= len)
				توڑو;
			dest[j++] = (uint8_t) (0xc0 | (c >> 6));
			dest[j++] = (uint8_t) (0x80 | (c & 0x3f));
		} ورنہ اگر (c < 0x10000) {
			اگر (j+3 >= len)
				توڑو;
			dest[j++] = (uint8_t) (0xe0 | (c >> 12));
			dest[j++] = (uint8_t) (0x80 | ((c >> 6) & 0x3f));
			dest[j++] = (uint8_t) (0x80 | (c & 0x3f));
		} ورنہ {
			اگر (j+4 >= len)
				توڑو;
			dest[j++] = (uint8_t) (0xf0 | (c >> 18));
			dest[j++] = (uint8_t) (0x80 | ((c >> 12) & 0x3f));
			dest[j++] = (uint8_t) (0x80 | ((c >> 6) & 0x3f));
			dest[j++] = (uint8_t) (0x80 | (c & 0x3f));
		}
	}
	dest[j] = '\0';
	واپس j;
}
