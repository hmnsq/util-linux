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
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <limits.h>
#అన్తర్భావ <ctype.h>

#అన్తర్భావ "c.h"
#అన్తర్భావ "mbsalign.h"
#అన్తర్భావ "strutils.h"
#అన్తర్భావ "widechar.h"

/* Return the byte after a complete ANSI escape sequence, or NULL. */
స్థిర స్థిరరాశి అక్షరం *skip_ansi_escape(స్థిరరాశి అక్షరం *p, స్థిరరాశి అక్షరం *చివర)
{
	స్థిరరాశి అక్షరం *q;

	ఒకవేళ (p >= చివర || (గుర్తులేని అక్షరం) *p != '\033' || p + 1 >= చివర)
		తిరిగి ఖాళీ;

	q = p + 1;
	ఒకవేళ (*q == '[') {
		q++;
		/* CSI parameter bytes (0x30-0x3f) are followed by intermediate
		 * bytes (0x20-0x2f) and a final byte (0x40-0x7e). None of them
		 * occupies a cell.
		 */
		అయితే (q < చివర && (గుర్తులేని అక్షరం) *q >= 0x30
				&& (గుర్తులేని అక్షరం) *q <= 0x3f)
			q++;
		అయితే (q < చివర && (గుర్తులేని అక్షరం) *q >= 0x20
				&& (గుర్తులేని అక్షరం) *q <= 0x2f)
			q++;
		ఒకవేళ (q < చివర && (గుర్తులేని అక్షరం) *q >= 0x40
				&& (గుర్తులేని అక్షరం) *q <= 0x7e)
			తిరిగి q + 1;
	} లేకపోతే ఒకవేళ (*q && strchr("()*+", *q) && q + 1 < చివర)
		తిరిగి q + 2;

	తిరిగి ఖాళీ;
}

/*
 * Counts number of cells in multibyte string. All control and
 * non-printable chars are ignored.
 *
 * Returns: number of cells.
 */
size_t mbs_nwidth(స్థిరరాశి అక్షరం *buf, size_t bufsz)
{
	స్థిరరాశి అక్షరం *p = buf, *చివరి = buf;
	size_t వెడల్పు = 0;

#నిర్వచితమైతే HAVE_WIDECHAR
	mbstate_t st;
	memset(&st, 0, పరిమాణం(st));
#అయితే_ముగింపు
	ఒకవేళ (p && *p && bufsz)
		చివరి = p + (bufsz - 1);

	అయితే (p && *p && p <= చివరి) {
		ఒకవేళ (*p == '\033') {
			స్థిరరాశి అక్షరం *e = skip_ansi_escape(p, చివరి + 1);

			p = e ? e : p + 1;
			కొనసాగు;
		}
		ఒకవేళ (iscntrl((గుర్తులేని అక్షరం) *p)) {
			p++;
			కొనసాగు;
		}
#నిర్వచితమైతే HAVE_WIDECHAR
		wchar_t wc;
		size_t len = mbrtowc(&wc, p, MB_CUR_MAX, &st);

		ఒకవేళ (len == 0)
			ఆపు;
		ఒకవేళ (len > 0 && iswprint(wc)) {
			పూర్ణాంకం x = wcwidth(wc);
			ఒకవేళ (x > 0)
				వెడల్పు += x;
		} లేకపోతే ఒకవేళ (len == (size_t) -1 || len == (size_t) -2)
			len = 1;
		p += len;
#లేకపోతే
		ఒకవేళ (isprint((గుర్తులేని అక్షరం) *p))
			వెడల్పు++;
		p++;
#అయితే_ముగింపు
	}

	తిరిగి వెడల్పు;
}

size_t mbs_వెడల్పు(స్థిరరాశి అక్షరం *s)
{
	ఒకవేళ (!s || !*s)
		తిరిగి 0;
	తిరిగి mbs_nwidth(s, అక్షరశ్రేణి_పొడవు(s));
}

/*
 * Counts number of cells in multibyte string. For all control and
 * non-printable chars is the result width enlarged to store \x?? hex
 * sequence. See mbs_safe_encode().
 *
 * Returns: number of cells, @sz returns number of bytes.
 */
size_t mbs_సురక్షిత_nwidth(స్థిరరాశి అక్షరం *buf, size_t bufsz, size_t *sz)
{
	స్థిరరాశి అక్షరం *p = buf, *చివరి = buf;
	size_t వెడల్పు = 0, బైట్లు = 0;

#నిర్వచితమైతే HAVE_WIDECHAR
	mbstate_t st;
	memset(&st, 0, పరిమాణం(st));
#అయితే_ముగింపు
	ఒకవేళ (p && *p && bufsz)
		చివరి = p + (bufsz - 1);

	అయితే (p && *p && p <= చివరి) {
		ఒకవేళ ((p < చివరి && *p == '\\' && *(p + 1) == 'x')
		    || iscntrl((గుర్తులేని అక్షరం) *p)) {
			వెడల్పు += 4, బైట్లు += 4;		/* *p encoded to \x?? */
			p++;
		}
#నిర్వచితమైతే HAVE_WIDECHAR
		లేకపోతే {
			wchar_t wc;
			size_t len = mbrtowc(&wc, p, MB_CUR_MAX, &st);

			ఒకవేళ (len == 0)
				ఆపు;

			ఒకవేళ (len == (size_t) -1 || len == (size_t) -2) {
				len = 1;
				ఒకవేళ (isprint((గుర్తులేని అక్షరం) *p))
					వెడల్పు += 1, బైట్లు += 1;
				లేకపోతే
					వెడల్పు += 4, బైట్లు += 4;

			} లేకపోతే ఒకవేళ (!iswprint(wc)) {
				వెడల్పు += len * 4;	/* hex encode whole sequence */
				బైట్లు += len * 4;
			} లేకపోతే {
				వెడల్పు += wcwidth(wc);	/* number of cells */
				బైట్లు += len;		/* number of bytes */
			}
			p += len;
		}
#లేకపోతే
		లేకపోతే ఒకవేళ (!isprint((గుర్తులేని అక్షరం) *p)) {
			వెడల్పు += 4, బైట్లు += 4;		/* *p encoded to \x?? */
			p++;
		} లేకపోతే {
			వెడల్పు++, బైట్లు++;
			p++;
		}
#అయితే_ముగింపు
	}

	ఒకవేళ (sz)
		*sz = బైట్లు;
	తిరిగి వెడల్పు;
}

size_t mbs_సురక్షిత_వెడల్పు(స్థిరరాశి అక్షరం *s)
{
	ఒకవేళ (!s || !*s)
		తిరిగి 0;
	తిరిగి mbs_సురక్షిత_nwidth(s, అక్షరశ్రేణి_పొడవు(s), ఖాళీ);
}

/*
 * Copy @s to @buf and replace control and non-printable chars with
 * \x?? hex sequence. The @width returns number of cells. The @safechars
 * are not encoded.
 *
 * The @buf has to be big enough to store mbs_safe_encode_size(strlen(s)))
 * bytes.
 */
అక్షరం *mbs_సురక్షిత_encode_గమ్యం_బఫరు(స్థిరరాశి అక్షరం *s, size_t *వెడల్పు, అక్షరం *buf, size_t bufsiz, స్థిరరాశి అక్షరం *safechars)
{
	స్థిరరాశి అక్షరం *p = s;
	అక్షరం *r;
	పూర్ణాంకం rsz;
	size_t sz = s ? అక్షరశ్రేణి_పొడవు(s) : 0;

#నిర్వచితమైతే HAVE_WIDECHAR
	mbstate_t st;
	memset(&st, 0, పరిమాణం(st));
#అయితే_ముగింపు
	ఒకవేళ (!sz || !buf || !bufsiz)
		తిరిగి ఖాళీ;

	r = buf;
	rsz = (పూర్ణాంకం) bufsiz;
	*వెడల్పు = 0;

	అయితే (p && *p) {
		ఒకవేళ (safechars && strchr(safechars, *p)) {
			ఒకవేళ (rsz < 2)
				ఆపు;
			*r++ = *p++;
			rsz--;
			కొనసాగు;
		}

		ఒకవేళ ((*p == '\\' && *(p + 1) == 'x')
		    || iscntrl((గుర్తులేని అక్షరం) *p)) {
			ఒకవేళ (rsz < 5)
				ఆపు;
			snprintf(r, rsz, "\\x%02x", (గుర్తులేని అక్షరం) *p);
			r += 4;
			rsz -= 4;
			*వెడల్పు += 4;
			p++;
		}
#నిర్వచితమైతే HAVE_WIDECHAR
		లేకపోతే {
			wchar_t wc;
			size_t len = mbrtowc(&wc, p, MB_CUR_MAX, &st);

			ఒకవేళ (len == 0)
				ఆపు;		/* end of string */

			ఒకవేళ (len == (size_t) -1 || len == (size_t) -2) {
				len = 1;
				/*
				 * Not valid multibyte sequence -- maybe it's
				 * printable char according to the current locales.
				 */
				ఒకవేళ (!isprint((గుర్తులేని అక్షరం) *p)) {
					ఒకవేళ (rsz < 5)
						ఆపు;
					snprintf(r, rsz, "\\x%02x", (గుర్తులేని అక్షరం) *p);
					r += 4;
					rsz -= 4;
					*వెడల్పు += 4;
				} లేకపోతే {
					ఒకవేళ (rsz < 2)
						ఆపు;
					(*వెడల్పు)++;
					*r++ = *p;
					rsz--;
				}
			} లేకపోతే ఒకవేళ (!iswprint(wc)) {
				size_t i;
				కొరకు (i = 0; i < len; i++) {
					ఒకవేళ (rsz < 5)
						ఆపు;
					snprintf(r, rsz, "\\x%02x", (గుర్తులేని అక్షరం) p[i]);
					r += 4;
					rsz -= 4;
					*వెడల్పు += 4;
				}
				ఒకవేళ (i < len)
					ఆపు;
			} లేకపోతే {
				ఒకవేళ (rsz < (పూర్ణాంకం)len + 1)
					ఆపు;
				జ్ఞాపకం_నకలు(r, p, len);
				r += len;
				rsz -= len;
				*వెడల్పు += wcwidth(wc);
			}
			p += len;
		}
#లేకపోతే
		లేకపోతే ఒకవేళ (!isprint((గుర్తులేని అక్షరం) *p)) {
			ఒకవేళ (rsz < 5)
				ఆపు;
			snprintf(r, rsz, "\\x%02x", (గుర్తులేని అక్షరం) *p);
			p++;
			r += 4;
			rsz -= 4;
			*వెడల్పు += 4;
		} లేకపోతే {
			ఒకవేళ (rsz < 2)
				ఆపు;
			*r++ = *p++;
			rsz--;
			(*వెడల్పు)++;
		}
#అయితే_ముగింపు
	}

	*r = '\0';
	తిరిగి buf;
}

/*
 * Copy @s to @buf and replace broken sequences to \x?? hex sequence. The
 * @width returns number of cells. The @safechars are not encoded.
 *
 * The @buf has to be big enough to store mbs_safe_encode_size(strlen(s)))
 * bytes.
 */
అక్షరం *mbs_invalid_encode_గమ్యం_బఫరు(స్థిరరాశి అక్షరం *s, size_t *వెడల్పు, అక్షరం *buf, size_t bufsiz)
{
	స్థిరరాశి అక్షరం *p = s;
	అక్షరం *r;
	పూర్ణాంకం rsz;
	size_t sz = s ? అక్షరశ్రేణి_పొడవు(s) : 0;

#నిర్వచితమైతే HAVE_WIDECHAR
	mbstate_t st;
	memset(&st, 0, పరిమాణం(st));
#అయితే_ముగింపు
	ఒకవేళ (!sz || !buf || !bufsiz)
		తిరిగి ఖాళీ;

	r = buf;
	rsz = (పూర్ణాంకం) bufsiz;
	*వెడల్పు = 0;

	అయితే (p && *p) {
#నిర్వచితమైతే HAVE_WIDECHAR
		wchar_t wc;
		size_t len = mbrtowc(&wc, p, MB_CUR_MAX, &st);
#లేకపోతే
		size_t len = 1;
#అయితే_ముగింపు

		ఒకవేళ (len == 0)
			ఆపు;		/* end of string */

		ఒకవేళ (len == (size_t) -1 || len == (size_t) -2) {
			len = 1;
			/*
			 * Not valid multibyte sequence -- maybe it's
			 * printable char according to the current locales.
			 */
			ఒకవేళ (!isprint((గుర్తులేని అక్షరం) *p)) {
				ఒకవేళ (rsz < 5)
					ఆపు;
				snprintf(r, rsz, "\\x%02x", (గుర్తులేని అక్షరం) *p);
				r += 4;
				rsz -= 4;
				*వెడల్పు += 4;
			} లేకపోతే {
				ఒకవేళ (rsz < 2)
					ఆపు;
				(*వెడల్పు)++;
				*r++ = *p;
				rsz--;
			}
		} లేకపోతే ఒకవేళ (*p == '\\' && *(p + 1) == 'x') {
			ఒకవేళ (rsz < 5)
				ఆపు;
			snprintf(r, rsz, "\\x%02x", (గుర్తులేని అక్షరం) *p);
			r += 4;
			rsz -= 4;
			*వెడల్పు += 4;
		} లేకపోతే {
			ఒకవేళ (rsz < (పూర్ణాంకం)len + 1)
				ఆపు;
			r = mempcpy(r, p, len);
			rsz -= len;
			*వెడల్పు += wcwidth(wc);
		}
		p += len;
	}

	*r = '\0';
	తిరిగి buf;
}

/*
 * Guess size
 */
size_t mbs_సురక్షిత_encode_కొలత(size_t బైట్లు)
{
	తిరిగి (బైట్లు * 4) + 1;
}

/*
 * Count size of the original string in bytes (count \x?? as one byte)
 */
size_t mbs_సురక్షిత_decode_కొలత(స్థిరరాశి అక్షరం *p)
{
	size_t బైట్లు = 0;

	అయితే (p && *p) {
		ఒకవేళ (*p == '\\' && *(p + 1) == 'x' &&
		    isxdigit(*(p + 2)) && isxdigit(*(p + 3)))
			p += 4;
		లేకపోతే
			p++;
		బైట్లు++;
	}
	తిరిగి బైట్లు;
}

/*
 * Returns allocated string where all control and non-printable chars are
 * replaced with \x?? hex sequence.
 */
అక్షరం *mbs_సురక్షిత_encode(స్థిరరాశి అక్షరం *s, size_t *వెడల్పు)
{
	size_t sz = s ? అక్షరశ్రేణి_పొడవు(s) : 0;
	size_t bufsz;
	అక్షరం *buf, *ret = ఖాళీ;

	ఒకవేళ (!sz)
		తిరిగి ఖాళీ;
	bufsz = mbs_సురక్షిత_encode_కొలత(sz);
	buf = జ్ఞాపకం_ఇవ్వు(bufsz);
	ఒకవేళ (buf)
		ret = mbs_సురక్షిత_encode_గమ్యం_బఫరు(s, వెడల్పు, buf, bufsz, ఖాళీ);
	ఒకవేళ (!ret)
		విడుదల(buf);
	తిరిగి ret;
}

/*
 * Returns allocated string where all broken widechars chars are
 * replaced with \x?? hex sequence.
 */
అక్షరం *mbs_invalid_encode(స్థిరరాశి అక్షరం *s, size_t *వెడల్పు)
{
	size_t sz = s ? అక్షరశ్రేణి_పొడవు(s) : 0;
	size_t bufsz;
	అక్షరం *buf, *ret = ఖాళీ;

	ఒకవేళ (!sz)
		తిరిగి ఖాళీ;
	bufsz = mbs_సురక్షిత_encode_కొలత(sz);
	buf = జ్ఞాపకం_ఇవ్వు(bufsz);
	ఒకవేళ (buf)
		ret = mbs_invalid_encode_గమ్యం_బఫరు(s, వెడల్పు, buf, bufsz);
	ఒకవేళ (!ret)
		విడుదల(buf);
	తిరిగి ret;
}

#నిర్వచితమైతే HAVE_WIDECHAR

స్థిర bool
wc_ensure_printable (wchar_t *wchars)
{
  bool replaced = false;
  wchar_t *wc = wchars;
  అయితే (*wc)
    {
      ఒకవేళ (!iswprint ((wint_t) *wc))
        {
          *wc = 0xFFFD; /* L'\uFFFD' (replacement char) */
          replaced = true;
        }
      wc++;
    }
  తిరిగి replaced;
}

/* Return the wchar after a complete ANSI escape sequence, or NULL. */
స్థిర wchar_t *
wc_skip_ansi_escape (wchar_t *wc)
{
  wchar_t *p = wc + 1;

  ఒకవేళ (*wc != L'\033' || !*p)
    తిరిగి ఖాళీ;

  ఒకవేళ (*p == L'[')
    {
      p++;
      అయితే (*p >= 0x30 && *p <= 0x3f)
        p++;
      అయితే (*p >= 0x20 && *p <= 0x2f)
        p++;
      ఒకవేళ (*p >= 0x40 && *p <= 0x7e)
        తిరిగి p + 1;
    }
  లేకపోతే ఒకవేళ ((*p == L'(' || *p == L')' || *p == L'*' || *p == L'+') && p[1])
    తిరిగి p + 2;

  తిరిగి ఖాళీ;
}

/* Truncate wchar string to width cells.
 * Returns number of cells used.  */

స్థిర size_t
wc_truncate (wchar_t *wc, size_t వెడల్పు)
{
  size_t cells = 0;
  పూర్ణాంకం తదుపరి_cells = 0;

  అయితే (*wc)
    {
      wchar_t *after_escape = wc_skip_ansi_escape (wc);
      ఒకవేళ (after_escape)
        {
          wc = after_escape;
          కొనసాగు;
        }

      తదుపరి_cells = wcwidth (*wc);
      ఒకవేళ (తదుపరి_cells == -1) /* non printable */
        {
          *wc = 0xFFFD; /* L'\uFFFD' (replacement char) */
          తదుపరి_cells = 1;
        }
      ఒకవేళ (cells + తదుపరి_cells > వెడల్పు)
        ఆపు;

      cells += తదుపరి_cells;
      wc++;
    }
  *wc = L'\0';
  తిరిగి cells;
}

స్థిర పూర్ణాంకం
rpl_wcswidth (స్థిరరాశి wchar_t *s, size_t n)
{
  పూర్ణాంకం ret = 0;

  అయితే (n-- > 0 && *s != L'\0')
    {
      పూర్ణాంకం nwidth = wcwidth (*s++);
      ఒకవేళ (nwidth == -1)             /* non printable */
        తిరిగి -1;
      ఒకవేళ (ret > (గరిష్ఠ_పూర్ణాంకం - nwidth)) /*అధికప్రవాహం*/
        తిరిగి -1;
      ret += nwidth;
    }

  తిరిగి ret;
}
#అయితే_ముగింపు /* HAVE_WIDECHAR */

/* Truncate multi-byte string to @width and returns number of
 * bytes of the new string @str, and in @width returns number
 * of cells.
 */
size_t
mbs_truncate(అక్షరం *str, size_t *వెడల్పు)
{
	ssize_t బైట్లు = అక్షరశ్రేణి_పొడవు(str);
#నిర్వచితమైతే HAVE_WIDECHAR
	ssize_t sz = mbstowcs(ఖాళీ, str, 0);
	wchar_t *wcs = ఖాళీ;

	ఒకవేళ (sz == (ssize_t) -1)
		వెళ్ళు పూర్తి;

	wcs = సున్నా_జ్ఞాపకం_ఇవ్వు(1, (sz + 1) * పరిమాణం(wchar_t));
	ఒకవేళ (!wcs)
		వెళ్ళు పూర్తి;

	ఒకవేళ (!mbstowcs(wcs, str, sz))
		వెళ్ళు పూర్తి;
	*వెడల్పు = wc_truncate(wcs, *వెడల్పు);
	బైట్లు = wcstombs(str, wcs, బైట్లు);
పూర్తి:
	విడుదల(wcs);
#లేకపోతే
	{
		స్థిరరాశి అక్షరం *p = str;
		స్థిరరాశి అక్షరం *చివర = str + బైట్లు;
		size_t cells = 0;

		అయితే (p < చివర) {
			స్థిరరాశి అక్షరం *after_escape = skip_ansi_escape(p, చివర);

			ఒకవేళ (after_escape) {
				p = after_escape;
				కొనసాగు;
			}
			ఒకవేళ (cells >= *వెడల్పు)
				ఆపు;
			p++;
			cells++;
		}
		బైట్లు = p - str;
		*వెడల్పు = cells;
	}
#అయితే_ముగింపు
	ఒకవేళ (బైట్లు >= 0)
		str[బైట్లు] = '\0';
	తిరిగి బైట్లు;
}

/* Write N_SPACES space characters to DEST while ensuring
   nothing is written beyond DEST_END. A terminating NUL
   is always added to DEST.
   A pointer to the terminating NUL is returned.  */

స్థిర అక్షరం*
mbs_align_pad (అక్షరం *dest, స్థిరరాశి అక్షరం* dest_చివర, size_t n_spaces, పూర్ణాంకం padchar)
{
  కొరకు (/* nothing */; n_spaces && (dest < dest_చివర); n_spaces--)
    *dest++ = padchar;
  *dest = '\0';
  తిరిగి dest;
}

size_t
mbsalign (స్థిరరాశి అక్షరం *src, అక్షరం *dest, size_t dest_కొలత,
          size_t *వెడల్పు, mbs_align_t align, పూర్ణాంకం flags)
{
	తిరిగి mbsalign_with_padding(src, dest, dest_కొలత, వెడల్పు, align, flags, ' ');
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
mbsalign_with_padding (స్థిరరాశి అక్షరం *src, అక్షరం *dest, size_t dest_కొలత,
	               size_t *వెడల్పు, mbs_align_t align,
#నిర్వచితమైతే HAVE_WIDECHAR
		       పూర్ణాంకం flags,
#లేకపోతే
		       పూర్ణాంకం flags __attribute__((__unused__)),
#అయితే_ముగింపు
		       పూర్ణాంకం padchar)
{
  size_t ret = -1;
  size_t src_కొలత = అక్షరశ్రేణి_పొడవు (src) + 1;
  అక్షరం *newstr = ఖాళీ;
  wchar_t *str_wc = ఖాళీ;
  స్థిరరాశి అక్షరం *str_గమ్యం_print = src;
  size_t n_నిలువుల_సంఖ్య = src_కొలత - 1;
  size_t n_వాడిన_బైట్లు = n_నిలువుల_సంఖ్య; /* Not including NUL */
  size_t n_spaces = 0, space_ఎడమ;

#నిర్వచితమైతే HAVE_WIDECHAR
  bool conversion = false;
  bool wc_enabled = false;

  /* In multi-byte locales convert to wide characters
     to allow easy truncation. Also determine number
     of screen columns used.  */
  ఒకవేళ (MB_CUR_MAX > 1)
    {
      size_t src_chars = mbstowcs (ఖాళీ, src, 0);
      ఒకవేళ (src_chars == (size_t) -1)
        {
          ఒకవేళ (flags & MBA_UNIBYTE_FALLBACK)
            వెళ్ళు mbsalign_unibyte;
          లేకపోతే
            వెళ్ళు mbsalign_cleanup;
        }
      src_chars += 1; /* make space for NUL */
      str_wc = జ్ఞాపకం_ఇవ్వు (src_chars * పరిమాణం (wchar_t));
      ఒకవేళ (str_wc == ఖాళీ)
        {
          ఒకవేళ (flags & MBA_UNIBYTE_FALLBACK)
            వెళ్ళు mbsalign_unibyte;
          లేకపోతే
            వెళ్ళు mbsalign_cleanup;
        }
      ఒకవేళ (mbstowcs (str_wc, src, src_chars) != 0)
        {
          str_wc[src_chars - 1] = L'\0';
          wc_enabled = true;
          conversion = wc_ensure_printable (str_wc);
          n_నిలువుల_సంఖ్య = rpl_wcswidth (str_wc, src_chars);
        }
    }

  /* If we transformed or need to truncate the source string
     then create a modified copy of it.  */
  ఒకవేళ (wc_enabled && (conversion || (n_నిలువుల_సంఖ్య > *వెడల్పు)))
    {
        ఒకవేళ (conversion)
          {
             /* May have increased the size by converting
                \t to \uFFFD for example.  */
            src_కొలత = wcstombs(ఖాళీ, str_wc, 0) + 1;
          }
        newstr = జ్ఞాపకం_ఇవ్వు (src_కొలత);
        ఒకవేళ (newstr == ఖాళీ)
        {
          ఒకవేళ (flags & MBA_UNIBYTE_FALLBACK)
            వెళ్ళు mbsalign_unibyte;
          లేకపోతే
            వెళ్ళు mbsalign_cleanup;
        }
        str_గమ్యం_print = newstr;
        n_నిలువుల_సంఖ్య = wc_truncate (str_wc, *వెడల్పు);
        n_వాడిన_బైట్లు = wcstombs (newstr, str_wc, src_కొలత);
    }

mbsalign_unibyte:
#అయితే_ముగింపు

  ఒకవేళ (n_నిలువుల_సంఖ్య > *వెడల్పు) /* Unibyte truncation required.  */
    {
      n_నిలువుల_సంఖ్య = *వెడల్పు;
      n_వాడిన_బైట్లు = n_నిలువుల_సంఖ్య;
    }

  ఒకవేళ (*వెడల్పు > n_నిలువుల_సంఖ్య) /* Padding required.  */
    n_spaces = *వెడల్పు - n_నిలువుల_సంఖ్య;

  /* indicate to caller how many cells needed (not including padding).  */
  *వెడల్పు = n_నిలువుల_సంఖ్య;

  /* indicate to caller how many bytes needed (not including NUL).  */
  ret = n_వాడిన_బైట్లు + (n_spaces * 1);

  /* Write as much NUL terminated output to DEST as possible.  */
  ఒకవేళ (dest_కొలత != 0)
    {
      అక్షరం *dest_చివర = dest + dest_కొలత - 1;
      size_t ప్రారంభం_spaces;
      size_t చివర_spaces;

      ఎంపిక (align)
        {
        స్థితి MBS_ALIGN_CENTER:
          ప్రారంభం_spaces = n_spaces / 2 + n_spaces % 2;
          చివర_spaces = n_spaces / 2;
          ఆపు;
        స్థితి MBS_ALIGN_ఎడమ:
          ప్రారంభం_spaces = 0;
          చివర_spaces = n_spaces;
          ఆపు;
        స్థితి MBS_ALIGN_కుడి:
          ప్రారంభం_spaces = n_spaces;
          చివర_spaces = 0;
          ఆపు;
	అప్రమేయ:
	  abort();
        }

      dest = mbs_align_pad (dest, dest_చివర, ప్రారంభం_spaces, padchar);
      space_ఎడమ = dest_చివర - dest;
      dest = mempcpy (dest, str_గమ్యం_print, min (n_వాడిన_బైట్లు, space_ఎడమ));
      mbs_align_pad (dest, dest_చివర, చివర_spaces, padchar);
    }
#నిర్వచితమైతే HAVE_WIDECHAR
mbsalign_cleanup:
#అయితే_ముగింపు
  విడుదల (str_wc);
  విడుదల (newstr);

  తిరిగి ret;
}
