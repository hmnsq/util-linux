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

माप_प्रकार ul_encode_to_utf8(पूर्णांक enc, अचिन्हित अक्षर *dest, माप_प्रकार len,
			स्थिरांक अचिन्हित अक्षर *src, माप_प्रकार मोजणी)
{
	माप_प्रकार i, j;
	uint32_t c;
	uint16_t c2;

	साठी (j = i = 0; i < मोजणी; i++) {
		जर (enc == UL_ENCODE_UTF16LE) {
			जर (i+2 > मोजणी)
				थांबा;
			c = (src[i+1] << 8) | src[i];
			i++;
		} अन्यथा जर (enc == UL_ENCODE_UTF16BE) {
			जर (i+2 > मोजणी)
				थांबा;
			c = (src[i] << 8) | src[i+1];
			i++;
		} अन्यथा जर (enc == UL_ENCODE_LATIN1) {
			c = src[i];
		} अन्यथा {
			परत 0;
		}
		जर ((enc == UL_ENCODE_UTF16LE || enc == UL_ENCODE_UTF16BE) &&
		    c >= 0xD800 && c <= 0xDBFF && i+2 < मोजणी) {
			जर (enc == UL_ENCODE_UTF16LE)
				c2 = (src[i+2] << 8) | src[i+1];
			अन्यथा
				c2 = (src[i+1] << 8) | src[i+2];
			जर (c2 >= 0xDC00 && c2 <= 0xDFFF) {
				c = 0x10000 + ((c - 0xD800) << 10) + (c2 - 0xDC00);
				i += 2;
			}
		}
		जर (c == 0) {
			dest[j] = '\0';
			थांबा;
		}

		जर (c < 0x80) {
			जर (j+1 >= len)
				थांबा;
			dest[j++] = (uint8_t) c;
		} अन्यथा जर (c < 0x800) {
			जर (j+2 >= len)
				थांबा;
			dest[j++] = (uint8_t) (0xc0 | (c >> 6));
			dest[j++] = (uint8_t) (0x80 | (c & 0x3f));
		} अन्यथा जर (c < 0x10000) {
			जर (j+3 >= len)
				थांबा;
			dest[j++] = (uint8_t) (0xe0 | (c >> 12));
			dest[j++] = (uint8_t) (0x80 | ((c >> 6) & 0x3f));
			dest[j++] = (uint8_t) (0x80 | (c & 0x3f));
		} अन्यथा {
			जर (j+4 >= len)
				थांबा;
			dest[j++] = (uint8_t) (0xf0 | (c >> 18));
			dest[j++] = (uint8_t) (0x80 | ((c >> 12) & 0x3f));
			dest[j++] = (uint8_t) (0x80 | ((c >> 6) & 0x3f));
			dest[j++] = (uint8_t) (0x80 | (c & 0x3f));
		}
	}
	dest[j] = '\0';
	परत j;
}
