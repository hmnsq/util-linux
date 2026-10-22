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

#অন্তর্ভাৱ <stdlib.h>
#অন্তর্ভাৱ <stdarg.h>
#অন্তর্ভাৱ <string.h>
#অন্তর্ভাৱ <errno.h>
#অন্তর্ভাৱ <stdbool.h>
#অন্তর্ভাৱ <assert.h>

#অন্তর্ভাৱ "strutils.h"
#অন্তর্ভাৱ "strv.h"

স্থির শূন্য ul_strv_clear(ৱর্ণ **l)
{
        ৱর্ণ **k;

        যদি (!l)
                ঘূৰাই;

        বাবে (k = l; *k; k++)
                মুক্ত(*k);

        *l = রিক্ত;
}

ৱর্ণ **ul_strv_free(ৱর্ণ **l)
{
        ul_strv_clear(l);
        মুক্ত(l);
        ঘূৰাই রিক্ত;
}

ৱর্ণ **ul_strv_copy(ৱর্ণ * ধ্রুৱ *l)
{
        ৱর্ণ **r, **k;

        k = r = malloc(পরিমাণ(ৱর্ণ *) * (ul_strv_length(l) + 1));
        যদি (!r)
                ঘূৰাই রিক্ত;

        যদি (l)
                বাবে (; *l; k++, l++) {
                        *k = strdup(*l);
                        যদি (!*k) {
                                ul_strv_free(r);
                                ঘূৰাই রিক্ত;
                        }
                }

        *k = রিক্ত;
        ঘূৰাই r;
}

unsigned ul_strv_length(ৱর্ণ * ধ্রুৱ *l) {
        unsigned n = 0;

        যদি (!l)
                ঘূৰাই 0;

        বাবে (; *l; l++)
                n++;

        ঘূৰাই n;
}

স্থির ৱর্ণ **ul_strv_new_ap(ধ্রুৱ ৱর্ণ *x, va_list ap)
{
        ধ্রুৱ ৱর্ণ *s;
        ৱর্ণ **a;
        unsigned n = 0, i = 0;
        va_list aq;

        /* As a special trick we ignore all listed strings that equal
         * (const char*) -1. This is supposed to be used with the
         * UL_STRV_IFNOTNULL() macro to include possibly NULL strings in
         * the string list. */

        যদি (x) {
                n = x == (ধ্রুৱ ৱর্ণ*) -1 ? 0 : 1;

                va_copy(aq, ap);
                যেতিয়ালৈ ((s = va_arg(aq, ধ্রুৱ ৱর্ণ*))) {
                        যদি (s == (ধ্রুৱ ৱর্ণ*) -1)
                                অনুৱর্ত;

                        n++;
                }

                va_end(aq);
        }

        a = malloc(পরিমাণ(ৱর্ণ *) * (n + 1));
        যদি (!a)
                ঘূৰাই রিক্ত;

        যদি (x) {
                যদি (x != (ধ্রুৱ ৱর্ণ*) -1) {
                        a[i] = strdup(x);
                        যদি (!a[i])
                                গচ্ছ fail;
                        i++;
                }

                যেতিয়ালৈ ((s = va_arg(ap, ধ্রুৱ ৱর্ণ*))) {

                        যদি (s == (ধ্রুৱ ৱর্ণ*) -1)
                                অনুৱর্ত;

                        a[i] = strdup(s);
                        যদি (!a[i])
                                গচ্ছ fail;

                        i++;
                }
        }

        a[i] = রিক্ত;

        ঘূৰাই a;

fail:
        ul_strv_free(a);
        ঘূৰাই রিক্ত;
}

ৱর্ণ **ul_strv_new(ধ্রুৱ ৱর্ণ *x, ...)
{
        ৱর্ণ **r;
        va_list ap;

        va_start(ap, x);
        r = ul_strv_new_ap(x, ap);
        va_end(ap);

        ঘূৰাই r;
}

পূর্ণাঙ্ক ul_strv_extend_strv(ৱর্ণ ***a, ৱর্ণ **b)
{
        পূর্ণাঙ্ক r;
        ৱর্ণ **s;

        UL_STRV_FOREACH(s, b) {
                r = ul_strv_extend(a, *s);
                যদি (r < 0)
                        ঘূৰাই r;
        }

        ঘূৰাই 0;
}

পূর্ণাঙ্ক ul_strv_extend_strv_concat(ৱর্ণ ***a, ৱর্ণ **b, ধ্রুৱ ৱর্ণ *suffix)
{
        পূর্ণাঙ্ক r;
        ৱর্ণ **s;

        UL_STRV_FOREACH(s, b) {
                ৱর্ণ *v;

                v = ul_strconcat(*s, suffix);
                যদি (!v)
                        ঘূৰাই -ENOMEM;

                r = ul_strv_ঠেলো(a, v);
                যদি (r < 0) {
                        মুক্ত(v);
                        ঘূৰাই r;
                }
        }

        ঘূৰাই 0;
}


#define _FOREACH_WORD(word, length, s, separator, quoted, state)        \
        বাবে ((state) = (s), (word) = ul_split(&(state), &(length), (separator), (quoted)); (word); (word) = ul_split(&(state), &(length), (separator), (quoted)))

#define FOREACH_WORD_SEPARATOR(word, length, s, separator, state)       \
        _FOREACH_WORD(word, length, s, separator, false, state)


ৱর্ণ **ul_strv_split(ধ্রুৱ ৱর্ণ *s, ধ্রুৱ ৱর্ণ *separator)
{
        ধ্রুৱ ৱর্ণ *word, *state;
        size_t l;
        unsigned n, i;
        ৱর্ণ **r;

        প্রতিজ্ঞা(s);

        n = 0;
        FOREACH_WORD_SEPARATOR(word, l, s, separator, state)
                n++;

        r = malloc(পরিমাণ(ৱর্ণ *) * (n + 1));
        যদি (!r)
                ঘূৰাই রিক্ত;

        i = 0;
        FOREACH_WORD_SEPARATOR(word, l, s, separator, state) {
                r[i] = strndup(word, l);
                যদি (!r[i]) {
                        ul_strv_free(r);
                        ঘূৰাই রিক্ত;
                }

                i++;
        }

        r[i] = রিক্ত;
        ঘূৰাই r;
}

ৱর্ণ *ul_strv_join(ৱর্ণ **l, ধ্রুৱ ৱর্ণ *separator)
{
        ৱর্ণ *r, *e;
        ৱর্ণ **s;
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
                ঘূৰাই রিক্ত;

        e = r;
        UL_STRV_FOREACH(s, l) {
                যদি (e != r)
                        e = stpcpy(e, separator);

                e = stpcpy(e, *s);
        }

        *e = 0;

        ঘূৰাই r;
}

পূর্ণাঙ্ক ul_strv_ঠেলো(ৱর্ণ ***l, ৱর্ণ *মান)
{
        ৱর্ণ **c;
        unsigned n, m;

        যদি (!মান)
                ঘূৰাই 0;

        n = ul_strv_length(*l);

        /* Increase and check for overflow */
        m = n + 2;
        যদি (m < n)
                ঘূৰাই -ENOMEM;

        c = reallocarray(*l, m, পরিমাণ(ৱর্ণ *));
        যদি (!c)
                ঘূৰাই -ENOMEM;

        c[n] = মান;
        c[n+1] = রিক্ত;

        *l = c;
        ঘূৰাই 0;
}

পূর্ণাঙ্ক ul_strv_ঠেলো_prepend(ৱর্ণ ***l, ৱর্ণ *মান)
{
        ৱর্ণ **c;
        unsigned n, m, i;

        যদি (!মান)
                ঘূৰাই 0;

        n = ul_strv_length(*l);

        /* increase and check for overflow */
        m = n + 2;
        যদি (m < n)
                ঘূৰাই -ENOMEM;

        c = malloc(পরিমাণ(ৱর্ণ *) * m);
        যদি (!c)
                ঘূৰাই -ENOMEM;

        বাবে (i = 0; i < n; i++)
                c[i+1] = (*l)[i];

        c[0] = মান;
        c[n+1] = রিক্ত;

        মুক্ত(*l);
        *l = c;

        ঘূৰাই 0;
}

পূর্ণাঙ্ক ul_strv_consume(ৱর্ণ ***l, ৱর্ণ *মান)
{
        পূর্ণাঙ্ক r;

        r = ul_strv_ঠেলো(l, মান);
        যদি (r < 0)
                মুক্ত(মান);

        ঘূৰাই r;
}

পূর্ণাঙ্ক ul_strv_consume_prepend(ৱর্ণ ***l, ৱর্ণ *মান)
{
        পূর্ণাঙ্ক r;

        r = ul_strv_ঠেলো_prepend(l, মান);
        যদি (r < 0)
                মুক্ত(মান);

        ঘূৰাই r;
}

পূর্ণাঙ্ক ul_strv_extend(ৱর্ণ ***l, ধ্রুৱ ৱর্ণ *মান)
{
        ৱর্ণ *v;

        যদি (!মান)
                ঘূৰাই 0;

        v = strdup(মান);
        যদি (!v)
                ঘূৰাই -ENOMEM;

        ঘূৰাই ul_strv_consume(l, v);
}

ৱর্ণ **ul_strv_remove(ৱর্ণ **l, ধ্রুৱ ৱর্ণ *s)
{
        ৱর্ণ **f, **t;

        যদি (!l)
                ঘূৰাই রিক্ত;

        প্রতিজ্ঞা(s);

        /* Drops every occurrence of s in the string list, edits
         * in-place. */

        বাবে (f = t = l; *f; f++)
                যদি (strcmp(*f, s) == 0)
                        মুক্ত(*f);
                অন্যথা
                        *(t++) = *f;

        *t = রিক্ত;
        ঘূৰাই l;
}

পূর্ণাঙ্ক ul_strv_extendf(ৱর্ণ ***l, ধ্রুৱ ৱর্ণ *format, ...)
{
        va_list ap;
        ৱর্ণ *x;
        পূর্ণাঙ্ক r;

        va_start(ap, format);
        r = vasprintf(&x, format, ap);
        va_end(ap);

        যদি (r < 0)
                ঘূৰাই -ENOMEM;

        ঘূৰাই ul_strv_consume(l, x);
}

পূর্ণাঙ্ক ul_strv_extendv(ৱর্ণ ***l, ধ্রুৱ ৱর্ণ *format, va_list ap)
{
        ৱর্ণ *x;
        পূর্ণাঙ্ক r;

        r = vasprintf(&x, format, ap);
        যদি (r < 0)
                ঘূৰাই -ENOMEM;

        ঘূৰাই ul_strv_consume(l, x);
}

ৱর্ণ **ul_strv_reverse(ৱর্ণ **l)
{
        unsigned n, i;

        n = ul_strv_length(l);
        যদি (n <= 1)
                ঘূৰাই l;

        বাবে (i = 0; i < n / 2; i++) {
                ৱর্ণ *t;

                t = l[i];
                l[i] = l[n-1-i];
                l[n-1-i] = t;
        }

        ঘূৰাই l;
}
