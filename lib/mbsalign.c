/*
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Align/Truncate a string in a given screen width
 * Copyright (C) 2009-2010 Free Software Foundation, Inc.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 2.1 of the License, or (at your
 * option) any later version.
 *
 * Written by Pádraig Brady.
 */
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <limits.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ctype.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "c.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "mbsalign.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "strutils.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "widechar.h"

/* Return the byte after a complete ANSI escape sequence, or NULL. */
ਸਥਿਰ ਅਚਲ ਅੱਖਰ *skip_ansi_escape(ਅਚਲ ਅੱਖਰ *p, ਅਚਲ ਅੱਖਰ *ਸਿਰਾ)
{
	ਅਚਲ ਅੱਖਰ *q;

	ਜੇ (p >= ਸਿਰਾ || (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ) *p != '\033' || p + 1 >= ਸਿਰਾ)
		ਵਾਪਸ ਨਿਰਾ;

	q = p + 1;
	ਜੇ (*q == '[') {
		q++;
		/* CSI parameter bytes (0x30-0x3f) are followed by intermediate
		 * bytes (0x20-0x2f) and a final byte (0x40-0x7e). None of them
		 * occupies a cell.
		 */
		ਜਦੋਂ (q < ਸਿਰਾ && (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ) *q >= 0x30
				&& (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ) *q <= 0x3f)
			q++;
		ਜਦੋਂ (q < ਸਿਰਾ && (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ) *q >= 0x20
				&& (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ) *q <= 0x2f)
			q++;
		ਜੇ (q < ਸਿਰਾ && (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ) *q >= 0x40
				&& (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ) *q <= 0x7e)
			ਵਾਪਸ q + 1;
	} ਨਹੀਂਤਾਂ ਜੇ (*q && strchr("()*+", *q) && q + 1 < ਸਿਰਾ)
		ਵਾਪਸ q + 2;

	ਵਾਪਸ ਨਿਰਾ;
}

/*
 * Counts number of cells in multibyte string. All control and
 * non-printable chars are ignored.
 *
 * Returns: number of cells.
 */
size_t mbs_nwidth(ਅਚਲ ਅੱਖਰ *buf, size_t bufsz)
{
	ਅਚਲ ਅੱਖਰ *p = buf, *ਆਖਰੀ = buf;
	size_t ਚੌੜਾਈ = 0;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_WIDECHAR
	mbstate_t st;
	memset(&st, 0, ਆਕਾਰ(st));
#ਜੇ_ਅੰਤ
	ਜੇ (p && *p && bufsz)
		ਆਖਰੀ = p + (bufsz - 1);

	ਜਦੋਂ (p && *p && p <= ਆਖਰੀ) {
		ਜੇ (*p == '\033') {
			ਅਚਲ ਅੱਖਰ *e = skip_ansi_escape(p, ਆਖਰੀ + 1);

			p = e ? e : p + 1;
			ਜਾਰੀ;
		}
		ਜੇ (iscntrl((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ) *p)) {
			p++;
			ਜਾਰੀ;
		}
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_WIDECHAR
		wchar_t wc;
		size_t len = mbrtowc(&wc, p, MB_CUR_MAX, &st);

		ਜੇ (len == 0)
			ਰੋਕੋ;
		ਜੇ (len > 0 && iswprint(wc)) {
			ਪੂਰਨ x = wcwidth(wc);
			ਜੇ (x > 0)
				ਚੌੜਾਈ += x;
		} ਨਹੀਂਤਾਂ ਜੇ (len == (size_t) -1 || len == (size_t) -2)
			len = 1;
		p += len;
#ਨਹੀਂਤਾਂ
		ਜੇ (isprint((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ) *p))
			ਚੌੜਾਈ++;
		p++;
#ਜੇ_ਅੰਤ
	}

	ਵਾਪਸ ਚੌੜਾਈ;
}

size_t mbs_ਚੌੜਾਈ(ਅਚਲ ਅੱਖਰ *s)
{
	ਜੇ (!s || !*s)
		ਵਾਪਸ 0;
	ਵਾਪਸ mbs_nwidth(s, ਲੜੀ_ਲੰਬਾਈ(s));
}

/*
 * Counts number of cells in multibyte string. For all control and
 * non-printable chars is the result width enlarged to store \x?? hex
 * sequence. See mbs_safe_encode().
 *
 * Returns: number of cells, @sz returns number of bytes.
 */
size_t mbs_ਸੁਰੱਖਿਅਤ_nwidth(ਅਚਲ ਅੱਖਰ *buf, size_t bufsz, size_t *sz)
{
	ਅਚਲ ਅੱਖਰ *p = buf, *ਆਖਰੀ = buf;
	size_t ਚੌੜਾਈ = 0, ਬਾਈਟਾਂ = 0;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_WIDECHAR
	mbstate_t st;
	memset(&st, 0, ਆਕਾਰ(st));
#ਜੇ_ਅੰਤ
	ਜੇ (p && *p && bufsz)
		ਆਖਰੀ = p + (bufsz - 1);

	ਜਦੋਂ (p && *p && p <= ਆਖਰੀ) {
		ਜੇ ((p < ਆਖਰੀ && *p == '\\' && *(p + 1) == 'x')
		    || iscntrl((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ) *p)) {
			ਚੌੜਾਈ += 4, ਬਾਈਟਾਂ += 4;		/* *p encoded to \x?? */
			p++;
		}
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_WIDECHAR
		ਨਹੀਂਤਾਂ {
			wchar_t wc;
			size_t len = mbrtowc(&wc, p, MB_CUR_MAX, &st);

			ਜੇ (len == 0)
				ਰੋਕੋ;

			ਜੇ (len == (size_t) -1 || len == (size_t) -2) {
				len = 1;
				ਜੇ (isprint((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ) *p))
					ਚੌੜਾਈ += 1, ਬਾਈਟਾਂ += 1;
				ਨਹੀਂਤਾਂ
					ਚੌੜਾਈ += 4, ਬਾਈਟਾਂ += 4;

			} ਨਹੀਂਤਾਂ ਜੇ (!iswprint(wc)) {
				ਚੌੜਾਈ += len * 4;	/* hex encode whole sequence */
				ਬਾਈਟਾਂ += len * 4;
			} ਨਹੀਂਤਾਂ {
				ਚੌੜਾਈ += wcwidth(wc);	/* number of cells */
				ਬਾਈਟਾਂ += len;		/* number of bytes */
			}
			p += len;
		}
#ਨਹੀਂਤਾਂ
		ਨਹੀਂਤਾਂ ਜੇ (!isprint((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ) *p)) {
			ਚੌੜਾਈ += 4, ਬਾਈਟਾਂ += 4;		/* *p encoded to \x?? */
			p++;
		} ਨਹੀਂਤਾਂ {
			ਚੌੜਾਈ++, ਬਾਈਟਾਂ++;
			p++;
		}
#ਜੇ_ਅੰਤ
	}

	ਜੇ (sz)
		*sz = ਬਾਈਟਾਂ;
	ਵਾਪਸ ਚੌੜਾਈ;
}

size_t mbs_ਸੁਰੱਖਿਅਤ_ਚੌੜਾਈ(ਅਚਲ ਅੱਖਰ *s)
{
	ਜੇ (!s || !*s)
		ਵਾਪਸ 0;
	ਵਾਪਸ mbs_ਸੁਰੱਖਿਅਤ_nwidth(s, ਲੜੀ_ਲੰਬਾਈ(s), ਨਿਰਾ);
}

/*
 * Copy @s to @buf and replace control and non-printable chars with
 * \x?? hex sequence. The @width returns number of cells. The @safechars
 * are not encoded.
 *
 * The @buf has to be big enough to store mbs_safe_encode_size(strlen(s)))
 * bytes.
 */
ਅੱਖਰ *mbs_ਸੁਰੱਖਿਅਤ_encode_ਮੰਜ਼ਿਲ_ਬਫਰ(ਅਚਲ ਅੱਖਰ *s, size_t *ਚੌੜਾਈ, ਅੱਖਰ *buf, size_t bufsiz, ਅਚਲ ਅੱਖਰ *safechars)
{
	ਅਚਲ ਅੱਖਰ *p = s;
	ਅੱਖਰ *r;
	ਪੂਰਨ rsz;
	size_t sz = s ? ਲੜੀ_ਲੰਬਾਈ(s) : 0;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_WIDECHAR
	mbstate_t st;
	memset(&st, 0, ਆਕਾਰ(st));
#ਜੇ_ਅੰਤ
	ਜੇ (!sz || !buf || !bufsiz)
		ਵਾਪਸ ਨਿਰਾ;

	r = buf;
	rsz = (ਪੂਰਨ) bufsiz;
	*ਚੌੜਾਈ = 0;

	ਜਦੋਂ (p && *p) {
		ਜੇ (safechars && strchr(safechars, *p)) {
			ਜੇ (rsz < 2)
				ਰੋਕੋ;
			*r++ = *p++;
			rsz--;
			ਜਾਰੀ;
		}

		ਜੇ ((*p == '\\' && *(p + 1) == 'x')
		    || iscntrl((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ) *p)) {
			ਜੇ (rsz < 5)
				ਰੋਕੋ;
			snprintf(r, rsz, "\\x%02x", (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ) *p);
			r += 4;
			rsz -= 4;
			*ਚੌੜਾਈ += 4;
			p++;
		}
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_WIDECHAR
		ਨਹੀਂਤਾਂ {
			wchar_t wc;
			size_t len = mbrtowc(&wc, p, MB_CUR_MAX, &st);

			ਜੇ (len == 0)
				ਰੋਕੋ;		/* end of string */

			ਜੇ (len == (size_t) -1 || len == (size_t) -2) {
				len = 1;
				/*
				 * Not valid multibyte sequence -- maybe it's
				 * printable char according to the current locales.
				 */
				ਜੇ (!isprint((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ) *p)) {
					ਜੇ (rsz < 5)
						ਰੋਕੋ;
					snprintf(r, rsz, "\\x%02x", (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ) *p);
					r += 4;
					rsz -= 4;
					*ਚੌੜਾਈ += 4;
				} ਨਹੀਂਤਾਂ {
					ਜੇ (rsz < 2)
						ਰੋਕੋ;
					(*ਚੌੜਾਈ)++;
					*r++ = *p;
					rsz--;
				}
			} ਨਹੀਂਤਾਂ ਜੇ (!iswprint(wc)) {
				size_t i;
				ਲਈ (i = 0; i < len; i++) {
					ਜੇ (rsz < 5)
						ਰੋਕੋ;
					snprintf(r, rsz, "\\x%02x", (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ) p[i]);
					r += 4;
					rsz -= 4;
					*ਚੌੜਾਈ += 4;
				}
				ਜੇ (i < len)
					ਰੋਕੋ;
			} ਨਹੀਂਤਾਂ {
				ਜੇ (rsz < (ਪੂਰਨ)len + 1)
					ਰੋਕੋ;
				ਸਮ੍ਰਿਤੀ_ਨਕਲ(r, p, len);
				r += len;
				rsz -= len;
				*ਚੌੜਾਈ += wcwidth(wc);
			}
			p += len;
		}
#ਨਹੀਂਤਾਂ
		ਨਹੀਂਤਾਂ ਜੇ (!isprint((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ) *p)) {
			ਜੇ (rsz < 5)
				ਰੋਕੋ;
			snprintf(r, rsz, "\\x%02x", (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ) *p);
			p++;
			r += 4;
			rsz -= 4;
			*ਚੌੜਾਈ += 4;
		} ਨਹੀਂਤਾਂ {
			ਜੇ (rsz < 2)
				ਰੋਕੋ;
			*r++ = *p++;
			rsz--;
			(*ਚੌੜਾਈ)++;
		}
#ਜੇ_ਅੰਤ
	}

	*r = '\0';
	ਵਾਪਸ buf;
}

/*
 * Copy @s to @buf and replace broken sequences to \x?? hex sequence. The
 * @width returns number of cells. The @safechars are not encoded.
 *
 * The @buf has to be big enough to store mbs_safe_encode_size(strlen(s)))
 * bytes.
 */
ਅੱਖਰ *mbs_invalid_encode_ਮੰਜ਼ਿਲ_ਬਫਰ(ਅਚਲ ਅੱਖਰ *s, size_t *ਚੌੜਾਈ, ਅੱਖਰ *buf, size_t bufsiz)
{
	ਅਚਲ ਅੱਖਰ *p = s;
	ਅੱਖਰ *r;
	ਪੂਰਨ rsz;
	size_t sz = s ? ਲੜੀ_ਲੰਬਾਈ(s) : 0;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_WIDECHAR
	mbstate_t st;
	memset(&st, 0, ਆਕਾਰ(st));
#ਜੇ_ਅੰਤ
	ਜੇ (!sz || !buf || !bufsiz)
		ਵਾਪਸ ਨਿਰਾ;

	r = buf;
	rsz = (ਪੂਰਨ) bufsiz;
	*ਚੌੜਾਈ = 0;

	ਜਦੋਂ (p && *p) {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_WIDECHAR
		wchar_t wc;
		size_t len = mbrtowc(&wc, p, MB_CUR_MAX, &st);
#ਨਹੀਂਤਾਂ
		size_t len = 1;
#ਜੇ_ਅੰਤ

		ਜੇ (len == 0)
			ਰੋਕੋ;		/* end of string */

		ਜੇ (len == (size_t) -1 || len == (size_t) -2) {
			len = 1;
			/*
			 * Not valid multibyte sequence -- maybe it's
			 * printable char according to the current locales.
			 */
			ਜੇ (!isprint((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ) *p)) {
				ਜੇ (rsz < 5)
					ਰੋਕੋ;
				snprintf(r, rsz, "\\x%02x", (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ) *p);
				r += 4;
				rsz -= 4;
				*ਚੌੜਾਈ += 4;
			} ਨਹੀਂਤਾਂ {
				ਜੇ (rsz < 2)
					ਰੋਕੋ;
				(*ਚੌੜਾਈ)++;
				*r++ = *p;
				rsz--;
			}
		} ਨਹੀਂਤਾਂ ਜੇ (*p == '\\' && *(p + 1) == 'x') {
			ਜੇ (rsz < 5)
				ਰੋਕੋ;
			snprintf(r, rsz, "\\x%02x", (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ) *p);
			r += 4;
			rsz -= 4;
			*ਚੌੜਾਈ += 4;
		} ਨਹੀਂਤਾਂ {
			ਜੇ (rsz < (ਪੂਰਨ)len + 1)
				ਰੋਕੋ;
			r = mempcpy(r, p, len);
			rsz -= len;
			*ਚੌੜਾਈ += wcwidth(wc);
		}
		p += len;
	}

	*r = '\0';
	ਵਾਪਸ buf;
}

/*
 * Guess size
 */
size_t mbs_ਸੁਰੱਖਿਅਤ_encode_ਮਾਪ(size_t ਬਾਈਟਾਂ)
{
	ਵਾਪਸ (ਬਾਈਟਾਂ * 4) + 1;
}

/*
 * Count size of the original string in bytes (count \x?? as one byte)
 */
size_t mbs_ਸੁਰੱਖਿਅਤ_decode_ਮਾਪ(ਅਚਲ ਅੱਖਰ *p)
{
	size_t ਬਾਈਟਾਂ = 0;

	ਜਦੋਂ (p && *p) {
		ਜੇ (*p == '\\' && *(p + 1) == 'x' &&
		    isxdigit(*(p + 2)) && isxdigit(*(p + 3)))
			p += 4;
		ਨਹੀਂਤਾਂ
			p++;
		ਬਾਈਟਾਂ++;
	}
	ਵਾਪਸ ਬਾਈਟਾਂ;
}

/*
 * Returns allocated string where all control and non-printable chars are
 * replaced with \x?? hex sequence.
 */
ਅੱਖਰ *mbs_ਸੁਰੱਖਿਅਤ_encode(ਅਚਲ ਅੱਖਰ *s, size_t *ਚੌੜਾਈ)
{
	size_t sz = s ? ਲੜੀ_ਲੰਬਾਈ(s) : 0;
	size_t bufsz;
	ਅੱਖਰ *buf, *ret = ਨਿਰਾ;

	ਜੇ (!sz)
		ਵਾਪਸ ਨਿਰਾ;
	bufsz = mbs_ਸੁਰੱਖਿਅਤ_encode_ਮਾਪ(sz);
	buf = ਸਮ੍ਰਿਤੀ_ਦਿਓ(bufsz);
	ਜੇ (buf)
		ret = mbs_ਸੁਰੱਖਿਅਤ_encode_ਮੰਜ਼ਿਲ_ਬਫਰ(s, ਚੌੜਾਈ, buf, bufsz, ਨਿਰਾ);
	ਜੇ (!ret)
		ਮੁਕਤ(buf);
	ਵਾਪਸ ret;
}

/*
 * Returns allocated string where all broken widechars chars are
 * replaced with \x?? hex sequence.
 */
ਅੱਖਰ *mbs_invalid_encode(ਅਚਲ ਅੱਖਰ *s, size_t *ਚੌੜਾਈ)
{
	size_t sz = s ? ਲੜੀ_ਲੰਬਾਈ(s) : 0;
	size_t bufsz;
	ਅੱਖਰ *buf, *ret = ਨਿਰਾ;

	ਜੇ (!sz)
		ਵਾਪਸ ਨਿਰਾ;
	bufsz = mbs_ਸੁਰੱਖਿਅਤ_encode_ਮਾਪ(sz);
	buf = ਸਮ੍ਰਿਤੀ_ਦਿਓ(bufsz);
	ਜੇ (buf)
		ret = mbs_invalid_encode_ਮੰਜ਼ਿਲ_ਬਫਰ(s, ਚੌੜਾਈ, buf, bufsz);
	ਜੇ (!ret)
		ਮੁਕਤ(buf);
	ਵਾਪਸ ret;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_WIDECHAR

ਸਥਿਰ bool
wc_ensure_printable (wchar_t *wchars)
{
  bool replaced = false;
  wchar_t *wc = wchars;
  ਜਦੋਂ (*wc)
    {
      ਜੇ (!iswprint ((wint_t) *wc))
        {
          *wc = 0xFFFD; /* L'\uFFFD' (replacement char) */
          replaced = true;
        }
      wc++;
    }
  ਵਾਪਸ replaced;
}

/* Return the wchar after a complete ANSI escape sequence, or NULL. */
ਸਥਿਰ wchar_t *
wc_skip_ansi_escape (wchar_t *wc)
{
  wchar_t *p = wc + 1;

  ਜੇ (*wc != L'\033' || !*p)
    ਵਾਪਸ ਨਿਰਾ;

  ਜੇ (*p == L'[')
    {
      p++;
      ਜਦੋਂ (*p >= 0x30 && *p <= 0x3f)
        p++;
      ਜਦੋਂ (*p >= 0x20 && *p <= 0x2f)
        p++;
      ਜੇ (*p >= 0x40 && *p <= 0x7e)
        ਵਾਪਸ p + 1;
    }
  ਨਹੀਂਤਾਂ ਜੇ ((*p == L'(' || *p == L')' || *p == L'*' || *p == L'+') && p[1])
    ਵਾਪਸ p + 2;

  ਵਾਪਸ ਨਿਰਾ;
}

/* Truncate wchar string to width cells.
 * Returns number of cells used.  */

ਸਥਿਰ size_t
wc_truncate (wchar_t *wc, size_t ਚੌੜਾਈ)
{
  size_t cells = 0;
  ਪੂਰਨ ਅਗਲਾ_cells = 0;

  ਜਦੋਂ (*wc)
    {
      wchar_t *after_escape = wc_skip_ansi_escape (wc);
      ਜੇ (after_escape)
        {
          wc = after_escape;
          ਜਾਰੀ;
        }

      ਅਗਲਾ_cells = wcwidth (*wc);
      ਜੇ (ਅਗਲਾ_cells == -1) /* non printable */
        {
          *wc = 0xFFFD; /* L'\uFFFD' (replacement char) */
          ਅਗਲਾ_cells = 1;
        }
      ਜੇ (cells + ਅਗਲਾ_cells > ਚੌੜਾਈ)
        ਰੋਕੋ;

      cells += ਅਗਲਾ_cells;
      wc++;
    }
  *wc = L'\0';
  ਵਾਪਸ cells;
}

ਸਥਿਰ ਪੂਰਨ
rpl_wcswidth (ਅਚਲ wchar_t *s, size_t n)
{
  ਪੂਰਨ ret = 0;

  ਜਦੋਂ (n-- > 0 && *s != L'\0')
    {
      ਪੂਰਨ nwidth = wcwidth (*s++);
      ਜੇ (nwidth == -1)             /* non printable */
        ਵਾਪਸ -1;
      ਜੇ (ret > (ਵੱਧ_ਤੋਂ_ਵੱਧ_ਅੰਕ - nwidth)) /*ਉਛਾਲ*/
        ਵਾਪਸ -1;
      ret += nwidth;
    }

  ਵਾਪਸ ret;
}
#ਜੇ_ਅੰਤ /* HAVE_WIDECHAR */

/* Truncate multi-byte string to @width and returns number of
 * bytes of the new string @str, and in @width returns number
 * of cells.
 */
size_t
mbs_truncate(ਅੱਖਰ *str, size_t *ਚੌੜਾਈ)
{
	ssize_t ਬਾਈਟਾਂ = ਲੜੀ_ਲੰਬਾਈ(str);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_WIDECHAR
	ssize_t sz = mbstowcs(ਨਿਰਾ, str, 0);
	wchar_t *wcs = ਨਿਰਾ;

	ਜੇ (sz == (ssize_t) -1)
		ਜਾਓ ਪੂਰਾ;

	wcs = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, (sz + 1) * ਆਕਾਰ(wchar_t));
	ਜੇ (!wcs)
		ਜਾਓ ਪੂਰਾ;

	ਜੇ (!mbstowcs(wcs, str, sz))
		ਜਾਓ ਪੂਰਾ;
	*ਚੌੜਾਈ = wc_truncate(wcs, *ਚੌੜਾਈ);
	ਬਾਈਟਾਂ = wcstombs(str, wcs, ਬਾਈਟਾਂ);
ਪੂਰਾ:
	ਮੁਕਤ(wcs);
#ਨਹੀਂਤਾਂ
	{
		ਅਚਲ ਅੱਖਰ *p = str;
		ਅਚਲ ਅੱਖਰ *ਸਿਰਾ = str + ਬਾਈਟਾਂ;
		size_t cells = 0;

		ਜਦੋਂ (p < ਸਿਰਾ) {
			ਅਚਲ ਅੱਖਰ *after_escape = skip_ansi_escape(p, ਸਿਰਾ);

			ਜੇ (after_escape) {
				p = after_escape;
				ਜਾਰੀ;
			}
			ਜੇ (cells >= *ਚੌੜਾਈ)
				ਰੋਕੋ;
			p++;
			cells++;
		}
		ਬਾਈਟਾਂ = p - str;
		*ਚੌੜਾਈ = cells;
	}
#ਜੇ_ਅੰਤ
	ਜੇ (ਬਾਈਟਾਂ >= 0)
		str[ਬਾਈਟਾਂ] = '\0';
	ਵਾਪਸ ਬਾਈਟਾਂ;
}

/* Write N_SPACES space characters to DEST while ensuring
   nothing is written beyond DEST_END. A terminating NUL
   is always added to DEST.
   A pointer to the terminating NUL is returned.  */

ਸਥਿਰ ਅੱਖਰ*
mbs_align_pad (ਅੱਖਰ *dest, ਅਚਲ ਅੱਖਰ* dest_ਸਿਰਾ, size_t n_spaces, ਪੂਰਨ padchar)
{
  ਲਈ (/* nothing */; n_spaces && (dest < dest_ਸਿਰਾ); n_spaces--)
    *dest++ = padchar;
  *dest = '\0';
  ਵਾਪਸ dest;
}

size_t
mbsalign (ਅਚਲ ਅੱਖਰ *src, ਅੱਖਰ *dest, size_t dest_ਮਾਪ,
          size_t *ਚੌੜਾਈ, mbs_align_t align, ਪੂਰਨ flags)
{
	ਵਾਪਸ mbsalign_with_padding(src, dest, dest_ਮਾਪ, ਚੌੜਾਈ, align, flags, ' ');
}

/* Align a string, SRC, in a field of *WIDTH columns, handling multi-byte
   characters; write the result into the DEST_SIZE-byte buffer, DEST.
   ALIGNMENT specifies whether to left- or right-justify or to center.
   If SRC requires more than *WIDTH columns, truncate it to fit.
   When centering, the number of trailing spaces may be one less than the
   number of leading spaces. The FLAGS parameter is unused at present.
   Return the length in bytes required for the final result, not counting
   the trailing NUL.  A return value of DEST_SIZE or larger means there
   wasn't enough space.  DEST will be NUL terminated in any case.
   Return (size_t) -1 upon error (invalid multi-byte sequence in SRC,
   or malloc failure), unless MBA_UNIBYTE_FALLBACK is specified.
   Update *WIDTH to indicate how many columns were used before padding.  */

size_t
mbsalign_with_padding (ਅਚਲ ਅੱਖਰ *src, ਅੱਖਰ *dest, size_t dest_ਮਾਪ,
	               size_t *ਚੌੜਾਈ, mbs_align_t align,
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_WIDECHAR
		       ਪੂਰਨ flags,
#ਨਹੀਂਤਾਂ
		       ਪੂਰਨ flags __attribute__((__unused__)),
#ਜੇ_ਅੰਤ
		       ਪੂਰਨ padchar)
{
  size_t ret = -1;
  size_t src_ਮਾਪ = ਲੜੀ_ਲੰਬਾਈ (src) + 1;
  ਅੱਖਰ *newstr = ਨਿਰਾ;
  wchar_t *str_wc = ਨਿਰਾ;
  ਅਚਲ ਅੱਖਰ *str_ਮੰਜ਼ਿਲ_print = src;
  size_t n_ਖਾਨੇ_ਗਿਣਤੀ = src_ਮਾਪ - 1;
  size_t n_ਵਰਤਿਆ_ਬਾਈਟਾਂ = n_ਖਾਨੇ_ਗਿਣਤੀ; /* Not including NUL */
  size_t n_spaces = 0, space_ਖੱਬਾ;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_WIDECHAR
  bool conversion = false;
  bool wc_enabled = false;

  /* In multi-byte locales convert to wide characters
     to allow easy truncation. Also determine number
     of screen columns used.  */
  ਜੇ (MB_CUR_MAX > 1)
    {
      size_t src_chars = mbstowcs (ਨਿਰਾ, src, 0);
      ਜੇ (src_chars == (size_t) -1)
        {
          ਜੇ (flags & MBA_UNIBYTE_FALLBACK)
            ਜਾਓ mbsalign_unibyte;
          ਨਹੀਂਤਾਂ
            ਜਾਓ mbsalign_cleanup;
        }
      src_chars += 1; /* make space for NUL */
      str_wc = ਸਮ੍ਰਿਤੀ_ਦਿਓ (src_chars * ਆਕਾਰ (wchar_t));
      ਜੇ (str_wc == ਨਿਰਾ)
        {
          ਜੇ (flags & MBA_UNIBYTE_FALLBACK)
            ਜਾਓ mbsalign_unibyte;
          ਨਹੀਂਤਾਂ
            ਜਾਓ mbsalign_cleanup;
        }
      ਜੇ (mbstowcs (str_wc, src, src_chars) != 0)
        {
          str_wc[src_chars - 1] = L'\0';
          wc_enabled = true;
          conversion = wc_ensure_printable (str_wc);
          n_ਖਾਨੇ_ਗਿਣਤੀ = rpl_wcswidth (str_wc, src_chars);
        }
    }

  /* If we transformed or need to truncate the source string
     then create a modified copy of it.  */
  ਜੇ (wc_enabled && (conversion || (n_ਖਾਨੇ_ਗਿਣਤੀ > *ਚੌੜਾਈ)))
    {
        ਜੇ (conversion)
          {
             /* May have increased the size by converting
                \t to \uFFFD for example.  */
            src_ਮਾਪ = wcstombs(ਨਿਰਾ, str_wc, 0) + 1;
          }
        newstr = ਸਮ੍ਰਿਤੀ_ਦਿਓ (src_ਮਾਪ);
        ਜੇ (newstr == ਨਿਰਾ)
        {
          ਜੇ (flags & MBA_UNIBYTE_FALLBACK)
            ਜਾਓ mbsalign_unibyte;
          ਨਹੀਂਤਾਂ
            ਜਾਓ mbsalign_cleanup;
        }
        str_ਮੰਜ਼ਿਲ_print = newstr;
        n_ਖਾਨੇ_ਗਿਣਤੀ = wc_truncate (str_wc, *ਚੌੜਾਈ);
        n_ਵਰਤਿਆ_ਬਾਈਟਾਂ = wcstombs (newstr, str_wc, src_ਮਾਪ);
    }

mbsalign_unibyte:
#ਜੇ_ਅੰਤ

  ਜੇ (n_ਖਾਨੇ_ਗਿਣਤੀ > *ਚੌੜਾਈ) /* Unibyte truncation required.  */
    {
      n_ਖਾਨੇ_ਗਿਣਤੀ = *ਚੌੜਾਈ;
      n_ਵਰਤਿਆ_ਬਾਈਟਾਂ = n_ਖਾਨੇ_ਗਿਣਤੀ;
    }

  ਜੇ (*ਚੌੜਾਈ > n_ਖਾਨੇ_ਗਿਣਤੀ) /* Padding required.  */
    n_spaces = *ਚੌੜਾਈ - n_ਖਾਨੇ_ਗਿਣਤੀ;

  /* indicate to caller how many cells needed (not including padding).  */
  *ਚੌੜਾਈ = n_ਖਾਨੇ_ਗਿਣਤੀ;

  /* indicate to caller how many bytes needed (not including NUL).  */
  ret = n_ਵਰਤਿਆ_ਬਾਈਟਾਂ + (n_spaces * 1);

  /* Write as much NUL terminated output to DEST as possible.  */
  ਜੇ (dest_ਮਾਪ != 0)
    {
      ਅੱਖਰ *dest_ਸਿਰਾ = dest + dest_ਮਾਪ - 1;
      size_t ਸ਼ੁਰੂ_spaces;
      size_t ਸਿਰਾ_spaces;

      ਚੋਣ (align)
        {
        ਹਾਲਤ MBS_ALIGN_CENTER:
          ਸ਼ੁਰੂ_spaces = n_spaces / 2 + n_spaces % 2;
          ਸਿਰਾ_spaces = n_spaces / 2;
          ਰੋਕੋ;
        ਹਾਲਤ MBS_ALIGN_ਖੱਬਾ:
          ਸ਼ੁਰੂ_spaces = 0;
          ਸਿਰਾ_spaces = n_spaces;
          ਰੋਕੋ;
        ਹਾਲਤ MBS_ALIGN_ਸੱਜਾ:
          ਸ਼ੁਰੂ_spaces = n_spaces;
          ਸਿਰਾ_spaces = 0;
          ਰੋਕੋ;
	ਮੂਲ:
	  abort();
        }

      dest = mbs_align_pad (dest, dest_ਸਿਰਾ, ਸ਼ੁਰੂ_spaces, padchar);
      space_ਖੱਬਾ = dest_ਸਿਰਾ - dest;
      dest = mempcpy (dest, str_ਮੰਜ਼ਿਲ_print, min (n_ਵਰਤਿਆ_ਬਾਈਟਾਂ, space_ਖੱਬਾ));
      mbs_align_pad (dest, dest_ਸਿਰਾ, ਸਿਰਾ_spaces, padchar);
    }
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_WIDECHAR
mbsalign_cleanup:
#ਜੇ_ਅੰਤ
  ਮੁਕਤ (str_wc);
  ਮੁਕਤ (newstr);

  ਵਾਪਸ ret;
}
