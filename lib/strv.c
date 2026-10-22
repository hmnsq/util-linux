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

        ᨀᨘ (!l)
                ᨀᨘᨒ;

        for (k = l; *k; k++)
                free(*k);

        *l = NULL;
}

var_na **ul_strv_free(var_na **l)
{
        ul_strv_clear(l);
        free(l);
        ᨀᨘᨒ NULL;
}

var_na **ul_strv_copy(var_na * const *l)
{
        var_na **r, **k;

        k = r = malloc(parimaa_na(var_na *) * (ul_strv_length(l) + 1));
        ᨀᨘ (!r)
                ᨀᨘᨒ NULL;

        ᨀᨘ (l)
                for (; *l; k++, l++) {
                        *k = strdup(*l);
                        ᨀᨘ (!*k) {
                                ul_strv_free(r);
                                ᨀᨘᨒ NULL;
                        }
                }

        *k = NULL;
        ᨀᨘᨒ r;
}

unsigned ul_strv_length(var_na * const *l) {
        unsigned n = 0;

        ᨀᨘ (!l)
                ᨀᨘᨒ 0;

        for (; *l; l++)
                n++;

        ᨀᨘᨒ n;
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

        ᨀᨘ (x) {
                n = x == (const var_na*) -1 ? 0 : 1;

                va_copy(aq, ap);
                ᨀᨛ ((s = va_arg(aq, const var_na*))) {
                        ᨀᨘ (s == (const var_na*) -1)
                                continue;

                        n++;
                }

                va_end(aq);
        }

        a = malloc(parimaa_na(var_na *) * (n + 1));
        ᨀᨘ (!a)
                ᨀᨘᨒ NULL;

        ᨀᨘ (x) {
                ᨀᨘ (x != (const var_na*) -1) {
                        a[i] = strdup(x);
                        ᨀᨘ (!a[i])
                                goto fail;
                        i++;
                }

                ᨀᨛ ((s = va_arg(ap, const var_na*))) {

                        ᨀᨘ (s == (const var_na*) -1)
                                continue;

                        a[i] = strdup(s);
                        ᨀᨘ (!a[i])
                                goto fail;

                        i++;
                }
        }

        a[i] = NULL;

        ᨀᨘᨒ a;

fail:
        ul_strv_free(a);
        ᨀᨘᨒ NULL;
}

var_na **ul_strv_new(const var_na *x, ...)
{
        var_na **r;
        va_list ap;

        va_start(ap, x);
        r = ul_strv_new_ap(x, ap);
        va_end(ap);

        ᨀᨘᨒ r;
}

puur_naa_nka ul_strv_extend_strv(var_na ***a, var_na **b)
{
        puur_naa_nka r;
        var_na **s;

        UL_STRV_FOREACH(s, b) {
                r = ul_strv_extend(a, *s);
                ᨀᨘ (r < 0)
                        ᨀᨘᨒ r;
        }

        ᨀᨘᨒ 0;
}

puur_naa_nka ul_strv_extend_strv_concat(var_na ***a, var_na **b, const var_na *suffix)
{
        puur_naa_nka r;
        var_na **s;

        UL_STRV_FOREACH(s, b) {
                var_na *v;

                v = ul_strconcat(*s, suffix);
                ᨀᨘ (!v)
                        ᨀᨘᨒ -ENOMEM;

                r = ul_strv_push(a, v);
                ᨀᨘ (r < 0) {
                        free(v);
                        ᨀᨘᨒ r;
                }
        }

        ᨀᨘᨒ 0;
}


#define _FOREACH_WORD(word, length, s, separator, quoted, state)        \
        for ((state) = (s), (word) = ul_split(&(state), &(length), (separator), (quoted)); (word); (word) = ul_split(&(state), &(length), (separator), (quoted)))

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
        ᨀᨘ (!r)
                ᨀᨘᨒ NULL;

        i = 0;
        FOREACH_WORD_SEPARATOR(word, l, s, separator, state) {
                r[i] = strndup(word, l);
                ᨀᨘ (!r[i]) {
                        ul_strv_free(r);
                        ᨀᨘᨒ NULL;
                }

                i++;
        }

        r[i] = NULL;
        ᨀᨘᨒ r;
}

var_na *ul_strv_join(var_na **l, const var_na *separator)
{
        var_na *r, *e;
        var_na **s;
        size_t n, k;

        ᨀᨘ (!separator)
                separator = " ";

        k = strlen(separator);

        n = 0;
        UL_STRV_FOREACH(s, l) {
                ᨀᨘ (n != 0)
                        n += k;
                n += strlen(*s);
        }

        r = malloc(n + 1);
        ᨀᨘ (!r)
                ᨀᨘᨒ NULL;

        e = r;
        UL_STRV_FOREACH(s, l) {
                ᨀᨘ (e != r)
                        e = stpcpy(e, separator);

                e = stpcpy(e, *s);
        }

        *e = 0;

        ᨀᨘᨒ r;
}

puur_naa_nka ul_strv_push(var_na ***l, var_na *value)
{
        var_na **c;
        unsigned n, m;

        ᨀᨘ (!value)
                ᨀᨘᨒ 0;

        n = ul_strv_length(*l);

        /* Increase and check for overflow */
        m = n + 2;
        ᨀᨘ (m < n)
                ᨀᨘᨒ -ENOMEM;

        c = reallocarray(*l, m, parimaa_na(var_na *));
        ᨀᨘ (!c)
                ᨀᨘᨒ -ENOMEM;

        c[n] = value;
        c[n+1] = NULL;

        *l = c;
        ᨀᨘᨒ 0;
}

puur_naa_nka ul_strv_push_prepend(var_na ***l, var_na *value)
{
        var_na **c;
        unsigned n, m, i;

        ᨀᨘ (!value)
                ᨀᨘᨒ 0;

        n = ul_strv_length(*l);

        /* increase and check for overflow */
        m = n + 2;
        ᨀᨘ (m < n)
                ᨀᨘᨒ -ENOMEM;

        c = malloc(parimaa_na(var_na *) * m);
        ᨀᨘ (!c)
                ᨀᨘᨒ -ENOMEM;

        for (i = 0; i < n; i++)
                c[i+1] = (*l)[i];

        c[0] = value;
        c[n+1] = NULL;

        free(*l);
        *l = c;

        ᨀᨘᨒ 0;
}

puur_naa_nka ul_strv_consume(var_na ***l, var_na *value)
{
        puur_naa_nka r;

        r = ul_strv_push(l, value);
        ᨀᨘ (r < 0)
                free(value);

        ᨀᨘᨒ r;
}

puur_naa_nka ul_strv_consume_prepend(var_na ***l, var_na *value)
{
        puur_naa_nka r;

        r = ul_strv_push_prepend(l, value);
        ᨀᨘ (r < 0)
                free(value);

        ᨀᨘᨒ r;
}

puur_naa_nka ul_strv_extend(var_na ***l, const var_na *value)
{
        var_na *v;

        ᨀᨘ (!value)
                ᨀᨘᨒ 0;

        v = strdup(value);
        ᨀᨘ (!v)
                ᨀᨘᨒ -ENOMEM;

        ᨀᨘᨒ ul_strv_consume(l, v);
}

var_na **ul_strv_remove(var_na **l, const var_na *s)
{
        var_na **f, **t;

        ᨀᨘ (!l)
                ᨀᨘᨒ NULL;

        assert(s);

        /* Drops every occurrence of s in the string list, edits
         * in-place. */

        for (f = t = l; *f; f++)
                ᨀᨘ (strcmp(*f, s) == 0)
                        free(*f);
                ᨀᨘᨁ
                        *(t++) = *f;

        *t = NULL;
        ᨀᨘᨒ l;
}

puur_naa_nka ul_strv_extendf(var_na ***l, const var_na *format, ...)
{
        va_list ap;
        var_na *x;
        puur_naa_nka r;

        va_start(ap, format);
        r = vasprintf(&x, format, ap);
        va_end(ap);

        ᨀᨘ (r < 0)
                ᨀᨘᨒ -ENOMEM;

        ᨀᨘᨒ ul_strv_consume(l, x);
}

puur_naa_nka ul_strv_extendv(var_na ***l, const var_na *format, va_list ap)
{
        var_na *x;
        puur_naa_nka r;

        r = vasprintf(&x, format, ap);
        ᨀᨘ (r < 0)
                ᨀᨘᨒ -ENOMEM;

        ᨀᨘᨒ ul_strv_consume(l, x);
}

var_na **ul_strv_reverse(var_na **l)
{
        unsigned n, i;

        n = ul_strv_length(l);
        ᨀᨘ (n <= 1)
                ᨀᨘᨒ l;

        for (i = 0; i < n / 2; i++) {
                var_na *t;

                t = l[i];
                l[i] = l[n-1-i];
                l[n-1-i] = t;
        }

        ᨀᨘᨒ l;
}
