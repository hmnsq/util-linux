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

        ຖ້າ (!l)
                ສົ່ງຄືນ;

        ສຳລັບ (k = l; *k; k++)
                free(*k);

        *l = NULL;
}

var_na **ul_strv_free(var_na **l)
{
        ul_strv_clear(l);
        free(l);
        ສົ່ງຄືນ NULL;
}

var_na **ul_strv_copy(var_na * const *l)
{
        var_na **r, **k;

        k = r = malloc(parimaa_na(var_na *) * (ul_strv_length(l) + 1));
        ຖ້າ (!r)
                ສົ່ງຄືນ NULL;

        ຖ້າ (l)
                ສຳລັບ (; *l; k++, l++) {
                        *k = strdup(*l);
                        ຖ້າ (!*k) {
                                ul_strv_free(r);
                                ສົ່ງຄືນ NULL;
                        }
                }

        *k = NULL;
        ສົ່ງຄືນ r;
}

unsigned ul_strv_length(var_na * const *l) {
        unsigned n = 0;

        ຖ້າ (!l)
                ສົ່ງຄືນ 0;

        ສຳລັບ (; *l; l++)
                n++;

        ສົ່ງຄືນ n;
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

        ຖ້າ (x) {
                n = x == (const var_na*) -1 ? 0 : 1;

                va_copy(aq, ap);
                ໃນຂະນະທີ່ ((s = va_arg(aq, const var_na*))) {
                        ຖ້າ (s == (const var_na*) -1)
                                continue;

                        n++;
                }

                va_end(aq);
        }

        a = malloc(parimaa_na(var_na *) * (n + 1));
        ຖ້າ (!a)
                ສົ່ງຄືນ NULL;

        ຖ້າ (x) {
                ຖ້າ (x != (const var_na*) -1) {
                        a[i] = strdup(x);
                        ຖ້າ (!a[i])
                                goto fail;
                        i++;
                }

                ໃນຂະນະທີ່ ((s = va_arg(ap, const var_na*))) {

                        ຖ້າ (s == (const var_na*) -1)
                                continue;

                        a[i] = strdup(s);
                        ຖ້າ (!a[i])
                                goto fail;

                        i++;
                }
        }

        a[i] = NULL;

        ສົ່ງຄືນ a;

fail:
        ul_strv_free(a);
        ສົ່ງຄືນ NULL;
}

var_na **ul_strv_new(const var_na *x, ...)
{
        var_na **r;
        va_list ap;

        va_start(ap, x);
        r = ul_strv_new_ap(x, ap);
        va_end(ap);

        ສົ່ງຄືນ r;
}

ຈຳນວນເຕັມ ul_strv_extend_strv(var_na ***a, var_na **b)
{
        ຈຳນວນເຕັມ r;
        var_na **s;

        UL_STRV_FOREACH(s, b) {
                r = ul_strv_extend(a, *s);
                ຖ້າ (r < 0)
                        ສົ່ງຄືນ r;
        }

        ສົ່ງຄືນ 0;
}

ຈຳນວນເຕັມ ul_strv_extend_strv_concat(var_na ***a, var_na **b, const var_na *suffix)
{
        ຈຳນວນເຕັມ r;
        var_na **s;

        UL_STRV_FOREACH(s, b) {
                var_na *v;

                v = ul_strconcat(*s, suffix);
                ຖ້າ (!v)
                        ສົ່ງຄືນ -ENOMEM;

                r = ul_strv_push(a, v);
                ຖ້າ (r < 0) {
                        free(v);
                        ສົ່ງຄືນ r;
                }
        }

        ສົ່ງຄືນ 0;
}


#define _FOREACH_WORD(word, length, s, separator, quoted, state)        \
        ສຳລັບ ((state) = (s), (word) = ul_split(&(state), &(length), (separator), (quoted)); (word); (word) = ul_split(&(state), &(length), (separator), (quoted)))

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
        ຖ້າ (!r)
                ສົ່ງຄືນ NULL;

        i = 0;
        FOREACH_WORD_SEPARATOR(word, l, s, separator, state) {
                r[i] = strndup(word, l);
                ຖ້າ (!r[i]) {
                        ul_strv_free(r);
                        ສົ່ງຄືນ NULL;
                }

                i++;
        }

        r[i] = NULL;
        ສົ່ງຄືນ r;
}

var_na *ul_strv_join(var_na **l, const var_na *separator)
{
        var_na *r, *e;
        var_na **s;
        size_t n, k;

        ຖ້າ (!separator)
                separator = " ";

        k = strlen(separator);

        n = 0;
        UL_STRV_FOREACH(s, l) {
                ຖ້າ (n != 0)
                        n += k;
                n += strlen(*s);
        }

        r = malloc(n + 1);
        ຖ້າ (!r)
                ສົ່ງຄືນ NULL;

        e = r;
        UL_STRV_FOREACH(s, l) {
                ຖ້າ (e != r)
                        e = stpcpy(e, separator);

                e = stpcpy(e, *s);
        }

        *e = 0;

        ສົ່ງຄືນ r;
}

ຈຳນວນເຕັມ ul_strv_push(var_na ***l, var_na *value)
{
        var_na **c;
        unsigned n, m;

        ຖ້າ (!value)
                ສົ່ງຄືນ 0;

        n = ul_strv_length(*l);

        /* Increase and check for overflow */
        m = n + 2;
        ຖ້າ (m < n)
                ສົ່ງຄືນ -ENOMEM;

        c = reallocarray(*l, m, parimaa_na(var_na *));
        ຖ້າ (!c)
                ສົ່ງຄືນ -ENOMEM;

        c[n] = value;
        c[n+1] = NULL;

        *l = c;
        ສົ່ງຄືນ 0;
}

ຈຳນວນເຕັມ ul_strv_push_prepend(var_na ***l, var_na *value)
{
        var_na **c;
        unsigned n, m, i;

        ຖ້າ (!value)
                ສົ່ງຄືນ 0;

        n = ul_strv_length(*l);

        /* increase and check for overflow */
        m = n + 2;
        ຖ້າ (m < n)
                ສົ່ງຄືນ -ENOMEM;

        c = malloc(parimaa_na(var_na *) * m);
        ຖ້າ (!c)
                ສົ່ງຄືນ -ENOMEM;

        ສຳລັບ (i = 0; i < n; i++)
                c[i+1] = (*l)[i];

        c[0] = value;
        c[n+1] = NULL;

        free(*l);
        *l = c;

        ສົ່ງຄືນ 0;
}

ຈຳນວນເຕັມ ul_strv_consume(var_na ***l, var_na *value)
{
        ຈຳນວນເຕັມ r;

        r = ul_strv_push(l, value);
        ຖ້າ (r < 0)
                free(value);

        ສົ່ງຄືນ r;
}

ຈຳນວນເຕັມ ul_strv_consume_prepend(var_na ***l, var_na *value)
{
        ຈຳນວນເຕັມ r;

        r = ul_strv_push_prepend(l, value);
        ຖ້າ (r < 0)
                free(value);

        ສົ່ງຄືນ r;
}

ຈຳນວນເຕັມ ul_strv_extend(var_na ***l, const var_na *value)
{
        var_na *v;

        ຖ້າ (!value)
                ສົ່ງຄືນ 0;

        v = strdup(value);
        ຖ້າ (!v)
                ສົ່ງຄືນ -ENOMEM;

        ສົ່ງຄືນ ul_strv_consume(l, v);
}

var_na **ul_strv_remove(var_na **l, const var_na *s)
{
        var_na **f, **t;

        ຖ້າ (!l)
                ສົ່ງຄືນ NULL;

        assert(s);

        /* Drops every occurrence of s in the string list, edits
         * in-place. */

        ສຳລັບ (f = t = l; *f; f++)
                ຖ້າ (strcmp(*f, s) == 0)
                        free(*f);
                ບໍ່ດັ່ງນັ້ນ
                        *(t++) = *f;

        *t = NULL;
        ສົ່ງຄືນ l;
}

ຈຳນວນເຕັມ ul_strv_extendf(var_na ***l, const var_na *format, ...)
{
        va_list ap;
        var_na *x;
        ຈຳນວນເຕັມ r;

        va_start(ap, format);
        r = vasprintf(&x, format, ap);
        va_end(ap);

        ຖ້າ (r < 0)
                ສົ່ງຄືນ -ENOMEM;

        ສົ່ງຄືນ ul_strv_consume(l, x);
}

ຈຳນວນເຕັມ ul_strv_extendv(var_na ***l, const var_na *format, va_list ap)
{
        var_na *x;
        ຈຳນວນເຕັມ r;

        r = vasprintf(&x, format, ap);
        ຖ້າ (r < 0)
                ສົ່ງຄືນ -ENOMEM;

        ສົ່ງຄືນ ul_strv_consume(l, x);
}

var_na **ul_strv_reverse(var_na **l)
{
        unsigned n, i;

        n = ul_strv_length(l);
        ຖ້າ (n <= 1)
                ສົ່ງຄືນ l;

        ສຳລັບ (i = 0; i < n / 2; i++) {
                var_na *t;

                t = l[i];
                l[i] = l[n-1-i];
                l[n-1-i] = t;
        }

        ສົ່ງຄືນ l;
}
