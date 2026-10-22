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
#অন্তর্ভাৱ "c.h"
#অন্তর্ভাৱ "encode.h"

size_t ul_encode_to_utf8(পূর্ণাঙ্ক enc, unsigned ৱর্ণ *dest, size_t len,
			ধ্রুৱ unsigned ৱর্ণ *src, size_t গুনতি)
{
	size_t i, j;
	uint32_t c;
	uint16_t c2;

	বাবে (j = i = 0; i < গুনতি; i++) {
		যদি (enc == UL_ENCODE_UTF16LE) {
			যদি (i+2 > গুনতি)
				ভঙ্গ;
			c = (src[i+1] << 8) | src[i];
			i++;
		} অন্যথা যদি (enc == UL_ENCODE_UTF16BE) {
			যদি (i+2 > গুনতি)
				ভঙ্গ;
			c = (src[i] << 8) | src[i+1];
			i++;
		} অন্যথা যদি (enc == UL_ENCODE_LATIN1) {
			c = src[i];
		} অন্যথা {
			ঘূৰাই 0;
		}
		যদি ((enc == UL_ENCODE_UTF16LE || enc == UL_ENCODE_UTF16BE) &&
		    c >= 0xD800 && c <= 0xDBFF && i+2 < গুনতি) {
			যদি (enc == UL_ENCODE_UTF16LE)
				c2 = (src[i+2] << 8) | src[i+1];
			অন্যথা
				c2 = (src[i+1] << 8) | src[i+2];
			যদি (c2 >= 0xDC00 && c2 <= 0xDFFF) {
				c = 0x10000 + ((c - 0xD800) << 10) + (c2 - 0xDC00);
				i += 2;
			}
		}
		যদি (c == 0) {
			dest[j] = '\0';
			ভঙ্গ;
		}

		যদি (c < 0x80) {
			যদি (j+1 >= len)
				ভঙ্গ;
			dest[j++] = (uint8_t) c;
		} অন্যথা যদি (c < 0x800) {
			যদি (j+2 >= len)
				ভঙ্গ;
			dest[j++] = (uint8_t) (0xc0 | (c >> 6));
			dest[j++] = (uint8_t) (0x80 | (c & 0x3f));
		} অন্যথা যদি (c < 0x10000) {
			যদি (j+3 >= len)
				ভঙ্গ;
			dest[j++] = (uint8_t) (0xe0 | (c >> 12));
			dest[j++] = (uint8_t) (0x80 | ((c >> 6) & 0x3f));
			dest[j++] = (uint8_t) (0x80 | (c & 0x3f));
		} অন্যথা {
			যদি (j+4 >= len)
				ভঙ্গ;
			dest[j++] = (uint8_t) (0xf0 | (c >> 18));
			dest[j++] = (uint8_t) (0x80 | ((c >> 12) & 0x3f));
			dest[j++] = (uint8_t) (0x80 | ((c >> 6) & 0x3f));
			dest[j++] = (uint8_t) (0x80 | (c & 0x3f));
		}
	}
	dest[j] = '\0';
	ঘূৰাই j;
}
