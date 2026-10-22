/*
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Very simple multibyte buffer editor. Allows to maintain the current
 * position in the string, add and remove chars on the current position.
 *
 * This file may be distributed under the terms of the
 * GNU Lesser General Public License.
 *
 * Copyright (C) 2017 Karel Zak <kzak@redhat.com>
 */
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <errno.h>
#ଅନ୍ତର୍ଭାଵ <stdio.h>

#ଅନ୍ତର୍ଭାଵ "mbsalign.h"
#ଅନ୍ତର୍ଭାଵ "mbsedit.h"

ସଂରଚନା mbs_editor *mbs_new_edit(ଅକ୍ଷର *buf, size_t bufsz, size_t ncells)
{
	ସଂରଚନା mbs_editor *edit = calloc(1, ଆକାର(*edit));

	ଯଦି (edit) {
		edit->buf = buf;
		edit->max_bytes = bufsz;
		edit->max_cells = ncells;
		edit->cur_cells = mbs_safe_width(buf);
		edit->cur_bytes = strlen(buf);
	}
	ଫେରନ୍ତୁ edit;
}

ଅକ୍ଷର *mbs_free_edit(ସଂରଚନା mbs_editor *edit)
{
	ଅକ୍ଷର *ret = edit ? edit->buf : ଖାଲି;

	ମୁକ୍ତ(edit);
	ଫେରନ୍ତୁ ret;
}

ସ୍ଥିର size_t mbs_next(ଅଚଳ ଅକ୍ଷର *str, size_t *ncells)
{
#ifdef HAVE_WIDECHAR
	wchar_t wc;
	size_t n = 0;

	ଯଦି (!str || !*str)
		ଫେରନ୍ତୁ 0;

	n = mbrtowc(&wc, str, MB_CUR_MAX, ଖାଲି);
	*ncells = wcwidth(wc);
	ଫେରନ୍ତୁ n;
#ଅନ୍ୟଥା
	ଯଦି (!str || !*str)
		ଫେରନ୍ତୁ 0;
	*ncells = 1;
	ଫେରନ୍ତୁ 1;
#endif
}

ସ୍ଥିର size_t mbs_prev(ଅଚଳ ଅକ୍ଷର *start, ଅଚଳ ଅକ୍ଷର *end, size_t *ncells)
{
#ifdef HAVE_WIDECHAR
	wchar_t wc = 0;
	ଅଚଳ ଅକ୍ଷର *p, *prev;
	size_t n = 0;

	ଯଦି (!start || !end || start == end || !*start)
		ଫେରନ୍ତୁ 0;

	prev = p = start;
	ଯେତେବେଳେ (p < end) {
		n = mbrtowc(&wc, p, MB_CUR_MAX, ଖାଲି);
		prev = p;

		ଯଦି (n == (size_t) -1 || n == (size_t) -2)
			p++;
		ଅନ୍ୟଥା
			p += n;
	}

	ଯଦି (prev == end)
		ଫେରନ୍ତୁ 0;
	*ncells = wcwidth(wc);
	ଫେରନ୍ତୁ n;
#ଅନ୍ୟଥା
	ଯଦି (!start || !end || start == end || !*start)
		ଫେରନ୍ତୁ 0;
	*ncells = 1;
	ଫେରନ୍ତୁ 1;
#endif
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା mbs_edit_goto(ସଂରଚନା mbs_editor *edit, ପୂର୍ଣ୍ଣସଂଖ୍ୟା where)
{
	ବାଛ (where) {
	ସ୍ଥିତି MBS_EDIT_ବାମ:
		ଯଦି (edit->cursor == 0)
			ଫେରନ୍ତୁ 1;
		ଅନ୍ୟଥା {
			size_t n, cells;
			n = mbs_prev(edit->buf, edit->buf + edit->cursor, &cells);
			ଯଦି (n) {
				edit->cursor -= n;
				edit->cursor_cells -= cells;
			}
		}
		ଭାଙ୍ଗ;
	ସ୍ଥିତି MBS_EDIT_ଡାହାଣ:
		ଯଦି (edit->cursor_cells >= edit->cur_cells)
			ଫେରନ୍ତୁ 1;
		ଅନ୍ୟଥା {
			size_t n, cells;
			n = mbs_next(edit->buf + edit->cursor, &cells);
			ଯଦି (n) {
				edit->cursor += n;
				edit->cursor_cells += cells;
			}
		}
		ଭାଙ୍ଗ;
	ସ୍ଥିତି MBS_EDIT_HOME:
		edit->cursor = 0;
		edit->cursor_cells = 0;
		ଭାଙ୍ଗ;
	ସ୍ଥିତି MBS_EDIT_END:
		edit->cursor = edit->cur_bytes;
		edit->cursor_cells = edit->cur_cells;
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଫେରନ୍ତୁ -EINVAL;
	}

	ଫେରନ୍ତୁ 0;
}

/* Remove next MB from @str, returns number of removed bytes */
ସ୍ଥିର size_t remove_next(ଅକ୍ଷର *str, size_t *ncells)
{
	/* all in bytes! */
	size_t bytes, move_bytes, n;

	n          = mbs_next(str, ncells);
	bytes      = strlen(str);
	move_bytes = bytes - n;

	memmove(str, str + n, move_bytes);
	str[bytes - n] = '\0';
	ଫେରନ୍ତୁ n;
}

ସ୍ଥିର size_t mbs_ଭର୍ତ୍ତି(ଅକ୍ଷର *str, wint_t c, size_t *ncells)
{
	/* all in bytes! */
	size_t n = 1, bytes;
	ଅକ୍ଷର *in;

#ifdef HAVE_WIDECHAR
	wchar_t wc = (wchar_t) c;
	in = malloc(MB_CUR_MAX);
	ଯଦି (!in)
		ଫେରନ୍ତୁ -1;

	n = wctomb(in, wc);
	ଯଦି (n == (size_t) -1)
		ଯାଅ out;
	*ncells = wcwidth(wc);
#ଅନ୍ୟଥା
	*ncells = 1;
	in = (ଅକ୍ଷର *) &c;
#endif
	bytes       = strlen(str);

	memmove(str + n, str, bytes);
	memcpy(str, in, n);
	str[bytes + n] = '\0';
#ifdef HAVE_WIDECHAR
out:
	ମୁକ୍ତ(in);
#endif
	ଫେରନ୍ତୁ n;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା mbs_edit_remove(ସଂରଚନା mbs_editor *edit)
{
	size_t n, ncells;

	ଯଦି (edit->cur_cells == 0 || edit->cursor >= edit->cur_bytes)
		ଫେରନ୍ତୁ 1;

	n = remove_next(edit->buf + edit->cursor, &ncells);
	ଯଦି (n == (size_t)-1)
		ଫେରନ୍ତୁ 1;

	edit->cur_bytes -= n;
	edit->cur_cells = mbs_safe_width(edit->buf);
	ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା mbs_edit_delete(ସଂରଚନା mbs_editor *edit)
{
	ଯଦି (edit->cursor >= edit->cur_bytes
	    && mbs_edit_goto(edit, MBS_EDIT_ବାମ) == 1)
		ଫେରନ୍ତୁ 1;

	ଫେରନ୍ତୁ mbs_edit_remove(edit);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା mbs_edit_backspace(ସଂରଚନା mbs_editor *edit)
{
	ଯଦି (mbs_edit_goto(edit, MBS_EDIT_ବାମ) == 0)
		ଫେରନ୍ତୁ mbs_edit_remove(edit);
	ଫେରନ୍ତୁ 1;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା mbs_edit_ଭର୍ତ୍ତି(ସଂରଚନା mbs_editor *edit, wint_t c)
{
	size_t n, ncells;

	ଯଦି (edit->cur_bytes + MB_CUR_MAX > edit->max_bytes)
		ଫେରନ୍ତୁ 1;

	n = mbs_ଭର୍ତ୍ତି(edit->buf + edit->cursor, c, &ncells);
	ଯଦି (n == (size_t)-1)
		ଫେରନ୍ତୁ 1;

	edit->cursor += n;
	edit->cursor_cells += ncells;
	edit->cur_bytes += n;
	edit->cur_cells = mbs_safe_width(edit->buf);
	ଫେରନ୍ତୁ 0;
}
