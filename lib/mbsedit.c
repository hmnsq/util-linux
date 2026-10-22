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
#समावेश <stdlib.h>
#समावेश <string.h>
#समावेश <errno.h>
#समावेश <stdio.h>

#समावेश "mbsalign.h"
#समावेश "mbsedit.h"

काष्ठा mbs_editor *mbs_new_edit(अक्षर *buf, माप_प्रकार bufsz, माप_प्रकार ncells)
{
	काष्ठा mbs_editor *edit = सुस्मृति(1, माप(*edit));

	अगर (edit) {
		edit->buf = buf;
		edit->max_bytes = bufsz;
		edit->max_cells = ncells;
		edit->cur_cells = mbs_safe_width(buf);
		edit->cur_bytes = म_माप(buf);
	}
	वापस edit;
}

अक्षर *mbs_free_edit(काष्ठा mbs_editor *edit)
{
	अक्षर *ret = edit ? edit->buf : शून्य;

	मुक्त(edit);
	वापस ret;
}

अटल माप_प्रकार mbs_अग्र(स्थिर अक्षर *str, माप_प्रकार *ncells)
{
#अगर_घोषित HAVE_WIDECHAR
	ब_अक्षर_प्रकार wc;
	माप_प्रकार n = 0;

	अगर (!str || !*str)
		वापस 0;

	n = mbrtowc(&wc, str, MB_CUR_MAX, शून्य);
	*ncells = wcwidth(wc);
	वापस n;
#अन्यथा
	अगर (!str || !*str)
		वापस 0;
	*ncells = 1;
	वापस 1;
#पूर्ण_अगर
}

अटल माप_प्रकार mbs_पूर्व(स्थिर अक्षर *start, स्थिर अक्षर *end, माप_प्रकार *ncells)
{
#अगर_घोषित HAVE_WIDECHAR
	ब_अक्षर_प्रकार wc = 0;
	स्थिर अक्षर *p, *पूर्व;
	माप_प्रकार n = 0;

	अगर (!start || !end || start == end || !*start)
		वापस 0;

	पूर्व = p = start;
	जबतक (p < end) {
		n = mbrtowc(&wc, p, MB_CUR_MAX, शून्य);
		पूर्व = p;

		अगर (n == (माप_प्रकार) -1 || n == (माप_प्रकार) -2)
			p++;
		अन्यथा
			p += n;
	}

	अगर (पूर्व == end)
		वापस 0;
	*ncells = wcwidth(wc);
	वापस n;
#अन्यथा
	अगर (!start || !end || start == end || !*start)
		वापस 0;
	*ncells = 1;
	वापस 1;
#पूर्ण_अगर
}

पूर्णांक mbs_edit_goto(काष्ठा mbs_editor *edit, पूर्णांक where)
{
	चयन (where) {
	हाल MBS_EDIT_बायाँ:
		अगर (edit->cursor == 0)
			वापस 1;
		अन्यथा {
			माप_प्रकार n, cells;
			n = mbs_पूर्व(edit->buf, edit->buf + edit->cursor, &cells);
			अगर (n) {
				edit->cursor -= n;
				edit->cursor_cells -= cells;
			}
		}
		अवरोध;
	हाल MBS_EDIT_दायाँ:
		अगर (edit->cursor_cells >= edit->cur_cells)
			वापस 1;
		अन्यथा {
			माप_प्रकार n, cells;
			n = mbs_अग्र(edit->buf + edit->cursor, &cells);
			अगर (n) {
				edit->cursor += n;
				edit->cursor_cells += cells;
			}
		}
		अवरोध;
	हाल MBS_EDIT_HOME:
		edit->cursor = 0;
		edit->cursor_cells = 0;
		अवरोध;
	हाल MBS_EDIT_END:
		edit->cursor = edit->cur_bytes;
		edit->cursor_cells = edit->cur_cells;
		अवरोध;
	शेष:
		वापस -EINVAL;
	}

	वापस 0;
}

/* Remove next MB from @str, returns number of removed bytes */
अटल माप_प्रकार remove_अग्र(अक्षर *str, माप_प्रकार *ncells)
{
	/* all in bytes! */
	माप_प्रकार bytes, move_bytes, n;

	n          = mbs_अग्र(str, ncells);
	bytes      = म_माप(str);
	move_bytes = bytes - n;

	स_हटाओ(str, str + n, move_bytes);
	str[bytes - n] = '\0';
	वापस n;
}

अटल माप_प्रकार mbs_जोड़ो(अक्षर *str, wint_t c, माप_प्रकार *ncells)
{
	/* all in bytes! */
	माप_प्रकार n = 1, bytes;
	अक्षर *in;

#अगर_घोषित HAVE_WIDECHAR
	ब_अक्षर_प्रकार wc = (ब_अक्षर_प्रकार) c;
	in = दो_स्मृति(MB_CUR_MAX);
	अगर (!in)
		वापस -1;

	n = wctomb(in, wc);
	अगर (n == (माप_प्रकार) -1)
		जाओ out;
	*ncells = wcwidth(wc);
#अन्यथा
	*ncells = 1;
	in = (अक्षर *) &c;
#पूर्ण_अगर
	bytes       = म_माप(str);

	स_हटाओ(str + n, str, bytes);
	स_नकल(str, in, n);
	str[bytes + n] = '\0';
#अगर_घोषित HAVE_WIDECHAR
out:
	मुक्त(in);
#पूर्ण_अगर
	वापस n;
}

अटल पूर्णांक mbs_edit_remove(काष्ठा mbs_editor *edit)
{
	माप_प्रकार n, ncells;

	अगर (edit->cur_cells == 0 || edit->cursor >= edit->cur_bytes)
		वापस 1;

	n = remove_अग्र(edit->buf + edit->cursor, &ncells);
	अगर (n == (माप_प्रकार)-1)
		वापस 1;

	edit->cur_bytes -= n;
	edit->cur_cells = mbs_safe_width(edit->buf);
	वापस 0;
}

पूर्णांक mbs_edit_delete(काष्ठा mbs_editor *edit)
{
	अगर (edit->cursor >= edit->cur_bytes
	    && mbs_edit_goto(edit, MBS_EDIT_बायाँ) == 1)
		वापस 1;

	वापस mbs_edit_remove(edit);
}

पूर्णांक mbs_edit_backspace(काष्ठा mbs_editor *edit)
{
	अगर (mbs_edit_goto(edit, MBS_EDIT_बायाँ) == 0)
		वापस mbs_edit_remove(edit);
	वापस 1;
}

पूर्णांक mbs_edit_जोड़ो(काष्ठा mbs_editor *edit, wint_t c)
{
	माप_प्रकार n, ncells;

	अगर (edit->cur_bytes + MB_CUR_MAX > edit->max_bytes)
		वापस 1;

	n = mbs_जोड़ो(edit->buf + edit->cursor, c, &ncells);
	अगर (n == (माप_प्रकार)-1)
		वापस 1;

	edit->cursor += n;
	edit->cursor_cells += ncells;
	edit->cur_bytes += n;
	edit->cur_cells = mbs_safe_width(edit->buf);
	वापस 0;
}
