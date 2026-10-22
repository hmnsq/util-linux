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

	если (edit) {
		edit->buf = buf;
		edit->max_bytes = bufsz;
		edit->max_cells = ncells;
		edit->cur_cells = mbs_safe_width(buf);
		edit->cur_bytes = strlen(buf);
	}
	вернуть edit;
}

char *mbs_free_edit(struct mbs_editor *edit)
{
	char *ret = edit ? edit->buf : NULL;

	free(edit);
	вернуть ret;
}

static size_t mbs_next(const char *str, size_t *ncells)
{
#ifdef HAVE_WIDECHAR
	wchar_t wc;
	size_t n = 0;

	если (!str || !*str)
		вернуть 0;

	n = mbrtowc(&wc, str, MB_CUR_MAX, NULL);
	*ncells = wcwidth(wc);
	вернуть n;
#иначе
	если (!str || !*str)
		вернуть 0;
	*ncells = 1;
	вернуть 1;
#endif
}

static size_t mbs_prev(const char *start, const char *end, size_t *ncells)
{
#ifdef HAVE_WIDECHAR
	wchar_t wc = 0;
	const char *p, *prev;
	size_t n = 0;

	если (!start || !end || start == end || !*start)
		вернуть 0;

	prev = p = start;
	пока (p < end) {
		n = mbrtowc(&wc, p, MB_CUR_MAX, NULL);
		prev = p;

		если (n == (size_t) -1 || n == (size_t) -2)
			p++;
		иначе
			p += n;
	}

	если (prev == end)
		вернуть 0;
	*ncells = wcwidth(wc);
	вернуть n;
#иначе
	если (!start || !end || start == end || !*start)
		вернуть 0;
	*ncells = 1;
	вернуть 1;
#endif
}

целое mbs_edit_goto(struct mbs_editor *edit, целое where)
{
	switch (where) {
	case MBS_EDIT_LEFT:
		если (edit->cursor == 0)
			вернуть 1;
		иначе {
			size_t n, cells;
			n = mbs_prev(edit->buf, edit->buf + edit->cursor, &cells);
			если (n) {
				edit->cursor -= n;
				edit->cursor_cells -= cells;
			}
		}
		break;
	case MBS_EDIT_RIGHT:
		если (edit->cursor_cells >= edit->cur_cells)
			вернуть 1;
		иначе {
			size_t n, cells;
			n = mbs_next(edit->buf + edit->cursor, &cells);
			если (n) {
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
		вернуть -EINVAL;
	}

	вернуть 0;
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
	вернуть n;
}

static size_t mbs_insert(char *str, wint_t c, size_t *ncells)
{
	/* all in bytes! */
	size_t n = 1, bytes;
	char *in;

#ifdef HAVE_WIDECHAR
	wchar_t wc = (wchar_t) c;
	in = malloc(MB_CUR_MAX);
	если (!in)
		вернуть -1;

	n = wctomb(in, wc);
	если (n == (size_t) -1)
		goto out;
	*ncells = wcwidth(wc);
#иначе
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
	вернуть n;
}

static целое mbs_edit_remove(struct mbs_editor *edit)
{
	size_t n, ncells;

	если (edit->cur_cells == 0 || edit->cursor >= edit->cur_bytes)
		вернуть 1;

	n = remove_next(edit->buf + edit->cursor, &ncells);
	если (n == (size_t)-1)
		вернуть 1;

	edit->cur_bytes -= n;
	edit->cur_cells = mbs_safe_width(edit->buf);
	вернуть 0;
}

целое mbs_edit_delete(struct mbs_editor *edit)
{
	если (edit->cursor >= edit->cur_bytes
	    && mbs_edit_goto(edit, MBS_EDIT_LEFT) == 1)
		вернуть 1;

	вернуть mbs_edit_remove(edit);
}

целое mbs_edit_backspace(struct mbs_editor *edit)
{
	если (mbs_edit_goto(edit, MBS_EDIT_LEFT) == 0)
		вернуть mbs_edit_remove(edit);
	вернуть 1;
}

целое mbs_edit_insert(struct mbs_editor *edit, wint_t c)
{
	size_t n, ncells;

	если (edit->cur_bytes + MB_CUR_MAX > edit->max_bytes)
		вернуть 1;

	n = mbs_insert(edit->buf + edit->cursor, c, &ncells);
	если (n == (size_t)-1)
		вернуть 1;

	edit->cursor += n;
	edit->cursor_cells += ncells;
	edit->cur_bytes += n;
	edit->cur_cells = mbs_safe_width(edit->buf);
	вернуть 0;
}
