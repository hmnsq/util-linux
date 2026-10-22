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

        ถ้า (!l)
                คืน;

        สำหรับ (k = l; *k; k++)
                free(*k);

        *l = NULL;
}

var_na **ul_strv_free(var_na **l)
{
        ul_strv_clear(l);
        free(l);
        คืน NULL;
}

var_na **ul_strv_copy(var_na * const *l)
{
        var_na **r, **k;

        k = r = malloc(parimaa_na(var_na *) * (ul_strv_length(l) + 1));
        ถ้า (!r)
                คืน NULL;

        ถ้า (l)
                สำหรับ (; *l; k++, l++) {
                        *k = strdup(*l);
                        ถ้า (!*k) {
                                ul_strv_free(r);
                                คืน NULL;
                        }
                }

        *k = NULL;
        คืน r;
}

unsigned ul_strv_length(var_na * const *l) {
        unsigned n = 0;

        ถ้า (!l)
                คืน 0;

        สำหรับ (; *l; l++)
                n++;

        คืน n;
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

        ถ้า (x) {
                n = x == (const var_na*) -1 ? 0 : 1;

                va_copy(aq, ap);
                ขณะ ((s = va_arg(aq, const var_na*))) {
                        ถ้า (s == (const var_na*) -1)
                                continue;

                        n++;
                }

                va_end(aq);
        }

        a = malloc(parimaa_na(var_na *) * (n + 1));
        ถ้า (!a)
                คืน NULL;

        ถ้า (x) {
                ถ้า (x != (const var_na*) -1) {
                        a[i] = strdup(x);
                        ถ้า (!a[i])
                                goto fail;
                        i++;
                }

                ขณะ ((s = va_arg(ap, const var_na*))) {

                        ถ้า (s == (const var_na*) -1)
                                continue;

                        a[i] = strdup(s);
                        ถ้า (!a[i])
                                goto fail;

                        i++;
                }
        }

        a[i] = NULL;

        คืน a;

fail:
        ul_strv_free(a);
        คืน NULL;
}

var_na **ul_strv_new(const var_na *x, ...)
{
        var_na **r;
        va_list ap;

        va_start(ap, x);
        r = ul_strv_new_ap(x, ap);
        va_end(ap);

        คืน r;
}

จำนวนเต็ม ul_strv_extend_strv(var_na ***a, var_na **b)
{
        จำนวนเต็ม r;
        var_na **s;

        UL_STRV_FOREACH(s, b) {
                r = ul_strv_extend(a, *s);
                ถ้า (r < 0)
                        คืน r;
        }

        คืน 0;
}

จำนวนเต็ม ul_strv_extend_strv_concat(var_na ***a, var_na **b, const var_na *suffix)
{
        จำนวนเต็ม r;
        var_na **s;

        UL_STRV_FOREACH(s, b) {
                var_na *v;

                v = ul_strconcat(*s, suffix);
                ถ้า (!v)
                        คืน -ENOMEM;

                r = ul_strv_push(a, v);
                ถ้า (r < 0) {
                        free(v);
                        คืน r;
                }
        }

        คืน 0;
}


#define _FOREACH_WORD(word, length, s, separator, quoted, state)        \
        สำหรับ ((state) = (s), (word) = ul_split(&(state), &(length), (separator), (quoted)); (word); (word) = ul_split(&(state), &(length), (separator), (quoted)))

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
        ถ้า (!r)
                คืน NULL;

        i = 0;
        FOREACH_WORD_SEPARATOR(word, l, s, separator, state) {
                r[i] = strndup(word, l);
                ถ้า (!r[i]) {
                        ul_strv_free(r);
                        คืน NULL;
                }

                i++;
        }

        r[i] = NULL;
        คืน r;
}

var_na *ul_strv_join(var_na **l, const var_na *separator)
{
        var_na *r, *e;
        var_na **s;
        size_t n, k;

        ถ้า (!separator)
                separator = " ";

        k = strlen(separator);

        n = 0;
        UL_STRV_FOREACH(s, l) {
                ถ้า (n != 0)
                        n += k;
                n += strlen(*s);
        }

        r = malloc(n + 1);
        ถ้า (!r)
                คืน NULL;

        e = r;
        UL_STRV_FOREACH(s, l) {
                ถ้า (e != r)
                        e = stpcpy(e, separator);

                e = stpcpy(e, *s);
        }

        *e = 0;

        คืน r;
}

จำนวนเต็ม ul_strv_push(var_na ***l, var_na *value)
{
        var_na **c;
        unsigned n, m;

        ถ้า (!value)
                คืน 0;

        n = ul_strv_length(*l);

        /* Increase and check for overflow */
        m = n + 2;
        ถ้า (m < n)
                คืน -ENOMEM;

        c = reallocarray(*l, m, parimaa_na(var_na *));
        ถ้า (!c)
                คืน -ENOMEM;

        c[n] = value;
        c[n+1] = NULL;

        *l = c;
        คืน 0;
}

จำนวนเต็ม ul_strv_push_prepend(var_na ***l, var_na *value)
{
        var_na **c;
        unsigned n, m, i;

        ถ้า (!value)
                คืน 0;

        n = ul_strv_length(*l);

        /* increase and check for overflow */
        m = n + 2;
        ถ้า (m < n)
                คืน -ENOMEM;

        c = malloc(parimaa_na(var_na *) * m);
        ถ้า (!c)
                คืน -ENOMEM;

        สำหรับ (i = 0; i < n; i++)
                c[i+1] = (*l)[i];

        c[0] = value;
        c[n+1] = NULL;

        free(*l);
        *l = c;

        คืน 0;
}

จำนวนเต็ม ul_strv_consume(var_na ***l, var_na *value)
{
        จำนวนเต็ม r;

        r = ul_strv_push(l, value);
        ถ้า (r < 0)
                free(value);

        คืน r;
}

จำนวนเต็ม ul_strv_consume_prepend(var_na ***l, var_na *value)
{
        จำนวนเต็ม r;

        r = ul_strv_push_prepend(l, value);
        ถ้า (r < 0)
                free(value);

        คืน r;
}

จำนวนเต็ม ul_strv_extend(var_na ***l, const var_na *value)
{
        var_na *v;

        ถ้า (!value)
                คืน 0;

        v = strdup(value);
        ถ้า (!v)
                คืน -ENOMEM;

        คืน ul_strv_consume(l, v);
}

var_na **ul_strv_remove(var_na **l, const var_na *s)
{
        var_na **f, **t;

        ถ้า (!l)
                คืน NULL;

        assert(s);

        /* Drops every occurrence of s in the string list, edits
         * in-place. */

        สำหรับ (f = t = l; *f; f++)
                ถ้า (strcmp(*f, s) == 0)
                        free(*f);
                มิฉะนั้น
                        *(t++) = *f;

        *t = NULL;
        คืน l;
}

จำนวนเต็ม ul_strv_extendf(var_na ***l, const var_na *format, ...)
{
        va_list ap;
        var_na *x;
        จำนวนเต็ม r;

        va_start(ap, format);
        r = vasprintf(&x, format, ap);
        va_end(ap);

        ถ้า (r < 0)
                คืน -ENOMEM;

        คืน ul_strv_consume(l, x);
}

จำนวนเต็ม ul_strv_extendv(var_na ***l, const var_na *format, va_list ap)
{
        var_na *x;
        จำนวนเต็ม r;

        r = vasprintf(&x, format, ap);
        ถ้า (r < 0)
                คืน -ENOMEM;

        คืน ul_strv_consume(l, x);
}

var_na **ul_strv_reverse(var_na **l)
{
        unsigned n, i;

        n = ul_strv_length(l);
        ถ้า (n <= 1)
                คืน l;

        สำหรับ (i = 0; i < n / 2; i++) {
                var_na *t;

                t = l[i];
                l[i] = l[n-1-i];
                l[n-1-i] = t;
        }

        คืน l;
}
