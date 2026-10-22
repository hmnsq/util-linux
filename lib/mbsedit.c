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
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <errno.h>
#અન્તર્ભાવ <stdio.h>

#અન્તર્ભાવ "mbsalign.h"
#અન્તર્ભાવ "mbsedit.h"

સંરચના mbs_editor *mbs_new_edit(અક્ષર *buf, size_t bufsz, size_t ncells)
{
	સંરચના mbs_editor *edit = calloc(1, કદ(*edit));

	જો (edit) {
		edit->buf = buf;
		edit->max_bytes = bufsz;
		edit->max_cells = ncells;
		edit->cur_cells = mbs_safe_width(buf);
		edit->cur_bytes = strlen(buf);
	}
	પાછા edit;
}

અક્ષર *mbs_free_edit(સંરચના mbs_editor *edit)
{
	અક્ષર *ret = edit ? edit->buf : ખાલી;

	મુક્ત(edit);
	પાછા ret;
}

સ્થિર size_t mbs_next(અચલ અક્ષર *str, size_t *ncells)
{
#ifdef HAVE_WIDECHAR
	wchar_t wc;
	size_t n = 0;

	જો (!str || !*str)
		પાછા 0;

	n = mbrtowc(&wc, str, MB_CUR_MAX, ખાલી);
	*ncells = wcwidth(wc);
	પાછા n;
#નહીંતર
	જો (!str || !*str)
		પાછા 0;
	*ncells = 1;
	પાછા 1;
#endif
}

સ્થિર size_t mbs_prev(અચલ અક્ષર *start, અચલ અક્ષર *end, size_t *ncells)
{
#ifdef HAVE_WIDECHAR
	wchar_t wc = 0;
	અચલ અક્ષર *p, *prev;
	size_t n = 0;

	જો (!start || !end || start == end || !*start)
		પાછા 0;

	prev = p = start;
	જ્યારે (p < end) {
		n = mbrtowc(&wc, p, MB_CUR_MAX, ખાલી);
		prev = p;

		જો (n == (size_t) -1 || n == (size_t) -2)
			p++;
		નહીંતર
			p += n;
	}

	જો (prev == end)
		પાછા 0;
	*ncells = wcwidth(wc);
	પાછા n;
#નહીંતર
	જો (!start || !end || start == end || !*start)
		પાછા 0;
	*ncells = 1;
	પાછા 1;
#endif
}

પૂર્ણાંક mbs_edit_goto(સંરચના mbs_editor *edit, પૂર્ણાંક where)
{
	પસંદગી (where) {
	સ્થિતિ MBS_EDIT_ડાબું:
		જો (edit->cursor == 0)
			પાછા 1;
		નહીંતર {
			size_t n, cells;
			n = mbs_prev(edit->buf, edit->buf + edit->cursor, &cells);
			જો (n) {
				edit->cursor -= n;
				edit->cursor_cells -= cells;
			}
		}
		તોડો;
	સ્થિતિ MBS_EDIT_જમણું:
		જો (edit->cursor_cells >= edit->cur_cells)
			પાછા 1;
		નહીંતર {
			size_t n, cells;
			n = mbs_next(edit->buf + edit->cursor, &cells);
			જો (n) {
				edit->cursor += n;
				edit->cursor_cells += cells;
			}
		}
		તોડો;
	સ્થિતિ MBS_EDIT_HOME:
		edit->cursor = 0;
		edit->cursor_cells = 0;
		તોડો;
	સ્થિતિ MBS_EDIT_END:
		edit->cursor = edit->cur_bytes;
		edit->cursor_cells = edit->cur_cells;
		તોડો;
	મૂળભૂત:
		પાછા -EINVAL;
	}

	પાછા 0;
}

/* Remove next MB from @str, returns number of removed bytes */
સ્થિર size_t remove_next(અક્ષર *str, size_t *ncells)
{
	/* all in bytes! */
	size_t bytes, move_bytes, n;

	n          = mbs_next(str, ncells);
	bytes      = strlen(str);
	move_bytes = bytes - n;

	memmove(str, str + n, move_bytes);
	str[bytes - n] = '\0';
	પાછા n;
}

સ્થિર size_t mbs_ઉમેરો(અક્ષર *str, wint_t c, size_t *ncells)
{
	/* all in bytes! */
	size_t n = 1, bytes;
	અક્ષર *in;

#ifdef HAVE_WIDECHAR
	wchar_t wc = (wchar_t) c;
	in = malloc(MB_CUR_MAX);
	જો (!in)
		પાછા -1;

	n = wctomb(in, wc);
	જો (n == (size_t) -1)
		જાઓ out;
	*ncells = wcwidth(wc);
#નહીંતર
	*ncells = 1;
	in = (અક્ષર *) &c;
#endif
	bytes       = strlen(str);

	memmove(str + n, str, bytes);
	memcpy(str, in, n);
	str[bytes + n] = '\0';
#ifdef HAVE_WIDECHAR
out:
	મુક્ત(in);
#endif
	પાછા n;
}

સ્થિર પૂર્ણાંક mbs_edit_remove(સંરચના mbs_editor *edit)
{
	size_t n, ncells;

	જો (edit->cur_cells == 0 || edit->cursor >= edit->cur_bytes)
		પાછા 1;

	n = remove_next(edit->buf + edit->cursor, &ncells);
	જો (n == (size_t)-1)
		પાછા 1;

	edit->cur_bytes -= n;
	edit->cur_cells = mbs_safe_width(edit->buf);
	પાછા 0;
}

પૂર્ણાંક mbs_edit_delete(સંરચના mbs_editor *edit)
{
	જો (edit->cursor >= edit->cur_bytes
	    && mbs_edit_goto(edit, MBS_EDIT_ડાબું) == 1)
		પાછા 1;

	પાછા mbs_edit_remove(edit);
}

પૂર્ણાંક mbs_edit_backspace(સંરચના mbs_editor *edit)
{
	જો (mbs_edit_goto(edit, MBS_EDIT_ડાબું) == 0)
		પાછા mbs_edit_remove(edit);
	પાછા 1;
}

પૂર્ણાંક mbs_edit_ઉમેરો(સંરચના mbs_editor *edit, wint_t c)
{
	size_t n, ncells;

	જો (edit->cur_bytes + MB_CUR_MAX > edit->max_bytes)
		પાછા 1;

	n = mbs_ઉમેરો(edit->buf + edit->cursor, c, &ncells);
	જો (n == (size_t)-1)
		પાછા 1;

	edit->cursor += n;
	edit->cursor_cells += ncells;
	edit->cur_bytes += n;
	edit->cur_cells = mbs_safe_width(edit->buf);
	પાછા 0;
}
