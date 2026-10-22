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
#ضمن <stdlib.h>
#ضمن <string.h>
#ضمن <errno.h>
#ضمن <stdio.h>

#ضمن "mbsalign.h"
#ضمن "mbsedit.h"

بنية mbs_editor *mbs_new_edit(حرف *buf, size_t bufsz, size_t ncells)
{
	بنية mbs_editor *edit = صفر_يادگيري_ڏيو(1, حجم(*edit));

	إذا (edit) {
		edit->buf = buf;
		edit->max_بائيٽون = bufsz;
		edit->max_cells = ncells;
		edit->cur_cells = mbs_محفوظ_ويڪر(buf);
		edit->cur_بائيٽون = لڙي_ڊيگهه(buf);
	}
	أرجع edit;
}

حرف *mbs_free_edit(بنية mbs_editor *edit)
{
	حرف *ret = edit ? edit->buf : عدم;

	حرر(edit);
	أرجع ret;
}

ساكن size_t mbs_ايندڙ(ثابت حرف *str, size_t *ncells)
{
#جيڪڏهن_وصف_ٿيل HAVE_WIDECHAR
	wchar_t wc;
	size_t n = 0;

	إذا (!str || !*str)
		أرجع 0;

	n = mbrtowc(&wc, str, MB_CUR_MAX, عدم);
	*ncells = wcwidth(wc);
	أرجع n;
#وإلا
	إذا (!str || !*str)
		أرجع 0;
	*ncells = 1;
	أرجع 1;
#جيڪڏهن_پورو
}

ساكن size_t mbs_prev(ثابت حرف *شروعات, ثابت حرف *پڇاڙي, size_t *ncells)
{
#جيڪڏهن_وصف_ٿيل HAVE_WIDECHAR
	wchar_t wc = 0;
	ثابت حرف *p, *prev;
	size_t n = 0;

	إذا (!شروعات || !پڇاڙي || شروعات == پڇاڙي || !*شروعات)
		أرجع 0;

	prev = p = شروعات;
	طالما (p < پڇاڙي) {
		n = mbrtowc(&wc, p, MB_CUR_MAX, عدم);
		prev = p;

		إذا (n == (size_t) -1 || n == (size_t) -2)
			p++;
		وإلا
			p += n;
	}

	إذا (prev == پڇاڙي)
		أرجع 0;
	*ncells = wcwidth(wc);
	أرجع n;
#وإلا
	إذا (!شروعات || !پڇاڙي || شروعات == پڇاڙي || !*شروعات)
		أرجع 0;
	*ncells = 1;
	أرجع 1;
#جيڪڏهن_پورو
}

صحيح mbs_edit_goto(بنية mbs_editor *edit, صحيح where)
{
	بدل (where) {
	حالة MBS_EDIT_کاٻو:
		إذا (edit->cursor == 0)
			أرجع 1;
		وإلا {
			size_t n, cells;
			n = mbs_prev(edit->buf, edit->buf + edit->cursor, &cells);
			إذا (n) {
				edit->cursor -= n;
				edit->cursor_cells -= cells;
			}
		}
		اكسر;
	حالة MBS_EDIT_ساڄو:
		إذا (edit->cursor_cells >= edit->cur_cells)
			أرجع 1;
		وإلا {
			size_t n, cells;
			n = mbs_ايندڙ(edit->buf + edit->cursor, &cells);
			إذا (n) {
				edit->cursor += n;
				edit->cursor_cells += cells;
			}
		}
		اكسر;
	حالة MBS_EDIT_HOME:
		edit->cursor = 0;
		edit->cursor_cells = 0;
		اكسر;
	حالة MBS_EDIT_پڇاڙي:
		edit->cursor = edit->cur_بائيٽون;
		edit->cursor_cells = edit->cur_cells;
		اكسر;
	افتراضي:
		أرجع -EINVAL;
	}

	أرجع 0;
}

/* Remove next MB from @str, returns number of removed bytes */
ساكن size_t remove_ايندڙ(حرف *str, size_t *ncells)
{
	/* all in bytes! */
	size_t بائيٽون, move_بائيٽون, n;

	n          = mbs_ايندڙ(str, ncells);
	بائيٽون      = لڙي_ڊيگهه(str);
	move_بائيٽون = بائيٽون - n;

	memmove(str, str + n, move_بائيٽون);
	str[بائيٽون - n] = '\0';
	أرجع n;
}

ساكن size_t mbs_وجهو(حرف *str, wint_t c, size_t *ncells)
{
	/* all in bytes! */
	size_t n = 1, بائيٽون;
	حرف *اندريون;

#جيڪڏهن_وصف_ٿيل HAVE_WIDECHAR
	wchar_t wc = (wchar_t) c;
	اندريون = يادگيري_ڏيو(MB_CUR_MAX);
	إذا (!اندريون)
		أرجع -1;

	n = wctomb(اندريون, wc);
	إذا (n == (size_t) -1)
		اذهب ٻاهريون;
	*ncells = wcwidth(wc);
#وإلا
	*ncells = 1;
	اندريون = (حرف *) &c;
#جيڪڏهن_پورو
	بائيٽون       = لڙي_ڊيگهه(str);

	memmove(str + n, str, بائيٽون);
	يادگيري_نقل(str, اندريون, n);
	str[بائيٽون + n] = '\0';
#جيڪڏهن_وصف_ٿيل HAVE_WIDECHAR
ٻاهريون:
	حرر(اندريون);
#جيڪڏهن_پورو
	أرجع n;
}

ساكن صحيح mbs_edit_remove(بنية mbs_editor *edit)
{
	size_t n, ncells;

	إذا (edit->cur_cells == 0 || edit->cursor >= edit->cur_بائيٽون)
		أرجع 1;

	n = remove_ايندڙ(edit->buf + edit->cursor, &ncells);
	إذا (n == (size_t)-1)
		أرجع 1;

	edit->cur_بائيٽون -= n;
	edit->cur_cells = mbs_محفوظ_ويڪر(edit->buf);
	أرجع 0;
}

صحيح mbs_edit_delete(بنية mbs_editor *edit)
{
	إذا (edit->cursor >= edit->cur_بائيٽون
	    && mbs_edit_goto(edit, MBS_EDIT_کاٻو) == 1)
		أرجع 1;

	أرجع mbs_edit_remove(edit);
}

صحيح mbs_edit_backspace(بنية mbs_editor *edit)
{
	إذا (mbs_edit_goto(edit, MBS_EDIT_کاٻو) == 0)
		أرجع mbs_edit_remove(edit);
	أرجع 1;
}

صحيح mbs_edit_وجهو(بنية mbs_editor *edit, wint_t c)
{
	size_t n, ncells;

	إذا (edit->cur_بائيٽون + MB_CUR_MAX > edit->max_بائيٽون)
		أرجع 1;

	n = mbs_وجهو(edit->buf + edit->cursor, c, &ncells);
	إذا (n == (size_t)-1)
		أرجع 1;

	edit->cursor += n;
	edit->cursor_cells += ncells;
	edit->cur_بائيٽون += n;
	edit->cur_cells = mbs_محفوظ_ويڪر(edit->buf);
	أرجع 0;
}
