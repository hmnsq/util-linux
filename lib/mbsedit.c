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
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>

#ಅನ್ತರ್ಭಾವ "mbsalign.h"
#ಅನ್ತರ್ಭಾವ "mbsedit.h"

ಸಂರಚನಾ mbs_editor *mbs_new_edit(ವರ್ಣ *buf, size_t bufsz, size_t ncells)
{
	ಸಂರಚನಾ mbs_editor *edit = calloc(1, ಪರಿಮಾಣ(*edit));

	ಯದಿ (edit) {
		edit->buf = buf;
		edit->max_bytes = bufsz;
		edit->max_cells = ncells;
		edit->cur_cells = mbs_safe_width(buf);
		edit->cur_bytes = strlen(buf);
	}
	ಪ್ರತ್ಯಾಗಮ edit;
}

ವರ್ಣ *mbs_free_edit(ಸಂರಚನಾ mbs_editor *edit)
{
	ವರ್ಣ *ret = edit ? edit->buf : ರಿಕ್ತ;

	ಮುಕ್ತ(edit);
	ಪ್ರತ್ಯಾಗಮ ret;
}

ಸ್ಥಿರ size_t mbs_next(ಧ್ರುವ ವರ್ಣ *str, size_t *ncells)
{
#ifdef HAVE_WIDECHAR
	wchar_t wc;
	size_t n = 0;

	ಯದಿ (!str || !*str)
		ಪ್ರತ್ಯಾಗಮ 0;

	n = mbrtowc(&wc, str, MB_CUR_MAX, ರಿಕ್ತ);
	*ncells = wcwidth(wc);
	ಪ್ರತ್ಯಾಗಮ n;
#ಅನ್ಯಥಾ
	ಯದಿ (!str || !*str)
		ಪ್ರತ್ಯಾಗಮ 0;
	*ncells = 1;
	ಪ್ರತ್ಯಾಗಮ 1;
#endif
}

ಸ್ಥಿರ size_t mbs_prev(ಧ್ರುವ ವರ್ಣ *start, ಧ್ರುವ ವರ್ಣ *end, size_t *ncells)
{
#ifdef HAVE_WIDECHAR
	wchar_t wc = 0;
	ಧ್ರುವ ವರ್ಣ *p, *prev;
	size_t n = 0;

	ಯದಿ (!start || !end || start == end || !*start)
		ಪ್ರತ್ಯಾಗಮ 0;

	prev = p = start;
	ಯಾವತ್ (p < end) {
		n = mbrtowc(&wc, p, MB_CUR_MAX, ರಿಕ್ತ);
		prev = p;

		ಯದಿ (n == (size_t) -1 || n == (size_t) -2)
			p++;
		ಅನ್ಯಥಾ
			p += n;
	}

	ಯದಿ (prev == end)
		ಪ್ರತ್ಯಾಗಮ 0;
	*ncells = wcwidth(wc);
	ಪ್ರತ್ಯಾಗಮ n;
#ಅನ್ಯಥಾ
	ಯದಿ (!start || !end || start == end || !*start)
		ಪ್ರತ್ಯಾಗಮ 0;
	*ncells = 1;
	ಪ್ರತ್ಯಾಗಮ 1;
#endif
}

ಪೂರ್ಣಾಙ್ಕ mbs_edit_goto(ಸಂರಚನಾ mbs_editor *edit, ಪೂರ್ಣಾಙ್ಕ where)
{
	ವಿಕಲ್ಪ (where) {
	ಪಕ್ಷ MBS_EDIT_ಎಡ:
		ಯದಿ (edit->cursor == 0)
			ಪ್ರತ್ಯಾಗಮ 1;
		ಅನ್ಯಥಾ {
			size_t n, cells;
			n = mbs_prev(edit->buf, edit->buf + edit->cursor, &cells);
			ಯದಿ (n) {
				edit->cursor -= n;
				edit->cursor_cells -= cells;
			}
		}
		ಭಙ್ಗ;
	ಪಕ್ಷ MBS_EDIT_ಬಲ:
		ಯದಿ (edit->cursor_cells >= edit->cur_cells)
			ಪ್ರತ್ಯಾಗಮ 1;
		ಅನ್ಯಥಾ {
			size_t n, cells;
			n = mbs_next(edit->buf + edit->cursor, &cells);
			ಯದಿ (n) {
				edit->cursor += n;
				edit->cursor_cells += cells;
			}
		}
		ಭಙ್ಗ;
	ಪಕ್ಷ MBS_EDIT_HOME:
		edit->cursor = 0;
		edit->cursor_cells = 0;
		ಭಙ್ಗ;
	ಪಕ್ಷ MBS_EDIT_END:
		edit->cursor = edit->cur_bytes;
		edit->cursor_cells = edit->cur_cells;
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		ಪ್ರತ್ಯಾಗಮ -EINVAL;
	}

	ಪ್ರತ್ಯಾಗಮ 0;
}

/* Remove next MB from @str, returns number of removed bytes */
ಸ್ಥಿರ size_t remove_next(ವರ್ಣ *str, size_t *ncells)
{
	/* all in bytes! */
	size_t bytes, move_bytes, n;

	n          = mbs_next(str, ncells);
	bytes      = strlen(str);
	move_bytes = bytes - n;

	memmove(str, str + n, move_bytes);
	str[bytes - n] = '\0';
	ಪ್ರತ್ಯಾಗಮ n;
}

ಸ್ಥಿರ size_t mbs_ಸೇರಿಸು(ವರ್ಣ *str, wint_t c, size_t *ncells)
{
	/* all in bytes! */
	size_t n = 1, bytes;
	ವರ್ಣ *in;

#ifdef HAVE_WIDECHAR
	wchar_t wc = (wchar_t) c;
	in = malloc(MB_CUR_MAX);
	ಯದಿ (!in)
		ಪ್ರತ್ಯಾಗಮ -1;

	n = wctomb(in, wc);
	ಯದಿ (n == (size_t) -1)
		ಗಚ್ಛ out;
	*ncells = wcwidth(wc);
#ಅನ್ಯಥಾ
	*ncells = 1;
	in = (ವರ್ಣ *) &c;
#endif
	bytes       = strlen(str);

	memmove(str + n, str, bytes);
	memcpy(str, in, n);
	str[bytes + n] = '\0';
#ifdef HAVE_WIDECHAR
out:
	ಮುಕ್ತ(in);
#endif
	ಪ್ರತ್ಯಾಗಮ n;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ mbs_edit_remove(ಸಂರಚನಾ mbs_editor *edit)
{
	size_t n, ncells;

	ಯದಿ (edit->cur_cells == 0 || edit->cursor >= edit->cur_bytes)
		ಪ್ರತ್ಯಾಗಮ 1;

	n = remove_next(edit->buf + edit->cursor, &ncells);
	ಯದಿ (n == (size_t)-1)
		ಪ್ರತ್ಯಾಗಮ 1;

	edit->cur_bytes -= n;
	edit->cur_cells = mbs_safe_width(edit->buf);
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಪೂರ್ಣಾಙ್ಕ mbs_edit_delete(ಸಂರಚನಾ mbs_editor *edit)
{
	ಯದಿ (edit->cursor >= edit->cur_bytes
	    && mbs_edit_goto(edit, MBS_EDIT_ಎಡ) == 1)
		ಪ್ರತ್ಯಾಗಮ 1;

	ಪ್ರತ್ಯಾಗಮ mbs_edit_remove(edit);
}

ಪೂರ್ಣಾಙ್ಕ mbs_edit_backspace(ಸಂರಚನಾ mbs_editor *edit)
{
	ಯದಿ (mbs_edit_goto(edit, MBS_EDIT_ಎಡ) == 0)
		ಪ್ರತ್ಯಾಗಮ mbs_edit_remove(edit);
	ಪ್ರತ್ಯಾಗಮ 1;
}

ಪೂರ್ಣಾಙ್ಕ mbs_edit_ಸೇರಿಸು(ಸಂರಚನಾ mbs_editor *edit, wint_t c)
{
	size_t n, ncells;

	ಯದಿ (edit->cur_bytes + MB_CUR_MAX > edit->max_bytes)
		ಪ್ರತ್ಯಾಗಮ 1;

	n = mbs_ಸೇರಿಸು(edit->buf + edit->cursor, c, &ncells);
	ಯದಿ (n == (size_t)-1)
		ಪ್ರತ್ಯಾಗಮ 1;

	edit->cursor += n;
	edit->cursor_cells += ncells;
	edit->cur_bytes += n;
	edit->cur_cells = mbs_safe_width(edit->buf);
	ಪ್ರತ್ಯಾಗಮ 0;
}
