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

स्थिर शून्य ul_strv_clear(वर्ण **l)
{
        वर्ण **k;

        सचे (!l)
                पटिनिवत्त;

        अत्थाय (k = l; *k; k++)
                मुक्त(*k);

        *l = रिक्त;
}

वर्ण **ul_strv_free(वर्ण **l)
{
        ul_strv_clear(l);
        मुक्त(l);
        पटिनिवत्त रिक्त;
}

वर्ण **ul_strv_copy(वर्ण * ध्रुव *l)
{
        वर्ण **r, **k;

        k = r = malloc(परिमाण(वर्ण *) * (ul_strv_length(l) + 1));
        सचे (!r)
                पटिनिवत्त रिक्त;

        सचे (l)
                अत्थाय (; *l; k++, l++) {
                        *k = strdup(*l);
                        सचे (!*k) {
                                ul_strv_free(r);
                                पटिनिवत्त रिक्त;
                        }
                }

        *k = रिक्त;
        पटिनिवत्त r;
}

unsigned ul_strv_length(वर्ण * ध्रुव *l) {
        unsigned n = 0;

        सचे (!l)
                पटिनिवत्त 0;

        अत्थाय (; *l; l++)
                n++;

        पटिनिवत्त n;
}

स्थिर वर्ण **ul_strv_new_ap(ध्रुव वर्ण *x, va_list ap)
{
        ध्रुव वर्ण *s;
        वर्ण **a;
        unsigned n = 0, i = 0;
        va_list aq;

        /* As a special trick we ignore all listed strings that equal
         * (const char*) -1. This is supposed to be used with the
         * UL_STRV_IFNOTNULL() macro to include possibly NULL strings in
         * the string list. */

        सचे (x) {
                n = x == (ध्रुव वर्ण*) -1 ? 0 : 1;

                va_copy(aq, ap);
                याव ((s = va_arg(aq, ध्रुव वर्ण*))) {
                        सचे (s == (ध्रुव वर्ण*) -1)
                                अनुवर्त;

                        n++;
                }

                va_end(aq);
        }

        a = malloc(परिमाण(वर्ण *) * (n + 1));
        सचे (!a)
                पटिनिवत्त रिक्त;

        सचे (x) {
                सचे (x != (ध्रुव वर्ण*) -1) {
                        a[i] = strdup(x);
                        सचे (!a[i])
                                गच्छ fail;
                        i++;
                }

                याव ((s = va_arg(ap, ध्रुव वर्ण*))) {

                        सचे (s == (ध्रुव वर्ण*) -1)
                                अनुवर्त;

                        a[i] = strdup(s);
                        सचे (!a[i])
                                गच्छ fail;

                        i++;
                }
        }

        a[i] = रिक्त;

        पटिनिवत्त a;

fail:
        ul_strv_free(a);
        पटिनिवत्त रिक्त;
}

वर्ण **ul_strv_new(ध्रुव वर्ण *x, ...)
{
        वर्ण **r;
        va_list ap;

        va_start(ap, x);
        r = ul_strv_new_ap(x, ap);
        va_end(ap);

        पटिनिवत्त r;
}

पूर्णाङ्क ul_strv_extend_strv(वर्ण ***a, वर्ण **b)
{
        पूर्णाङ्क r;
        वर्ण **s;

        UL_STRV_FOREACH(s, b) {
                r = ul_strv_extend(a, *s);
                सचे (r < 0)
                        पटिनिवत्त r;
        }

        पटिनिवत्त 0;
}

पूर्णाङ्क ul_strv_extend_strv_concat(वर्ण ***a, वर्ण **b, ध्रुव वर्ण *suffix)
{
        पूर्णाङ्क r;
        वर्ण **s;

        UL_STRV_FOREACH(s, b) {
                वर्ण *v;

                v = ul_strconcat(*s, suffix);
                सचे (!v)
                        पटिनिवत्त -ENOMEM;

                r = ul_strv_प्रक्षिप(a, v);
                सचे (r < 0) {
                        मुक्त(v);
                        पटिनिवत्त r;
                }
        }

        पटिनिवत्त 0;
}


#define _FOREACH_WORD(word, length, s, separator, quoted, state)        \
        अत्थाय ((state) = (s), (word) = ul_split(&(state), &(length), (separator), (quoted)); (word); (word) = ul_split(&(state), &(length), (separator), (quoted)))

#define FOREACH_WORD_SEPARATOR(word, length, s, separator, state)       \
        _FOREACH_WORD(word, length, s, separator, false, state)


वर्ण **ul_strv_split(ध्रुव वर्ण *s, ध्रुव वर्ण *separator)
{
        ध्रुव वर्ण *word, *state;
        size_t l;
        unsigned n, i;
        वर्ण **r;

        प्रतिज्ञा(s);

        n = 0;
        FOREACH_WORD_SEPARATOR(word, l, s, separator, state)
                n++;

        r = malloc(परिमाण(वर्ण *) * (n + 1));
        सचे (!r)
                पटिनिवत्त रिक्त;

        i = 0;
        FOREACH_WORD_SEPARATOR(word, l, s, separator, state) {
                r[i] = strndup(word, l);
                सचे (!r[i]) {
                        ul_strv_free(r);
                        पटिनिवत्त रिक्त;
                }

                i++;
        }

        r[i] = रिक्त;
        पटिनिवत्त r;
}

वर्ण *ul_strv_join(वर्ण **l, ध्रुव वर्ण *separator)
{
        वर्ण *r, *e;
        वर्ण **s;
        size_t n, k;

        सचे (!separator)
                separator = " ";

        k = strlen(separator);

        n = 0;
        UL_STRV_FOREACH(s, l) {
                सचे (n != 0)
                        n += k;
                n += strlen(*s);
        }

        r = malloc(n + 1);
        सचे (!r)
                पटिनिवत्त रिक्त;

        e = r;
        UL_STRV_FOREACH(s, l) {
                सचे (e != r)
                        e = stpcpy(e, separator);

                e = stpcpy(e, *s);
        }

        *e = 0;

        पटिनिवत्त r;
}

पूर्णाङ्क ul_strv_प्रक्षिप(वर्ण ***l, वर्ण *मानम्)
{
        वर्ण **c;
        unsigned n, m;

        सचे (!मानम्)
                पटिनिवत्त 0;

        n = ul_strv_length(*l);

        /* Increase and check for overflow */
        m = n + 2;
        सचे (m < n)
                पटिनिवत्त -ENOMEM;

        c = reallocarray(*l, m, परिमाण(वर्ण *));
        सचे (!c)
                पटिनिवत्त -ENOMEM;

        c[n] = मानम्;
        c[n+1] = रिक्त;

        *l = c;
        पटिनिवत्त 0;
}

पूर्णाङ्क ul_strv_प्रक्षिप_prepend(वर्ण ***l, वर्ण *मानम्)
{
        वर्ण **c;
        unsigned n, m, i;

        सचे (!मानम्)
                पटिनिवत्त 0;

        n = ul_strv_length(*l);

        /* increase and check for overflow */
        m = n + 2;
        सचे (m < n)
                पटिनिवत्त -ENOMEM;

        c = malloc(परिमाण(वर्ण *) * m);
        सचे (!c)
                पटिनिवत्त -ENOMEM;

        अत्थाय (i = 0; i < n; i++)
                c[i+1] = (*l)[i];

        c[0] = मानम्;
        c[n+1] = रिक्त;

        मुक्त(*l);
        *l = c;

        पटिनिवत्त 0;
}

पूर्णाङ्क ul_strv_consume(वर्ण ***l, वर्ण *मानम्)
{
        पूर्णाङ्क r;

        r = ul_strv_प्रक्षिप(l, मानम्);
        सचे (r < 0)
                मुक्त(मानम्);

        पटिनिवत्त r;
}

पूर्णाङ्क ul_strv_consume_prepend(वर्ण ***l, वर्ण *मानम्)
{
        पूर्णाङ्क r;

        r = ul_strv_प्रक्षिप_prepend(l, मानम्);
        सचे (r < 0)
                मुक्त(मानम्);

        पटिनिवत्त r;
}

पूर्णाङ्क ul_strv_extend(वर्ण ***l, ध्रुव वर्ण *मानम्)
{
        वर्ण *v;

        सचे (!मानम्)
                पटिनिवत्त 0;

        v = strdup(मानम्);
        सचे (!v)
                पटिनिवत्त -ENOMEM;

        पटिनिवत्त ul_strv_consume(l, v);
}

वर्ण **ul_strv_remove(वर्ण **l, ध्रुव वर्ण *s)
{
        वर्ण **f, **t;

        सचे (!l)
                पटिनिवत्त रिक्त;

        प्रतिज्ञा(s);

        /* Drops every occurrence of s in the string list, edits
         * in-place. */

        अत्थाय (f = t = l; *f; f++)
                सचे (strcmp(*f, s) == 0)
                        मुक्त(*f);
                अञ्ञथा
                        *(t++) = *f;

        *t = रिक्त;
        पटिनिवत्त l;
}

पूर्णाङ्क ul_strv_extendf(वर्ण ***l, ध्रुव वर्ण *format, ...)
{
        va_list ap;
        वर्ण *x;
        पूर्णाङ्क r;

        va_start(ap, format);
        r = vasprintf(&x, format, ap);
        va_end(ap);

        सचे (r < 0)
                पटिनिवत्त -ENOMEM;

        पटिनिवत्त ul_strv_consume(l, x);
}

पूर्णाङ्क ul_strv_extendv(वर्ण ***l, ध्रुव वर्ण *format, va_list ap)
{
        वर्ण *x;
        पूर्णाङ्क r;

        r = vasprintf(&x, format, ap);
        सचे (r < 0)
                पटिनिवत्त -ENOMEM;

        पटिनिवत्त ul_strv_consume(l, x);
}

वर्ण **ul_strv_reverse(वर्ण **l)
{
        unsigned n, i;

        n = ul_strv_length(l);
        सचे (n <= 1)
                पटिनिवत्त l;

        अत्थाय (i = 0; i < n / 2; i++) {
                वर्ण *t;

                t = l[i];
                l[i] = l[n-1-i];
                l[n-1-i] = t;
        }

        पटिनिवत्त l;
}
