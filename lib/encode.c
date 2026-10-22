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
#അന്തര്ഭാവ "c.h"
#അന്തര്ഭാവ "encode.h"

size_t ul_encode_to_utf8(പൂർണ്ണസംഖ്യ enc, unsigned അക്ഷരം *dest, size_t len,
			സ്ഥിരം unsigned അക്ഷരം *src, size_t എണ്ണം)
{
	size_t i, j;
	uint32_t c;
	uint16_t c2;

	വേണ്ടി (j = i = 0; i < എണ്ണം; i++) {
		എങ്കിൽ (enc == UL_ENCODE_UTF16LE) {
			എങ്കിൽ (i+2 > എണ്ണം)
				നിർത്തുക;
			c = (src[i+1] << 8) | src[i];
			i++;
		} അല്ലെങ്കിൽ എങ്കിൽ (enc == UL_ENCODE_UTF16BE) {
			എങ്കിൽ (i+2 > എണ്ണം)
				നിർത്തുക;
			c = (src[i] << 8) | src[i+1];
			i++;
		} അല്ലെങ്കിൽ എങ്കിൽ (enc == UL_ENCODE_LATIN1) {
			c = src[i];
		} അല്ലെങ്കിൽ {
			തിരികെ 0;
		}
		എങ്കിൽ ((enc == UL_ENCODE_UTF16LE || enc == UL_ENCODE_UTF16BE) &&
		    c >= 0xD800 && c <= 0xDBFF && i+2 < എണ്ണം) {
			എങ്കിൽ (enc == UL_ENCODE_UTF16LE)
				c2 = (src[i+2] << 8) | src[i+1];
			അല്ലെങ്കിൽ
				c2 = (src[i+1] << 8) | src[i+2];
			എങ്കിൽ (c2 >= 0xDC00 && c2 <= 0xDFFF) {
				c = 0x10000 + ((c - 0xD800) << 10) + (c2 - 0xDC00);
				i += 2;
			}
		}
		എങ്കിൽ (c == 0) {
			dest[j] = '\0';
			നിർത്തുക;
		}

		എങ്കിൽ (c < 0x80) {
			എങ്കിൽ (j+1 >= len)
				നിർത്തുക;
			dest[j++] = (uint8_t) c;
		} അല്ലെങ്കിൽ എങ്കിൽ (c < 0x800) {
			എങ്കിൽ (j+2 >= len)
				നിർത്തുക;
			dest[j++] = (uint8_t) (0xc0 | (c >> 6));
			dest[j++] = (uint8_t) (0x80 | (c & 0x3f));
		} അല്ലെങ്കിൽ എങ്കിൽ (c < 0x10000) {
			എങ്കിൽ (j+3 >= len)
				നിർത്തുക;
			dest[j++] = (uint8_t) (0xe0 | (c >> 12));
			dest[j++] = (uint8_t) (0x80 | ((c >> 6) & 0x3f));
			dest[j++] = (uint8_t) (0x80 | (c & 0x3f));
		} അല്ലെങ്കിൽ {
			എങ്കിൽ (j+4 >= len)
				നിർത്തുക;
			dest[j++] = (uint8_t) (0xf0 | (c >> 18));
			dest[j++] = (uint8_t) (0x80 | ((c >> 12) & 0x3f));
			dest[j++] = (uint8_t) (0x80 | ((c >> 6) & 0x3f));
			dest[j++] = (uint8_t) (0x80 | (c & 0x3f));
		}
	}
	dest[j] = '\0';
	തിരികെ j;
}
