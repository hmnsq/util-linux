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
#ଅନ୍ତର୍ଭାଵ "c.h"
#ଅନ୍ତର୍ଭାଵ "encode.h"

size_t ul_encode_to_utf8(ପୂର୍ଣ୍ଣସଂଖ୍ୟା enc, unsigned ଅକ୍ଷର *dest, size_t len,
			ଅଚଳ unsigned ଅକ୍ଷର *src, size_t ଗଣନା_2)
{
	size_t i, j;
	uint32_t c;
	uint16_t c2;

	ପାଇଁ (j = i = 0; i < ଗଣନା_2; i++) {
		ଯଦି (enc == UL_ENCODE_UTF16LE) {
			ଯଦି (i+2 > ଗଣନା_2)
				ଭାଙ୍ଗ;
			c = (src[i+1] << 8) | src[i];
			i++;
		} ଅନ୍ୟଥା ଯଦି (enc == UL_ENCODE_UTF16BE) {
			ଯଦି (i+2 > ଗଣନା_2)
				ଭାଙ୍ଗ;
			c = (src[i] << 8) | src[i+1];
			i++;
		} ଅନ୍ୟଥା ଯଦି (enc == UL_ENCODE_LATIN1) {
			c = src[i];
		} ଅନ୍ୟଥା {
			ଫେରନ୍ତୁ 0;
		}
		ଯଦି ((enc == UL_ENCODE_UTF16LE || enc == UL_ENCODE_UTF16BE) &&
		    c >= 0xD800 && c <= 0xDBFF && i+2 < ଗଣନା_2) {
			ଯଦି (enc == UL_ENCODE_UTF16LE)
				c2 = (src[i+2] << 8) | src[i+1];
			ଅନ୍ୟଥା
				c2 = (src[i+1] << 8) | src[i+2];
			ଯଦି (c2 >= 0xDC00 && c2 <= 0xDFFF) {
				c = 0x10000 + ((c - 0xD800) << 10) + (c2 - 0xDC00);
				i += 2;
			}
		}
		ଯଦି (c == 0) {
			dest[j] = '\0';
			ଭାଙ୍ଗ;
		}

		ଯଦି (c < 0x80) {
			ଯଦି (j+1 >= len)
				ଭାଙ୍ଗ;
			dest[j++] = (uint8_t) c;
		} ଅନ୍ୟଥା ଯଦି (c < 0x800) {
			ଯଦି (j+2 >= len)
				ଭାଙ୍ଗ;
			dest[j++] = (uint8_t) (0xc0 | (c >> 6));
			dest[j++] = (uint8_t) (0x80 | (c & 0x3f));
		} ଅନ୍ୟଥା ଯଦି (c < 0x10000) {
			ଯଦି (j+3 >= len)
				ଭାଙ୍ଗ;
			dest[j++] = (uint8_t) (0xe0 | (c >> 12));
			dest[j++] = (uint8_t) (0x80 | ((c >> 6) & 0x3f));
			dest[j++] = (uint8_t) (0x80 | (c & 0x3f));
		} ଅନ୍ୟଥା {
			ଯଦି (j+4 >= len)
				ଭାଙ୍ଗ;
			dest[j++] = (uint8_t) (0xf0 | (c >> 18));
			dest[j++] = (uint8_t) (0x80 | ((c >> 12) & 0x3f));
			dest[j++] = (uint8_t) (0x80 | ((c >> 6) & 0x3f));
			dest[j++] = (uint8_t) (0x80 | (c & 0x3f));
		}
	}
	dest[j] = '\0';
	ଫେରନ୍ତୁ j;
}
