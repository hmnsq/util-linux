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
#समावेश "c.h"
#समावेश "encode.h"

माप_प्रकार ul_encode_to_utf8(पूर्णांक enc, अचिन्हित अक्षर *dest, माप_प्रकार len,
			स्थिर अचिन्हित अक्षर *src, माप_प्रकार गिनती)
{
	माप_प्रकार i, j;
	uint32_t c;
	uint16_t c2;

	क्रम (j = i = 0; i < गिनती; i++) {
		अगर (enc == UL_ENCODE_UTF16LE) {
			अगर (i+2 > गिनती)
				अवरोध;
			c = (src[i+1] << 8) | src[i];
			i++;
		} अन्यथा अगर (enc == UL_ENCODE_UTF16BE) {
			अगर (i+2 > गिनती)
				अवरोध;
			c = (src[i] << 8) | src[i+1];
			i++;
		} अन्यथा अगर (enc == UL_ENCODE_LATIN1) {
			c = src[i];
		} अन्यथा {
			वापस 0;
		}
		अगर ((enc == UL_ENCODE_UTF16LE || enc == UL_ENCODE_UTF16BE) &&
		    c >= 0xD800 && c <= 0xDBFF && i+2 < गिनती) {
			अगर (enc == UL_ENCODE_UTF16LE)
				c2 = (src[i+2] << 8) | src[i+1];
			अन्यथा
				c2 = (src[i+1] << 8) | src[i+2];
			अगर (c2 >= 0xDC00 && c2 <= 0xDFFF) {
				c = 0x10000 + ((c - 0xD800) << 10) + (c2 - 0xDC00);
				i += 2;
			}
		}
		अगर (c == 0) {
			dest[j] = '\0';
			अवरोध;
		}

		अगर (c < 0x80) {
			अगर (j+1 >= len)
				अवरोध;
			dest[j++] = (uint8_t) c;
		} अन्यथा अगर (c < 0x800) {
			अगर (j+2 >= len)
				अवरोध;
			dest[j++] = (uint8_t) (0xc0 | (c >> 6));
			dest[j++] = (uint8_t) (0x80 | (c & 0x3f));
		} अन्यथा अगर (c < 0x10000) {
			अगर (j+3 >= len)
				अवरोध;
			dest[j++] = (uint8_t) (0xe0 | (c >> 12));
			dest[j++] = (uint8_t) (0x80 | ((c >> 6) & 0x3f));
			dest[j++] = (uint8_t) (0x80 | (c & 0x3f));
		} अन्यथा {
			अगर (j+4 >= len)
				अवरोध;
			dest[j++] = (uint8_t) (0xf0 | (c >> 18));
			dest[j++] = (uint8_t) (0x80 | ((c >> 12) & 0x3f));
			dest[j++] = (uint8_t) (0x80 | ((c >> 6) & 0x3f));
			dest[j++] = (uint8_t) (0x80 | (c & 0x3f));
		}
	}
	dest[j] = '\0';
	वापस j;
}
