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
#ಅನ್ತರ್ಭಾವ "c.h"
#ಅನ್ತರ್ಭಾವ "encode.h"

size_t ul_encode_to_utf8(ಪೂರ್ಣಾಙ್ಕ enc, unsigned ವರ್ಣ *dest, size_t len,
			ಧ್ರುವ unsigned ವರ್ಣ *src, size_t ಎಣಿಕೆ)
{
	size_t i, j;
	uint32_t c;
	uint16_t c2;

	ಪ್ರತಿ (j = i = 0; i < ಎಣಿಕೆ; i++) {
		ಯದಿ (enc == UL_ENCODE_UTF16LE) {
			ಯದಿ (i+2 > ಎಣಿಕೆ)
				ಭಙ್ಗ;
			c = (src[i+1] << 8) | src[i];
			i++;
		} ಅನ್ಯಥಾ ಯದಿ (enc == UL_ENCODE_UTF16BE) {
			ಯದಿ (i+2 > ಎಣಿಕೆ)
				ಭಙ್ಗ;
			c = (src[i] << 8) | src[i+1];
			i++;
		} ಅನ್ಯಥಾ ಯದಿ (enc == UL_ENCODE_LATIN1) {
			c = src[i];
		} ಅನ್ಯಥಾ {
			ಪ್ರತ್ಯಾಗಮ 0;
		}
		ಯದಿ ((enc == UL_ENCODE_UTF16LE || enc == UL_ENCODE_UTF16BE) &&
		    c >= 0xD800 && c <= 0xDBFF && i+2 < ಎಣಿಕೆ) {
			ಯದಿ (enc == UL_ENCODE_UTF16LE)
				c2 = (src[i+2] << 8) | src[i+1];
			ಅನ್ಯಥಾ
				c2 = (src[i+1] << 8) | src[i+2];
			ಯದಿ (c2 >= 0xDC00 && c2 <= 0xDFFF) {
				c = 0x10000 + ((c - 0xD800) << 10) + (c2 - 0xDC00);
				i += 2;
			}
		}
		ಯದಿ (c == 0) {
			dest[j] = '\0';
			ಭಙ್ಗ;
		}

		ಯದಿ (c < 0x80) {
			ಯದಿ (j+1 >= len)
				ಭಙ್ಗ;
			dest[j++] = (uint8_t) c;
		} ಅನ್ಯಥಾ ಯದಿ (c < 0x800) {
			ಯದಿ (j+2 >= len)
				ಭಙ್ಗ;
			dest[j++] = (uint8_t) (0xc0 | (c >> 6));
			dest[j++] = (uint8_t) (0x80 | (c & 0x3f));
		} ಅನ್ಯಥಾ ಯದಿ (c < 0x10000) {
			ಯದಿ (j+3 >= len)
				ಭಙ್ಗ;
			dest[j++] = (uint8_t) (0xe0 | (c >> 12));
			dest[j++] = (uint8_t) (0x80 | ((c >> 6) & 0x3f));
			dest[j++] = (uint8_t) (0x80 | (c & 0x3f));
		} ಅನ್ಯಥಾ {
			ಯದಿ (j+4 >= len)
				ಭಙ್ಗ;
			dest[j++] = (uint8_t) (0xf0 | (c >> 18));
			dest[j++] = (uint8_t) (0x80 | ((c >> 12) & 0x3f));
			dest[j++] = (uint8_t) (0x80 | ((c >> 6) & 0x3f));
			dest[j++] = (uint8_t) (0x80 | (c & 0x3f));
		}
	}
	dest[j] = '\0';
	ಪ್ರತ್ಯಾಗಮ j;
}
