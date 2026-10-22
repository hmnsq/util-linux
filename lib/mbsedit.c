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

संरचना mbs_editor *mbs_new_edit(वर्ण *buf, माप_प्रकार bufsz, माप_प्रकार ncells)
{
	संरचना mbs_editor *edit = सुस्मृति(1, परिमाण(*edit));

	यदि (edit) {
		edit->buf = buf;
		edit->max_bytes = bufsz;
		edit->max_cells = ncells;
		edit->cur_cells = mbs_safe_width(buf);
		edit->cur_bytes = म_माप(buf);
	}
	प्रत्यागम edit;
}

वर्ण *mbs_free_edit(संरचना mbs_editor *edit)
{
	वर्ण *ret = edit ? edit->buf : रिक्त;

	मुक्त(edit);
	प्रत्यागम ret;
}

स्थिर माप_प्रकार mbs_next(ध्रुव वर्ण *str, माप_प्रकार *ncells)
{
#ifdef HAVE_WIDECHAR
	ब_अक्षर_प्रकार wc;
	माप_प्रकार n = 0;

	यदि (!str || !*str)
		प्रत्यागम 0;

	n = mbrtowc(&wc, str, MB_CUR_MAX, रिक्त);
	*ncells = wcwidth(wc);
	प्रत्यागम n;
#अन्यथा
	यदि (!str || !*str)
		प्रत्यागम 0;
	*ncells = 1;
	प्रत्यागम 1;
#endif
}

स्थिर माप_प्रकार mbs_prev(ध्रुव वर्ण *start, ध्रुव वर्ण *end, माप_प्रकार *ncells)
{
#ifdef HAVE_WIDECHAR
	ब_अक्षर_प्रकार wc = 0;
	ध्रुव वर्ण *p, *prev;
	माप_प्रकार n = 0;

	यदि (!start || !end || start == end || !*start)
		प्रत्यागम 0;

	prev = p = start;
	यावत् (p < end) {
		n = mbrtowc(&wc, p, MB_CUR_MAX, रिक्त);
		prev = p;

		यदि (n == (माप_प्रकार) -1 || n == (माप_प्रकार) -2)
			p++;
		अन्यथा
			p += n;
	}

	यदि (prev == end)
		प्रत्यागम 0;
	*ncells = wcwidth(wc);
	प्रत्यागम n;
#अन्यथा
	यदि (!start || !end || start == end || !*start)
		प्रत्यागम 0;
	*ncells = 1;
	प्रत्यागम 1;
#endif
}

पूर्णाङ्क mbs_edit_goto(संरचना mbs_editor *edit, पूर्णाङ्क where)
{
	विकल्प (where) {
	पक्ष MBS_EDIT_बायाँ:
		यदि (edit->cursor == 0)
			प्रत्यागम 1;
		अन्यथा {
			माप_प्रकार n, cells;
			n = mbs_prev(edit->buf, edit->buf + edit->cursor, &cells);
			यदि (n) {
				edit->cursor -= n;
				edit->cursor_cells -= cells;
			}
		}
		भङ्ग;
	पक्ष MBS_EDIT_दायाँ:
		यदि (edit->cursor_cells >= edit->cur_cells)
			प्रत्यागम 1;
		अन्यथा {
			माप_प्रकार n, cells;
			n = mbs_next(edit->buf + edit->cursor, &cells);
			यदि (n) {
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
स्थिर माप_प्रकार remove_next(वर्ण *str, माप_प्रकार *ncells)
{
	/* all in bytes! */
	माप_प्रकार bytes, move_bytes, n;

	n          = mbs_next(str, ncells);
	bytes      = म_माप(str);
	move_bytes = bytes - n;

	स_हटाओ(str, str + n, move_bytes);
	str[bytes - n] = '\0';
	प्रत्यागम n;
}

स्थिर माप_प्रकार mbs_जोड़ो(वर्ण *str, wint_t c, माप_प्रकार *ncells)
{
	/* all in bytes! */
	माप_प्रकार n = 1, bytes;
	वर्ण *in;

#ifdef HAVE_WIDECHAR
	ब_अक्षर_प्रकार wc = (ब_अक्षर_प्रकार) c;
	in = दो_स्मृति(MB_CUR_MAX);
	यदि (!in)
		प्रत्यागम -1;

	n = wctomb(in, wc);
	यदि (n == (माप_प्रकार) -1)
		गच्छ out;
	*ncells = wcwidth(wc);
#अन्यथा
	*ncells = 1;
	in = (वर्ण *) &c;
#endif
	bytes       = म_माप(str);

	स_हटाओ(str + n, str, bytes);
	स_नकल(str, in, n);
	str[bytes + n] = '\0';
#ifdef HAVE_WIDECHAR
out:
	मुक्त(in);
#endif
	प्रत्यागम n;
}

स्थिर पूर्णाङ्क mbs_edit_remove(संरचना mbs_editor *edit)
{
	माप_प्रकार n, ncells;

	यदि (edit->cur_cells == 0 || edit->cursor >= edit->cur_bytes)
		प्रत्यागम 1;

	n = remove_next(edit->buf + edit->cursor, &ncells);
	यदि (n == (माप_प्रकार)-1)
		प्रत्यागम 1;

	edit->cur_bytes -= n;
	edit->cur_cells = mbs_safe_width(edit->buf);
	प्रत्यागम 0;
}

पूर्णाङ्क mbs_edit_delete(संरचना mbs_editor *edit)
{
	यदि (edit->cursor >= edit->cur_bytes
	    && mbs_edit_goto(edit, MBS_EDIT_बायाँ) == 1)
		प्रत्यागम 1;

	प्रत्यागम mbs_edit_remove(edit);
}

पूर्णाङ्क mbs_edit_backspace(संरचना mbs_editor *edit)
{
	यदि (mbs_edit_goto(edit, MBS_EDIT_बायाँ) == 0)
		प्रत्यागम mbs_edit_remove(edit);
	प्रत्यागम 1;
}

पूर्णाङ्क mbs_edit_जोड़ो(संरचना mbs_editor *edit, wint_t c)
{
	माप_प्रकार n, ncells;

	यदि (edit->cur_bytes + MB_CUR_MAX > edit->max_bytes)
		प्रत्यागम 1;

	n = mbs_जोड़ो(edit->buf + edit->cursor, c, &ncells);
	यदि (n == (माप_प्रकार)-1)
		प्रत्यागम 1;

	edit->cursor += n;
	edit->cursor_cells += ncells;
	edit->cur_bytes += n;
	edit->cur_cells = mbs_safe_width(edit->buf);
	प्रत्यागम 0;
}
