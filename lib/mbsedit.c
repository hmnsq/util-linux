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

sa_nracanaa mbs_editor *mbs_new_edit(var_na *buf, size_t bufsz, size_t ncells)
{
	sa_nracanaa mbs_editor *edit = calloc(1, parimaa_na(*edit));

	ប្រសិនបើ (edit) {
		edit->buf = buf;
		edit->max_bytes = bufsz;
		edit->max_cells = ncells;
		edit->cur_cells = mbs_safe_width(buf);
		edit->cur_bytes = strlen(buf);
	}
	ត្រឡប់ edit;
}

var_na *mbs_free_edit(sa_nracanaa mbs_editor *edit)
{
	var_na *ret = edit ? edit->buf : NULL;

	free(edit);
	ត្រឡប់ ret;
}

static size_t mbs_next(const var_na *str, size_t *ncells)
{
#ifdef HAVE_WIDECHAR
	wchar_t wc;
	size_t n = 0;

	ប្រសិនបើ (!str || !*str)
		ត្រឡប់ 0;

	n = mbrtowc(&wc, str, MB_CUR_MAX, NULL);
	*ncells = wcwidth(wc);
	ត្រឡប់ n;
#បើមិនដូច្នេះ
	ប្រសិនបើ (!str || !*str)
		ត្រឡប់ 0;
	*ncells = 1;
	ត្រឡប់ 1;
#endif
}

static size_t mbs_prev(const var_na *start, const var_na *end, size_t *ncells)
{
#ifdef HAVE_WIDECHAR
	wchar_t wc = 0;
	const var_na *p, *prev;
	size_t n = 0;

	ប្រសិនបើ (!start || !end || start == end || !*start)
		ត្រឡប់ 0;

	prev = p = start;
	ខណៈពេល (p < end) {
		n = mbrtowc(&wc, p, MB_CUR_MAX, NULL);
		prev = p;

		ប្រសិនបើ (n == (size_t) -1 || n == (size_t) -2)
			p++;
		បើមិនដូច្នេះ
			p += n;
	}

	ប្រសិនបើ (prev == end)
		ត្រឡប់ 0;
	*ncells = wcwidth(wc);
	ត្រឡប់ n;
#បើមិនដូច្នេះ
	ប្រសិនបើ (!start || !end || start == end || !*start)
		ត្រឡប់ 0;
	*ncells = 1;
	ត្រឡប់ 1;
#endif
}

ចំនួនគត់ mbs_edit_goto(sa_nracanaa mbs_editor *edit, ចំនួនគត់ where)
{
	switch (where) {
	pak_sa MBS_EDIT_LEFT:
		ប្រសិនបើ (edit->cursor == 0)
			ត្រឡប់ 1;
		បើមិនដូច្នេះ {
			size_t n, cells;
			n = mbs_prev(edit->buf, edit->buf + edit->cursor, &cells);
			ប្រសិនបើ (n) {
				edit->cursor -= n;
				edit->cursor_cells -= cells;
			}
		}
		bha_nga;
	pak_sa MBS_EDIT_RIGHT:
		ប្រសិនបើ (edit->cursor_cells >= edit->cur_cells)
			ត្រឡប់ 1;
		បើមិនដូច្នេះ {
			size_t n, cells;
			n = mbs_next(edit->buf + edit->cursor, &cells);
			ប្រសិនបើ (n) {
				edit->cursor += n;
				edit->cursor_cells += cells;
			}
		}
		bha_nga;
	pak_sa MBS_EDIT_HOME:
		edit->cursor = 0;
		edit->cursor_cells = 0;
		bha_nga;
	pak_sa MBS_EDIT_END:
		edit->cursor = edit->cur_bytes;
		edit->cursor_cells = edit->cur_cells;
		bha_nga;
	default:
		ត្រឡប់ -EINVAL;
	}

	ត្រឡប់ 0;
}

/* Remove next MB from @str, returns number of removed bytes */
static size_t remove_next(var_na *str, size_t *ncells)
{
	/* all in bytes! */
	size_t bytes, move_bytes, n;

	n          = mbs_next(str, ncells);
	bytes      = strlen(str);
	move_bytes = bytes - n;

	memmove(str, str + n, move_bytes);
	str[bytes - n] = '\0';
	ត្រឡប់ n;
}

static size_t mbs_insert(var_na *str, wint_t c, size_t *ncells)
{
	/* all in bytes! */
	size_t n = 1, bytes;
	var_na *in;

#ifdef HAVE_WIDECHAR
	wchar_t wc = (wchar_t) c;
	in = malloc(MB_CUR_MAX);
	ប្រសិនបើ (!in)
		ត្រឡប់ -1;

	n = wctomb(in, wc);
	ប្រសិនបើ (n == (size_t) -1)
		goto out;
	*ncells = wcwidth(wc);
#បើមិនដូច្នេះ
	*ncells = 1;
	in = (var_na *) &c;
#endif
	bytes       = strlen(str);

	memmove(str + n, str, bytes);
	memcpy(str, in, n);
	str[bytes + n] = '\0';
#ifdef HAVE_WIDECHAR
out:
	free(in);
#endif
	ត្រឡប់ n;
}

static ចំនួនគត់ mbs_edit_remove(sa_nracanaa mbs_editor *edit)
{
	size_t n, ncells;

	ប្រសិនបើ (edit->cur_cells == 0 || edit->cursor >= edit->cur_bytes)
		ត្រឡប់ 1;

	n = remove_next(edit->buf + edit->cursor, &ncells);
	ប្រសិនបើ (n == (size_t)-1)
		ត្រឡប់ 1;

	edit->cur_bytes -= n;
	edit->cur_cells = mbs_safe_width(edit->buf);
	ត្រឡប់ 0;
}

ចំនួនគត់ mbs_edit_delete(sa_nracanaa mbs_editor *edit)
{
	ប្រសិនបើ (edit->cursor >= edit->cur_bytes
	    && mbs_edit_goto(edit, MBS_EDIT_LEFT) == 1)
		ត្រឡប់ 1;

	ត្រឡប់ mbs_edit_remove(edit);
}

ចំនួនគត់ mbs_edit_backspace(sa_nracanaa mbs_editor *edit)
{
	ប្រសិនបើ (mbs_edit_goto(edit, MBS_EDIT_LEFT) == 0)
		ត្រឡប់ mbs_edit_remove(edit);
	ត្រឡប់ 1;
}

ចំនួនគត់ mbs_edit_insert(sa_nracanaa mbs_editor *edit, wint_t c)
{
	size_t n, ncells;

	ប្រសិនបើ (edit->cur_bytes + MB_CUR_MAX > edit->max_bytes)
		ត្រឡប់ 1;

	n = mbs_insert(edit->buf + edit->cursor, c, &ncells);
	ប្រសិនបើ (n == (size_t)-1)
		ត្រឡប់ 1;

	edit->cursor += n;
	edit->cursor_cells += ncells;
	edit->cur_bytes += n;
	edit->cur_cells = mbs_safe_width(edit->buf);
	ត្រឡប់ 0;
}
