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

স্থির শূন্য ul_strv_clear(অক্ষর **l)
{
        অক্ষর **k;

        যদি (!l)
                ফেরত;

        জন্য (k = l; *k; k++)
                মুক্ত(*k);

        *l = ফাঁকা;
}

অক্ষর **ul_strv_free(অক্ষর **l)
{
        ul_strv_clear(l);
        মুক্ত(l);
        ফেরত ফাঁকা;
}

অক্ষর **ul_strv_copy(অক্ষর * ধ্রুবক *l)
{
        অক্ষর **r, **k;

        k = r = malloc(আকার(অক্ষর *) * (ul_strv_দৈর্ঘ্য(l) + 1));
        যদি (!r)
                ফেরত ফাঁকা;

        যদি (l)
                জন্য (; *l; k++, l++) {
                        *k = strdup(*l);
                        যদি (!*k) {
                                ul_strv_free(r);
                                ফেরত ফাঁকা;
                        }
                }

        *k = ফাঁকা;
        ফেরত r;
}

unsigned ul_strv_দৈর্ঘ্য(অক্ষর * ধ্রুবক *l) {
        unsigned n = 0;

        যদি (!l)
                ফেরত 0;

        জন্য (; *l; l++)
                n++;

        ফেরত n;
}

স্থির অক্ষর **ul_strv_new_ap(ধ্রুবক অক্ষর *x, va_list ap)
{
        ধ্রুবক অক্ষর *s;
        অক্ষর **a;
        unsigned n = 0, i = 0;
        va_list aq;

        /* As a special trick we ignore all listed strings that equal
         * (const char*) -1. This is supposed to be used with the
         * UL_STRV_IFNOTNULL() macro to include possibly NULL strings in
         * the string list. */

        যদি (x) {
                n = x == (ধ্রুবক অক্ষর*) -1 ? 0 : 1;

                va_copy(aq, ap);
                যখন ((s = va_arg(aq, ধ্রুবক অক্ষর*))) {
                        যদি (s == (ধ্রুবক অক্ষর*) -1)
                                চালু;

                        n++;
                }

                va_end(aq);
        }

        a = malloc(আকার(অক্ষর *) * (n + 1));
        যদি (!a)
                ফেরত ফাঁকা;

        যদি (x) {
                যদি (x != (ধ্রুবক অক্ষর*) -1) {
                        a[i] = strdup(x);
                        যদি (!a[i])
                                যাও fail;
                        i++;
                }

                যখন ((s = va_arg(ap, ধ্রুবক অক্ষর*))) {

                        যদি (s == (ধ্রুবক অক্ষর*) -1)
                                চালু;

                        a[i] = strdup(s);
                        যদি (!a[i])
                                যাও fail;

                        i++;
                }
        }

        a[i] = ফাঁকা;

        ফেরত a;

fail:
        ul_strv_free(a);
        ফেরত ফাঁকা;
}

অক্ষর **ul_strv_new(ধ্রুবক অক্ষর *x, ...)
{
        অক্ষর **r;
        va_list ap;

        va_start(ap, x);
        r = ul_strv_new_ap(x, ap);
        va_end(ap);

        ফেরত r;
}

পূর্ণসংখ্যা ul_strv_extend_strv(অক্ষর ***a, অক্ষর **b)
{
        পূর্ণসংখ্যা r;
        অক্ষর **s;

        UL_STRV_FOREACH(s, b) {
                r = ul_strv_extend(a, *s);
                যদি (r < 0)
                        ফেরত r;
        }

        ফেরত 0;
}

পূর্ণসংখ্যা ul_strv_extend_strv_concat(অক্ষর ***a, অক্ষর **b, ধ্রুবক অক্ষর *suffix)
{
        পূর্ণসংখ্যা r;
        অক্ষর **s;

        UL_STRV_FOREACH(s, b) {
                অক্ষর *v;

                v = ul_strconcat(*s, suffix);
                যদি (!v)
                        ফেরত -ENOMEM;

                r = ul_strv_ঠেলো(a, v);
                যদি (r < 0) {
                        মুক্ত(v);
                        ফেরত r;
                }
        }

        ফেরত 0;
}


#define _FOREACH_WORD(word, দৈর্ঘ্য, s, separator, quoted, state)        \
        জন্য ((state) = (s), (word) = ul_split(&(state), &(দৈর্ঘ্য), (separator), (quoted)); (word); (word) = ul_split(&(state), &(দৈর্ঘ্য), (separator), (quoted)))

#define FOREACH_WORD_SEPARATOR(word, দৈর্ঘ্য, s, separator, state)       \
        _FOREACH_WORD(word, দৈর্ঘ্য, s, separator, false, state)


অক্ষর **ul_strv_split(ধ্রুবক অক্ষর *s, ধ্রুবক অক্ষর *separator)
{
        ধ্রুবক অক্ষর *word, *state;
        size_t l;
        unsigned n, i;
        অক্ষর **r;

        প্রতিজ্ঞা(s);

        n = 0;
        FOREACH_WORD_SEPARATOR(word, l, s, separator, state)
                n++;

        r = malloc(আকার(অক্ষর *) * (n + 1));
        যদি (!r)
                ফেরত ফাঁকা;

        i = 0;
        FOREACH_WORD_SEPARATOR(word, l, s, separator, state) {
                r[i] = strndup(word, l);
                যদি (!r[i]) {
                        ul_strv_free(r);
                        ফেরত ফাঁকা;
                }

                i++;
        }

        r[i] = ফাঁকা;
        ফেরত r;
}

অক্ষর *ul_strv_join(অক্ষর **l, ধ্রুবক অক্ষর *separator)
{
        অক্ষর *r, *e;
        অক্ষর **s;
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
                ফেরত ফাঁকা;

        e = r;
        UL_STRV_FOREACH(s, l) {
                যদি (e != r)
                        e = stpcpy(e, separator);

                e = stpcpy(e, *s);
        }

        *e = 0;

        ফেরত r;
}

পূর্ণসংখ্যা ul_strv_ঠেলো(অক্ষর ***l, অক্ষর *মান)
{
        অক্ষর **c;
        unsigned n, m;

        যদি (!মান)
                ফেরত 0;

        n = ul_strv_দৈর্ঘ্য(*l);

        /* Increase and check for overflow */
        m = n + 2;
        যদি (m < n)
                ফেরত -ENOMEM;

        c = reallocarray(*l, m, আকার(অক্ষর *));
        যদি (!c)
                ফেরত -ENOMEM;

        c[n] = মান;
        c[n+1] = ফাঁকা;

        *l = c;
        ফেরত 0;
}

পূর্ণসংখ্যা ul_strv_ঠেলো_prepend(অক্ষর ***l, অক্ষর *মান)
{
        অক্ষর **c;
        unsigned n, m, i;

        যদি (!মান)
                ফেরত 0;

        n = ul_strv_দৈর্ঘ্য(*l);

        /* increase and check for overflow */
        m = n + 2;
        যদি (m < n)
                ফেরত -ENOMEM;

        c = malloc(আকার(অক্ষর *) * m);
        যদি (!c)
                ফেরত -ENOMEM;

        জন্য (i = 0; i < n; i++)
                c[i+1] = (*l)[i];

        c[0] = মান;
        c[n+1] = ফাঁকা;

        মুক্ত(*l);
        *l = c;

        ফেরত 0;
}

পূর্ণসংখ্যা ul_strv_consume(অক্ষর ***l, অক্ষর *মান)
{
        পূর্ণসংখ্যা r;

        r = ul_strv_ঠেলো(l, মান);
        যদি (r < 0)
                মুক্ত(মান);

        ফেরত r;
}

পূর্ণসংখ্যা ul_strv_consume_prepend(অক্ষর ***l, অক্ষর *মান)
{
        পূর্ণসংখ্যা r;

        r = ul_strv_ঠেলো_prepend(l, মান);
        যদি (r < 0)
                মুক্ত(মান);

        ফেরত r;
}

পূর্ণসংখ্যা ul_strv_extend(অক্ষর ***l, ধ্রুবক অক্ষর *মান)
{
        অক্ষর *v;

        যদি (!মান)
                ফেরত 0;

        v = strdup(মান);
        যদি (!v)
                ফেরত -ENOMEM;

        ফেরত ul_strv_consume(l, v);
}

অক্ষর **ul_strv_remove(অক্ষর **l, ধ্রুবক অক্ষর *s)
{
        অক্ষর **f, **t;

        যদি (!l)
                ফেরত ফাঁকা;

        প্রতিজ্ঞা(s);

        /* Drops every occurrence of s in the string list, edits
         * in-place. */

        জন্য (f = t = l; *f; f++)
                যদি (strcmp(*f, s) == 0)
                        মুক্ত(*f);
                অন্যথা
                        *(t++) = *f;

        *t = ফাঁকা;
        ফেরত l;
}

পূর্ণসংখ্যা ul_strv_extendf(অক্ষর ***l, ধ্রুবক অক্ষর *format, ...)
{
        va_list ap;
        অক্ষর *x;
        পূর্ণসংখ্যা r;

        va_start(ap, format);
        r = vasprintf(&x, format, ap);
        va_end(ap);

        যদি (r < 0)
                ফেরত -ENOMEM;

        ফেরত ul_strv_consume(l, x);
}

পূর্ণসংখ্যা ul_strv_extendv(অক্ষর ***l, ধ্রুবক অক্ষর *format, va_list ap)
{
        অক্ষর *x;
        পূর্ণসংখ্যা r;

        r = vasprintf(&x, format, ap);
        যদি (r < 0)
                ফেরত -ENOMEM;

        ফেরত ul_strv_consume(l, x);
}

অক্ষর **ul_strv_reverse(অক্ষর **l)
{
        unsigned n, i;

        n = ul_strv_দৈর্ঘ্য(l);
        যদি (n <= 1)
                ফেরত l;

        জন্য (i = 0; i < n / 2; i++) {
                অক্ষর *t;

                t = l[i];
                l[i] = l[n-1-i];
                l[n-1-i] = t;
        }

        ফেরত l;
}
