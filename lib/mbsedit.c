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

संरचना mbs_editor *mbs_new_edit(अक्षर *buf, माप_प्रकार bufsz, माप_प्रकार ncells)
{
	संरचना mbs_editor *edit = सुस्मृति(1, आकार(*edit));

	जर (edit) {
		edit->buf = buf;
		edit->max_bytes = bufsz;
		edit->max_cells = ncells;
		edit->cur_cells = mbs_safe_width(buf);
		edit->cur_bytes = म_माप(buf);
	}
	परत edit;
}

अक्षर *mbs_free_edit(संरचना mbs_editor *edit)
{
	अक्षर *ret = edit ? edit->buf : रिक्त;

	मुक्त(edit);
	परत ret;
}

स्थिर माप_प्रकार mbs_next(स्थिरांक अक्षर *str, माप_प्रकार *ncells)
{
#ifdef HAVE_WIDECHAR
	ब_अक्षर_प्रकार wc;
	माप_प्रकार n = 0;

	जर (!str || !*str)
		परत 0;

	n = mbrtowc(&wc, str, MB_CUR_MAX, रिक्त);
	*ncells = wcwidth(wc);
	परत n;
#अन्यथा
	जर (!str || !*str)
		परत 0;
	*ncells = 1;
	परत 1;
#endif
}

स्थिर माप_प्रकार mbs_prev(स्थिरांक अक्षर *start, स्थिरांक अक्षर *end, माप_प्रकार *ncells)
{
#ifdef HAVE_WIDECHAR
	ब_अक्षर_प्रकार wc = 0;
	स्थिरांक अक्षर *p, *prev;
	माप_प्रकार n = 0;

	जर (!start || !end || start == end || !*start)
		परत 0;

	prev = p = start;
	पर्यंत (p < end) {
		n = mbrtowc(&wc, p, MB_CUR_MAX, रिक्त);
		prev = p;

		जर (n == (माप_प्रकार) -1 || n == (माप_प्रकार) -2)
			p++;
		अन्यथा
			p += n;
	}

	जर (prev == end)
		परत 0;
	*ncells = wcwidth(wc);
	परत n;
#अन्यथा
	जर (!start || !end || start == end || !*start)
		परत 0;
	*ncells = 1;
	परत 1;
#endif
}

पूर्णांक mbs_edit_goto(संरचना mbs_editor *edit, पूर्णांक where)
{
	निवड (where) {
	स्थिती MBS_EDIT_डावा:
		जर (edit->cursor == 0)
			परत 1;
		अन्यथा {
			माप_प्रकार n, cells;
			n = mbs_prev(edit->buf, edit->buf + edit->cursor, &cells);
			जर (n) {
				edit->cursor -= n;
				edit->cursor_cells -= cells;
			}
		}
		थांबा;
	स्थिती MBS_EDIT_उजवा:
		जर (edit->cursor_cells >= edit->cur_cells)
			परत 1;
		अन्यथा {
			माप_प्रकार n, cells;
			n = mbs_next(edit->buf + edit->cursor, &cells);
			जर (n) {
				edit->cursor += n;
				edit->cursor_cells += cells;
			}
		}
		थांबा;
	स्थिती MBS_EDIT_HOME:
		edit->cursor = 0;
		edit->cursor_cells = 0;
		थांबा;
	स्थिती MBS_EDIT_END:
		edit->cursor = edit->cur_bytes;
		edit->cursor_cells = edit->cur_cells;
		थांबा;
	पूर्वनिर्धारित:
		परत -EINVAL;
	}

	परत 0;
}

/* Remove next MB from @str, returns number of removed bytes */
स्थिर माप_प्रकार remove_next(अक्षर *str, माप_प्रकार *ncells)
{
	/* all in bytes! */
	माप_प्रकार bytes, move_bytes, n;

	n          = mbs_next(str, ncells);
	bytes      = म_माप(str);
	move_bytes = bytes - n;

	स_हटाओ(str, str + n, move_bytes);
	str[bytes - n] = '\0';
	परत n;
}

स्थिर माप_प्रकार mbs_घाला(अक्षर *str, wint_t c, माप_प्रकार *ncells)
{
	/* all in bytes! */
	माप_प्रकार n = 1, bytes;
	अक्षर *in;

#ifdef HAVE_WIDECHAR
	ब_अक्षर_प्रकार wc = (ब_अक्षर_प्रकार) c;
	in = दो_स्मृति(MB_CUR_MAX);
	जर (!in)
		परत -1;

	n = wctomb(in, wc);
	जर (n == (माप_प्रकार) -1)
		जा out;
	*ncells = wcwidth(wc);
#अन्यथा
	*ncells = 1;
	in = (अक्षर *) &c;
#endif
	bytes       = म_माप(str);

	स_हटाओ(str + n, str, bytes);
	स_नकल(str, in, n);
	str[bytes + n] = '\0';
#ifdef HAVE_WIDECHAR
out:
	मुक्त(in);
#endif
	परत n;
}

स्थिर पूर्णांक mbs_edit_remove(संरचना mbs_editor *edit)
{
	माप_प्रकार n, ncells;

	जर (edit->cur_cells == 0 || edit->cursor >= edit->cur_bytes)
		परत 1;

	n = remove_next(edit->buf + edit->cursor, &ncells);
	जर (n == (माप_प्रकार)-1)
		परत 1;

	edit->cur_bytes -= n;
	edit->cur_cells = mbs_safe_width(edit->buf);
	परत 0;
}

पूर्णांक mbs_edit_delete(संरचना mbs_editor *edit)
{
	जर (edit->cursor >= edit->cur_bytes
	    && mbs_edit_goto(edit, MBS_EDIT_डावा) == 1)
		परत 1;

	परत mbs_edit_remove(edit);
}

पूर्णांक mbs_edit_backspace(संरचना mbs_editor *edit)
{
	जर (mbs_edit_goto(edit, MBS_EDIT_डावा) == 0)
		परत mbs_edit_remove(edit);
	परत 1;
}

पूर्णांक mbs_edit_घाला(संरचना mbs_editor *edit, wint_t c)
{
	माप_प्रकार n, ncells;

	जर (edit->cur_bytes + MB_CUR_MAX > edit->max_bytes)
		परत 1;

	n = mbs_घाला(edit->buf + edit->cursor, c, &ncells);
	जर (n == (माप_प्रकार)-1)
		परत 1;

	edit->cursor += n;
	edit->cursor_cells += ncells;
	edit->cur_bytes += n;
	edit->cur_cells = mbs_safe_width(edit->buf);
	परत 0;
}
