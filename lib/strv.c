/*
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Copyright (C) 2010 Lennart Poettering
 * Copyright (C) 2015-2022 Karel Zak <kzak@redhat.com>
 *
 * This is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 *
 * Copyright (C) 2015 Karel Zak <kzak@redhat.com>
 *    Modified the original version from systemd project for util-linux.
 */

#समावेश <stdlib.h>
#समावेश <stdarg.h>
#समावेश <string.h>
#समावेश <errno.h>
#समावेश <stdbool.h>
#समावेश <assert.h>

#समावेश "strutils.h"
#समावेश "strv.h"

अटल व्योम ul_strv_clear(अक्षर **l)
{
        अक्षर **k;

        अगर (!l)
                वापस;

        क्रम (k = l; *k; k++)
                मुक्त(*k);

        *l = शून्य;
}

अक्षर **ul_strv_free(अक्षर **l)
{
        ul_strv_clear(l);
        मुक्त(l);
        वापस शून्य;
}

अक्षर **ul_strv_copy(अक्षर * स्थिर *l)
{
        अक्षर **r, **k;

        k = r = दो_स्मृति(माप(अक्षर *) * (ul_strv_लंबाई(l) + 1));
        अगर (!r)
                वापस शून्य;

        अगर (l)
                क्रम (; *l; k++, l++) {
                        *k = strdup(*l);
                        अगर (!*k) {
                                ul_strv_free(r);
                                वापस शून्य;
                        }
                }

        *k = शून्य;
        वापस r;
}

अचिन्हित ul_strv_लंबाई(अक्षर * स्थिर *l) {
        अचिन्हित n = 0;

        अगर (!l)
                वापस 0;

        क्रम (; *l; l++)
                n++;

        वापस n;
}

अटल अक्षर **ul_strv_new_ap(स्थिर अक्षर *x, बहु_सूची ap)
{
        स्थिर अक्षर *s;
        अक्षर **a;
        अचिन्हित n = 0, i = 0;
        बहु_सूची aq;

        /* As a special trick we ignore all listed strings that equal
         * (const char*) -1. This is supposed to be used with the
         * UL_STRV_IFNOTNULL() macro to include possibly NULL strings in
         * the string list. */

        अगर (x) {
                n = x == (स्थिर अक्षर*) -1 ? 0 : 1;

                va_copy(aq, ap);
                जबतक ((s = बहु_तर्क(aq, स्थिर अक्षर*))) {
                        अगर (s == (स्थिर अक्षर*) -1)
                                जारी;

                        n++;
                }

                बहु_पूर्ण(aq);
        }

        a = दो_स्मृति(माप(अक्षर *) * (n + 1));
        अगर (!a)
                वापस शून्य;

        अगर (x) {
                अगर (x != (स्थिर अक्षर*) -1) {
                        a[i] = strdup(x);
                        अगर (!a[i])
                                जाओ fail;
                        i++;
                }

                जबतक ((s = बहु_तर्क(ap, स्थिर अक्षर*))) {

                        अगर (s == (स्थिर अक्षर*) -1)
                                जारी;

                        a[i] = strdup(s);
                        अगर (!a[i])
                                जाओ fail;

                        i++;
                }
        }

        a[i] = शून्य;

        वापस a;

fail:
        ul_strv_free(a);
        वापस शून्य;
}

अक्षर **ul_strv_new(स्थिर अक्षर *x, ...)
{
        अक्षर **r;
        बहु_सूची ap;

        बहु_शुरू(ap, x);
        r = ul_strv_new_ap(x, ap);
        बहु_पूर्ण(ap);

        वापस r;
}

पूर्णांक ul_strv_extend_strv(अक्षर ***a, अक्षर **b)
{
        पूर्णांक r;
        अक्षर **s;

        UL_STRV_FOREACH(s, b) {
                r = ul_strv_extend(a, *s);
                अगर (r < 0)
                        वापस r;
        }

        वापस 0;
}

पूर्णांक ul_strv_extend_strv_concat(अक्षर ***a, अक्षर **b, स्थिर अक्षर *suffix)
{
        पूर्णांक r;
        अक्षर **s;

        UL_STRV_FOREACH(s, b) {
                अक्षर *v;

                v = ul_strconcat(*s, suffix);
                अगर (!v)
                        वापस -ENOMEM;

                r = ul_strv_धकेलो(a, v);
                अगर (r < 0) {
                        मुक्त(v);
                        वापस r;
                }
        }

        वापस 0;
}


#घोषणा _FOREACH_WORD(word, लंबाई, s, separator, quoted, state)        \
        क्रम ((state) = (s), (word) = ul_split(&(state), &(लंबाई), (separator), (quoted)); (word); (word) = ul_split(&(state), &(लंबाई), (separator), (quoted)))

#घोषणा FOREACH_WORD_SEPARATOR(word, लंबाई, s, separator, state)       \
        _FOREACH_WORD(word, लंबाई, s, separator, false, state)


अक्षर **ul_strv_split(स्थिर अक्षर *s, स्थिर अक्षर *separator)
{
        स्थिर अक्षर *word, *state;
        माप_प्रकार l;
        अचिन्हित n, i;
        अक्षर **r;

        निश्चित(s);

        n = 0;
        FOREACH_WORD_SEPARATOR(word, l, s, separator, state)
                n++;

        r = दो_स्मृति(माप(अक्षर *) * (n + 1));
        अगर (!r)
                वापस शून्य;

        i = 0;
        FOREACH_WORD_SEPARATOR(word, l, s, separator, state) {
                r[i] = strndup(word, l);
                अगर (!r[i]) {
                        ul_strv_free(r);
                        वापस शून्य;
                }

                i++;
        }

        r[i] = शून्य;
        वापस r;
}

अक्षर *ul_strv_join(अक्षर **l, स्थिर अक्षर *separator)
{
        अक्षर *r, *e;
        अक्षर **s;
        माप_प्रकार n, k;

        अगर (!separator)
                separator = " ";

        k = म_माप(separator);

        n = 0;
        UL_STRV_FOREACH(s, l) {
                अगर (n != 0)
                        n += k;
                n += म_माप(*s);
        }

        r = दो_स्मृति(n + 1);
        अगर (!r)
                वापस शून्य;

        e = r;
        UL_STRV_FOREACH(s, l) {
                अगर (e != r)
                        e = stpcpy(e, separator);

                e = stpcpy(e, *s);
        }

        *e = 0;

        वापस r;
}

पूर्णांक ul_strv_धकेलो(अक्षर ***l, अक्षर *मूल्य)
{
        अक्षर **c;
        अचिन्हित n, m;

        अगर (!मूल्य)
                वापस 0;

        n = ul_strv_लंबाई(*l);

        /* Increase and check for overflow */
        m = n + 2;
        अगर (m < n)
                वापस -ENOMEM;

        c = reallocarray(*l, m, माप(अक्षर *));
        अगर (!c)
                वापस -ENOMEM;

        c[n] = मूल्य;
        c[n+1] = शून्य;

        *l = c;
        वापस 0;
}

पूर्णांक ul_strv_धकेलो_prepend(अक्षर ***l, अक्षर *मूल्य)
{
        अक्षर **c;
        अचिन्हित n, m, i;

        अगर (!मूल्य)
                वापस 0;

        n = ul_strv_लंबाई(*l);

        /* increase and check for overflow */
        m = n + 2;
        अगर (m < n)
                वापस -ENOMEM;

        c = दो_स्मृति(माप(अक्षर *) * m);
        अगर (!c)
                वापस -ENOMEM;

        क्रम (i = 0; i < n; i++)
                c[i+1] = (*l)[i];

        c[0] = मूल्य;
        c[n+1] = शून्य;

        मुक्त(*l);
        *l = c;

        वापस 0;
}

पूर्णांक ul_strv_consume(अक्षर ***l, अक्षर *मूल्य)
{
        पूर्णांक r;

        r = ul_strv_धकेलो(l, मूल्य);
        अगर (r < 0)
                मुक्त(मूल्य);

        वापस r;
}

पूर्णांक ul_strv_consume_prepend(अक्षर ***l, अक्षर *मूल्य)
{
        पूर्णांक r;

        r = ul_strv_धकेलो_prepend(l, मूल्य);
        अगर (r < 0)
                मुक्त(मूल्य);

        वापस r;
}

पूर्णांक ul_strv_extend(अक्षर ***l, स्थिर अक्षर *मूल्य)
{
        अक्षर *v;

        अगर (!मूल्य)
                वापस 0;

        v = strdup(मूल्य);
        अगर (!v)
                वापस -ENOMEM;

        वापस ul_strv_consume(l, v);
}

अक्षर **ul_strv_remove(अक्षर **l, स्थिर अक्षर *s)
{
        अक्षर **f, **t;

        अगर (!l)
                वापस शून्य;

        निश्चित(s);

        /* Drops every occurrence of s in the string list, edits
         * in-place. */

        क्रम (f = t = l; *f; f++)
                अगर (म_भेद(*f, s) == 0)
                        मुक्त(*f);
                अन्यथा
                        *(t++) = *f;

        *t = शून्य;
        वापस l;
}

पूर्णांक ul_strv_extendf(अक्षर ***l, स्थिर अक्षर *format, ...)
{
        बहु_सूची ap;
        अक्षर *x;
        पूर्णांक r;

        बहु_शुरू(ap, format);
        r = vasprintf(&x, format, ap);
        बहु_पूर्ण(ap);

        अगर (r < 0)
                वापस -ENOMEM;

        वापस ul_strv_consume(l, x);
}

पूर्णांक ul_strv_extendv(अक्षर ***l, स्थिर अक्षर *format, बहु_सूची ap)
{
        अक्षर *x;
        पूर्णांक r;

        r = vasprintf(&x, format, ap);
        अगर (r < 0)
                वापस -ENOMEM;

        वापस ul_strv_consume(l, x);
}

अक्षर **ul_strv_reverse(अक्षर **l)
{
        अचिन्हित n, i;

        n = ul_strv_लंबाई(l);
        अगर (n <= 1)
                वापस l;

        क्रम (i = 0; i < n / 2; i++) {
                अक्षर *t;

                t = l[i];
                l[i] = l[n-1-i];
                l[n-1-i] = t;
        }

        वापस l;
}
