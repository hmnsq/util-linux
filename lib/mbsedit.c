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

	ပေႃး (edit) {
		edit->buf = buf;
		edit->max_bytes = bufsz;
		edit->max_cells = ncells;
		edit->cur_cells = mbs_safe_width(buf);
		edit->cur_bytes = strlen(buf);
	}
	ပွၵ်ႈ edit;
}

var_na *mbs_free_edit(sa_nracanaa mbs_editor *edit)
{
	var_na *ret = edit ? edit->buf : NULL;

	free(edit);
	ပွၵ်ႈ ret;
}

static size_t mbs_next(const var_na *str, size_t *ncells)
{
#ifdef HAVE_WIDECHAR
	wchar_t wc;
	size_t n = 0;

	ပေႃး (!str || !*str)
		ပွၵ်ႈ 0;

	n = mbrtowc(&wc, str, MB_CUR_MAX, NULL);
	*ncells = wcwidth(wc);
	ပွၵ်ႈ n;
#ပေႃးဢမ်ႇ
	ပေႃး (!str || !*str)
		ပွၵ်ႈ 0;
	*ncells = 1;
	ပွၵ်ႈ 1;
#endif
}

static size_t mbs_prev(const var_na *start, const var_na *end, size_t *ncells)
{
#ifdef HAVE_WIDECHAR
	wchar_t wc = 0;
	const var_na *p, *prev;
	size_t n = 0;

	ပေႃး (!start || !end || start == end || !*start)
		ပွၵ်ႈ 0;

	prev = p = start;
	ၶၢဝ်း (p < end) {
		n = mbrtowc(&wc, p, MB_CUR_MAX, NULL);
		prev = p;

		ပေႃး (n == (size_t) -1 || n == (size_t) -2)
			p++;
		ပေႃးဢမ်ႇ
			p += n;
	}

	ပေႃး (prev == end)
		ပွၵ်ႈ 0;
	*ncells = wcwidth(wc);
	ပွၵ်ႈ n;
#ပေႃးဢမ်ႇ
	ပေႃး (!start || !end || start == end || !*start)
		ပွၵ်ႈ 0;
	*ncells = 1;
	ပွၵ်ႈ 1;
#endif
}

puur_naa_nka mbs_edit_goto(sa_nracanaa mbs_editor *edit, puur_naa_nka where)
{
	switch (where) {
	pak_sa MBS_EDIT_LEFT:
		ပေႃး (edit->cursor == 0)
			ပွၵ်ႈ 1;
		ပေႃးဢမ်ႇ {
			size_t n, cells;
			n = mbs_prev(edit->buf, edit->buf + edit->cursor, &cells);
			ပေႃး (n) {
				edit->cursor -= n;
				edit->cursor_cells -= cells;
			}
		}
		bha_nga;
	pak_sa MBS_EDIT_RIGHT:
		ပေႃး (edit->cursor_cells >= edit->cur_cells)
			ပွၵ်ႈ 1;
		ပေႃးဢမ်ႇ {
			size_t n, cells;
			n = mbs_next(edit->buf + edit->cursor, &cells);
			ပေႃး (n) {
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
		ပွၵ်ႈ -EINVAL;
	}

	ပွၵ်ႈ 0;
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
	ပွၵ်ႈ n;
}

static size_t mbs_insert(var_na *str, wint_t c, size_t *ncells)
{
	/* all in bytes! */
	size_t n = 1, bytes;
	var_na *in;

#ifdef HAVE_WIDECHAR
	wchar_t wc = (wchar_t) c;
	in = malloc(MB_CUR_MAX);
	ပေႃး (!in)
		ပွၵ်ႈ -1;

	n = wctomb(in, wc);
	ပေႃး (n == (size_t) -1)
		goto out;
	*ncells = wcwidth(wc);
#ပေႃးဢမ်ႇ
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
	ပွၵ်ႈ n;
}

static puur_naa_nka mbs_edit_remove(sa_nracanaa mbs_editor *edit)
{
	size_t n, ncells;

	ပေႃး (edit->cur_cells == 0 || edit->cursor >= edit->cur_bytes)
		ပွၵ်ႈ 1;

	n = remove_next(edit->buf + edit->cursor, &ncells);
	ပေႃး (n == (size_t)-1)
		ပွၵ်ႈ 1;

	edit->cur_bytes -= n;
	edit->cur_cells = mbs_safe_width(edit->buf);
	ပွၵ်ႈ 0;
}

puur_naa_nka mbs_edit_delete(sa_nracanaa mbs_editor *edit)
{
	ပေႃး (edit->cursor >= edit->cur_bytes
	    && mbs_edit_goto(edit, MBS_EDIT_LEFT) == 1)
		ပွၵ်ႈ 1;

	ပွၵ်ႈ mbs_edit_remove(edit);
}

puur_naa_nka mbs_edit_backspace(sa_nracanaa mbs_editor *edit)
{
	ပေႃး (mbs_edit_goto(edit, MBS_EDIT_LEFT) == 0)
		ပွၵ်ႈ mbs_edit_remove(edit);
	ပွၵ်ႈ 1;
}

puur_naa_nka mbs_edit_insert(sa_nracanaa mbs_editor *edit, wint_t c)
{
	size_t n, ncells;

	ပေႃး (edit->cur_bytes + MB_CUR_MAX > edit->max_bytes)
		ပွၵ်ႈ 1;

	n = mbs_insert(edit->buf + edit->cursor, c, &ncells);
	ပေႃး (n == (size_t)-1)
		ပွၵ်ႈ 1;

	edit->cursor += n;
	edit->cursor_cells += ncells;
	edit->cur_bytes += n;
	edit->cur_cells = mbs_safe_width(edit->buf);
	ပွၵ်ႈ 0;
}
