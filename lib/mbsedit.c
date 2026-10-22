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
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <errno.h>
#अन्तर्भाव <stdio.h>

#अन्तर्भाव "mbsalign.h"
#अन्तर्भाव "mbsedit.h"

संरचना mbs_editor *mbs_new_edit(वर्ण *buf, size_t bufsz, size_t ncells)
{
	संरचना mbs_editor *edit = calloc(1, परिमाण(*edit));

	जइ (edit) {
		edit->buf = buf;
		edit->max_bytes = bufsz;
		edit->max_cells = ncells;
		edit->cur_cells = mbs_safe_width(buf);
		edit->cur_bytes = strlen(buf);
	}
	प्रत्यागम edit;
}

वर्ण *mbs_free_edit(संरचना mbs_editor *edit)
{
	वर्ण *ret = edit ? edit->buf : रिक्त;

	मुक्त(edit);
	प्रत्यागम ret;
}

स्थिर size_t mbs_next(ध्रुव वर्ण *str, size_t *ncells)
{
#ifdef HAVE_WIDECHAR
	wchar_t wc;
	size_t n = 0;

	जइ (!str || !*str)
		प्रत्यागम 0;

	n = mbrtowc(&wc, str, MB_CUR_MAX, रिक्त);
	*ncells = wcwidth(wc);
	प्रत्यागम n;
#अण्णहा
	जइ (!str || !*str)
		प्रत्यागम 0;
	*ncells = 1;
	प्रत्यागम 1;
#endif
}

स्थिर size_t mbs_prev(ध्रुव वर्ण *start, ध्रुव वर्ण *end, size_t *ncells)
{
#ifdef HAVE_WIDECHAR
	wchar_t wc = 0;
	ध्रुव वर्ण *p, *prev;
	size_t n = 0;

	जइ (!start || !end || start == end || !*start)
		प्रत्यागम 0;

	prev = p = start;
	जाव (p < end) {
		n = mbrtowc(&wc, p, MB_CUR_MAX, रिक्त);
		prev = p;

		जइ (n == (size_t) -1 || n == (size_t) -2)
			p++;
		अण्णहा
			p += n;
	}

	जइ (prev == end)
		प्रत्यागम 0;
	*ncells = wcwidth(wc);
	प्रत्यागम n;
#अण्णहा
	जइ (!start || !end || start == end || !*start)
		प्रत्यागम 0;
	*ncells = 1;
	प्रत्यागम 1;
#endif
}

पूर्णाङ्क mbs_edit_goto(संरचना mbs_editor *edit, पूर्णाङ्क where)
{
	विकल्प (where) {
	पक्ष MBS_EDIT_वामः:
		जइ (edit->cursor == 0)
			प्रत्यागम 1;
		अण्णहा {
			size_t n, cells;
			n = mbs_prev(edit->buf, edit->buf + edit->cursor, &cells);
			जइ (n) {
				edit->cursor -= n;
				edit->cursor_cells -= cells;
			}
		}
		भङ्ग;
	पक्ष MBS_EDIT_दक्षिणः:
		जइ (edit->cursor_cells >= edit->cur_cells)
			प्रत्यागम 1;
		अण्णहा {
			size_t n, cells;
			n = mbs_next(edit->buf + edit->cursor, &cells);
			जइ (n) {
				edit->cursor += n;
				edit->cursor_cells += cells;
			}
		}
		भङ्ग;
	पक्ष MBS_EDIT_HOME:
		edit->cursor = 0;
		edit->cursor_cells = 0;
		भङ्ग;
	पक्ष MBS_EDIT_END:
		edit->cursor = edit->cur_bytes;
		edit->cursor_cells = edit->cur_cells;
		भङ्ग;
	स्वभाव:
		प्रत्यागम -EINVAL;
	}

	प्रत्यागम 0;
}

/* Remove next MB from @str, returns number of removed bytes */
स्थिर size_t remove_next(वर्ण *str, size_t *ncells)
{
	/* all in bytes! */
	size_t bytes, move_bytes, n;

	n          = mbs_next(str, ncells);
	bytes      = strlen(str);
	move_bytes = bytes - n;

	memmove(str, str + n, move_bytes);
	str[bytes - n] = '\0';
	प्रत्यागम n;
}

स्थिर size_t mbs_निवेशय(वर्ण *str, wint_t c, size_t *ncells)
{
	/* all in bytes! */
	size_t n = 1, bytes;
	वर्ण *in;

#ifdef HAVE_WIDECHAR
	wchar_t wc = (wchar_t) c;
	in = malloc(MB_CUR_MAX);
	जइ (!in)
		प्रत्यागम -1;

	n = wctomb(in, wc);
	जइ (n == (size_t) -1)
		गच्छ out;
	*ncells = wcwidth(wc);
#अण्णहा
	*ncells = 1;
	in = (वर्ण *) &c;
#endif
	bytes       = strlen(str);

	memmove(str + n, str, bytes);
	memcpy(str, in, n);
	str[bytes + n] = '\0';
#ifdef HAVE_WIDECHAR
out:
	मुक्त(in);
#endif
	प्रत्यागम n;
}

स्थिर पूर्णाङ्क mbs_edit_remove(संरचना mbs_editor *edit)
{
	size_t n, ncells;

	जइ (edit->cur_cells == 0 || edit->cursor >= edit->cur_bytes)
		प्रत्यागम 1;

	n = remove_next(edit->buf + edit->cursor, &ncells);
	जइ (n == (size_t)-1)
		प्रत्यागम 1;

	edit->cur_bytes -= n;
	edit->cur_cells = mbs_safe_width(edit->buf);
	प्रत्यागम 0;
}

पूर्णाङ्क mbs_edit_delete(संरचना mbs_editor *edit)
{
	जइ (edit->cursor >= edit->cur_bytes
	    && mbs_edit_goto(edit, MBS_EDIT_वामः) == 1)
		प्रत्यागम 1;

	प्रत्यागम mbs_edit_remove(edit);
}

पूर्णाङ्क mbs_edit_backspace(संरचना mbs_editor *edit)
{
	जइ (mbs_edit_goto(edit, MBS_EDIT_वामः) == 0)
		प्रत्यागम mbs_edit_remove(edit);
	प्रत्यागम 1;
}

पूर्णाङ्क mbs_edit_निवेशय(संरचना mbs_editor *edit, wint_t c)
{
	size_t n, ncells;

	जइ (edit->cur_bytes + MB_CUR_MAX > edit->max_bytes)
		प्रत्यागम 1;

	n = mbs_निवेशय(edit->buf + edit->cursor, c, &ncells);
	जइ (n == (size_t)-1)
		प्रत्यागम 1;

	edit->cursor += n;
	edit->cursor_cells += ncells;
	edit->cur_bytes += n;
	edit->cur_cells = mbs_safe_width(edit->buf);
	प्रत्यागम 0;
}
