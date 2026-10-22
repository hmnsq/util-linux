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

        もし (!l)
                戻る;

        ために (k = l; *k; k++)
                free(*k);

        *l = NULL;
}

char **ul_strv_free(char **l)
{
        ul_strv_clear(l);
        free(l);
        戻る NULL;
}

char **ul_strv_copy(char * const *l)
{
        char **r, **k;

        k = r = malloc(sizeof(char *) * (ul_strv_length(l) + 1));
        もし (!r)
                戻る NULL;

        もし (l)
                ために (; *l; k++, l++) {
                        *k = strdup(*l);
                        もし (!*k) {
                                ul_strv_free(r);
                                戻る NULL;
                        }
                }

        *k = NULL;
        戻る r;
}

unsigned ul_strv_length(char * const *l) {
        unsigned n = 0;

        もし (!l)
                戻る 0;

        ために (; *l; l++)
                n++;

        戻る n;
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

        もし (x) {
                n = x == (const char*) -1 ? 0 : 1;

                va_copy(aq, ap);
                ながら ((s = va_arg(aq, const char*))) {
                        もし (s == (const char*) -1)
                                continue;

                        n++;
                }

                va_end(aq);
        }

        a = malloc(sizeof(char *) * (n + 1));
        もし (!a)
                戻る NULL;

        もし (x) {
                もし (x != (const char*) -1) {
                        a[i] = strdup(x);
                        もし (!a[i])
                                goto fail;
                        i++;
                }

                ながら ((s = va_arg(ap, const char*))) {

                        もし (s == (const char*) -1)
                                continue;

                        a[i] = strdup(s);
                        もし (!a[i])
                                goto fail;

                        i++;
                }
        }

        a[i] = NULL;

        戻る a;

fail:
        ul_strv_free(a);
        戻る NULL;
}

char **ul_strv_new(const char *x, ...)
{
        char **r;
        va_list ap;

        va_start(ap, x);
        r = ul_strv_new_ap(x, ap);
        va_end(ap);

        戻る r;
}

整数 ul_strv_extend_strv(char ***a, char **b)
{
        整数 r;
        char **s;

        UL_STRV_FOREACH(s, b) {
                r = ul_strv_extend(a, *s);
                もし (r < 0)
                        戻る r;
        }

        戻る 0;
}

整数 ul_strv_extend_strv_concat(char ***a, char **b, const char *suffix)
{
        整数 r;
        char **s;

        UL_STRV_FOREACH(s, b) {
                char *v;

                v = ul_strconcat(*s, suffix);
                もし (!v)
                        戻る -ENOMEM;

                r = ul_strv_push(a, v);
                もし (r < 0) {
                        free(v);
                        戻る r;
                }
        }

        戻る 0;
}


#define _FOREACH_WORD(word, length, s, separator, quoted, state)        \
        ために ((state) = (s), (word) = ul_split(&(state), &(length), (separator), (quoted)); (word); (word) = ul_split(&(state), &(length), (separator), (quoted)))

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
        もし (!r)
                戻る NULL;

        i = 0;
        FOREACH_WORD_SEPARATOR(word, l, s, separator, state) {
                r[i] = strndup(word, l);
                もし (!r[i]) {
                        ul_strv_free(r);
                        戻る NULL;
                }

                i++;
        }

        r[i] = NULL;
        戻る r;
}

char *ul_strv_join(char **l, const char *separator)
{
        char *r, *e;
        char **s;
        size_t n, k;

        もし (!separator)
                separator = " ";

        k = strlen(separator);

        n = 0;
        UL_STRV_FOREACH(s, l) {
                もし (n != 0)
                        n += k;
                n += strlen(*s);
        }

        r = malloc(n + 1);
        もし (!r)
                戻る NULL;

        e = r;
        UL_STRV_FOREACH(s, l) {
                もし (e != r)
                        e = stpcpy(e, separator);

                e = stpcpy(e, *s);
        }

        *e = 0;

        戻る r;
}

整数 ul_strv_push(char ***l, char *value)
{
        char **c;
        unsigned n, m;

        もし (!value)
                戻る 0;

        n = ul_strv_length(*l);

        /* Increase and check for overflow */
        m = n + 2;
        もし (m < n)
                戻る -ENOMEM;

        c = reallocarray(*l, m, sizeof(char *));
        もし (!c)
                戻る -ENOMEM;

        c[n] = value;
        c[n+1] = NULL;

        *l = c;
        戻る 0;
}

整数 ul_strv_push_prepend(char ***l, char *value)
{
        char **c;
        unsigned n, m, i;

        もし (!value)
                戻る 0;

        n = ul_strv_length(*l);

        /* increase and check for overflow */
        m = n + 2;
        もし (m < n)
                戻る -ENOMEM;

        c = malloc(sizeof(char *) * m);
        もし (!c)
                戻る -ENOMEM;

        ために (i = 0; i < n; i++)
                c[i+1] = (*l)[i];

        c[0] = value;
        c[n+1] = NULL;

        free(*l);
        *l = c;

        戻る 0;
}

整数 ul_strv_consume(char ***l, char *value)
{
        整数 r;

        r = ul_strv_push(l, value);
        もし (r < 0)
                free(value);

        戻る r;
}

整数 ul_strv_consume_prepend(char ***l, char *value)
{
        整数 r;

        r = ul_strv_push_prepend(l, value);
        もし (r < 0)
                free(value);

        戻る r;
}

整数 ul_strv_extend(char ***l, const char *value)
{
        char *v;

        もし (!value)
                戻る 0;

        v = strdup(value);
        もし (!v)
                戻る -ENOMEM;

        戻る ul_strv_consume(l, v);
}

char **ul_strv_remove(char **l, const char *s)
{
        char **f, **t;

        もし (!l)
                戻る NULL;

        assert(s);

        /* Drops every occurrence of s in the string list, edits
         * in-place. */

        ために (f = t = l; *f; f++)
                もし (strcmp(*f, s) == 0)
                        free(*f);
                否则
                        *(t++) = *f;

        *t = NULL;
        戻る l;
}

整数 ul_strv_extendf(char ***l, const char *format, ...)
{
        va_list ap;
        char *x;
        整数 r;

        va_start(ap, format);
        r = vasprintf(&x, format, ap);
        va_end(ap);

        もし (r < 0)
                戻る -ENOMEM;

        戻る ul_strv_consume(l, x);
}

整数 ul_strv_extendv(char ***l, const char *format, va_list ap)
{
        char *x;
        整数 r;

        r = vasprintf(&x, format, ap);
        もし (r < 0)
                戻る -ENOMEM;

        戻る ul_strv_consume(l, x);
}

char **ul_strv_reverse(char **l)
{
        unsigned n, i;

        n = ul_strv_length(l);
        もし (n <= 1)
                戻る l;

        ために (i = 0; i < n / 2; i++) {
                char *t;

                t = l[i];
                l[i] = l[n-1-i];
                l[n-1-i] = t;
        }

        戻る l;
}
