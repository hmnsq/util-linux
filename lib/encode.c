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
#ਅਨ੍ਤਰ੍ਭਾਵ "c.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "encode.h"

size_t ul_encode_ਮੰਜ਼ਿਲ_utf8(ਪੂਰਨ enc, ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *dest, size_t len,
			ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *src, size_t ਗਿਣਤੀ_2)
{
	size_t i, j;
	uint32_t c;
	uint16_t c2;

	ਲਈ (j = i = 0; i < ਗਿਣਤੀ_2; i++) {
		ਜੇ (enc == UL_ENCODE_UTF16LE) {
			ਜੇ (i+2 > ਗਿਣਤੀ_2)
				ਰੋਕੋ;
			c = (src[i+1] << 8) | src[i];
			i++;
		} ਨਹੀਂਤਾਂ ਜੇ (enc == UL_ENCODE_UTF16BE) {
			ਜੇ (i+2 > ਗਿਣਤੀ_2)
				ਰੋਕੋ;
			c = (src[i] << 8) | src[i+1];
			i++;
		} ਨਹੀਂਤਾਂ ਜੇ (enc == UL_ENCODE_LATIN1) {
			c = src[i];
		} ਨਹੀਂਤਾਂ {
			ਵਾਪਸ 0;
		}
		ਜੇ ((enc == UL_ENCODE_UTF16LE || enc == UL_ENCODE_UTF16BE) &&
		    c >= 0xD800 && c <= 0xDBFF && i+2 < ਗਿਣਤੀ_2) {
			ਜੇ (enc == UL_ENCODE_UTF16LE)
				c2 = (src[i+2] << 8) | src[i+1];
			ਨਹੀਂਤਾਂ
				c2 = (src[i+1] << 8) | src[i+2];
			ਜੇ (c2 >= 0xDC00 && c2 <= 0xDFFF) {
				c = 0x10000 + ((c - 0xD800) << 10) + (c2 - 0xDC00);
				i += 2;
			}
		}
		ਜੇ (c == 0) {
			dest[j] = '\0';
			ਰੋਕੋ;
		}

		ਜੇ (c < 0x80) {
			ਜੇ (j+1 >= len)
				ਰੋਕੋ;
			dest[j++] = (uint8_t) c;
		} ਨਹੀਂਤਾਂ ਜੇ (c < 0x800) {
			ਜੇ (j+2 >= len)
				ਰੋਕੋ;
			dest[j++] = (uint8_t) (0xc0 | (c >> 6));
			dest[j++] = (uint8_t) (0x80 | (c & 0x3f));
		} ਨਹੀਂਤਾਂ ਜੇ (c < 0x10000) {
			ਜੇ (j+3 >= len)
				ਰੋਕੋ;
			dest[j++] = (uint8_t) (0xe0 | (c >> 12));
			dest[j++] = (uint8_t) (0x80 | ((c >> 6) & 0x3f));
			dest[j++] = (uint8_t) (0x80 | (c & 0x3f));
		} ਨਹੀਂਤਾਂ {
			ਜੇ (j+4 >= len)
				ਰੋਕੋ;
			dest[j++] = (uint8_t) (0xf0 | (c >> 18));
			dest[j++] = (uint8_t) (0x80 | ((c >> 12) & 0x3f));
			dest[j++] = (uint8_t) (0x80 | ((c >> 6) & 0x3f));
			dest[j++] = (uint8_t) (0x80 | (c & 0x3f));
		}
	}
	dest[j] = '\0';
	ਵਾਪਸ j;
}
