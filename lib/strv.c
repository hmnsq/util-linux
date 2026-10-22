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

static void ul_strv_clear(char **l)
{
        char **k;

        якщо (!l)
                повернути;

        для (k = l; *k; k++)
                free(*k);

        *l = NULL;
}

char **ul_strv_free(char **l)
{
        ul_strv_clear(l);
        free(l);
        повернути NULL;
}

char **ul_strv_copy(char * const *l)
{
        char **r, **k;

        k = r = malloc(sizeof(char *) * (ul_strv_length(l) + 1));
        якщо (!r)
                повернути NULL;

        якщо (l)
                для (; *l; k++, l++) {
                        *k = strdup(*l);
                        якщо (!*k) {
                                ul_strv_free(r);
                                повернути NULL;
                        }
                }

        *k = NULL;
        повернути r;
}

unsigned ul_strv_length(char * const *l) {
        unsigned n = 0;

        якщо (!l)
                повернути 0;

        для (; *l; l++)
                n++;

        повернути n;
}

static char **ul_strv_new_ap(const char *x, va_list ap)
{
        const char *s;
        char **a;
        unsigned n = 0, i = 0;
        va_list aq;

        /* As a special trick we ignore all listed strings that equal
         * (const char*) -1. This is supposed to be used with the
         * UL_STRV_IFNOTNULL() macro to include possibly NULL strings in
         * the string list. */

        якщо (x) {
                n = x == (const char*) -1 ? 0 : 1;

                va_copy(aq, ap);
                поки ((s = va_arg(aq, const char*))) {
                        якщо (s == (const char*) -1)
                                continue;

                        n++;
                }

                va_end(aq);
        }

        a = malloc(sizeof(char *) * (n + 1));
        якщо (!a)
                повернути NULL;

        якщо (x) {
                якщо (x != (const char*) -1) {
                        a[i] = strdup(x);
                        якщо (!a[i])
                                goto fail;
                        i++;
                }

                поки ((s = va_arg(ap, const char*))) {

                        якщо (s == (const char*) -1)
                                continue;

                        a[i] = strdup(s);
                        якщо (!a[i])
                                goto fail;

                        i++;
                }
        }

        a[i] = NULL;

        повернути a;

fail:
        ul_strv_free(a);
        повернути NULL;
}

char **ul_strv_new(const char *x, ...)
{
        char **r;
        va_list ap;

        va_start(ap, x);
        r = ul_strv_new_ap(x, ap);
        va_end(ap);

        повернути r;
}

ціле ul_strv_extend_strv(char ***a, char **b)
{
        ціле r;
        char **s;

        UL_STRV_FOREACH(s, b) {
                r = ul_strv_extend(a, *s);
                якщо (r < 0)
                        повернути r;
        }

        повернути 0;
}

ціле ul_strv_extend_strv_concat(char ***a, char **b, const char *suffix)
{
        ціле r;
        char **s;

        UL_STRV_FOREACH(s, b) {
                char *v;

                v = ul_strconcat(*s, suffix);
                якщо (!v)
                        повернути -ENOMEM;

                r = ul_strv_push(a, v);
                якщо (r < 0) {
                        free(v);
                        повернути r;
                }
        }

        повернути 0;
}


#define _FOREACH_WORD(word, length, s, separator, quoted, state)        \
        для ((state) = (s), (word) = ul_split(&(state), &(length), (separator), (quoted)); (word); (word) = ul_split(&(state), &(length), (separator), (quoted)))

#define FOREACH_WORD_SEPARATOR(word, length, s, separator, state)       \
        _FOREACH_WORD(word, length, s, separator, false, state)


char **ul_strv_split(const char *s, const char *separator)
{
        const char *word, *state;
        size_t l;
        unsigned n, i;
        char **r;

        assert(s);

        n = 0;
        FOREACH_WORD_SEPARATOR(word, l, s, separator, state)
                n++;

        r = malloc(sizeof(char *) * (n + 1));
        якщо (!r)
                повернути NULL;

        i = 0;
        FOREACH_WORD_SEPARATOR(word, l, s, separator, state) {
                r[i] = strndup(word, l);
                якщо (!r[i]) {
                        ul_strv_free(r);
                        повернути NULL;
                }

                i++;
        }

        r[i] = NULL;
        повернути r;
}

char *ul_strv_join(char **l, const char *separator)
{
        char *r, *e;
        char **s;
        size_t n, k;

        якщо (!separator)
                separator = " ";

        k = strlen(separator);

        n = 0;
        UL_STRV_FOREACH(s, l) {
                якщо (n != 0)
                        n += k;
                n += strlen(*s);
        }

        r = malloc(n + 1);
        якщо (!r)
                повернути NULL;

        e = r;
        UL_STRV_FOREACH(s, l) {
                якщо (e != r)
                        e = stpcpy(e, separator);

                e = stpcpy(e, *s);
        }

        *e = 0;

        повернути r;
}

ціле ul_strv_push(char ***l, char *value)
{
        char **c;
        unsigned n, m;

        якщо (!value)
                повернути 0;

        n = ul_strv_length(*l);

        /* Increase and check for overflow */
        m = n + 2;
        якщо (m < n)
                повернути -ENOMEM;

        c = reallocarray(*l, m, sizeof(char *));
        якщо (!c)
                повернути -ENOMEM;

        c[n] = value;
        c[n+1] = NULL;

        *l = c;
        повернути 0;
}

ціле ul_strv_push_prepend(char ***l, char *value)
{
        char **c;
        unsigned n, m, i;

        якщо (!value)
                повернути 0;

        n = ul_strv_length(*l);

        /* increase and check for overflow */
        m = n + 2;
        якщо (m < n)
                повернути -ENOMEM;

        c = malloc(sizeof(char *) * m);
        якщо (!c)
                повернути -ENOMEM;

        для (i = 0; i < n; i++)
                c[i+1] = (*l)[i];

        c[0] = value;
        c[n+1] = NULL;

        free(*l);
        *l = c;

        повернути 0;
}

ціле ul_strv_consume(char ***l, char *value)
{
        ціле r;

        r = ul_strv_push(l, value);
        якщо (r < 0)
                free(value);

        повернути r;
}

ціле ul_strv_consume_prepend(char ***l, char *value)
{
        ціле r;

        r = ul_strv_push_prepend(l, value);
        якщо (r < 0)
                free(value);

        повернути r;
}

ціле ul_strv_extend(char ***l, const char *value)
{
        char *v;

        якщо (!value)
                повернути 0;

        v = strdup(value);
        якщо (!v)
                повернути -ENOMEM;

        повернути ul_strv_consume(l, v);
}

char **ul_strv_remove(char **l, const char *s)
{
        char **f, **t;

        якщо (!l)
                повернути NULL;

        assert(s);

        /* Drops every occurrence of s in the string list, edits
         * in-place. */

        для (f = t = l; *f; f++)
                якщо (strcmp(*f, s) == 0)
                        free(*f);
                інакше
                        *(t++) = *f;

        *t = NULL;
        повернути l;
}

ціле ul_strv_extendf(char ***l, const char *format, ...)
{
        va_list ap;
        char *x;
        ціле r;

        va_start(ap, format);
        r = vasprintf(&x, format, ap);
        va_end(ap);

        якщо (r < 0)
                повернути -ENOMEM;

        повернути ul_strv_consume(l, x);
}

ціле ul_strv_extendv(char ***l, const char *format, va_list ap)
{
        char *x;
        ціле r;

        r = vasprintf(&x, format, ap);
        якщо (r < 0)
                повернути -ENOMEM;

        повернути ul_strv_consume(l, x);
}

char **ul_strv_reverse(char **l)
{
        unsigned n, i;

        n = ul_strv_length(l);
        якщо (n <= 1)
                повернути l;

        для (i = 0; i < n / 2; i++) {
                char *t;

                t = l[i];
                l[i] = l[n-1-i];
                l[n-1-i] = t;
        }

        повернути l;
}
