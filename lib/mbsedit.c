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
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "mbsalign.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "mbsedit.h"

ਬਣਤਰ mbs_editor *mbs_new_edit(ਅੱਖਰ *buf, size_t bufsz, size_t ncells)
{
	ਬਣਤਰ mbs_editor *edit = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(1, ਆਕਾਰ(*edit));

	ਜੇ (edit) {
		edit->buf = buf;
		edit->max_ਬਾਈਟਾਂ = bufsz;
		edit->max_cells = ncells;
		edit->cur_cells = mbs_ਸੁਰੱਖਿਅਤ_ਚੌੜਾਈ(buf);
		edit->cur_ਬਾਈਟਾਂ = ਲੜੀ_ਲੰਬਾਈ(buf);
	}
	ਵਾਪਸ edit;
}

ਅੱਖਰ *mbs_free_edit(ਬਣਤਰ mbs_editor *edit)
{
	ਅੱਖਰ *ret = edit ? edit->buf : ਨਿਰਾ;

	ਮੁਕਤ(edit);
	ਵਾਪਸ ret;
}

ਸਥਿਰ size_t mbs_ਅਗਲਾ(ਅਚਲ ਅੱਖਰ *str, size_t *ncells)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_WIDECHAR
	wchar_t wc;
	size_t n = 0;

	ਜੇ (!str || !*str)
		ਵਾਪਸ 0;

	n = mbrtowc(&wc, str, MB_CUR_MAX, ਨਿਰਾ);
	*ncells = wcwidth(wc);
	ਵਾਪਸ n;
#ਨਹੀਂਤਾਂ
	ਜੇ (!str || !*str)
		ਵਾਪਸ 0;
	*ncells = 1;
	ਵਾਪਸ 1;
#ਜੇ_ਅੰਤ
}

ਸਥਿਰ size_t mbs_prev(ਅਚਲ ਅੱਖਰ *ਸ਼ੁਰੂ, ਅਚਲ ਅੱਖਰ *ਸਿਰਾ, size_t *ncells)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_WIDECHAR
	wchar_t wc = 0;
	ਅਚਲ ਅੱਖਰ *p, *prev;
	size_t n = 0;

	ਜੇ (!ਸ਼ੁਰੂ || !ਸਿਰਾ || ਸ਼ੁਰੂ == ਸਿਰਾ || !*ਸ਼ੁਰੂ)
		ਵਾਪਸ 0;

	prev = p = ਸ਼ੁਰੂ;
	ਜਦੋਂ (p < ਸਿਰਾ) {
		n = mbrtowc(&wc, p, MB_CUR_MAX, ਨਿਰਾ);
		prev = p;

		ਜੇ (n == (size_t) -1 || n == (size_t) -2)
			p++;
		ਨਹੀਂਤਾਂ
			p += n;
	}

	ਜੇ (prev == ਸਿਰਾ)
		ਵਾਪਸ 0;
	*ncells = wcwidth(wc);
	ਵਾਪਸ n;
#ਨਹੀਂਤਾਂ
	ਜੇ (!ਸ਼ੁਰੂ || !ਸਿਰਾ || ਸ਼ੁਰੂ == ਸਿਰਾ || !*ਸ਼ੁਰੂ)
		ਵਾਪਸ 0;
	*ncells = 1;
	ਵਾਪਸ 1;
#ਜੇ_ਅੰਤ
}

ਪੂਰਨ mbs_edit_goto(ਬਣਤਰ mbs_editor *edit, ਪੂਰਨ where)
{
	ਚੋਣ (where) {
	ਹਾਲਤ MBS_EDIT_ਖੱਬਾ:
		ਜੇ (edit->cursor == 0)
			ਵਾਪਸ 1;
		ਨਹੀਂਤਾਂ {
			size_t n, cells;
			n = mbs_prev(edit->buf, edit->buf + edit->cursor, &cells);
			ਜੇ (n) {
				edit->cursor -= n;
				edit->cursor_cells -= cells;
			}
		}
		ਰੋਕੋ;
	ਹਾਲਤ MBS_EDIT_ਸੱਜਾ:
		ਜੇ (edit->cursor_cells >= edit->cur_cells)
			ਵਾਪਸ 1;
		ਨਹੀਂਤਾਂ {
			size_t n, cells;
			n = mbs_ਅਗਲਾ(edit->buf + edit->cursor, &cells);
			ਜੇ (n) {
				edit->cursor += n;
				edit->cursor_cells += cells;
			}
		}
		ਰੋਕੋ;
	ਹਾਲਤ MBS_EDIT_HOME:
		edit->cursor = 0;
		edit->cursor_cells = 0;
		ਰੋਕੋ;
	ਹਾਲਤ MBS_EDIT_ਸਿਰਾ:
		edit->cursor = edit->cur_ਬਾਈਟਾਂ;
		edit->cursor_cells = edit->cur_cells;
		ਰੋਕੋ;
	ਮੂਲ:
		ਵਾਪਸ -EINVAL;
	}

	ਵਾਪਸ 0;
}

/* Remove next MB from @str, returns number of removed bytes */
ਸਥਿਰ size_t remove_ਅਗਲਾ(ਅੱਖਰ *str, size_t *ncells)
{
	/* all in bytes! */
	size_t ਬਾਈਟਾਂ, move_ਬਾਈਟਾਂ, n;

	n          = mbs_ਅਗਲਾ(str, ncells);
	ਬਾਈਟਾਂ      = ਲੜੀ_ਲੰਬਾਈ(str);
	move_ਬਾਈਟਾਂ = ਬਾਈਟਾਂ - n;

	memmove(str, str + n, move_ਬਾਈਟਾਂ);
	str[ਬਾਈਟਾਂ - n] = '\0';
	ਵਾਪਸ n;
}

ਸਥਿਰ size_t mbs_ਪਾਓ(ਅੱਖਰ *str, wint_t c, size_t *ncells)
{
	/* all in bytes! */
	size_t n = 1, ਬਾਈਟਾਂ;
	ਅੱਖਰ *ਵਿੱਚ;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_WIDECHAR
	wchar_t wc = (wchar_t) c;
	ਵਿੱਚ = ਸਮ੍ਰਿਤੀ_ਦਿਓ(MB_CUR_MAX);
	ਜੇ (!ਵਿੱਚ)
		ਵਾਪਸ -1;

	n = wctomb(ਵਿੱਚ, wc);
	ਜੇ (n == (size_t) -1)
		ਜਾਓ ਬਾਹਰਲਾ;
	*ncells = wcwidth(wc);
#ਨਹੀਂਤਾਂ
	*ncells = 1;
	ਵਿੱਚ = (ਅੱਖਰ *) &c;
#ਜੇ_ਅੰਤ
	ਬਾਈਟਾਂ       = ਲੜੀ_ਲੰਬਾਈ(str);

	memmove(str + n, str, ਬਾਈਟਾਂ);
	ਸਮ੍ਰਿਤੀ_ਨਕਲ(str, ਵਿੱਚ, n);
	str[ਬਾਈਟਾਂ + n] = '\0';
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_WIDECHAR
ਬਾਹਰਲਾ:
	ਮੁਕਤ(ਵਿੱਚ);
#ਜੇ_ਅੰਤ
	ਵਾਪਸ n;
}

ਸਥਿਰ ਪੂਰਨ mbs_edit_remove(ਬਣਤਰ mbs_editor *edit)
{
	size_t n, ncells;

	ਜੇ (edit->cur_cells == 0 || edit->cursor >= edit->cur_ਬਾਈਟਾਂ)
		ਵਾਪਸ 1;

	n = remove_ਅਗਲਾ(edit->buf + edit->cursor, &ncells);
	ਜੇ (n == (size_t)-1)
		ਵਾਪਸ 1;

	edit->cur_ਬਾਈਟਾਂ -= n;
	edit->cur_cells = mbs_ਸੁਰੱਖਿਅਤ_ਚੌੜਾਈ(edit->buf);
	ਵਾਪਸ 0;
}

ਪੂਰਨ mbs_edit_delete(ਬਣਤਰ mbs_editor *edit)
{
	ਜੇ (edit->cursor >= edit->cur_ਬਾਈਟਾਂ
	    && mbs_edit_goto(edit, MBS_EDIT_ਖੱਬਾ) == 1)
		ਵਾਪਸ 1;

	ਵਾਪਸ mbs_edit_remove(edit);
}

ਪੂਰਨ mbs_edit_backspace(ਬਣਤਰ mbs_editor *edit)
{
	ਜੇ (mbs_edit_goto(edit, MBS_EDIT_ਖੱਬਾ) == 0)
		ਵਾਪਸ mbs_edit_remove(edit);
	ਵਾਪਸ 1;
}

ਪੂਰਨ mbs_edit_ਪਾਓ(ਬਣਤਰ mbs_editor *edit, wint_t c)
{
	size_t n, ncells;

	ਜੇ (edit->cur_ਬਾਈਟਾਂ + MB_CUR_MAX > edit->max_ਬਾਈਟਾਂ)
		ਵਾਪਸ 1;

	n = mbs_ਪਾਓ(edit->buf + edit->cursor, c, &ncells);
	ਜੇ (n == (size_t)-1)
		ਵਾਪਸ 1;

	edit->cursor += n;
	edit->cursor_cells += ncells;
	edit->cur_ਬਾਈਟਾਂ += n;
	edit->cur_cells = mbs_ਸੁਰੱਖਿਅਤ_ਚੌੜਾਈ(edit->buf);
	ਵਾਪਸ 0;
}
