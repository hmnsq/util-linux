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

#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <stdarg.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <errno.h>
#অন্তর্ভাব <stdbool.h>
#অন্তর্ভাব <assert.h>

#অন্তর্ভাব "strutils.h"
#অন্তর্ভাব "strv.h"

স্থির শূন্য ul_strv_clear(বর্ণ **l)
{
        বর্ণ **k;

        যদি (!l)
                প্রত্যাগম;

        প্রতি (k = l; *k; k++)
                মুক্ত(*k);

        *l = রিক্ত;
}

বর্ণ **ul_strv_free(বর্ণ **l)
{
        ul_strv_clear(l);
        মুক্ত(l);
        প্রত্যাগম রিক্ত;
}

বর্ণ **ul_strv_copy(বর্ণ * ধ্রুব *l)
{
        বর্ণ **r, **k;

        k = r = malloc(পরিমাণ(বর্ণ *) * (ul_strv_length(l) + 1));
        যদি (!r)
                প্রত্যাগম রিক্ত;

        যদি (l)
                প্রতি (; *l; k++, l++) {
                        *k = strdup(*l);
                        যদি (!*k) {
                                ul_strv_free(r);
                                প্রত্যাগম রিক্ত;
                        }
                }

        *k = রিক্ত;
        প্রত্যাগম r;
}

unsigned ul_strv_length(বর্ণ * ধ্রুব *l) {
        unsigned n = 0;

        যদি (!l)
                প্রত্যাগম 0;

        প্রতি (; *l; l++)
                n++;

        প্রত্যাগম n;
}

স্থির বর্ণ **ul_strv_new_ap(ধ্রুব বর্ণ *x, va_list ap)
{
        ধ্রুব বর্ণ *s;
        বর্ণ **a;
        unsigned n = 0, i = 0;
        va_list aq;

        /* As a special trick we ignore all listed strings that equal
         * (const char*) -1. This is supposed to be used with the
         * UL_STRV_IFNOTNULL() macro to include possibly NULL strings in
         * the string list. */

        যদি (x) {
                n = x == (ধ্রুব বর্ণ*) -1 ? 0 : 1;

                va_copy(aq, ap);
                যাবত্ ((s = va_arg(aq, ধ্রুব বর্ণ*))) {
                        যদি (s == (ধ্রুব বর্ণ*) -1)
                                অনুবর্ত;

                        n++;
                }

                va_end(aq);
        }

        a = malloc(পরিমাণ(বর্ণ *) * (n + 1));
        যদি (!a)
                প্রত্যাগম রিক্ত;

        যদি (x) {
                যদি (x != (ধ্রুব বর্ণ*) -1) {
                        a[i] = strdup(x);
                        যদি (!a[i])
                                গচ্ছ fail;
                        i++;
                }

                যাবত্ ((s = va_arg(ap, ধ্রুব বর্ণ*))) {

                        যদি (s == (ধ্রুব বর্ণ*) -1)
                                অনুবর্ত;

                        a[i] = strdup(s);
                        যদি (!a[i])
                                গচ্ছ fail;

                        i++;
                }
        }

        a[i] = রিক্ত;

        প্রত্যাগম a;

fail:
        ul_strv_free(a);
        প্রত্যাগম রিক্ত;
}

বর্ণ **ul_strv_new(ধ্রুব বর্ণ *x, ...)
{
        বর্ণ **r;
        va_list ap;

        va_start(ap, x);
        r = ul_strv_new_ap(x, ap);
        va_end(ap);

        প্রত্যাগম r;
}

পূর্ণাঙ্ক ul_strv_extend_strv(বর্ণ ***a, বর্ণ **b)
{
        পূর্ণাঙ্ক r;
        বর্ণ **s;

        UL_STRV_FOREACH(s, b) {
                r = ul_strv_extend(a, *s);
                যদি (r < 0)
                        প্রত্যাগম r;
        }

        প্রত্যাগম 0;
}

পূর্ণাঙ্ক ul_strv_extend_strv_concat(বর্ণ ***a, বর্ণ **b, ধ্রুব বর্ণ *suffix)
{
        পূর্ণাঙ্ক r;
        বর্ণ **s;

        UL_STRV_FOREACH(s, b) {
                বর্ণ *v;

                v = ul_strconcat(*s, suffix);
                যদি (!v)
                        প্রত্যাগম -ENOMEM;

                r = ul_strv_ঠেলো(a, v);
                যদি (r < 0) {
                        মুক্ত(v);
                        প্রত্যাগম r;
                }
        }

        প্রত্যাগম 0;
}


#define _FOREACH_WORD(word, length, s, separator, quoted, state)        \
        প্রতি ((state) = (s), (word) = ul_split(&(state), &(length), (separator), (quoted)); (word); (word) = ul_split(&(state), &(length), (separator), (quoted)))

#define FOREACH_WORD_SEPARATOR(word, length, s, separator, state)       \
        _FOREACH_WORD(word, length, s, separator, false, state)


বর্ণ **ul_strv_split(ধ্রুব বর্ণ *s, ধ্রুব বর্ণ *separator)
{
        ধ্রুব বর্ণ *word, *state;
        size_t l;
        unsigned n, i;
        বর্ণ **r;

        প্রতিজ্ঞা(s);

        n = 0;
        FOREACH_WORD_SEPARATOR(word, l, s, separator, state)
                n++;

        r = malloc(পরিমাণ(বর্ণ *) * (n + 1));
        যদি (!r)
                প্রত্যাগম রিক্ত;

        i = 0;
        FOREACH_WORD_SEPARATOR(word, l, s, separator, state) {
                r[i] = strndup(word, l);
                যদি (!r[i]) {
                        ul_strv_free(r);
                        প্রত্যাগম রিক্ত;
                }

                i++;
        }

        r[i] = রিক্ত;
        প্রত্যাগম r;
}

বর্ণ *ul_strv_join(বর্ণ **l, ধ্রুব বর্ণ *separator)
{
        বর্ণ *r, *e;
        বর্ণ **s;
        size_t n, k;

        যদি (!separator)
                separator = " ";

        k = strlen(separator);

        n = 0;
        UL_STRV_FOREACH(s, l) {
                যদি (n != 0)
                        n += k;
                n += strlen(*s);
        }

        r = malloc(n + 1);
        যদি (!r)
                প্রত্যাগম রিক্ত;

        e = r;
        UL_STRV_FOREACH(s, l) {
                যদি (e != r)
                        e = stpcpy(e, separator);

                e = stpcpy(e, *s);
        }

        *e = 0;

        প্রত্যাগম r;
}

পূর্ণাঙ্ক ul_strv_ঠেলো(বর্ণ ***l, বর্ণ *মান)
{
        বর্ণ **c;
        unsigned n, m;

        যদি (!মান)
                প্রত্যাগম 0;

        n = ul_strv_length(*l);

        /* Increase and check for overflow */
        m = n + 2;
        যদি (m < n)
                প্রত্যাগম -ENOMEM;

        c = reallocarray(*l, m, পরিমাণ(বর্ণ *));
        যদি (!c)
                প্রত্যাগম -ENOMEM;

        c[n] = মান;
        c[n+1] = রিক্ত;

        *l = c;
        প্রত্যাগম 0;
}

পূর্ণাঙ্ক ul_strv_ঠেলো_prepend(বর্ণ ***l, বর্ণ *মান)
{
        বর্ণ **c;
        unsigned n, m, i;

        যদি (!মান)
                প্রত্যাগম 0;

        n = ul_strv_length(*l);

        /* increase and check for overflow */
        m = n + 2;
        যদি (m < n)
                প্রত্যাগম -ENOMEM;

        c = malloc(পরিমাণ(বর্ণ *) * m);
        যদি (!c)
                প্রত্যাগম -ENOMEM;

        প্রতি (i = 0; i < n; i++)
                c[i+1] = (*l)[i];

        c[0] = মান;
        c[n+1] = রিক্ত;

        মুক্ত(*l);
        *l = c;

        প্রত্যাগম 0;
}

পূর্ণাঙ্ক ul_strv_consume(বর্ণ ***l, বর্ণ *মান)
{
        পূর্ণাঙ্ক r;

        r = ul_strv_ঠেলো(l, মান);
        যদি (r < 0)
                মুক্ত(মান);

        প্রত্যাগম r;
}

পূর্ণাঙ্ক ul_strv_consume_prepend(বর্ণ ***l, বর্ণ *মান)
{
        পূর্ণাঙ্ক r;

        r = ul_strv_ঠেলো_prepend(l, মান);
        যদি (r < 0)
                মুক্ত(মান);

        প্রত্যাগম r;
}

পূর্ণাঙ্ক ul_strv_extend(বর্ণ ***l, ধ্রুব বর্ণ *মান)
{
        বর্ণ *v;

        যদি (!মান)
                প্রত্যাগম 0;

        v = strdup(মান);
        যদি (!v)
                প্রত্যাগম -ENOMEM;

        প্রত্যাগম ul_strv_consume(l, v);
}

বর্ণ **ul_strv_remove(বর্ণ **l, ধ্রুব বর্ণ *s)
{
        বর্ণ **f, **t;

        যদি (!l)
                প্রত্যাগম রিক্ত;

        প্রতিজ্ঞা(s);

        /* Drops every occurrence of s in the string list, edits
         * in-place. */

        প্রতি (f = t = l; *f; f++)
                যদি (strcmp(*f, s) == 0)
                        মুক্ত(*f);
                অন্যথা
                        *(t++) = *f;

        *t = রিক্ত;
        প্রত্যাগম l;
}

পূর্ণাঙ্ক ul_strv_extendf(বর্ণ ***l, ধ্রুব বর্ণ *format, ...)
{
        va_list ap;
        বর্ণ *x;
        পূর্ণাঙ্ক r;

        va_start(ap, format);
        r = vasprintf(&x, format, ap);
        va_end(ap);

        যদি (r < 0)
                প্রত্যাগম -ENOMEM;

        প্রত্যাগম ul_strv_consume(l, x);
}

পূর্ণাঙ্ক ul_strv_extendv(বর্ণ ***l, ধ্রুব বর্ণ *format, va_list ap)
{
        বর্ণ *x;
        পূর্ণাঙ্ক r;

        r = vasprintf(&x, format, ap);
        যদি (r < 0)
                প্রত্যাগম -ENOMEM;

        প্রত্যাগম ul_strv_consume(l, x);
}

বর্ণ **ul_strv_reverse(বর্ণ **l)
{
        unsigned n, i;

        n = ul_strv_length(l);
        যদি (n <= 1)
                প্রত্যাগম l;

        প্রতি (i = 0; i < n / 2; i++) {
                বর্ণ *t;

                t = l[i];
                l[i] = l[n-1-i];
                l[n-1-i] = t;
        }

        প্রত্যাগম l;
}
