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
#अन्तर्भाव "c.h"
#अन्तर्भाव "encode.h"

size_t ul_encode_to_utf8(पूर्णाङ्क enc, unsigned वर्ण *dest, size_t len,
			ध्रुव unsigned वर्ण *src, size_t गणनम्)
{
	size_t i, j;
	uint32_t c;
	uint16_t c2;

	प्रति (j = i = 0; i < गणनम्; i++) {
		जइ (enc == UL_ENCODE_UTF16LE) {
			जइ (i+2 > गणनम्)
				भङ्ग;
			c = (src[i+1] << 8) | src[i];
			i++;
		} अण्णहा जइ (enc == UL_ENCODE_UTF16BE) {
			जइ (i+2 > गणनम्)
				भङ्ग;
			c = (src[i] << 8) | src[i+1];
			i++;
		} अण्णहा जइ (enc == UL_ENCODE_LATIN1) {
			c = src[i];
		} अण्णहा {
			प्रत्यागम 0;
		}
		जइ ((enc == UL_ENCODE_UTF16LE || enc == UL_ENCODE_UTF16BE) &&
		    c >= 0xD800 && c <= 0xDBFF && i+2 < गणनम्) {
			जइ (enc == UL_ENCODE_UTF16LE)
				c2 = (src[i+2] << 8) | src[i+1];
			अण्णहा
				c2 = (src[i+1] << 8) | src[i+2];
			जइ (c2 >= 0xDC00 && c2 <= 0xDFFF) {
				c = 0x10000 + ((c - 0xD800) << 10) + (c2 - 0xDC00);
				i += 2;
			}
		}
		जइ (c == 0) {
			dest[j] = '\0';
			भङ्ग;
		}

		जइ (c < 0x80) {
			जइ (j+1 >= len)
				भङ्ग;
			dest[j++] = (uint8_t) c;
		} अण्णहा जइ (c < 0x800) {
			जइ (j+2 >= len)
				भङ्ग;
			dest[j++] = (uint8_t) (0xc0 | (c >> 6));
			dest[j++] = (uint8_t) (0x80 | (c & 0x3f));
		} अण्णहा जइ (c < 0x10000) {
			जइ (j+3 >= len)
				भङ्ग;
			dest[j++] = (uint8_t) (0xe0 | (c >> 12));
			dest[j++] = (uint8_t) (0x80 | ((c >> 6) & 0x3f));
			dest[j++] = (uint8_t) (0x80 | (c & 0x3f));
		} अण्णहा {
			जइ (j+4 >= len)
				भङ्ग;
			dest[j++] = (uint8_t) (0xf0 | (c >> 18));
			dest[j++] = (uint8_t) (0x80 | ((c >> 12) & 0x3f));
			dest[j++] = (uint8_t) (0x80 | ((c >> 6) & 0x3f));
			dest[j++] = (uint8_t) (0x80 | (c & 0x3f));
		}
	}
	dest[j] = '\0';
	प्रत्यागम j;
}
