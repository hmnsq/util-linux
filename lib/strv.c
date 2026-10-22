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

#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <stdarg.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <errno.h>
#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <assert.h>

#अन्तर्भाव "strutils.h"
#अन्तर्भाव "strv.h"

स्थिर शून्य ul_strv_clear(अक्षर **l)
{
        अक्षर **k;

        जर (!l)
                परत;

        साठी (k = l; *k; k++)
                मुक्त(*k);

        *l = रिक्त;
}

अक्षर **ul_strv_free(अक्षर **l)
{
        ul_strv_clear(l);
        मुक्त(l);
        परत रिक्त;
}

अक्षर **ul_strv_copy(अक्षर * स्थिरांक *l)
{
        अक्षर **r, **k;

        k = r = दो_स्मृति(आकार(अक्षर *) * (ul_strv_length(l) + 1));
        जर (!r)
                परत रिक्त;

        जर (l)
                साठी (; *l; k++, l++) {
                        *k = strdup(*l);
                        जर (!*k) {
                                ul_strv_free(r);
                                परत रिक्त;
                        }
                }

        *k = रिक्त;
        परत r;
}

अचिन्हित ul_strv_length(अक्षर * स्थिरांक *l) {
        अचिन्हित n = 0;

        जर (!l)
                परत 0;

        साठी (; *l; l++)
                n++;

        परत n;
}

स्थिर अक्षर **ul_strv_new_ap(स्थिरांक अक्षर *x, बहु_सूची ap)
{
        स्थिरांक अक्षर *s;
        अक्षर **a;
        अचिन्हित n = 0, i = 0;
        बहु_सूची aq;

        /* As a special trick we ignore all listed strings that equal
         * (const char*) -1. This is supposed to be used with the
         * UL_STRV_IFNOTNULL() macro to include possibly NULL strings in
         * the string list. */

        जर (x) {
                n = x == (स्थिरांक अक्षर*) -1 ? 0 : 1;

                va_copy(aq, ap);
                पर्यंत ((s = बहु_तर्क(aq, स्थिरांक अक्षर*))) {
                        जर (s == (स्थिरांक अक्षर*) -1)
                                सुरूठेवा;

                        n++;
                }

                बहु_पूर्ण(aq);
        }

        a = दो_स्मृति(आकार(अक्षर *) * (n + 1));
        जर (!a)
                परत रिक्त;

        जर (x) {
                जर (x != (स्थिरांक अक्षर*) -1) {
                        a[i] = strdup(x);
                        जर (!a[i])
                                जा fail;
                        i++;
                }

                पर्यंत ((s = बहु_तर्क(ap, स्थिरांक अक्षर*))) {

                        जर (s == (स्थिरांक अक्षर*) -1)
                                सुरूठेवा;

                        a[i] = strdup(s);
                        जर (!a[i])
                                जा fail;

                        i++;
                }
        }

        a[i] = रिक्त;

        परत a;

fail:
        ul_strv_free(a);
        परत रिक्त;
}

अक्षर **ul_strv_new(स्थिरांक अक्षर *x, ...)
{
        अक्षर **r;
        बहु_सूची ap;

        बहु_शुरू(ap, x);
        r = ul_strv_new_ap(x, ap);
        बहु_पूर्ण(ap);

        परत r;
}

पूर्णांक ul_strv_extend_strv(अक्षर ***a, अक्षर **b)
{
        पूर्णांक r;
        अक्षर **s;

        UL_STRV_FOREACH(s, b) {
                r = ul_strv_extend(a, *s);
                जर (r < 0)
                        परत r;
        }

        परत 0;
}

पूर्णांक ul_strv_extend_strv_concat(अक्षर ***a, अक्षर **b, स्थिरांक अक्षर *suffix)
{
        पूर्णांक r;
        अक्षर **s;

        UL_STRV_FOREACH(s, b) {
                अक्षर *v;

                v = ul_strconcat(*s, suffix);
                जर (!v)
                        परत -ENOMEM;

                r = ul_strv_ढकला(a, v);
                जर (r < 0) {
                        मुक्त(v);
                        परत r;
                }
        }

        परत 0;
}


#define _FOREACH_WORD(word, length, s, separator, quoted, state)        \
        साठी ((state) = (s), (word) = ul_split(&(state), &(length), (separator), (quoted)); (word); (word) = ul_split(&(state), &(length), (separator), (quoted)))

#define FOREACH_WORD_SEPARATOR(word, length, s, separator, state)       \
        _FOREACH_WORD(word, length, s, separator, false, state)


अक्षर **ul_strv_split(स्थिरांक अक्षर *s, स्थिरांक अक्षर *separator)
{
        स्थिरांक अक्षर *word, *state;
        माप_प्रकार l;
        अचिन्हित n, i;
        अक्षर **r;

        प्रतिज्ञा(s);

        n = 0;
        FOREACH_WORD_SEPARATOR(word, l, s, separator, state)
                n++;

        r = दो_स्मृति(आकार(अक्षर *) * (n + 1));
        जर (!r)
                परत रिक्त;

        i = 0;
        FOREACH_WORD_SEPARATOR(word, l, s, separator, state) {
                r[i] = strndup(word, l);
                जर (!r[i]) {
                        ul_strv_free(r);
                        परत रिक्त;
                }

                i++;
        }

        r[i] = रिक्त;
        परत r;
}

अक्षर *ul_strv_join(अक्षर **l, स्थिरांक अक्षर *separator)
{
        अक्षर *r, *e;
        अक्षर **s;
        माप_प्रकार n, k;

        जर (!separator)
                separator = " ";

        k = म_माप(separator);

        n = 0;
        UL_STRV_FOREACH(s, l) {
                जर (n != 0)
                        n += k;
                n += म_माप(*s);
        }

        r = दो_स्मृति(n + 1);
        जर (!r)
                परत रिक्त;

        e = r;
        UL_STRV_FOREACH(s, l) {
                जर (e != r)
                        e = stpcpy(e, separator);

                e = stpcpy(e, *s);
        }

        *e = 0;

        परत r;
}

पूर्णांक ul_strv_ढकला(अक्षर ***l, अक्षर *मूल्य)
{
        अक्षर **c;
        अचिन्हित n, m;

        जर (!मूल्य)
                परत 0;

        n = ul_strv_length(*l);

        /* Increase and check for overflow */
        m = n + 2;
        जर (m < n)
                परत -ENOMEM;

        c = reallocarray(*l, m, आकार(अक्षर *));
        जर (!c)
                परत -ENOMEM;

        c[n] = मूल्य;
        c[n+1] = रिक्त;

        *l = c;
        परत 0;
}

पूर्णांक ul_strv_ढकला_prepend(अक्षर ***l, अक्षर *मूल्य)
{
        अक्षर **c;
        अचिन्हित n, m, i;

        जर (!मूल्य)
                परत 0;

        n = ul_strv_length(*l);

        /* increase and check for overflow */
        m = n + 2;
        जर (m < n)
                परत -ENOMEM;

        c = दो_स्मृति(आकार(अक्षर *) * m);
        जर (!c)
                परत -ENOMEM;

        साठी (i = 0; i < n; i++)
                c[i+1] = (*l)[i];

        c[0] = मूल्य;
        c[n+1] = रिक्त;

        मुक्त(*l);
        *l = c;

        परत 0;
}

पूर्णांक ul_strv_consume(अक्षर ***l, अक्षर *मूल्य)
{
        पूर्णांक r;

        r = ul_strv_ढकला(l, मूल्य);
        जर (r < 0)
                मुक्त(मूल्य);

        परत r;
}

पूर्णांक ul_strv_consume_prepend(अक्षर ***l, अक्षर *मूल्य)
{
        पूर्णांक r;

        r = ul_strv_ढकला_prepend(l, मूल्य);
        जर (r < 0)
                मुक्त(मूल्य);

        परत r;
}

पूर्णांक ul_strv_extend(अक्षर ***l, स्थिरांक अक्षर *मूल्य)
{
        अक्षर *v;

        जर (!मूल्य)
                परत 0;

        v = strdup(मूल्य);
        जर (!v)
                परत -ENOMEM;

        परत ul_strv_consume(l, v);
}

अक्षर **ul_strv_remove(अक्षर **l, स्थिरांक अक्षर *s)
{
        अक्षर **f, **t;

        जर (!l)
                परत रिक्त;

        प्रतिज्ञा(s);

        /* Drops every occurrence of s in the string list, edits
         * in-place. */

        साठी (f = t = l; *f; f++)
                जर (म_भेद(*f, s) == 0)
                        मुक्त(*f);
                अन्यथा
                        *(t++) = *f;

        *t = रिक्त;
        परत l;
}

पूर्णांक ul_strv_extendf(अक्षर ***l, स्थिरांक अक्षर *format, ...)
{
        बहु_सूची ap;
        अक्षर *x;
        पूर्णांक r;

        बहु_शुरू(ap, format);
        r = vasprintf(&x, format, ap);
        बहु_पूर्ण(ap);

        जर (r < 0)
                परत -ENOMEM;

        परत ul_strv_consume(l, x);
}

पूर्णांक ul_strv_extendv(अक्षर ***l, स्थिरांक अक्षर *format, बहु_सूची ap)
{
        अक्षर *x;
        पूर्णांक r;

        r = vasprintf(&x, format, ap);
        जर (r < 0)
                परत -ENOMEM;

        परत ul_strv_consume(l, x);
}

अक्षर **ul_strv_reverse(अक्षर **l)
{
        अचिन्हित n, i;

        n = ul_strv_length(l);
        जर (n <= 1)
                परत l;

        साठी (i = 0; i < n / 2; i++) {
                अक्षर *t;

                t = l[i];
                l[i] = l[n-1-i];
                l[n-1-i] = t;
        }

        परत l;
}
