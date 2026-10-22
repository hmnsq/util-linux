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

size_t ul_encode_ಗಮ್ಯ_utf8(ಪೂರ್ಣಾಂಕ enc, ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *dest, size_t len,
			ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *src, size_t ಎಣಿಕೆ_2)
{
	size_t i, j;
	uint32_t c;
	uint16_t c2;

	ಗಾಗಿ (j = i = 0; i < ಎಣಿಕೆ_2; i++) {
		ಆದರೆ (enc == UL_ENCODE_UTF16LE) {
			ಆದರೆ (i+2 > ಎಣಿಕೆ_2)
				ನಿಲ್ಲಿಸು;
			c = (src[i+1] << 8) | src[i];
			i++;
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (enc == UL_ENCODE_UTF16BE) {
			ಆದರೆ (i+2 > ಎಣಿಕೆ_2)
				ನಿಲ್ಲಿಸು;
			c = (src[i] << 8) | src[i+1];
			i++;
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (enc == UL_ENCODE_LATIN1) {
			c = src[i];
		} ಇಲ್ಲದಿದ್ದರೆ {
			ಹಿಂತಿರುಗಿ 0;
		}
		ಆದರೆ ((enc == UL_ENCODE_UTF16LE || enc == UL_ENCODE_UTF16BE) &&
		    c >= 0xD800 && c <= 0xDBFF && i+2 < ಎಣಿಕೆ_2) {
			ಆದರೆ (enc == UL_ENCODE_UTF16LE)
				c2 = (src[i+2] << 8) | src[i+1];
			ಇಲ್ಲದಿದ್ದರೆ
				c2 = (src[i+1] << 8) | src[i+2];
			ಆದರೆ (c2 >= 0xDC00 && c2 <= 0xDFFF) {
				c = 0x10000 + ((c - 0xD800) << 10) + (c2 - 0xDC00);
				i += 2;
			}
		}
		ಆದರೆ (c == 0) {
			dest[j] = '\0';
			ನಿಲ್ಲಿಸು;
		}

		ಆದರೆ (c < 0x80) {
			ಆದರೆ (j+1 >= len)
				ನಿಲ್ಲಿಸು;
			dest[j++] = (uint8_t) c;
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (c < 0x800) {
			ಆದರೆ (j+2 >= len)
				ನಿಲ್ಲಿಸು;
			dest[j++] = (uint8_t) (0xc0 | (c >> 6));
			dest[j++] = (uint8_t) (0x80 | (c & 0x3f));
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (c < 0x10000) {
			ಆದರೆ (j+3 >= len)
				ನಿಲ್ಲಿಸು;
			dest[j++] = (uint8_t) (0xe0 | (c >> 12));
			dest[j++] = (uint8_t) (0x80 | ((c >> 6) & 0x3f));
			dest[j++] = (uint8_t) (0x80 | (c & 0x3f));
		} ಇಲ್ಲದಿದ್ದರೆ {
			ಆದರೆ (j+4 >= len)
				ನಿಲ್ಲಿಸು;
			dest[j++] = (uint8_t) (0xf0 | (c >> 18));
			dest[j++] = (uint8_t) (0x80 | ((c >> 12) & 0x3f));
			dest[j++] = (uint8_t) (0x80 | ((c >> 6) & 0x3f));
			dest[j++] = (uint8_t) (0x80 | (c & 0x3f));
		}
	}
	dest[j] = '\0';
	ಹಿಂತಿರುಗಿ j;
}
