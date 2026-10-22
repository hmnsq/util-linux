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

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <assert.h>

#include "strutils.h"
#include "strv.h"

static void ul_strv_clear(var_na **l)
{
        var_na **k;

        အကယ်၍ (!l)
                ပြန်ပေး;

        အတွက် (k = l; *k; k++)
                free(*k);

        *l = NULL;
}

var_na **ul_strv_free(var_na **l)
{
        ul_strv_clear(l);
        free(l);
        ပြန်ပေး NULL;
}

var_na **ul_strv_copy(var_na * const *l)
{
        var_na **r, **k;

        k = r = malloc(parimaa_na(var_na *) * (ul_strv_length(l) + 1));
        အကယ်၍ (!r)
                ပြန်ပေး NULL;

        အကယ်၍ (l)
                အတွက် (; *l; k++, l++) {
                        *k = strdup(*l);
                        အကယ်၍ (!*k) {
                                ul_strv_free(r);
                                ပြန်ပေး NULL;
                        }
                }

        *k = NULL;
        ပြန်ပေး r;
}

unsigned ul_strv_length(var_na * const *l) {
        unsigned n = 0;

        အကယ်၍ (!l)
                ပြန်ပေး 0;

        အတွက် (; *l; l++)
                n++;

        ပြန်ပေး n;
}

static var_na **ul_strv_new_ap(const var_na *x, va_list ap)
{
        const var_na *s;
        var_na **a;
        unsigned n = 0, i = 0;
        va_list aq;

        /* As a special trick we ignore all listed strings that equal
         * (const char*) -1. This is supposed to be used with the
         * UL_STRV_IFNOTNULL() macro to include possibly NULL strings in
         * the string list. */

        အကယ်၍ (x) {
                n = x == (const var_na*) -1 ? 0 : 1;

                va_copy(aq, ap);
                နေစဉ် ((s = va_arg(aq, const var_na*))) {
                        အကယ်၍ (s == (const var_na*) -1)
                                continue;

                        n++;
                }

                va_end(aq);
        }

        a = malloc(parimaa_na(var_na *) * (n + 1));
        အကယ်၍ (!a)
                ပြန်ပေး NULL;

        အကယ်၍ (x) {
                အကယ်၍ (x != (const var_na*) -1) {
                        a[i] = strdup(x);
                        အကယ်၍ (!a[i])
                                goto fail;
                        i++;
                }

                နေစဉ် ((s = va_arg(ap, const var_na*))) {

                        အကယ်၍ (s == (const var_na*) -1)
                                continue;

                        a[i] = strdup(s);
                        အကယ်၍ (!a[i])
                                goto fail;

                        i++;
                }
        }

        a[i] = NULL;

        ပြန်ပေး a;

fail:
        ul_strv_free(a);
        ပြန်ပေး NULL;
}

var_na **ul_strv_new(const var_na *x, ...)
{
        var_na **r;
        va_list ap;

        va_start(ap, x);
        r = ul_strv_new_ap(x, ap);
        va_end(ap);

        ပြန်ပေး r;
}

ကိန်းပြည့် ul_strv_extend_strv(var_na ***a, var_na **b)
{
        ကိန်းပြည့် r;
        var_na **s;

        UL_STRV_FOREACH(s, b) {
                r = ul_strv_extend(a, *s);
                အကယ်၍ (r < 0)
                        ပြန်ပေး r;
        }

        ပြန်ပေး 0;
}

ကိန်းပြည့် ul_strv_extend_strv_concat(var_na ***a, var_na **b, const var_na *suffix)
{
        ကိန်းပြည့် r;
        var_na **s;

        UL_STRV_FOREACH(s, b) {
                var_na *v;

                v = ul_strconcat(*s, suffix);
                အကယ်၍ (!v)
                        ပြန်ပေး -ENOMEM;

                r = ul_strv_push(a, v);
                အကယ်၍ (r < 0) {
                        free(v);
                        ပြန်ပေး r;
                }
        }

        ပြန်ပေး 0;
}


#define _FOREACH_WORD(word, length, s, separator, quoted, state)        \
        အတွက် ((state) = (s), (word) = ul_split(&(state), &(length), (separator), (quoted)); (word); (word) = ul_split(&(state), &(length), (separator), (quoted)))

#define FOREACH_WORD_SEPARATOR(word, length, s, separator, state)       \
        _FOREACH_WORD(word, length, s, separator, false, state)


var_na **ul_strv_split(const var_na *s, const var_na *separator)
{
        const var_na *word, *state;
        size_t l;
        unsigned n, i;
        var_na **r;

        assert(s);

        n = 0;
        FOREACH_WORD_SEPARATOR(word, l, s, separator, state)
                n++;

        r = malloc(parimaa_na(var_na *) * (n + 1));
        အကယ်၍ (!r)
                ပြန်ပေး NULL;

        i = 0;
        FOREACH_WORD_SEPARATOR(word, l, s, separator, state) {
                r[i] = strndup(word, l);
                အကယ်၍ (!r[i]) {
                        ul_strv_free(r);
                        ပြန်ပေး NULL;
                }

                i++;
        }

        r[i] = NULL;
        ပြန်ပေး r;
}

var_na *ul_strv_join(var_na **l, const var_na *separator)
{
        var_na *r, *e;
        var_na **s;
        size_t n, k;

        အကယ်၍ (!separator)
                separator = " ";

        k = strlen(separator);

        n = 0;
        UL_STRV_FOREACH(s, l) {
                အကယ်၍ (n != 0)
                        n += k;
                n += strlen(*s);
        }

        r = malloc(n + 1);
        အကယ်၍ (!r)
                ပြန်ပေး NULL;

        e = r;
        UL_STRV_FOREACH(s, l) {
                အကယ်၍ (e != r)
                        e = stpcpy(e, separator);

                e = stpcpy(e, *s);
        }

        *e = 0;

        ပြန်ပေး r;
}

ကိန်းပြည့် ul_strv_push(var_na ***l, var_na *value)
{
        var_na **c;
        unsigned n, m;

        အကယ်၍ (!value)
                ပြန်ပေး 0;

        n = ul_strv_length(*l);

        /* Increase and check for overflow */
        m = n + 2;
        အကယ်၍ (m < n)
                ပြန်ပေး -ENOMEM;

        c = reallocarray(*l, m, parimaa_na(var_na *));
        အကယ်၍ (!c)
                ပြန်ပေး -ENOMEM;

        c[n] = value;
        c[n+1] = NULL;

        *l = c;
        ပြန်ပေး 0;
}

ကိန်းပြည့် ul_strv_push_prepend(var_na ***l, var_na *value)
{
        var_na **c;
        unsigned n, m, i;

        အကယ်၍ (!value)
                ပြန်ပေး 0;

        n = ul_strv_length(*l);

        /* increase and check for overflow */
        m = n + 2;
        အကယ်၍ (m < n)
                ပြန်ပေး -ENOMEM;

        c = malloc(parimaa_na(var_na *) * m);
        အကယ်၍ (!c)
                ပြန်ပေး -ENOMEM;

        အတွက် (i = 0; i < n; i++)
                c[i+1] = (*l)[i];

        c[0] = value;
        c[n+1] = NULL;

        free(*l);
        *l = c;

        ပြန်ပေး 0;
}

ကိန်းပြည့် ul_strv_consume(var_na ***l, var_na *value)
{
        ကိန်းပြည့် r;

        r = ul_strv_push(l, value);
        အကယ်၍ (r < 0)
                free(value);

        ပြန်ပေး r;
}

ကိန်းပြည့် ul_strv_consume_prepend(var_na ***l, var_na *value)
{
        ကိန်းပြည့် r;

        r = ul_strv_push_prepend(l, value);
        အကယ်၍ (r < 0)
                free(value);

        ပြန်ပေး r;
}

ကိန်းပြည့် ul_strv_extend(var_na ***l, const var_na *value)
{
        var_na *v;

        အကယ်၍ (!value)
                ပြန်ပေး 0;

        v = strdup(value);
        အကယ်၍ (!v)
                ပြန်ပေး -ENOMEM;

        ပြန်ပေး ul_strv_consume(l, v);
}

var_na **ul_strv_remove(var_na **l, const var_na *s)
{
        var_na **f, **t;

        အကယ်၍ (!l)
                ပြန်ပေး NULL;

        assert(s);

        /* Drops every occurrence of s in the string list, edits
         * in-place. */

        အတွက် (f = t = l; *f; f++)
                အကယ်၍ (strcmp(*f, s) == 0)
                        free(*f);
                မဟုတ်ပါက
                        *(t++) = *f;

        *t = NULL;
        ပြန်ပေး l;
}

ကိန်းပြည့် ul_strv_extendf(var_na ***l, const var_na *format, ...)
{
        va_list ap;
        var_na *x;
        ကိန်းပြည့် r;

        va_start(ap, format);
        r = vasprintf(&x, format, ap);
        va_end(ap);

        အကယ်၍ (r < 0)
                ပြန်ပေး -ENOMEM;

        ပြန်ပေး ul_strv_consume(l, x);
}

ကိန်းပြည့် ul_strv_extendv(var_na ***l, const var_na *format, va_list ap)
{
        var_na *x;
        ကိန်းပြည့် r;

        r = vasprintf(&x, format, ap);
        အကယ်၍ (r < 0)
                ပြန်ပေး -ENOMEM;

        ပြန်ပေး ul_strv_consume(l, x);
}

var_na **ul_strv_reverse(var_na **l)
{
        unsigned n, i;

        n = ul_strv_length(l);
        အကယ်၍ (n <= 1)
                ပြန်ပေး l;

        အတွက် (i = 0; i < n / 2; i++) {
                var_na *t;

                t = l[i];
                l[i] = l[n-1-i];
                l[n-1-i] = t;
        }

        ပြန်ပေး l;
}
