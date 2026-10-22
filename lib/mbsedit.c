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
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <errno.h>
#അന്തര്ഭാവ <stdio.h>

#അന്തര്ഭാവ "mbsalign.h"
#അന്തര്ഭാവ "mbsedit.h"

ഘടന mbs_editor *mbs_new_edit(അക്ഷരം *buf, size_t bufsz, size_t ncells)
{
	ഘടന mbs_editor *edit = calloc(1, വലുപ്പം(*edit));

	എങ്കിൽ (edit) {
		edit->buf = buf;
		edit->max_bytes = bufsz;
		edit->max_cells = ncells;
		edit->cur_cells = mbs_safe_width(buf);
		edit->cur_bytes = strlen(buf);
	}
	തിരികെ edit;
}

അക്ഷരം *mbs_free_edit(ഘടന mbs_editor *edit)
{
	അക്ഷരം *ret = edit ? edit->buf : ശൂന്യ;

	സ്വതന്ത്രം(edit);
	തിരികെ ret;
}

സ്ഥിതം size_t mbs_next(സ്ഥിരം അക്ഷരം *str, size_t *ncells)
{
#ifdef HAVE_WIDECHAR
	wchar_t wc;
	size_t n = 0;

	എങ്കിൽ (!str || !*str)
		തിരികെ 0;

	n = mbrtowc(&wc, str, MB_CUR_MAX, ശൂന്യ);
	*ncells = wcwidth(wc);
	തിരികെ n;
#അല്ലെങ്കിൽ
	എങ്കിൽ (!str || !*str)
		തിരികെ 0;
	*ncells = 1;
	തിരികെ 1;
#endif
}

സ്ഥിതം size_t mbs_prev(സ്ഥിരം അക്ഷരം *start, സ്ഥിരം അക്ഷരം *end, size_t *ncells)
{
#ifdef HAVE_WIDECHAR
	wchar_t wc = 0;
	സ്ഥിരം അക്ഷരം *p, *prev;
	size_t n = 0;

	എങ്കിൽ (!start || !end || start == end || !*start)
		തിരികെ 0;

	prev = p = start;
	ആകുമ്പോൾ (p < end) {
		n = mbrtowc(&wc, p, MB_CUR_MAX, ശൂന്യ);
		prev = p;

		എങ്കിൽ (n == (size_t) -1 || n == (size_t) -2)
			p++;
		അല്ലെങ്കിൽ
			p += n;
	}

	എങ്കിൽ (prev == end)
		തിരികെ 0;
	*ncells = wcwidth(wc);
	തിരികെ n;
#അല്ലെങ്കിൽ
	എങ്കിൽ (!start || !end || start == end || !*start)
		തിരികെ 0;
	*ncells = 1;
	തിരികെ 1;
#endif
}

പൂർണ്ണസംഖ്യ mbs_edit_goto(ഘടന mbs_editor *edit, പൂർണ്ണസംഖ്യ where)
{
	തിരഞ്ഞെടുപ്പ് (where) {
	അവസ്ഥ MBS_EDIT_ഇടത്:
		എങ്കിൽ (edit->cursor == 0)
			തിരികെ 1;
		അല്ലെങ്കിൽ {
			size_t n, cells;
			n = mbs_prev(edit->buf, edit->buf + edit->cursor, &cells);
			എങ്കിൽ (n) {
				edit->cursor -= n;
				edit->cursor_cells -= cells;
			}
		}
		നിർത്തുക;
	അവസ്ഥ MBS_EDIT_വലത്:
		എങ്കിൽ (edit->cursor_cells >= edit->cur_cells)
			തിരികെ 1;
		അല്ലെങ്കിൽ {
			size_t n, cells;
			n = mbs_next(edit->buf + edit->cursor, &cells);
			എങ്കിൽ (n) {
				edit->cursor += n;
				edit->cursor_cells += cells;
			}
		}
		നിർത്തുക;
	അവസ്ഥ MBS_EDIT_HOME:
		edit->cursor = 0;
		edit->cursor_cells = 0;
		നിർത്തുക;
	അവസ്ഥ MBS_EDIT_END:
		edit->cursor = edit->cur_bytes;
		edit->cursor_cells = edit->cur_cells;
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		തിരികെ -EINVAL;
	}

	തിരികെ 0;
}

/* Remove next MB from @str, returns number of removed bytes */
സ്ഥിതം size_t remove_next(അക്ഷരം *str, size_t *ncells)
{
	/* all in bytes! */
	size_t bytes, move_bytes, n;

	n          = mbs_next(str, ncells);
	bytes      = strlen(str);
	move_bytes = bytes - n;

	memmove(str, str + n, move_bytes);
	str[bytes - n] = '\0';
	തിരികെ n;
}

സ്ഥിതം size_t mbs_തിരുകുക(അക്ഷരം *str, wint_t c, size_t *ncells)
{
	/* all in bytes! */
	size_t n = 1, bytes;
	അക്ഷരം *in;

#ifdef HAVE_WIDECHAR
	wchar_t wc = (wchar_t) c;
	in = malloc(MB_CUR_MAX);
	എങ്കിൽ (!in)
		തിരികെ -1;

	n = wctomb(in, wc);
	എങ്കിൽ (n == (size_t) -1)
		പോകുക out;
	*ncells = wcwidth(wc);
#അല്ലെങ്കിൽ
	*ncells = 1;
	in = (അക്ഷരം *) &c;
#endif
	bytes       = strlen(str);

	memmove(str + n, str, bytes);
	memcpy(str, in, n);
	str[bytes + n] = '\0';
#ifdef HAVE_WIDECHAR
out:
	സ്വതന്ത്രം(in);
#endif
	തിരികെ n;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ mbs_edit_remove(ഘടന mbs_editor *edit)
{
	size_t n, ncells;

	എങ്കിൽ (edit->cur_cells == 0 || edit->cursor >= edit->cur_bytes)
		തിരികെ 1;

	n = remove_next(edit->buf + edit->cursor, &ncells);
	എങ്കിൽ (n == (size_t)-1)
		തിരികെ 1;

	edit->cur_bytes -= n;
	edit->cur_cells = mbs_safe_width(edit->buf);
	തിരികെ 0;
}

പൂർണ്ണസംഖ്യ mbs_edit_delete(ഘടന mbs_editor *edit)
{
	എങ്കിൽ (edit->cursor >= edit->cur_bytes
	    && mbs_edit_goto(edit, MBS_EDIT_ഇടത്) == 1)
		തിരികെ 1;

	തിരികെ mbs_edit_remove(edit);
}

പൂർണ്ണസംഖ്യ mbs_edit_backspace(ഘടന mbs_editor *edit)
{
	എങ്കിൽ (mbs_edit_goto(edit, MBS_EDIT_ഇടത്) == 0)
		തിരികെ mbs_edit_remove(edit);
	തിരികെ 1;
}

പൂർണ്ണസംഖ്യ mbs_edit_തിരുകുക(ഘടന mbs_editor *edit, wint_t c)
{
	size_t n, ncells;

	എങ്കിൽ (edit->cur_bytes + MB_CUR_MAX > edit->max_bytes)
		തിരികെ 1;

	n = mbs_തിരുകുക(edit->buf + edit->cursor, c, &ncells);
	എങ്കിൽ (n == (size_t)-1)
		തിരികെ 1;

	edit->cursor += n;
	edit->cursor_cells += ncells;
	edit->cur_bytes += n;
	edit->cur_cells = mbs_safe_width(edit->buf);
	തിരികെ 0;
}
