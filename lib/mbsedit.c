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
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

#include "mbsalign.h"
#include "mbsedit.h"

struct mbs_editor *mbs_new_edit(char *buf, size_t bufsz, size_t ncells)
{
	struct mbs_editor *edit = calloc(1, sizeof(*edit));

	nếu (edit) {
		edit->buf = buf;
		edit->max_bytes = bufsz;
		edit->max_cells = ncells;
		edit->cur_cells = mbs_safe_width(buf);
		edit->cur_bytes = strlen(buf);
	}
	trả_về edit;
}

char *mbs_free_edit(struct mbs_editor *edit)
{
	char *ret = edit ? edit->buf : NULL;

	free(edit);
	trả_về ret;
}

static size_t mbs_next(const char *str, size_t *ncells)
{
#ifdef HAVE_WIDECHAR
	wchar_t wc;
	size_t n = 0;

	nếu (!str || !*str)
		trả_về 0;

	n = mbrtowc(&wc, str, MB_CUR_MAX, NULL);
	*ncells = wcwidth(wc);
	trả_về n;
#không_thì
	nếu (!str || !*str)
		trả_về 0;
	*ncells = 1;
	trả_về 1;
#endif
}

static size_t mbs_prev(const char *start, const char *end, size_t *ncells)
{
#ifdef HAVE_WIDECHAR
	wchar_t wc = 0;
	const char *p, *prev;
	size_t n = 0;

	nếu (!start || !end || start == end || !*start)
		trả_về 0;

	prev = p = start;
	trong_khi (p < end) {
		n = mbrtowc(&wc, p, MB_CUR_MAX, NULL);
		prev = p;

		nếu (n == (size_t) -1 || n == (size_t) -2)
			p++;
		không_thì
			p += n;
	}

	nếu (prev == end)
		trả_về 0;
	*ncells = wcwidth(wc);
	trả_về n;
#không_thì
	nếu (!start || !end || start == end || !*start)
		trả_về 0;
	*ncells = 1;
	trả_về 1;
#endif
}

số_nguyên mbs_edit_goto(struct mbs_editor *edit, số_nguyên where)
{
	switch (where) {
	case MBS_EDIT_LEFT:
		nếu (edit->cursor == 0)
			trả_về 1;
		không_thì {
			size_t n, cells;
			n = mbs_prev(edit->buf, edit->buf + edit->cursor, &cells);
			nếu (n) {
				edit->cursor -= n;
				edit->cursor_cells -= cells;
			}
		}
		break;
	case MBS_EDIT_RIGHT:
		nếu (edit->cursor_cells >= edit->cur_cells)
			trả_về 1;
		không_thì {
			size_t n, cells;
			n = mbs_next(edit->buf + edit->cursor, &cells);
			nếu (n) {
				edit->cursor += n;
				edit->cursor_cells += cells;
			}
		}
		break;
	case MBS_EDIT_HOME:
		edit->cursor = 0;
		edit->cursor_cells = 0;
		break;
	case MBS_EDIT_END:
		edit->cursor = edit->cur_bytes;
		edit->cursor_cells = edit->cur_cells;
		break;
	default:
		trả_về -EINVAL;
	}

	trả_về 0;
}

/* Remove next MB from @str, returns number of removed bytes */
static size_t remove_next(char *str, size_t *ncells)
{
	/* all in bytes! */
	size_t bytes, move_bytes, n;

	n          = mbs_next(str, ncells);
	bytes      = strlen(str);
	move_bytes = bytes - n;

	memmove(str, str + n, move_bytes);
	str[bytes - n] = '\0';
	trả_về n;
}

static size_t mbs_insert(char *str, wint_t c, size_t *ncells)
{
	/* all in bytes! */
	size_t n = 1, bytes;
	char *in;

#ifdef HAVE_WIDECHAR
	wchar_t wc = (wchar_t) c;
	in = malloc(MB_CUR_MAX);
	nếu (!in)
		trả_về -1;

	n = wctomb(in, wc);
	nếu (n == (size_t) -1)
		goto out;
	*ncells = wcwidth(wc);
#không_thì
	*ncells = 1;
	in = (char *) &c;
#endif
	bytes       = strlen(str);

	memmove(str + n, str, bytes);
	memcpy(str, in, n);
	str[bytes + n] = '\0';
#ifdef HAVE_WIDECHAR
out:
	free(in);
#endif
	trả_về n;
}

static số_nguyên mbs_edit_remove(struct mbs_editor *edit)
{
	size_t n, ncells;

	nếu (edit->cur_cells == 0 || edit->cursor >= edit->cur_bytes)
		trả_về 1;

	n = remove_next(edit->buf + edit->cursor, &ncells);
	nếu (n == (size_t)-1)
		trả_về 1;

	edit->cur_bytes -= n;
	edit->cur_cells = mbs_safe_width(edit->buf);
	trả_về 0;
}

số_nguyên mbs_edit_delete(struct mbs_editor *edit)
{
	nếu (edit->cursor >= edit->cur_bytes
	    && mbs_edit_goto(edit, MBS_EDIT_LEFT) == 1)
		trả_về 1;

	trả_về mbs_edit_remove(edit);
}

số_nguyên mbs_edit_backspace(struct mbs_editor *edit)
{
	nếu (mbs_edit_goto(edit, MBS_EDIT_LEFT) == 0)
		trả_về mbs_edit_remove(edit);
	trả_về 1;
}

số_nguyên mbs_edit_insert(struct mbs_editor *edit, wint_t c)
{
	size_t n, ncells;

	nếu (edit->cur_bytes + MB_CUR_MAX > edit->max_bytes)
		trả_về 1;

	n = mbs_insert(edit->buf + edit->cursor, c, &ncells);
	nếu (n == (size_t)-1)
		trả_về 1;

	edit->cursor += n;
	edit->cursor_cells += ncells;
	edit->cur_bytes += n;
	edit->cur_cells = mbs_safe_width(edit->buf);
	trả_về 0;
}
