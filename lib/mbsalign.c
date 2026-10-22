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
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <limits.h>
#अन्तर्भाव <ctype.h>

#अन्तर्भाव "c.h"
#अन्तर्भाव "mbsalign.h"
#अन्तर्भाव "strutils.h"
#अन्तर्भाव "widechar.h"

/* Return the byte after a complete ANSI escape sequence, or NULL. */
स्थिर ध्रुव वर्ण *skip_ansi_escape(ध्रुव वर्ण *p, ध्रुव वर्ण *end)
{
	ध्रुव वर्ण *q;

	जइ (p >= end || (unsigned वर्ण) *p != '\033' || p + 1 >= end)
		प्रत्यागम रिक्त;

	q = p + 1;
	जइ (*q == '[') {
		q++;
		/* CSI parameter bytes (0x30-0x3f) are followed by intermediate
		 * bytes (0x20-0x2f) and a final byte (0x40-0x7e). None of them
		 * occupies a cell.
		 */
		जाव (q < end && (unsigned वर्ण) *q >= 0x30
				&& (unsigned वर्ण) *q <= 0x3f)
			q++;
		जाव (q < end && (unsigned वर्ण) *q >= 0x20
				&& (unsigned वर्ण) *q <= 0x2f)
			q++;
		जइ (q < end && (unsigned वर्ण) *q >= 0x40
				&& (unsigned वर्ण) *q <= 0x7e)
			प्रत्यागम q + 1;
	} अण्णहा जइ (*q && strchr("()*+", *q) && q + 1 < end)
		प्रत्यागम q + 2;

	प्रत्यागम रिक्त;
}

/*
 * Counts number of cells in multibyte string. All control and
 * non-printable chars are ignored.
 *
 * Returns: number of cells.
 */
size_t mbs_nwidth(ध्रुव वर्ण *buf, size_t bufsz)
{
	ध्रुव वर्ण *p = buf, *last = buf;
	size_t width = 0;

#ifdef HAVE_WIDECHAR
	mbstate_t st;
	memset(&st, 0, परिमाण(st));
#endif
	जइ (p && *p && bufsz)
		last = p + (bufsz - 1);

	जाव (p && *p && p <= last) {
		जइ (*p == '\033') {
			ध्रुव वर्ण *e = skip_ansi_escape(p, last + 1);

			p = e ? e : p + 1;
			अनुवर्त;
		}
		जइ (iscntrl((unsigned वर्ण) *p)) {
			p++;
			अनुवर्त;
		}
#ifdef HAVE_WIDECHAR
		wchar_t wc;
		size_t len = mbrtowc(&wc, p, MB_CUR_MAX, &st);

		जइ (len == 0)
			भङ्ग;
		जइ (len > 0 && iswprint(wc)) {
			पूर्णाङ्क x = wcwidth(wc);
			जइ (x > 0)
				width += x;
		} अण्णहा जइ (len == (size_t) -1 || len == (size_t) -2)
			len = 1;
		p += len;
#अण्णहा
		जइ (isprint((unsigned वर्ण) *p))
			width++;
		p++;
#endif
	}

	प्रत्यागम width;
}

size_t mbs_width(ध्रुव वर्ण *s)
{
	जइ (!s || !*s)
		प्रत्यागम 0;
	प्रत्यागम mbs_nwidth(s, strlen(s));
}

/*
 * Counts number of cells in multibyte string. For all control and
 * non-printable chars is the result width enlarged to store \x?? hex
 * sequence. See mbs_safe_encode().
 *
 * Returns: number of cells, @sz returns number of bytes.
 */
size_t mbs_safe_nwidth(ध्रुव वर्ण *buf, size_t bufsz, size_t *sz)
{
	ध्रुव वर्ण *p = buf, *last = buf;
	size_t width = 0, bytes = 0;

#ifdef HAVE_WIDECHAR
	mbstate_t st;
	memset(&st, 0, परिमाण(st));
#endif
	जइ (p && *p && bufsz)
		last = p + (bufsz - 1);

	जाव (p && *p && p <= last) {
		जइ ((p < last && *p == '\\' && *(p + 1) == 'x')
		    || iscntrl((unsigned वर्ण) *p)) {
			width += 4, bytes += 4;		/* *p encoded to \x?? */
			p++;
		}
#ifdef HAVE_WIDECHAR
		अण्णहा {
			wchar_t wc;
			size_t len = mbrtowc(&wc, p, MB_CUR_MAX, &st);

			जइ (len == 0)
				भङ्ग;

			जइ (len == (size_t) -1 || len == (size_t) -2) {
				len = 1;
				जइ (isprint((unsigned वर्ण) *p))
					width += 1, bytes += 1;
				अण्णहा
					width += 4, bytes += 4;

			} अण्णहा जइ (!iswprint(wc)) {
				width += len * 4;	/* hex encode whole sequence */
				bytes += len * 4;
			} अण्णहा {
				width += wcwidth(wc);	/* number of cells */
				bytes += len;		/* number of bytes */
			}
			p += len;
		}
#अण्णहा
		अण्णहा जइ (!isprint((unsigned वर्ण) *p)) {
			width += 4, bytes += 4;		/* *p encoded to \x?? */
			p++;
		} अण्णहा {
			width++, bytes++;
			p++;
		}
#endif
	}

	जइ (sz)
		*sz = bytes;
	प्रत्यागम width;
}

size_t mbs_safe_width(ध्रुव वर्ण *s)
{
	जइ (!s || !*s)
		प्रत्यागम 0;
	प्रत्यागम mbs_safe_nwidth(s, strlen(s), रिक्त);
}

/*
 * Copy @s to @buf and replace control and non-printable chars with
 * \x?? hex sequence. The @width returns number of cells. The @safechars
 * are not encoded.
 *
 * The @buf has to be big enough to store mbs_safe_encode_size(strlen(s)))
 * bytes.
 */
वर्ण *mbs_safe_encode_to_buffer(ध्रुव वर्ण *s, size_t *width, वर्ण *buf, size_t bufsiz, ध्रुव वर्ण *safechars)
{
	ध्रुव वर्ण *p = s;
	वर्ण *r;
	पूर्णाङ्क rsz;
	size_t sz = s ? strlen(s) : 0;

#ifdef HAVE_WIDECHAR
	mbstate_t st;
	memset(&st, 0, परिमाण(st));
#endif
	जइ (!sz || !buf || !bufsiz)
		प्रत्यागम रिक्त;

	r = buf;
	rsz = (पूर्णाङ्क) bufsiz;
	*width = 0;

	जाव (p && *p) {
		जइ (safechars && strchr(safechars, *p)) {
			जइ (rsz < 2)
				भङ्ग;
			*r++ = *p++;
			rsz--;
			अनुवर्त;
		}

		जइ ((*p == '\\' && *(p + 1) == 'x')
		    || iscntrl((unsigned वर्ण) *p)) {
			जइ (rsz < 5)
				भङ्ग;
			snprintf(r, rsz, "\\x%02x", (unsigned वर्ण) *p);
			r += 4;
			rsz -= 4;
			*width += 4;
			p++;
		}
#ifdef HAVE_WIDECHAR
		अण्णहा {
			wchar_t wc;
			size_t len = mbrtowc(&wc, p, MB_CUR_MAX, &st);

			जइ (len == 0)
				भङ्ग;		/* end of string */

			जइ (len == (size_t) -1 || len == (size_t) -2) {
				len = 1;
				/*
				 * Not valid multibyte sequence -- maybe it's
				 * printable char according to the current locales.
				 */
				जइ (!isprint((unsigned वर्ण) *p)) {
					जइ (rsz < 5)
						भङ्ग;
					snprintf(r, rsz, "\\x%02x", (unsigned वर्ण) *p);
					r += 4;
					rsz -= 4;
					*width += 4;
				} अण्णहा {
					जइ (rsz < 2)
						भङ्ग;
					(*width)++;
					*r++ = *p;
					rsz--;
				}
			} अण्णहा जइ (!iswprint(wc)) {
				size_t i;
				प्रति (i = 0; i < len; i++) {
					जइ (rsz < 5)
						भङ्ग;
					snprintf(r, rsz, "\\x%02x", (unsigned वर्ण) p[i]);
					r += 4;
					rsz -= 4;
					*width += 4;
				}
				जइ (i < len)
					भङ्ग;
			} अण्णहा {
				जइ (rsz < (पूर्णाङ्क)len + 1)
					भङ्ग;
				memcpy(r, p, len);
				r += len;
				rsz -= len;
				*width += wcwidth(wc);
			}
			p += len;
		}
#अण्णहा
		अण्णहा जइ (!isprint((unsigned वर्ण) *p)) {
			जइ (rsz < 5)
				भङ्ग;
			snprintf(r, rsz, "\\x%02x", (unsigned वर्ण) *p);
			p++;
			r += 4;
			rsz -= 4;
			*width += 4;
		} अण्णहा {
			जइ (rsz < 2)
				भङ्ग;
			*r++ = *p++;
			rsz--;
			(*width)++;
		}
#endif
	}

	*r = '\0';
	प्रत्यागम buf;
}

/*
 * Copy @s to @buf and replace broken sequences to \x?? hex sequence. The
 * @width returns number of cells. The @safechars are not encoded.
 *
 * The @buf has to be big enough to store mbs_safe_encode_size(strlen(s)))
 * bytes.
 */
वर्ण *mbs_invalid_encode_to_buffer(ध्रुव वर्ण *s, size_t *width, वर्ण *buf, size_t bufsiz)
{
	ध्रुव वर्ण *p = s;
	वर्ण *r;
	पूर्णाङ्क rsz;
	size_t sz = s ? strlen(s) : 0;

#ifdef HAVE_WIDECHAR
	mbstate_t st;
	memset(&st, 0, परिमाण(st));
#endif
	जइ (!sz || !buf || !bufsiz)
		प्रत्यागम रिक्त;

	r = buf;
	rsz = (पूर्णाङ्क) bufsiz;
	*width = 0;

	जाव (p && *p) {
#ifdef HAVE_WIDECHAR
		wchar_t wc;
		size_t len = mbrtowc(&wc, p, MB_CUR_MAX, &st);
#अण्णहा
		size_t len = 1;
#endif

		जइ (len == 0)
			भङ्ग;		/* end of string */

		जइ (len == (size_t) -1 || len == (size_t) -2) {
			len = 1;
			/*
			 * Not valid multibyte sequence -- maybe it's
			 * printable char according to the current locales.
			 */
			जइ (!isprint((unsigned वर्ण) *p)) {
				जइ (rsz < 5)
					भङ्ग;
				snprintf(r, rsz, "\\x%02x", (unsigned वर्ण) *p);
				r += 4;
				rsz -= 4;
				*width += 4;
			} अण्णहा {
				जइ (rsz < 2)
					भङ्ग;
				(*width)++;
				*r++ = *p;
				rsz--;
			}
		} अण्णहा जइ (*p == '\\' && *(p + 1) == 'x') {
			जइ (rsz < 5)
				भङ्ग;
			snprintf(r, rsz, "\\x%02x", (unsigned वर्ण) *p);
			r += 4;
			rsz -= 4;
			*width += 4;
		} अण्णहा {
			जइ (rsz < (पूर्णाङ्क)len + 1)
				भङ्ग;
			r = mempcpy(r, p, len);
			rsz -= len;
			*width += wcwidth(wc);
		}
		p += len;
	}

	*r = '\0';
	प्रत्यागम buf;
}

/*
 * Guess size
 */
size_t mbs_safe_encode_size(size_t bytes)
{
	प्रत्यागम (bytes * 4) + 1;
}

/*
 * Count size of the original string in bytes (count \x?? as one byte)
 */
size_t mbs_safe_decode_size(ध्रुव वर्ण *p)
{
	size_t bytes = 0;

	जाव (p && *p) {
		जइ (*p == '\\' && *(p + 1) == 'x' &&
		    isxdigit(*(p + 2)) && isxdigit(*(p + 3)))
			p += 4;
		अण्णहा
			p++;
		bytes++;
	}
	प्रत्यागम bytes;
}

/*
 * Returns allocated string where all control and non-printable chars are
 * replaced with \x?? hex sequence.
 */
वर्ण *mbs_safe_encode(ध्रुव वर्ण *s, size_t *width)
{
	size_t sz = s ? strlen(s) : 0;
	size_t bufsz;
	वर्ण *buf, *ret = रिक्त;

	जइ (!sz)
		प्रत्यागम रिक्त;
	bufsz = mbs_safe_encode_size(sz);
	buf = malloc(bufsz);
	जइ (buf)
		ret = mbs_safe_encode_to_buffer(s, width, buf, bufsz, रिक्त);
	जइ (!ret)
		मुक्त(buf);
	प्रत्यागम ret;
}

/*
 * Returns allocated string where all broken widechars chars are
 * replaced with \x?? hex sequence.
 */
वर्ण *mbs_invalid_encode(ध्रुव वर्ण *s, size_t *width)
{
	size_t sz = s ? strlen(s) : 0;
	size_t bufsz;
	वर्ण *buf, *ret = रिक्त;

	जइ (!sz)
		प्रत्यागम रिक्त;
	bufsz = mbs_safe_encode_size(sz);
	buf = malloc(bufsz);
	जइ (buf)
		ret = mbs_invalid_encode_to_buffer(s, width, buf, bufsz);
	जइ (!ret)
		मुक्त(buf);
	प्रत्यागम ret;
}

#ifdef HAVE_WIDECHAR

स्थिर bool
wc_ensure_printable (wchar_t *wchars)
{
  bool replaced = false;
  wchar_t *wc = wchars;
  जाव (*wc)
    {
      जइ (!iswprint ((wint_t) *wc))
        {
          *wc = 0xFFFD; /* L'\uFFFD' (replacement char) */
          replaced = true;
        }
      wc++;
    }
  प्रत्यागम replaced;
}

/* Return the wchar after a complete ANSI escape sequence, or NULL. */
स्थिर wchar_t *
wc_skip_ansi_escape (wchar_t *wc)
{
  wchar_t *p = wc + 1;

  जइ (*wc != L'\033' || !*p)
    प्रत्यागम रिक्त;

  जइ (*p == L'[')
    {
      p++;
      जाव (*p >= 0x30 && *p <= 0x3f)
        p++;
      जाव (*p >= 0x20 && *p <= 0x2f)
        p++;
      जइ (*p >= 0x40 && *p <= 0x7e)
        प्रत्यागम p + 1;
    }
  अण्णहा जइ ((*p == L'(' || *p == L')' || *p == L'*' || *p == L'+') && p[1])
    प्रत्यागम p + 2;

  प्रत्यागम रिक्त;
}

/* Truncate wchar string to width cells.
 * Returns number of cells used.  */

स्थिर size_t
wc_truncate (wchar_t *wc, size_t width)
{
  size_t cells = 0;
  पूर्णाङ्क next_cells = 0;

  जाव (*wc)
    {
      wchar_t *after_escape = wc_skip_ansi_escape (wc);
      जइ (after_escape)
        {
          wc = after_escape;
          अनुवर्त;
        }

      next_cells = wcwidth (*wc);
      जइ (next_cells == -1) /* non printable */
        {
          *wc = 0xFFFD; /* L'\uFFFD' (replacement char) */
          next_cells = 1;
        }
      जइ (cells + next_cells > width)
        भङ्ग;

      cells += next_cells;
      wc++;
    }
  *wc = L'\0';
  प्रत्यागम cells;
}

स्थिर पूर्णाङ्क
rpl_wcswidth (ध्रुव wchar_t *s, size_t n)
{
  पूर्णाङ्क ret = 0;

  जाव (n-- > 0 && *s != L'\0')
    {
      पूर्णाङ्क nwidth = wcwidth (*s++);
      जइ (nwidth == -1)             /* non printable */
        प्रत्यागम -1;
      जइ (ret > (INT_MAX - nwidth)) /* overflow */
        प्रत्यागम -1;
      ret += nwidth;
    }

  प्रत्यागम ret;
}
#endif /* HAVE_WIDECHAR */

/* Truncate multi-byte string to @width and returns number of
 * bytes of the new string @str, and in @width returns number
 * of cells.
 */
size_t
mbs_truncate(वर्ण *str, size_t *width)
{
	ssize_t bytes = strlen(str);
#ifdef HAVE_WIDECHAR
	ssize_t sz = mbstowcs(रिक्त, str, 0);
	wchar_t *wcs = रिक्त;

	जइ (sz == (ssize_t) -1)
		गच्छ done;

	wcs = calloc(1, (sz + 1) * परिमाण(wchar_t));
	जइ (!wcs)
		गच्छ done;

	जइ (!mbstowcs(wcs, str, sz))
		गच्छ done;
	*width = wc_truncate(wcs, *width);
	bytes = wcstombs(str, wcs, bytes);
done:
	मुक्त(wcs);
#अण्णहा
	{
		ध्रुव वर्ण *p = str;
		ध्रुव वर्ण *end = str + bytes;
		size_t cells = 0;

		जाव (p < end) {
			ध्रुव वर्ण *after_escape = skip_ansi_escape(p, end);

			जइ (after_escape) {
				p = after_escape;
				अनुवर्त;
			}
			जइ (cells >= *width)
				भङ्ग;
			p++;
			cells++;
		}
		bytes = p - str;
		*width = cells;
	}
#endif
	जइ (bytes >= 0)
		str[bytes] = '\0';
	प्रत्यागम bytes;
}

/* Write N_SPACES space characters to DEST while ensuring
   nothing is written beyond DEST_END. A terminating NUL
   is always added to DEST.
   A pointer to the terminating NUL is returned.  */

स्थिर वर्ण*
mbs_align_pad (वर्ण *dest, ध्रुव वर्ण* dest_end, size_t n_spaces, पूर्णाङ्क padchar)
{
  प्रति (/* nothing */; n_spaces && (dest < dest_end); n_spaces--)
    *dest++ = padchar;
  *dest = '\0';
  प्रत्यागम dest;
}

size_t
mbsalign (ध्रुव वर्ण *src, वर्ण *dest, size_t dest_size,
          size_t *width, mbs_align_t align, पूर्णाङ्क flags)
{
	प्रत्यागम mbsalign_with_padding(src, dest, dest_size, width, align, flags, ' ');
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
mbsalign_with_padding (ध्रुव वर्ण *src, वर्ण *dest, size_t dest_size,
	               size_t *width, mbs_align_t align,
#ifdef HAVE_WIDECHAR
		       पूर्णाङ्क flags,
#अण्णहा
		       पूर्णाङ्क flags __attribute__((__unused__)),
#endif
		       पूर्णाङ्क padchar)
{
  size_t ret = -1;
  size_t src_size = strlen (src) + 1;
  वर्ण *newstr = रिक्त;
  wchar_t *str_wc = रिक्त;
  ध्रुव वर्ण *str_to_print = src;
  size_t n_cols = src_size - 1;
  size_t n_used_bytes = n_cols; /* Not including NUL */
  size_t n_spaces = 0, space_वामः;

#ifdef HAVE_WIDECHAR
  bool conversion = false;
  bool wc_enabled = false;

  /* In multi-byte locales convert to wide characters
     to allow easy truncation. Also determine number
     of screen columns used.  */
  जइ (MB_CUR_MAX > 1)
    {
      size_t src_chars = mbstowcs (रिक्त, src, 0);
      जइ (src_chars == (size_t) -1)
        {
          जइ (flags & MBA_UNIBYTE_FALLBACK)
            गच्छ mbsalign_unibyte;
          अण्णहा
            गच्छ mbsalign_cleanup;
        }
      src_chars += 1; /* make space for NUL */
      str_wc = malloc (src_chars * परिमाण (wchar_t));
      जइ (str_wc == रिक्त)
        {
          जइ (flags & MBA_UNIBYTE_FALLBACK)
            गच्छ mbsalign_unibyte;
          अण्णहा
            गच्छ mbsalign_cleanup;
        }
      जइ (mbstowcs (str_wc, src, src_chars) != 0)
        {
          str_wc[src_chars - 1] = L'\0';
          wc_enabled = true;
          conversion = wc_ensure_printable (str_wc);
          n_cols = rpl_wcswidth (str_wc, src_chars);
        }
    }

  /* If we transformed or need to truncate the source string
     then create a modified copy of it.  */
  जइ (wc_enabled && (conversion || (n_cols > *width)))
    {
        जइ (conversion)
          {
             /* May have increased the size by converting
                \t to \uFFFD for example.  */
            src_size = wcstombs(रिक्त, str_wc, 0) + 1;
          }
        newstr = malloc (src_size);
        जइ (newstr == रिक्त)
        {
          जइ (flags & MBA_UNIBYTE_FALLBACK)
            गच्छ mbsalign_unibyte;
          अण्णहा
            गच्छ mbsalign_cleanup;
        }
        str_to_print = newstr;
        n_cols = wc_truncate (str_wc, *width);
        n_used_bytes = wcstombs (newstr, str_wc, src_size);
    }

mbsalign_unibyte:
#endif

  जइ (n_cols > *width) /* Unibyte truncation required.  */
    {
      n_cols = *width;
      n_used_bytes = n_cols;
    }

  जइ (*width > n_cols) /* Padding required.  */
    n_spaces = *width - n_cols;

  /* indicate to caller how many cells needed (not including padding).  */
  *width = n_cols;

  /* indicate to caller how many bytes needed (not including NUL).  */
  ret = n_used_bytes + (n_spaces * 1);

  /* Write as much NUL terminated output to DEST as possible.  */
  जइ (dest_size != 0)
    {
      वर्ण *dest_end = dest + dest_size - 1;
      size_t start_spaces;
      size_t end_spaces;

      विकल्प (align)
        {
        पक्ष MBS_ALIGN_CENTER:
          start_spaces = n_spaces / 2 + n_spaces % 2;
          end_spaces = n_spaces / 2;
          भङ्ग;
        पक्ष MBS_ALIGN_वामः:
          start_spaces = 0;
          end_spaces = n_spaces;
          भङ्ग;
        पक्ष MBS_ALIGN_दक्षिणः:
          start_spaces = n_spaces;
          end_spaces = 0;
          भङ्ग;
	स्वभाव:
	  abort();
        }

      dest = mbs_align_pad (dest, dest_end, start_spaces, padchar);
      space_वामः = dest_end - dest;
      dest = mempcpy (dest, str_to_print, min (n_used_bytes, space_वामः));
      mbs_align_pad (dest, dest_end, end_spaces, padchar);
    }
#ifdef HAVE_WIDECHAR
mbsalign_cleanup:
#endif
  मुक्त (str_wc);
  मुक्त (newstr);

  प्रत्यागम ret;
}
