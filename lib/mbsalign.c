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
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <limits.h>
#ಅನ್ತರ್ಭಾವ <ctype.h>

#ಅನ್ತರ್ಭಾವ "c.h"
#ಅನ್ತರ್ಭಾವ "mbsalign.h"
#ಅನ್ತರ್ಭಾವ "strutils.h"
#ಅನ್ತರ್ಭಾವ "widechar.h"

/* Return the byte after a complete ANSI escape sequence, or NULL. */
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *skip_ansi_escape(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಕೊನೆ)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *q;

	ಆದರೆ (p >= ಕೊನೆ || (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ) *p != '\033' || p + 1 >= ಕೊನೆ)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	q = p + 1;
	ಆದರೆ (*q == '[') {
		q++;
		/* CSI parameter bytes (0x30-0x3f) are followed by intermediate
		 * bytes (0x20-0x2f) and a final byte (0x40-0x7e). None of them
		 * occupies a cell.
		 */
		ಆಗ (q < ಕೊನೆ && (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ) *q >= 0x30
				&& (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ) *q <= 0x3f)
			q++;
		ಆಗ (q < ಕೊನೆ && (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ) *q >= 0x20
				&& (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ) *q <= 0x2f)
			q++;
		ಆದರೆ (q < ಕೊನೆ && (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ) *q >= 0x40
				&& (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ) *q <= 0x7e)
			ಹಿಂತಿರುಗಿ q + 1;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (*q && strchr("()*+", *q) && q + 1 < ಕೊನೆ)
		ಹಿಂತಿರುಗಿ q + 2;

	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

/*
 * Counts number of cells in multibyte string. All control and
 * non-printable chars are ignored.
 *
 * Returns: number of cells.
 */
size_t mbs_nwidth(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *buf, size_t bufsz)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p = buf, *ಕೊನೆಯ = buf;
	size_t ಅಗಲ = 0;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_WIDECHAR
	mbstate_t st;
	memset(&st, 0, ಗಾತ್ರ(st));
#ಆದರೆ_ಅಂತ್ಯ
	ಆದರೆ (p && *p && bufsz)
		ಕೊನೆಯ = p + (bufsz - 1);

	ಆಗ (p && *p && p <= ಕೊನೆಯ) {
		ಆದರೆ (*p == '\033') {
			ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *e = skip_ansi_escape(p, ಕೊನೆಯ + 1);

			p = e ? e : p + 1;
			ಮುಂದುವರಿ;
		}
		ಆದರೆ (iscntrl((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ) *p)) {
			p++;
			ಮುಂದುವರಿ;
		}
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_WIDECHAR
		wchar_t wc;
		size_t len = mbrtowc(&wc, p, MB_CUR_MAX, &st);

		ಆದರೆ (len == 0)
			ನಿಲ್ಲಿಸು;
		ಆದರೆ (len > 0 && iswprint(wc)) {
			ಪೂರ್ಣಾಂಕ x = wcwidth(wc);
			ಆದರೆ (x > 0)
				ಅಗಲ += x;
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (len == (size_t) -1 || len == (size_t) -2)
			len = 1;
		p += len;
#ಇಲ್ಲದಿದ್ದರೆ
		ಆದರೆ (isprint((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ) *p))
			ಅಗಲ++;
		p++;
#ಆದರೆ_ಅಂತ್ಯ
	}

	ಹಿಂತಿರುಗಿ ಅಗಲ;
}

size_t mbs_ಅಗಲ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s)
{
	ಆದರೆ (!s || !*s)
		ಹಿಂತಿರುಗಿ 0;
	ಹಿಂತಿರುಗಿ mbs_nwidth(s, ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(s));
}

/*
 * Counts number of cells in multibyte string. For all control and
 * non-printable chars is the result width enlarged to store \x?? hex
 * sequence. See mbs_safe_encode().
 *
 * Returns: number of cells, @sz returns number of bytes.
 */
size_t mbs_ಸುರಕ್ಷಿತ_nwidth(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *buf, size_t bufsz, size_t *sz)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p = buf, *ಕೊನೆಯ = buf;
	size_t ಅಗಲ = 0, ಬೈಟುಗಳು = 0;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_WIDECHAR
	mbstate_t st;
	memset(&st, 0, ಗಾತ್ರ(st));
#ಆದರೆ_ಅಂತ್ಯ
	ಆದರೆ (p && *p && bufsz)
		ಕೊನೆಯ = p + (bufsz - 1);

	ಆಗ (p && *p && p <= ಕೊನೆಯ) {
		ಆದರೆ ((p < ಕೊನೆಯ && *p == '\\' && *(p + 1) == 'x')
		    || iscntrl((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ) *p)) {
			ಅಗಲ += 4, ಬೈಟುಗಳು += 4;		/* *p encoded to \x?? */
			p++;
		}
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_WIDECHAR
		ಇಲ್ಲದಿದ್ದರೆ {
			wchar_t wc;
			size_t len = mbrtowc(&wc, p, MB_CUR_MAX, &st);

			ಆದರೆ (len == 0)
				ನಿಲ್ಲಿಸು;

			ಆದರೆ (len == (size_t) -1 || len == (size_t) -2) {
				len = 1;
				ಆದರೆ (isprint((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ) *p))
					ಅಗಲ += 1, ಬೈಟುಗಳು += 1;
				ಇಲ್ಲದಿದ್ದರೆ
					ಅಗಲ += 4, ಬೈಟುಗಳು += 4;

			} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (!iswprint(wc)) {
				ಅಗಲ += len * 4;	/* hex encode whole sequence */
				ಬೈಟುಗಳು += len * 4;
			} ಇಲ್ಲದಿದ್ದರೆ {
				ಅಗಲ += wcwidth(wc);	/* number of cells */
				ಬೈಟುಗಳು += len;		/* number of bytes */
			}
			p += len;
		}
#ಇಲ್ಲದಿದ್ದರೆ
		ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (!isprint((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ) *p)) {
			ಅಗಲ += 4, ಬೈಟುಗಳು += 4;		/* *p encoded to \x?? */
			p++;
		} ಇಲ್ಲದಿದ್ದರೆ {
			ಅಗಲ++, ಬೈಟುಗಳು++;
			p++;
		}
#ಆದರೆ_ಅಂತ್ಯ
	}

	ಆದರೆ (sz)
		*sz = ಬೈಟುಗಳು;
	ಹಿಂತಿರುಗಿ ಅಗಲ;
}

size_t mbs_ಸುರಕ್ಷಿತ_ಅಗಲ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s)
{
	ಆದರೆ (!s || !*s)
		ಹಿಂತಿರುಗಿ 0;
	ಹಿಂತಿರುಗಿ mbs_ಸುರಕ್ಷಿತ_nwidth(s, ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(s), ಖಾಲಿ);
}

/*
 * Copy @s to @buf and replace control and non-printable chars with
 * \x?? hex sequence. The @width returns number of cells. The @safechars
 * are not encoded.
 *
 * The @buf has to be big enough to store mbs_safe_encode_size(strlen(s)))
 * bytes.
 */
ಅಕ್ಷರ *mbs_ಸುರಕ್ಷಿತ_encode_ಗಮ್ಯ_ಬಫರ್(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s, size_t *ಅಗಲ, ಅಕ್ಷರ *buf, size_t bufsiz, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *safechars)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p = s;
	ಅಕ್ಷರ *r;
	ಪೂರ್ಣಾಂಕ rsz;
	size_t sz = s ? ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(s) : 0;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_WIDECHAR
	mbstate_t st;
	memset(&st, 0, ಗಾತ್ರ(st));
#ಆದರೆ_ಅಂತ್ಯ
	ಆದರೆ (!sz || !buf || !bufsiz)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	r = buf;
	rsz = (ಪೂರ್ಣಾಂಕ) bufsiz;
	*ಅಗಲ = 0;

	ಆಗ (p && *p) {
		ಆದರೆ (safechars && strchr(safechars, *p)) {
			ಆದರೆ (rsz < 2)
				ನಿಲ್ಲಿಸು;
			*r++ = *p++;
			rsz--;
			ಮುಂದುವರಿ;
		}

		ಆದರೆ ((*p == '\\' && *(p + 1) == 'x')
		    || iscntrl((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ) *p)) {
			ಆದರೆ (rsz < 5)
				ನಿಲ್ಲಿಸು;
			snprintf(r, rsz, "\\x%02x", (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ) *p);
			r += 4;
			rsz -= 4;
			*ಅಗಲ += 4;
			p++;
		}
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_WIDECHAR
		ಇಲ್ಲದಿದ್ದರೆ {
			wchar_t wc;
			size_t len = mbrtowc(&wc, p, MB_CUR_MAX, &st);

			ಆದರೆ (len == 0)
				ನಿಲ್ಲಿಸು;		/* end of string */

			ಆದರೆ (len == (size_t) -1 || len == (size_t) -2) {
				len = 1;
				/*
				 * Not valid multibyte sequence -- maybe it's
				 * printable char according to the current locales.
				 */
				ಆದರೆ (!isprint((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ) *p)) {
					ಆದರೆ (rsz < 5)
						ನಿಲ್ಲಿಸು;
					snprintf(r, rsz, "\\x%02x", (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ) *p);
					r += 4;
					rsz -= 4;
					*ಅಗಲ += 4;
				} ಇಲ್ಲದಿದ್ದರೆ {
					ಆದರೆ (rsz < 2)
						ನಿಲ್ಲಿಸು;
					(*ಅಗಲ)++;
					*r++ = *p;
					rsz--;
				}
			} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (!iswprint(wc)) {
				size_t i;
				ಗಾಗಿ (i = 0; i < len; i++) {
					ಆದರೆ (rsz < 5)
						ನಿಲ್ಲಿಸು;
					snprintf(r, rsz, "\\x%02x", (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ) p[i]);
					r += 4;
					rsz -= 4;
					*ಅಗಲ += 4;
				}
				ಆದರೆ (i < len)
					ನಿಲ್ಲಿಸು;
			} ಇಲ್ಲದಿದ್ದರೆ {
				ಆದರೆ (rsz < (ಪೂರ್ಣಾಂಕ)len + 1)
					ನಿಲ್ಲಿಸು;
				ಸ್ಮರಣೆ_ನಕಲು(r, p, len);
				r += len;
				rsz -= len;
				*ಅಗಲ += wcwidth(wc);
			}
			p += len;
		}
#ಇಲ್ಲದಿದ್ದರೆ
		ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (!isprint((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ) *p)) {
			ಆದರೆ (rsz < 5)
				ನಿಲ್ಲಿಸು;
			snprintf(r, rsz, "\\x%02x", (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ) *p);
			p++;
			r += 4;
			rsz -= 4;
			*ಅಗಲ += 4;
		} ಇಲ್ಲದಿದ್ದರೆ {
			ಆದರೆ (rsz < 2)
				ನಿಲ್ಲಿಸು;
			*r++ = *p++;
			rsz--;
			(*ಅಗಲ)++;
		}
#ಆದರೆ_ಅಂತ್ಯ
	}

	*r = '\0';
	ಹಿಂತಿರುಗಿ buf;
}

/*
 * Copy @s to @buf and replace broken sequences to \x?? hex sequence. The
 * @width returns number of cells. The @safechars are not encoded.
 *
 * The @buf has to be big enough to store mbs_safe_encode_size(strlen(s)))
 * bytes.
 */
ಅಕ್ಷರ *mbs_invalid_encode_ಗಮ್ಯ_ಬಫರ್(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s, size_t *ಅಗಲ, ಅಕ್ಷರ *buf, size_t bufsiz)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p = s;
	ಅಕ್ಷರ *r;
	ಪೂರ್ಣಾಂಕ rsz;
	size_t sz = s ? ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(s) : 0;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_WIDECHAR
	mbstate_t st;
	memset(&st, 0, ಗಾತ್ರ(st));
#ಆದರೆ_ಅಂತ್ಯ
	ಆದರೆ (!sz || !buf || !bufsiz)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	r = buf;
	rsz = (ಪೂರ್ಣಾಂಕ) bufsiz;
	*ಅಗಲ = 0;

	ಆಗ (p && *p) {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_WIDECHAR
		wchar_t wc;
		size_t len = mbrtowc(&wc, p, MB_CUR_MAX, &st);
#ಇಲ್ಲದಿದ್ದರೆ
		size_t len = 1;
#ಆದರೆ_ಅಂತ್ಯ

		ಆದರೆ (len == 0)
			ನಿಲ್ಲಿಸು;		/* end of string */

		ಆದರೆ (len == (size_t) -1 || len == (size_t) -2) {
			len = 1;
			/*
			 * Not valid multibyte sequence -- maybe it's
			 * printable char according to the current locales.
			 */
			ಆದರೆ (!isprint((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ) *p)) {
				ಆದರೆ (rsz < 5)
					ನಿಲ್ಲಿಸು;
				snprintf(r, rsz, "\\x%02x", (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ) *p);
				r += 4;
				rsz -= 4;
				*ಅಗಲ += 4;
			} ಇಲ್ಲದಿದ್ದರೆ {
				ಆದರೆ (rsz < 2)
					ನಿಲ್ಲಿಸು;
				(*ಅಗಲ)++;
				*r++ = *p;
				rsz--;
			}
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (*p == '\\' && *(p + 1) == 'x') {
			ಆದರೆ (rsz < 5)
				ನಿಲ್ಲಿಸು;
			snprintf(r, rsz, "\\x%02x", (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ) *p);
			r += 4;
			rsz -= 4;
			*ಅಗಲ += 4;
		} ಇಲ್ಲದಿದ್ದರೆ {
			ಆದರೆ (rsz < (ಪೂರ್ಣಾಂಕ)len + 1)
				ನಿಲ್ಲಿಸು;
			r = mempcpy(r, p, len);
			rsz -= len;
			*ಅಗಲ += wcwidth(wc);
		}
		p += len;
	}

	*r = '\0';
	ಹಿಂತಿರುಗಿ buf;
}

/*
 * Guess size
 */
size_t mbs_ಸುರಕ್ಷಿತ_encode_ಗಾತ್ರ(size_t ಬೈಟುಗಳು)
{
	ಹಿಂತಿರುಗಿ (ಬೈಟುಗಳು * 4) + 1;
}

/*
 * Count size of the original string in bytes (count \x?? as one byte)
 */
size_t mbs_ಸುರಕ್ಷಿತ_decode_ಗಾತ್ರ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p)
{
	size_t ಬೈಟುಗಳು = 0;

	ಆಗ (p && *p) {
		ಆದರೆ (*p == '\\' && *(p + 1) == 'x' &&
		    isxdigit(*(p + 2)) && isxdigit(*(p + 3)))
			p += 4;
		ಇಲ್ಲದಿದ್ದರೆ
			p++;
		ಬೈಟುಗಳು++;
	}
	ಹಿಂತಿರುಗಿ ಬೈಟುಗಳು;
}

/*
 * Returns allocated string where all control and non-printable chars are
 * replaced with \x?? hex sequence.
 */
ಅಕ್ಷರ *mbs_ಸುರಕ್ಷಿತ_encode(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s, size_t *ಅಗಲ)
{
	size_t sz = s ? ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(s) : 0;
	size_t bufsz;
	ಅಕ್ಷರ *buf, *ret = ಖಾಲಿ;

	ಆದರೆ (!sz)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	bufsz = mbs_ಸುರಕ್ಷಿತ_encode_ಗಾತ್ರ(sz);
	buf = ಸ್ಮರಣೆ_ಕೊಡು(bufsz);
	ಆದರೆ (buf)
		ret = mbs_ಸುರಕ್ಷಿತ_encode_ಗಮ್ಯ_ಬಫರ್(s, ಅಗಲ, buf, bufsz, ಖಾಲಿ);
	ಆದರೆ (!ret)
		ಬಿಡುಗಡೆ(buf);
	ಹಿಂತಿರುಗಿ ret;
}

/*
 * Returns allocated string where all broken widechars chars are
 * replaced with \x?? hex sequence.
 */
ಅಕ್ಷರ *mbs_invalid_encode(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s, size_t *ಅಗಲ)
{
	size_t sz = s ? ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(s) : 0;
	size_t bufsz;
	ಅಕ್ಷರ *buf, *ret = ಖಾಲಿ;

	ಆದರೆ (!sz)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	bufsz = mbs_ಸುರಕ್ಷಿತ_encode_ಗಾತ್ರ(sz);
	buf = ಸ್ಮರಣೆ_ಕೊಡು(bufsz);
	ಆದರೆ (buf)
		ret = mbs_invalid_encode_ಗಮ್ಯ_ಬಫರ್(s, ಅಗಲ, buf, bufsz);
	ಆದರೆ (!ret)
		ಬಿಡುಗಡೆ(buf);
	ಹಿಂತಿರುಗಿ ret;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_WIDECHAR

ಸ್ಥಿರ bool
wc_ensure_printable (wchar_t *wchars)
{
  bool replaced = false;
  wchar_t *wc = wchars;
  ಆಗ (*wc)
    {
      ಆದರೆ (!iswprint ((wint_t) *wc))
        {
          *wc = 0xFFFD; /* L'\uFFFD' (replacement char) */
          replaced = true;
        }
      wc++;
    }
  ಹಿಂತಿರುಗಿ replaced;
}

/* Return the wchar after a complete ANSI escape sequence, or NULL. */
ಸ್ಥಿರ wchar_t *
wc_skip_ansi_escape (wchar_t *wc)
{
  wchar_t *p = wc + 1;

  ಆದರೆ (*wc != L'\033' || !*p)
    ಹಿಂತಿರುಗಿ ಖಾಲಿ;

  ಆದರೆ (*p == L'[')
    {
      p++;
      ಆಗ (*p >= 0x30 && *p <= 0x3f)
        p++;
      ಆಗ (*p >= 0x20 && *p <= 0x2f)
        p++;
      ಆದರೆ (*p >= 0x40 && *p <= 0x7e)
        ಹಿಂತಿರುಗಿ p + 1;
    }
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ ((*p == L'(' || *p == L')' || *p == L'*' || *p == L'+') && p[1])
    ಹಿಂತಿರುಗಿ p + 2;

  ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

/* Truncate wchar string to width cells.
 * Returns number of cells used.  */

ಸ್ಥಿರ size_t
wc_truncate (wchar_t *wc, size_t ಅಗಲ)
{
  size_t cells = 0;
  ಪೂರ್ಣಾಂಕ ಮುಂದಿನ_cells = 0;

  ಆಗ (*wc)
    {
      wchar_t *after_escape = wc_skip_ansi_escape (wc);
      ಆದರೆ (after_escape)
        {
          wc = after_escape;
          ಮುಂದುವರಿ;
        }

      ಮುಂದಿನ_cells = wcwidth (*wc);
      ಆದರೆ (ಮುಂದಿನ_cells == -1) /* non printable */
        {
          *wc = 0xFFFD; /* L'\uFFFD' (replacement char) */
          ಮುಂದಿನ_cells = 1;
        }
      ಆದರೆ (cells + ಮುಂದಿನ_cells > ಅಗಲ)
        ನಿಲ್ಲಿಸು;

      cells += ಮುಂದಿನ_cells;
      wc++;
    }
  *wc = L'\0';
  ಹಿಂತಿರುಗಿ cells;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
rpl_wcswidth (ಸ್ಥಿರಾಂಕ wchar_t *s, size_t n)
{
  ಪೂರ್ಣಾಂಕ ret = 0;

  ಆಗ (n-- > 0 && *s != L'\0')
    {
      ಪೂರ್ಣಾಂಕ nwidth = wcwidth (*s++);
      ಆದರೆ (nwidth == -1)             /* non printable */
        ಹಿಂತಿರುಗಿ -1;
      ಆದರೆ (ret > (ಗರಿಷ್ಠ_ಪೂರ್ಣಾಂಕ - nwidth)) /*ಅತಿಪ್ರವಾಹ*/
        ಹಿಂತಿರುಗಿ -1;
      ret += nwidth;
    }

  ಹಿಂತಿರುಗಿ ret;
}
#ಆದರೆ_ಅಂತ್ಯ /* HAVE_WIDECHAR */

/* Truncate multi-byte string to @width and returns number of
 * bytes of the new string @str, and in @width returns number
 * of cells.
 */
size_t
mbs_truncate(ಅಕ್ಷರ *str, size_t *ಅಗಲ)
{
	ssize_t ಬೈಟುಗಳು = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(str);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_WIDECHAR
	ssize_t sz = mbstowcs(ಖಾಲಿ, str, 0);
	wchar_t *wcs = ಖಾಲಿ;

	ಆದರೆ (sz == (ssize_t) -1)
		ಹೋಗು ಮುಗಿದ;

	wcs = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(1, (sz + 1) * ಗಾತ್ರ(wchar_t));
	ಆದರೆ (!wcs)
		ಹೋಗು ಮುಗಿದ;

	ಆದರೆ (!mbstowcs(wcs, str, sz))
		ಹೋಗು ಮುಗಿದ;
	*ಅಗಲ = wc_truncate(wcs, *ಅಗಲ);
	ಬೈಟುಗಳು = wcstombs(str, wcs, ಬೈಟುಗಳು);
ಮುಗಿದ:
	ಬಿಡುಗಡೆ(wcs);
#ಇಲ್ಲದಿದ್ದರೆ
	{
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p = str;
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಕೊನೆ = str + ಬೈಟುಗಳು;
		size_t cells = 0;

		ಆಗ (p < ಕೊನೆ) {
			ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *after_escape = skip_ansi_escape(p, ಕೊನೆ);

			ಆದರೆ (after_escape) {
				p = after_escape;
				ಮುಂದುವರಿ;
			}
			ಆದರೆ (cells >= *ಅಗಲ)
				ನಿಲ್ಲಿಸು;
			p++;
			cells++;
		}
		ಬೈಟುಗಳು = p - str;
		*ಅಗಲ = cells;
	}
#ಆದರೆ_ಅಂತ್ಯ
	ಆದರೆ (ಬೈಟುಗಳು >= 0)
		str[ಬೈಟುಗಳು] = '\0';
	ಹಿಂತಿರುಗಿ ಬೈಟುಗಳು;
}

/* Write N_SPACES space characters to DEST while ensuring
   nothing is written beyond DEST_END. A terminating NUL
   is always added to DEST.
   A pointer to the terminating NUL is returned.  */

ಸ್ಥಿರ ಅಕ್ಷರ*
mbs_align_pad (ಅಕ್ಷರ *dest, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* dest_ಕೊನೆ, size_t n_spaces, ಪೂರ್ಣಾಂಕ padchar)
{
  ಗಾಗಿ (/* nothing */; n_spaces && (dest < dest_ಕೊನೆ); n_spaces--)
    *dest++ = padchar;
  *dest = '\0';
  ಹಿಂತಿರುಗಿ dest;
}

size_t
mbsalign (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *src, ಅಕ್ಷರ *dest, size_t dest_ಗಾತ್ರ,
          size_t *ಅಗಲ, mbs_align_t align, ಪೂರ್ಣಾಂಕ flags)
{
	ಹಿಂತಿರುಗಿ mbsalign_with_padding(src, dest, dest_ಗಾತ್ರ, ಅಗಲ, align, flags, ' ');
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
mbsalign_with_padding (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *src, ಅಕ್ಷರ *dest, size_t dest_ಗಾತ್ರ,
	               size_t *ಅಗಲ, mbs_align_t align,
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_WIDECHAR
		       ಪೂರ್ಣಾಂಕ flags,
#ಇಲ್ಲದಿದ್ದರೆ
		       ಪೂರ್ಣಾಂಕ flags __attribute__((__unused__)),
#ಆದರೆ_ಅಂತ್ಯ
		       ಪೂರ್ಣಾಂಕ padchar)
{
  size_t ret = -1;
  size_t src_ಗಾತ್ರ = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ (src) + 1;
  ಅಕ್ಷರ *newstr = ಖಾಲಿ;
  wchar_t *str_wc = ಖಾಲಿ;
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str_ಗಮ್ಯ_print = src;
  size_t n_ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ = src_ಗಾತ್ರ - 1;
  size_t n_ಬಳಸಿದ_ಬೈಟುಗಳು = n_ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ; /* Not including NUL */
  size_t n_spaces = 0, space_ಎಡ;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_WIDECHAR
  bool conversion = false;
  bool wc_enabled = false;

  /* In multi-byte locales convert to wide characters
     to allow easy truncation. Also determine number
     of screen columns used.  */
  ಆದರೆ (MB_CUR_MAX > 1)
    {
      size_t src_chars = mbstowcs (ಖಾಲಿ, src, 0);
      ಆದರೆ (src_chars == (size_t) -1)
        {
          ಆದರೆ (flags & MBA_UNIBYTE_FALLBACK)
            ಹೋಗು mbsalign_unibyte;
          ಇಲ್ಲದಿದ್ದರೆ
            ಹೋಗು mbsalign_cleanup;
        }
      src_chars += 1; /* make space for NUL */
      str_wc = ಸ್ಮರಣೆ_ಕೊಡು (src_chars * ಗಾತ್ರ (wchar_t));
      ಆದರೆ (str_wc == ಖಾಲಿ)
        {
          ಆದರೆ (flags & MBA_UNIBYTE_FALLBACK)
            ಹೋಗು mbsalign_unibyte;
          ಇಲ್ಲದಿದ್ದರೆ
            ಹೋಗು mbsalign_cleanup;
        }
      ಆದರೆ (mbstowcs (str_wc, src, src_chars) != 0)
        {
          str_wc[src_chars - 1] = L'\0';
          wc_enabled = true;
          conversion = wc_ensure_printable (str_wc);
          n_ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ = rpl_wcswidth (str_wc, src_chars);
        }
    }

  /* If we transformed or need to truncate the source string
     then create a modified copy of it.  */
  ಆದರೆ (wc_enabled && (conversion || (n_ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ > *ಅಗಲ)))
    {
        ಆದರೆ (conversion)
          {
             /* May have increased the size by converting
                \t to \uFFFD for example.  */
            src_ಗಾತ್ರ = wcstombs(ಖಾಲಿ, str_wc, 0) + 1;
          }
        newstr = ಸ್ಮರಣೆ_ಕೊಡು (src_ಗಾತ್ರ);
        ಆದರೆ (newstr == ಖಾಲಿ)
        {
          ಆದರೆ (flags & MBA_UNIBYTE_FALLBACK)
            ಹೋಗು mbsalign_unibyte;
          ಇಲ್ಲದಿದ್ದರೆ
            ಹೋಗು mbsalign_cleanup;
        }
        str_ಗಮ್ಯ_print = newstr;
        n_ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ = wc_truncate (str_wc, *ಅಗಲ);
        n_ಬಳಸಿದ_ಬೈಟುಗಳು = wcstombs (newstr, str_wc, src_ಗಾತ್ರ);
    }

mbsalign_unibyte:
#ಆದರೆ_ಅಂತ್ಯ

  ಆದರೆ (n_ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ > *ಅಗಲ) /* Unibyte truncation required.  */
    {
      n_ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ = *ಅಗಲ;
      n_ಬಳಸಿದ_ಬೈಟುಗಳು = n_ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ;
    }

  ಆದರೆ (*ಅಗಲ > n_ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ) /* Padding required.  */
    n_spaces = *ಅಗಲ - n_ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ;

  /* indicate to caller how many cells needed (not including padding).  */
  *ಅಗಲ = n_ಲಂಬಸಾಲು_ಸಂಖ್ಯೆ;

  /* indicate to caller how many bytes needed (not including NUL).  */
  ret = n_ಬಳಸಿದ_ಬೈಟುಗಳು + (n_spaces * 1);

  /* Write as much NUL terminated output to DEST as possible.  */
  ಆದರೆ (dest_ಗಾತ್ರ != 0)
    {
      ಅಕ್ಷರ *dest_ಕೊನೆ = dest + dest_ಗಾತ್ರ - 1;
      size_t ಆರಂಭ_spaces;
      size_t ಕೊನೆ_spaces;

      ಆಯ್ಕೆ (align)
        {
        ಸ್ಥಿತಿ MBS_ALIGN_CENTER:
          ಆರಂಭ_spaces = n_spaces / 2 + n_spaces % 2;
          ಕೊನೆ_spaces = n_spaces / 2;
          ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ MBS_ALIGN_ಎಡ:
          ಆರಂಭ_spaces = 0;
          ಕೊನೆ_spaces = n_spaces;
          ನಿಲ್ಲಿಸು;
        ಸ್ಥಿತಿ MBS_ALIGN_ಬಲ:
          ಆರಂಭ_spaces = n_spaces;
          ಕೊನೆ_spaces = 0;
          ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
	  abort();
        }

      dest = mbs_align_pad (dest, dest_ಕೊನೆ, ಆರಂಭ_spaces, padchar);
      space_ಎಡ = dest_ಕೊನೆ - dest;
      dest = mempcpy (dest, str_ಗಮ್ಯ_print, min (n_ಬಳಸಿದ_ಬೈಟುಗಳು, space_ಎಡ));
      mbs_align_pad (dest, dest_ಕೊನೆ, ಕೊನೆ_spaces, padchar);
    }
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HAVE_WIDECHAR
mbsalign_cleanup:
#ಆದರೆ_ಅಂತ್ಯ
  ಬಿಡುಗಡೆ (str_wc);
  ಬಿಡುಗಡೆ (newstr);

  ಹಿಂತಿರುಗಿ ret;
}
