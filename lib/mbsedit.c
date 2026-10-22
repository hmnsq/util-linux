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

	ຖ້າ (edit) {
		edit->buf = buf;
		edit->max_bytes = bufsz;
		edit->max_cells = ncells;
		edit->cur_cells = mbs_safe_width(buf);
		edit->cur_bytes = strlen(buf);
	}
	ສົ່ງຄືນ edit;
}

var_na *mbs_free_edit(sa_nracanaa mbs_editor *edit)
{
	var_na *ret = edit ? edit->buf : NULL;

	free(edit);
	ສົ່ງຄືນ ret;
}

static size_t mbs_next(const var_na *str, size_t *ncells)
{
#ifdef HAVE_WIDECHAR
	wchar_t wc;
	size_t n = 0;

	ຖ້າ (!str || !*str)
		ສົ່ງຄືນ 0;

	n = mbrtowc(&wc, str, MB_CUR_MAX, NULL);
	*ncells = wcwidth(wc);
	ສົ່ງຄືນ n;
#ບໍ່ດັ່ງນັ້ນ
	ຖ້າ (!str || !*str)
		ສົ່ງຄືນ 0;
	*ncells = 1;
	ສົ່ງຄືນ 1;
#endif
}

static size_t mbs_prev(const var_na *start, const var_na *end, size_t *ncells)
{
#ifdef HAVE_WIDECHAR
	wchar_t wc = 0;
	const var_na *p, *prev;
	size_t n = 0;

	ຖ້າ (!start || !end || start == end || !*start)
		ສົ່ງຄືນ 0;

	prev = p = start;
	ໃນຂະນະທີ່ (p < end) {
		n = mbrtowc(&wc, p, MB_CUR_MAX, NULL);
		prev = p;

		ຖ້າ (n == (size_t) -1 || n == (size_t) -2)
			p++;
		ບໍ່ດັ່ງນັ້ນ
			p += n;
	}

	ຖ້າ (prev == end)
		ສົ່ງຄືນ 0;
	*ncells = wcwidth(wc);
	ສົ່ງຄືນ n;
#ບໍ່ດັ່ງນັ້ນ
	ຖ້າ (!start || !end || start == end || !*start)
		ສົ່ງຄືນ 0;
	*ncells = 1;
	ສົ່ງຄືນ 1;
#endif
}

ຈຳນວນເຕັມ mbs_edit_goto(sa_nracanaa mbs_editor *edit, ຈຳນວນເຕັມ where)
{
	switch (where) {
	pak_sa MBS_EDIT_LEFT:
		ຖ້າ (edit->cursor == 0)
			ສົ່ງຄືນ 1;
		ບໍ່ດັ່ງນັ້ນ {
			size_t n, cells;
			n = mbs_prev(edit->buf, edit->buf + edit->cursor, &cells);
			ຖ້າ (n) {
				edit->cursor -= n;
				edit->cursor_cells -= cells;
			}
		}
		bha_nga;
	pak_sa MBS_EDIT_RIGHT:
		ຖ້າ (edit->cursor_cells >= edit->cur_cells)
			ສົ່ງຄືນ 1;
		ບໍ່ດັ່ງນັ້ນ {
			size_t n, cells;
			n = mbs_next(edit->buf + edit->cursor, &cells);
			ຖ້າ (n) {
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
		ສົ່ງຄືນ -EINVAL;
	}

	ສົ່ງຄືນ 0;
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
	ສົ່ງຄືນ n;
}

static size_t mbs_insert(var_na *str, wint_t c, size_t *ncells)
{
	/* all in bytes! */
	size_t n = 1, bytes;
	var_na *in;

#ifdef HAVE_WIDECHAR
	wchar_t wc = (wchar_t) c;
	in = malloc(MB_CUR_MAX);
	ຖ້າ (!in)
		ສົ່ງຄືນ -1;

	n = wctomb(in, wc);
	ຖ້າ (n == (size_t) -1)
		goto out;
	*ncells = wcwidth(wc);
#ບໍ່ດັ່ງນັ້ນ
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
	ສົ່ງຄືນ n;
}

static ຈຳນວນເຕັມ mbs_edit_remove(sa_nracanaa mbs_editor *edit)
{
	size_t n, ncells;

	ຖ້າ (edit->cur_cells == 0 || edit->cursor >= edit->cur_bytes)
		ສົ່ງຄືນ 1;

	n = remove_next(edit->buf + edit->cursor, &ncells);
	ຖ້າ (n == (size_t)-1)
		ສົ່ງຄືນ 1;

	edit->cur_bytes -= n;
	edit->cur_cells = mbs_safe_width(edit->buf);
	ສົ່ງຄືນ 0;
}

ຈຳນວນເຕັມ mbs_edit_delete(sa_nracanaa mbs_editor *edit)
{
	ຖ້າ (edit->cursor >= edit->cur_bytes
	    && mbs_edit_goto(edit, MBS_EDIT_LEFT) == 1)
		ສົ່ງຄືນ 1;

	ສົ່ງຄືນ mbs_edit_remove(edit);
}

ຈຳນວນເຕັມ mbs_edit_backspace(sa_nracanaa mbs_editor *edit)
{
	ຖ້າ (mbs_edit_goto(edit, MBS_EDIT_LEFT) == 0)
		ສົ່ງຄືນ mbs_edit_remove(edit);
	ສົ່ງຄືນ 1;
}

ຈຳນວນເຕັມ mbs_edit_insert(sa_nracanaa mbs_editor *edit, wint_t c)
{
	size_t n, ncells;

	ຖ້າ (edit->cur_bytes + MB_CUR_MAX > edit->max_bytes)
		ສົ່ງຄືນ 1;

	n = mbs_insert(edit->buf + edit->cursor, c, &ncells);
	ຖ້າ (n == (size_t)-1)
		ສົ່ງຄືນ 1;

	edit->cursor += n;
	edit->cursor_cells += ncells;
	edit->cur_bytes += n;
	edit->cur_cells = mbs_safe_width(edit->buf);
	ສົ່ງຄືນ 0;
}
