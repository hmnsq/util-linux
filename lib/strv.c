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

#شامل <stdlib.h>
#شامل <stdarg.h>
#شامل <string.h>
#شامل <errno.h>
#شامل <stdbool.h>
#شامل <assert.h>

#شامل "strutils.h"
#شامل "strv.h"

ایستا تهی ul_strv_clear(نویسه **l)
{
        نویسه **k;

        اگر (!l)
                بازگشت;

        برای (k = l; *k; k++)
                آزاد(*k);

        *l = پوچ;
}

نویسه **ul_strv_free(نویسه **l)
{
        ul_strv_clear(l);
        آزاد(l);
        بازگشت پوچ;
}

نویسه **ul_strv_copy(نویسه * ثابت *l)
{
        نویسه **r, **k;

        k = r = malloc(اندازه(نویسه *) * (ul_strv_length(l) + 1));
        اگر (!r)
                بازگشت پوچ;

        اگر (l)
                برای (; *l; k++, l++) {
                        *k = strdup(*l);
                        اگر (!*k) {
                                ul_strv_free(r);
                                بازگشت پوچ;
                        }
                }

        *k = پوچ;
        بازگشت r;
}

unsigned ul_strv_length(نویسه * ثابت *l) {
        unsigned n = 0;

        اگر (!l)
                بازگشت 0;

        برای (; *l; l++)
                n++;

        بازگشت n;
}

ایستا نویسه **ul_strv_new_ap(ثابت نویسه *x, va_list ap)
{
        ثابت نویسه *s;
        نویسه **a;
        unsigned n = 0, i = 0;
        va_list aq;

        /* As a special trick we ignore all listed strings that equal
         * (const char*) -1. This is supposed to be used with the
         * UL_STRV_IFNOTNULL() macro to include possibly NULL strings in
         * the string list. */

        اگر (x) {
                n = x == (ثابت نویسه*) -1 ? 0 : 1;

                va_copy(aq, ap);
                وقتی ((s = va_arg(aq, ثابت نویسه*))) {
                        اگر (s == (ثابت نویسه*) -1)
                                ادامه;

                        n++;
                }

                va_end(aq);
        }

        a = malloc(اندازه(نویسه *) * (n + 1));
        اگر (!a)
                بازگشت پوچ;

        اگر (x) {
                اگر (x != (ثابت نویسه*) -1) {
                        a[i] = strdup(x);
                        اگر (!a[i])
                                برو fail;
                        i++;
                }

                وقتی ((s = va_arg(ap, ثابت نویسه*))) {

                        اگر (s == (ثابت نویسه*) -1)
                                ادامه;

                        a[i] = strdup(s);
                        اگر (!a[i])
                                برو fail;

                        i++;
                }
        }

        a[i] = پوچ;

        بازگشت a;

fail:
        ul_strv_free(a);
        بازگشت پوچ;
}

نویسه **ul_strv_new(ثابت نویسه *x, ...)
{
        نویسه **r;
        va_list ap;

        va_start(ap, x);
        r = ul_strv_new_ap(x, ap);
        va_end(ap);

        بازگشت r;
}

صحیح ul_strv_extend_strv(نویسه ***a, نویسه **b)
{
        صحیح r;
        نویسه **s;

        UL_STRV_FOREACH(s, b) {
                r = ul_strv_extend(a, *s);
                اگر (r < 0)
                        بازگشت r;
        }

        بازگشت 0;
}

صحیح ul_strv_extend_strv_concat(نویسه ***a, نویسه **b, ثابت نویسه *suffix)
{
        صحیح r;
        نویسه **s;

        UL_STRV_FOREACH(s, b) {
                نویسه *v;

                v = ul_strconcat(*s, suffix);
                اگر (!v)
                        بازگشت -ENOMEM;

                r = ul_strv_افزودن(a, v);
                اگر (r < 0) {
                        آزاد(v);
                        بازگشت r;
                }
        }

        بازگشت 0;
}


#define _FOREACH_WORD(word, length, s, separator, quoted, state)        \
        برای ((state) = (s), (word) = ul_split(&(state), &(length), (separator), (quoted)); (word); (word) = ul_split(&(state), &(length), (separator), (quoted)))

#define FOREACH_WORD_SEPARATOR(word, length, s, separator, state)       \
        _FOREACH_WORD(word, length, s, separator, false, state)


نویسه **ul_strv_split(ثابت نویسه *s, ثابت نویسه *separator)
{
        ثابت نویسه *word, *state;
        size_t l;
        unsigned n, i;
        نویسه **r;

        أكد(s);

        n = 0;
        FOREACH_WORD_SEPARATOR(word, l, s, separator, state)
                n++;

        r = malloc(اندازه(نویسه *) * (n + 1));
        اگر (!r)
                بازگشت پوچ;

        i = 0;
        FOREACH_WORD_SEPARATOR(word, l, s, separator, state) {
                r[i] = strndup(word, l);
                اگر (!r[i]) {
                        ul_strv_free(r);
                        بازگشت پوچ;
                }

                i++;
        }

        r[i] = پوچ;
        بازگشت r;
}

نویسه *ul_strv_join(نویسه **l, ثابت نویسه *separator)
{
        نویسه *r, *e;
        نویسه **s;
        size_t n, k;

        اگر (!separator)
                separator = " ";

        k = strlen(separator);

        n = 0;
        UL_STRV_FOREACH(s, l) {
                اگر (n != 0)
                        n += k;
                n += strlen(*s);
        }

        r = malloc(n + 1);
        اگر (!r)
                بازگشت پوچ;

        e = r;
        UL_STRV_FOREACH(s, l) {
                اگر (e != r)
                        e = stpcpy(e, separator);

                e = stpcpy(e, *s);
        }

        *e = 0;

        بازگشت r;
}

صحیح ul_strv_افزودن(نویسه ***l, نویسه *مقدار)
{
        نویسه **c;
        unsigned n, m;

        اگر (!مقدار)
                بازگشت 0;

        n = ul_strv_length(*l);

        /* Increase and check for overflow */
        m = n + 2;
        اگر (m < n)
                بازگشت -ENOMEM;

        c = reallocarray(*l, m, اندازه(نویسه *));
        اگر (!c)
                بازگشت -ENOMEM;

        c[n] = مقدار;
        c[n+1] = پوچ;

        *l = c;
        بازگشت 0;
}

صحیح ul_strv_افزودن_prepend(نویسه ***l, نویسه *مقدار)
{
        نویسه **c;
        unsigned n, m, i;

        اگر (!مقدار)
                بازگشت 0;

        n = ul_strv_length(*l);

        /* increase and check for overflow */
        m = n + 2;
        اگر (m < n)
                بازگشت -ENOMEM;

        c = malloc(اندازه(نویسه *) * m);
        اگر (!c)
                بازگشت -ENOMEM;

        برای (i = 0; i < n; i++)
                c[i+1] = (*l)[i];

        c[0] = مقدار;
        c[n+1] = پوچ;

        آزاد(*l);
        *l = c;

        بازگشت 0;
}

صحیح ul_strv_consume(نویسه ***l, نویسه *مقدار)
{
        صحیح r;

        r = ul_strv_افزودن(l, مقدار);
        اگر (r < 0)
                آزاد(مقدار);

        بازگشت r;
}

صحیح ul_strv_consume_prepend(نویسه ***l, نویسه *مقدار)
{
        صحیح r;

        r = ul_strv_افزودن_prepend(l, مقدار);
        اگر (r < 0)
                آزاد(مقدار);

        بازگشت r;
}

صحیح ul_strv_extend(نویسه ***l, ثابت نویسه *مقدار)
{
        نویسه *v;

        اگر (!مقدار)
                بازگشت 0;

        v = strdup(مقدار);
        اگر (!v)
                بازگشت -ENOMEM;

        بازگشت ul_strv_consume(l, v);
}

نویسه **ul_strv_remove(نویسه **l, ثابت نویسه *s)
{
        نویسه **f, **t;

        اگر (!l)
                بازگشت پوچ;

        أكد(s);

        /* Drops every occurrence of s in the string list, edits
         * in-place. */

        برای (f = t = l; *f; f++)
                اگر (strcmp(*f, s) == 0)
                        آزاد(*f);
                وگرنه
                        *(t++) = *f;

        *t = پوچ;
        بازگشت l;
}

صحیح ul_strv_extendf(نویسه ***l, ثابت نویسه *format, ...)
{
        va_list ap;
        نویسه *x;
        صحیح r;

        va_start(ap, format);
        r = vasprintf(&x, format, ap);
        va_end(ap);

        اگر (r < 0)
                بازگشت -ENOMEM;

        بازگشت ul_strv_consume(l, x);
}

صحیح ul_strv_extendv(نویسه ***l, ثابت نویسه *format, va_list ap)
{
        نویسه *x;
        صحیح r;

        r = vasprintf(&x, format, ap);
        اگر (r < 0)
                بازگشت -ENOMEM;

        بازگشت ul_strv_consume(l, x);
}

نویسه **ul_strv_reverse(نویسه **l)
{
        unsigned n, i;

        n = ul_strv_length(l);
        اگر (n <= 1)
                بازگشت l;

        برای (i = 0; i < n / 2; i++) {
                نویسه *t;

                t = l[i];
                l[i] = l[n-1-i];
                l[n-1-i] = t;
        }

        بازگشت l;
}
