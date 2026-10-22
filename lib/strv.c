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

#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <stdarg.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <errno.h>
#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <assert.h>

#అన్తర్భావ "strutils.h"
#అన్తర్భావ "strv.h"

స్థిర శూన్య ul_strv_clear(అక్షరం **l)
{
        అక్షరం **k;

        ఒకవేళ (!l)
                తిరిగి;

        కొరకు (k = l; *k; k++)
                విడుదల(*k);

        *l = ఖాళీ;
}

అక్షరం **ul_strv_free(అక్షరం **l)
{
        ul_strv_clear(l);
        విడుదల(l);
        తిరిగి ఖాళీ;
}

అక్షరం **ul_strv_copy(అక్షరం * స్థిరరాశి *l)
{
        అక్షరం **r, **k;

        k = r = జ్ఞాపకం_ఇవ్వు(పరిమాణం(అక్షరం *) * (ul_strv_నిడివి(l) + 1));
        ఒకవేళ (!r)
                తిరిగి ఖాళీ;

        ఒకవేళ (l)
                కొరకు (; *l; k++, l++) {
                        *k = strdup(*l);
                        ఒకవేళ (!*k) {
                                ul_strv_free(r);
                                తిరిగి ఖాళీ;
                        }
                }

        *k = ఖాళీ;
        తిరిగి r;
}

గుర్తులేని ul_strv_నిడివి(అక్షరం * స్థిరరాశి *l) {
        గుర్తులేని n = 0;

        ఒకవేళ (!l)
                తిరిగి 0;

        కొరకు (; *l; l++)
                n++;

        తిరిగి n;
}

స్థిర అక్షరం **ul_strv_new_ap(స్థిరరాశి అక్షరం *x, va_list ap)
{
        స్థిరరాశి అక్షరం *s;
        అక్షరం **a;
        గుర్తులేని n = 0, i = 0;
        va_list aq;

        /* As a special trick we ignore all listed strings that equal
         * (const char*) -1. This is supposed to be used with the
         * UL_STRV_IFNOTNULL() macro to include possibly NULL strings in
         * the string list. */

        ఒకవేళ (x) {
                n = x == (స్థిరరాశి అక్షరం*) -1 ? 0 : 1;

                va_copy(aq, ap);
                అయితే ((s = va_arg(aq, స్థిరరాశి అక్షరం*))) {
                        ఒకవేళ (s == (స్థిరరాశి అక్షరం*) -1)
                                కొనసాగు;

                        n++;
                }

                va_end(aq);
        }

        a = జ్ఞాపకం_ఇవ్వు(పరిమాణం(అక్షరం *) * (n + 1));
        ఒకవేళ (!a)
                తిరిగి ఖాళీ;

        ఒకవేళ (x) {
                ఒకవేళ (x != (స్థిరరాశి అక్షరం*) -1) {
                        a[i] = strdup(x);
                        ఒకవేళ (!a[i])
                                వెళ్ళు fail;
                        i++;
                }

                అయితే ((s = va_arg(ap, స్థిరరాశి అక్షరం*))) {

                        ఒకవేళ (s == (స్థిరరాశి అక్షరం*) -1)
                                కొనసాగు;

                        a[i] = strdup(s);
                        ఒకవేళ (!a[i])
                                వెళ్ళు fail;

                        i++;
                }
        }

        a[i] = ఖాళీ;

        తిరిగి a;

fail:
        ul_strv_free(a);
        తిరిగి ఖాళీ;
}

అక్షరం **ul_strv_new(స్థిరరాశి అక్షరం *x, ...)
{
        అక్షరం **r;
        va_list ap;

        va_start(ap, x);
        r = ul_strv_new_ap(x, ap);
        va_end(ap);

        తిరిగి r;
}

పూర్ణాంకం ul_strv_extend_strv(అక్షరం ***a, అక్షరం **b)
{
        పూర్ణాంకం r;
        అక్షరం **s;

        UL_STRV_FOREACH(s, b) {
                r = ul_strv_extend(a, *s);
                ఒకవేళ (r < 0)
                        తిరిగి r;
        }

        తిరిగి 0;
}

పూర్ణాంకం ul_strv_extend_strv_concat(అక్షరం ***a, అక్షరం **b, స్థిరరాశి అక్షరం *suffix)
{
        పూర్ణాంకం r;
        అక్షరం **s;

        UL_STRV_FOREACH(s, b) {
                అక్షరం *v;

                v = ul_strconcat(*s, suffix);
                ఒకవేళ (!v)
                        తిరిగి -ENOMEM;

                r = ul_strv_నెట్టు(a, v);
                ఒకవేళ (r < 0) {
                        విడుదల(v);
                        తిరిగి r;
                }
        }

        తిరిగి 0;
}


#నిర్వచించు FOREACH_మాట(మాట, నిడివి, s, separator, quoted, state)        \
        కొరకు ((state) = (s), (మాట) = ul_విభజించు(&(state), &(నిడివి), (separator), (quoted)); (మాట); (మాట) = ul_విభజించు(&(state), &(నిడివి), (separator), (quoted)))

#నిర్వచించు FOREACH_మాట_SEPARATOR(మాట, నిడివి, s, separator, state)       \
        FOREACH_మాట(మాట, నిడివి, s, separator, false, state)


అక్షరం **ul_strv_విభజించు(స్థిరరాశి అక్షరం *s, స్థిరరాశి అక్షరం *separator)
{
        స్థిరరాశి అక్షరం *మాట, *state;
        size_t l;
        గుర్తులేని n, i;
        అక్షరం **r;

        ప్రతిజ్ఞా(s);

        n = 0;
        FOREACH_మాట_SEPARATOR(మాట, l, s, separator, state)
                n++;

        r = జ్ఞాపకం_ఇవ్వు(పరిమాణం(అక్షరం *) * (n + 1));
        ఒకవేళ (!r)
                తిరిగి ఖాళీ;

        i = 0;
        FOREACH_మాట_SEPARATOR(మాట, l, s, separator, state) {
                r[i] = strndup(మాట, l);
                ఒకవేళ (!r[i]) {
                        ul_strv_free(r);
                        తిరిగి ఖాళీ;
                }

                i++;
        }

        r[i] = ఖాళీ;
        తిరిగి r;
}

అక్షరం *ul_strv_join(అక్షరం **l, స్థిరరాశి అక్షరం *separator)
{
        అక్షరం *r, *e;
        అక్షరం **s;
        size_t n, k;

        ఒకవేళ (!separator)
                separator = " ";

        k = అక్షరశ్రేణి_పొడవు(separator);

        n = 0;
        UL_STRV_FOREACH(s, l) {
                ఒకవేళ (n != 0)
                        n += k;
                n += అక్షరశ్రేణి_పొడవు(*s);
        }

        r = జ్ఞాపకం_ఇవ్వు(n + 1);
        ఒకవేళ (!r)
                తిరిగి ఖాళీ;

        e = r;
        UL_STRV_FOREACH(s, l) {
                ఒకవేళ (e != r)
                        e = stpcpy(e, separator);

                e = stpcpy(e, *s);
        }

        *e = 0;

        తిరిగి r;
}

పూర్ణాంకం ul_strv_నెట్టు(అక్షరం ***l, అక్షరం *విలువ)
{
        అక్షరం **c;
        గుర్తులేని n, m;

        ఒకవేళ (!విలువ)
                తిరిగి 0;

        n = ul_strv_నిడివి(*l);

        /* Increase and check for overflow */
        m = n + 2;
        ఒకవేళ (m < n)
                తిరిగి -ENOMEM;

        c = reallocarray(*l, m, పరిమాణం(అక్షరం *));
        ఒకవేళ (!c)
                తిరిగి -ENOMEM;

        c[n] = విలువ;
        c[n+1] = ఖాళీ;

        *l = c;
        తిరిగి 0;
}

పూర్ణాంకం ul_strv_నెట్టు_prepend(అక్షరం ***l, అక్షరం *విలువ)
{
        అక్షరం **c;
        గుర్తులేని n, m, i;

        ఒకవేళ (!విలువ)
                తిరిగి 0;

        n = ul_strv_నిడివి(*l);

        /* increase and check for overflow */
        m = n + 2;
        ఒకవేళ (m < n)
                తిరిగి -ENOMEM;

        c = జ్ఞాపకం_ఇవ్వు(పరిమాణం(అక్షరం *) * m);
        ఒకవేళ (!c)
                తిరిగి -ENOMEM;

        కొరకు (i = 0; i < n; i++)
                c[i+1] = (*l)[i];

        c[0] = విలువ;
        c[n+1] = ఖాళీ;

        విడుదల(*l);
        *l = c;

        తిరిగి 0;
}

పూర్ణాంకం ul_strv_consume(అక్షరం ***l, అక్షరం *విలువ)
{
        పూర్ణాంకం r;

        r = ul_strv_నెట్టు(l, విలువ);
        ఒకవేళ (r < 0)
                విడుదల(విలువ);

        తిరిగి r;
}

పూర్ణాంకం ul_strv_consume_prepend(అక్షరం ***l, అక్షరం *విలువ)
{
        పూర్ణాంకం r;

        r = ul_strv_నెట్టు_prepend(l, విలువ);
        ఒకవేళ (r < 0)
                విడుదల(విలువ);

        తిరిగి r;
}

పూర్ణాంకం ul_strv_extend(అక్షరం ***l, స్థిరరాశి అక్షరం *విలువ)
{
        అక్షరం *v;

        ఒకవేళ (!విలువ)
                తిరిగి 0;

        v = strdup(విలువ);
        ఒకవేళ (!v)
                తిరిగి -ENOMEM;

        తిరిగి ul_strv_consume(l, v);
}

అక్షరం **ul_strv_remove(అక్షరం **l, స్థిరరాశి అక్షరం *s)
{
        అక్షరం **f, **t;

        ఒకవేళ (!l)
                తిరిగి ఖాళీ;

        ప్రతిజ్ఞా(s);

        /* Drops every occurrence of s in the string list, edits
         * in-place. */

        కొరకు (f = t = l; *f; f++)
                ఒకవేళ (strcmp(*f, s) == 0)
                        విడుదల(*f);
                లేకపోతే
                        *(t++) = *f;

        *t = ఖాళీ;
        తిరిగి l;
}

పూర్ణాంకం ul_strv_extendf(అక్షరం ***l, స్థిరరాశి అక్షరం *format, ...)
{
        va_list ap;
        అక్షరం *x;
        పూర్ణాంకం r;

        va_start(ap, format);
        r = vasprintf(&x, format, ap);
        va_end(ap);

        ఒకవేళ (r < 0)
                తిరిగి -ENOMEM;

        తిరిగి ul_strv_consume(l, x);
}

పూర్ణాంకం ul_strv_extendv(అక్షరం ***l, స్థిరరాశి అక్షరం *format, va_list ap)
{
        అక్షరం *x;
        పూర్ణాంకం r;

        r = vasprintf(&x, format, ap);
        ఒకవేళ (r < 0)
                తిరిగి -ENOMEM;

        తిరిగి ul_strv_consume(l, x);
}

అక్షరం **ul_strv_తిరగేయి(అక్షరం **l)
{
        గుర్తులేని n, i;

        n = ul_strv_నిడివి(l);
        ఒకవేళ (n <= 1)
                తిరిగి l;

        కొరకు (i = 0; i < n / 2; i++) {
                అక్షరం *t;

                t = l[i];
                l[i] = l[n-1-i];
                l[n-1-i] = t;
        }

        తిరిగి l;
}
