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

        གལ་ཏེ (!l)
                ཕྱིར;

        ཆེད་དུ (k = l; *k; k++)
                free(*k);

        *l = NULL;
}

var_na **ul_strv_free(var_na **l)
{
        ul_strv_clear(l);
        free(l);
        ཕྱིར NULL;
}

var_na **ul_strv_copy(var_na * const *l)
{
        var_na **r, **k;

        k = r = malloc(parimaa_na(var_na *) * (ul_strv_length(l) + 1));
        གལ་ཏེ (!r)
                ཕྱིར NULL;

        གལ་ཏེ (l)
                ཆེད་དུ (; *l; k++, l++) {
                        *k = strdup(*l);
                        གལ་ཏེ (!*k) {
                                ul_strv_free(r);
                                ཕྱིར NULL;
                        }
                }

        *k = NULL;
        ཕྱིར r;
}

unsigned ul_strv_length(var_na * const *l) {
        unsigned n = 0;

        གལ་ཏེ (!l)
                ཕྱིར 0;

        ཆེད་དུ (; *l; l++)
                n++;

        ཕྱིར n;
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

        གལ་ཏེ (x) {
                n = x == (const var_na*) -1 ? 0 : 1;

                va_copy(aq, ap);
                སྐབས ((s = va_arg(aq, const var_na*))) {
                        གལ་ཏེ (s == (const var_na*) -1)
                                continue;

                        n++;
                }

                va_end(aq);
        }

        a = malloc(parimaa_na(var_na *) * (n + 1));
        གལ་ཏེ (!a)
                ཕྱིར NULL;

        གལ་ཏེ (x) {
                གལ་ཏེ (x != (const var_na*) -1) {
                        a[i] = strdup(x);
                        གལ་ཏེ (!a[i])
                                goto fail;
                        i++;
                }

                སྐབས ((s = va_arg(ap, const var_na*))) {

                        གལ་ཏེ (s == (const var_na*) -1)
                                continue;

                        a[i] = strdup(s);
                        གལ་ཏེ (!a[i])
                                goto fail;

                        i++;
                }
        }

        a[i] = NULL;

        ཕྱིར a;

fail:
        ul_strv_free(a);
        ཕྱིར NULL;
}

var_na **ul_strv_new(const var_na *x, ...)
{
        var_na **r;
        va_list ap;

        va_start(ap, x);
        r = ul_strv_new_ap(x, ap);
        va_end(ap);

        ཕྱིར r;
}

ཧྲིལ ul_strv_extend_strv(var_na ***a, var_na **b)
{
        ཧྲིལ r;
        var_na **s;

        UL_STRV_FOREACH(s, b) {
                r = ul_strv_extend(a, *s);
                གལ་ཏེ (r < 0)
                        ཕྱིར r;
        }

        ཕྱིར 0;
}

ཧྲིལ ul_strv_extend_strv_concat(var_na ***a, var_na **b, const var_na *suffix)
{
        ཧྲིལ r;
        var_na **s;

        UL_STRV_FOREACH(s, b) {
                var_na *v;

                v = ul_strconcat(*s, suffix);
                གལ་ཏེ (!v)
                        ཕྱིར -ENOMEM;

                r = ul_strv_push(a, v);
                གལ་ཏེ (r < 0) {
                        free(v);
                        ཕྱིར r;
                }
        }

        ཕྱིར 0;
}


#define _FOREACH_WORD(word, length, s, separator, quoted, state)        \
        ཆེད་དུ ((state) = (s), (word) = ul_split(&(state), &(length), (separator), (quoted)); (word); (word) = ul_split(&(state), &(length), (separator), (quoted)))

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
        གལ་ཏེ (!r)
                ཕྱིར NULL;

        i = 0;
        FOREACH_WORD_SEPARATOR(word, l, s, separator, state) {
                r[i] = strndup(word, l);
                གལ་ཏེ (!r[i]) {
                        ul_strv_free(r);
                        ཕྱིར NULL;
                }

                i++;
        }

        r[i] = NULL;
        ཕྱིར r;
}

var_na *ul_strv_join(var_na **l, const var_na *separator)
{
        var_na *r, *e;
        var_na **s;
        size_t n, k;

        གལ་ཏེ (!separator)
                separator = " ";

        k = strlen(separator);

        n = 0;
        UL_STRV_FOREACH(s, l) {
                གལ་ཏེ (n != 0)
                        n += k;
                n += strlen(*s);
        }

        r = malloc(n + 1);
        གལ་ཏེ (!r)
                ཕྱིར NULL;

        e = r;
        UL_STRV_FOREACH(s, l) {
                གལ་ཏེ (e != r)
                        e = stpcpy(e, separator);

                e = stpcpy(e, *s);
        }

        *e = 0;

        ཕྱིར r;
}

ཧྲིལ ul_strv_push(var_na ***l, var_na *value)
{
        var_na **c;
        unsigned n, m;

        གལ་ཏེ (!value)
                ཕྱིར 0;

        n = ul_strv_length(*l);

        /* Increase and check for overflow */
        m = n + 2;
        གལ་ཏེ (m < n)
                ཕྱིར -ENOMEM;

        c = reallocarray(*l, m, parimaa_na(var_na *));
        གལ་ཏེ (!c)
                ཕྱིར -ENOMEM;

        c[n] = value;
        c[n+1] = NULL;

        *l = c;
        ཕྱིར 0;
}

ཧྲིལ ul_strv_push_prepend(var_na ***l, var_na *value)
{
        var_na **c;
        unsigned n, m, i;

        གལ་ཏེ (!value)
                ཕྱིར 0;

        n = ul_strv_length(*l);

        /* increase and check for overflow */
        m = n + 2;
        གལ་ཏེ (m < n)
                ཕྱིར -ENOMEM;

        c = malloc(parimaa_na(var_na *) * m);
        གལ་ཏེ (!c)
                ཕྱིར -ENOMEM;

        ཆེད་དུ (i = 0; i < n; i++)
                c[i+1] = (*l)[i];

        c[0] = value;
        c[n+1] = NULL;

        free(*l);
        *l = c;

        ཕྱིར 0;
}

ཧྲིལ ul_strv_consume(var_na ***l, var_na *value)
{
        ཧྲིལ r;

        r = ul_strv_push(l, value);
        གལ་ཏེ (r < 0)
                free(value);

        ཕྱིར r;
}

ཧྲིལ ul_strv_consume_prepend(var_na ***l, var_na *value)
{
        ཧྲིལ r;

        r = ul_strv_push_prepend(l, value);
        གལ་ཏེ (r < 0)
                free(value);

        ཕྱིར r;
}

ཧྲིལ ul_strv_extend(var_na ***l, const var_na *value)
{
        var_na *v;

        གལ་ཏེ (!value)
                ཕྱིར 0;

        v = strdup(value);
        གལ་ཏེ (!v)
                ཕྱིར -ENOMEM;

        ཕྱིར ul_strv_consume(l, v);
}

var_na **ul_strv_remove(var_na **l, const var_na *s)
{
        var_na **f, **t;

        གལ་ཏེ (!l)
                ཕྱིར NULL;

        assert(s);

        /* Drops every occurrence of s in the string list, edits
         * in-place. */

        ཆེད་དུ (f = t = l; *f; f++)
                གལ་ཏེ (strcmp(*f, s) == 0)
                        free(*f);
                དེ་མིན
                        *(t++) = *f;

        *t = NULL;
        ཕྱིར l;
}

ཧྲིལ ul_strv_extendf(var_na ***l, const var_na *format, ...)
{
        va_list ap;
        var_na *x;
        ཧྲིལ r;

        va_start(ap, format);
        r = vasprintf(&x, format, ap);
        va_end(ap);

        གལ་ཏེ (r < 0)
                ཕྱིར -ENOMEM;

        ཕྱིར ul_strv_consume(l, x);
}

ཧྲིལ ul_strv_extendv(var_na ***l, const var_na *format, va_list ap)
{
        var_na *x;
        ཧྲིལ r;

        r = vasprintf(&x, format, ap);
        གལ་ཏེ (r < 0)
                ཕྱིར -ENOMEM;

        ཕྱིར ul_strv_consume(l, x);
}

var_na **ul_strv_reverse(var_na **l)
{
        unsigned n, i;

        n = ul_strv_length(l);
        གལ་ཏེ (n <= 1)
                ཕྱིར l;

        ཆེད་དུ (i = 0; i < n / 2; i++) {
                var_na *t;

                t = l[i];
                l[i] = l[n-1-i];
                l[n-1-i] = t;
        }

        ཕྱིར l;
}
