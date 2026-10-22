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

        यदि (!l)
                प्रत्यागम;

        प्रति (k = l; *k; k++)
                मुक्त(*k);

        *l = रिक्त;
}

वर्ण **ul_strv_free(वर्ण **l)
{
        ul_strv_clear(l);
        मुक्त(l);
        प्रत्यागम रिक्त;
}

वर्ण **ul_strv_copy(वर्ण * ध्रुव *l)
{
        वर्ण **r, **k;

        k = r = malloc(परिमाण(वर्ण *) * (ul_strv_length(l) + 1));
        यदि (!r)
                प्रत्यागम रिक्त;

        यदि (l)
                प्रति (; *l; k++, l++) {
                        *k = strdup(*l);
                        यदि (!*k) {
                                ul_strv_free(r);
                                प्रत्यागम रिक्त;
                        }
                }

        *k = रिक्त;
        प्रत्यागम r;
}

unsigned ul_strv_length(वर्ण * ध्रुव *l) {
        unsigned n = 0;

        यदि (!l)
                प्रत्यागम 0;

        प्रति (; *l; l++)
                n++;

        प्रत्यागम n;
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

        यदि (x) {
                n = x == (ध्रुव वर्ण*) -1 ? 0 : 1;

                va_copy(aq, ap);
                यावत् ((s = va_arg(aq, ध्रुव वर्ण*))) {
                        यदि (s == (ध्रुव वर्ण*) -1)
                                अनुवर्त;

                        n++;
                }

                va_end(aq);
        }

        a = malloc(परिमाण(वर्ण *) * (n + 1));
        यदि (!a)
                प्रत्यागम रिक्त;

        यदि (x) {
                यदि (x != (ध्रुव वर्ण*) -1) {
                        a[i] = strdup(x);
                        यदि (!a[i])
                                गच्छ fail;
                        i++;
                }

                यावत् ((s = va_arg(ap, ध्रुव वर्ण*))) {

                        यदि (s == (ध्रुव वर्ण*) -1)
                                अनुवर्त;

                        a[i] = strdup(s);
                        यदि (!a[i])
                                गच्छ fail;

                        i++;
                }
        }

        a[i] = रिक्त;

        प्रत्यागम a;

fail:
        ul_strv_free(a);
        प्रत्यागम रिक्त;
}

वर्ण **ul_strv_new(ध्रुव वर्ण *x, ...)
{
        वर्ण **r;
        va_list ap;

        va_start(ap, x);
        r = ul_strv_new_ap(x, ap);
        va_end(ap);

        प्रत्यागम r;
}

पूर्णाङ्क ul_strv_extend_strv(वर्ण ***a, वर्ण **b)
{
        पूर्णाङ्क r;
        वर्ण **s;

        UL_STRV_FOREACH(s, b) {
                r = ul_strv_extend(a, *s);
                यदि (r < 0)
                        प्रत्यागम r;
        }

        प्रत्यागम 0;
}

पूर्णाङ्क ul_strv_extend_strv_concat(वर्ण ***a, वर्ण **b, ध्रुव वर्ण *suffix)
{
        पूर्णाङ्क r;
        वर्ण **s;

        UL_STRV_FOREACH(s, b) {
                वर्ण *v;

                v = ul_strconcat(*s, suffix);
                यदि (!v)
                        प्रत्यागम -ENOMEM;

                r = ul_strv_धकेलो(a, v);
                यदि (r < 0) {
                        मुक्त(v);
                        प्रत्यागम r;
                }
        }

        प्रत्यागम 0;
}


#define _FOREACH_WORD(word, length, s, separator, quoted, state)        \
        प्रति ((state) = (s), (word) = ul_split(&(state), &(length), (separator), (quoted)); (word); (word) = ul_split(&(state), &(length), (separator), (quoted)))

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
        यदि (!r)
                प्रत्यागम रिक्त;

        i = 0;
        FOREACH_WORD_SEPARATOR(word, l, s, separator, state) {
                r[i] = strndup(word, l);
                यदि (!r[i]) {
                        ul_strv_free(r);
                        प्रत्यागम रिक्त;
                }

                i++;
        }

        r[i] = रिक्त;
        प्रत्यागम r;
}

वर्ण *ul_strv_join(वर्ण **l, ध्रुव वर्ण *separator)
{
        वर्ण *r, *e;
        वर्ण **s;
        size_t n, k;

        यदि (!separator)
                separator = " ";

        k = strlen(separator);

        n = 0;
        UL_STRV_FOREACH(s, l) {
                यदि (n != 0)
                        n += k;
                n += strlen(*s);
        }

        r = malloc(n + 1);
        यदि (!r)
                प्रत्यागम रिक्त;

        e = r;
        UL_STRV_FOREACH(s, l) {
                यदि (e != r)
                        e = stpcpy(e, separator);

                e = stpcpy(e, *s);
        }

        *e = 0;

        प्रत्यागम r;
}

पूर्णाङ्क ul_strv_धकेलो(वर्ण ***l, वर्ण *मूल्य)
{
        वर्ण **c;
        unsigned n, m;

        यदि (!मूल्य)
                प्रत्यागम 0;

        n = ul_strv_length(*l);

        /* Increase and check for overflow */
        m = n + 2;
        यदि (m < n)
                प्रत्यागम -ENOMEM;

        c = reallocarray(*l, m, परिमाण(वर्ण *));
        यदि (!c)
                प्रत्यागम -ENOMEM;

        c[n] = मूल्य;
        c[n+1] = रिक्त;

        *l = c;
        प्रत्यागम 0;
}

पूर्णाङ्क ul_strv_धकेलो_prepend(वर्ण ***l, वर्ण *मूल्य)
{
        वर्ण **c;
        unsigned n, m, i;

        यदि (!मूल्य)
                प्रत्यागम 0;

        n = ul_strv_length(*l);

        /* increase and check for overflow */
        m = n + 2;
        यदि (m < n)
                प्रत्यागम -ENOMEM;

        c = malloc(परिमाण(वर्ण *) * m);
        यदि (!c)
                प्रत्यागम -ENOMEM;

        प्रति (i = 0; i < n; i++)
                c[i+1] = (*l)[i];

        c[0] = मूल्य;
        c[n+1] = रिक्त;

        मुक्त(*l);
        *l = c;

        प्रत्यागम 0;
}

पूर्णाङ्क ul_strv_consume(वर्ण ***l, वर्ण *मूल्य)
{
        पूर्णाङ्क r;

        r = ul_strv_धकेलो(l, मूल्य);
        यदि (r < 0)
                मुक्त(मूल्य);

        प्रत्यागम r;
}

पूर्णाङ्क ul_strv_consume_prepend(वर्ण ***l, वर्ण *मूल्य)
{
        पूर्णाङ्क r;

        r = ul_strv_धकेलो_prepend(l, मूल्य);
        यदि (r < 0)
                मुक्त(मूल्य);

        प्रत्यागम r;
}

पूर्णाङ्क ul_strv_extend(वर्ण ***l, ध्रुव वर्ण *मूल्य)
{
        वर्ण *v;

        यदि (!मूल्य)
                प्रत्यागम 0;

        v = strdup(मूल्य);
        यदि (!v)
                प्रत्यागम -ENOMEM;

        प्रत्यागम ul_strv_consume(l, v);
}

वर्ण **ul_strv_remove(वर्ण **l, ध्रुव वर्ण *s)
{
        वर्ण **f, **t;

        यदि (!l)
                प्रत्यागम रिक्त;

        प्रतिज्ञा(s);

        /* Drops every occurrence of s in the string list, edits
         * in-place. */

        प्रति (f = t = l; *f; f++)
                यदि (strcmp(*f, s) == 0)
                        मुक्त(*f);
                अन्यथा
                        *(t++) = *f;

        *t = रिक्त;
        प्रत्यागम l;
}

पूर्णाङ्क ul_strv_extendf(वर्ण ***l, ध्रुव वर्ण *format, ...)
{
        va_list ap;
        वर्ण *x;
        पूर्णाङ्क r;

        va_start(ap, format);
        r = vasprintf(&x, format, ap);
        va_end(ap);

        यदि (r < 0)
                प्रत्यागम -ENOMEM;

        प्रत्यागम ul_strv_consume(l, x);
}

पूर्णाङ्क ul_strv_extendv(वर्ण ***l, ध्रुव वर्ण *format, va_list ap)
{
        वर्ण *x;
        पूर्णाङ्क r;

        r = vasprintf(&x, format, ap);
        यदि (r < 0)
                प्रत्यागम -ENOMEM;

        प्रत्यागम ul_strv_consume(l, x);
}

वर्ण **ul_strv_reverse(वर्ण **l)
{
        unsigned n, i;

        n = ul_strv_length(l);
        यदि (n <= 1)
                प्रत्यागम l;

        प्रति (i = 0; i < n / 2; i++) {
                वर्ण *t;

                t = l[i];
                l[i] = l[n-1-i];
                l[n-1-i] = t;
        }

        प्रत्यागम l;
}
