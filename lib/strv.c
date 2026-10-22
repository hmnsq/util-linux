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

#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <stdarg.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <assert.h>

#ಅನ್ತರ್ಭಾವ "strutils.h"
#ಅನ್ತರ್ಭಾವ "strv.h"

ಸ್ಥಿರ ಶೂನ್ಯ ul_strv_clear(ಅಕ್ಷರ **l)
{
        ಅಕ್ಷರ **k;

        ಆದರೆ (!l)
                ಹಿಂತಿರುಗಿ;

        ಗಾಗಿ (k = l; *k; k++)
                ಬಿಡುಗಡೆ(*k);

        *l = ಖಾಲಿ;
}

ಅಕ್ಷರ **ul_strv_free(ಅಕ್ಷರ **l)
{
        ul_strv_clear(l);
        ಬಿಡುಗಡೆ(l);
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

ಅಕ್ಷರ **ul_strv_copy(ಅಕ್ಷರ * ಸ್ಥಿರಾಂಕ *l)
{
        ಅಕ್ಷರ **r, **k;

        k = r = ಸ್ಮರಣೆ_ಕೊಡು(ಗಾತ್ರ(ಅಕ್ಷರ *) * (ul_strv_ಉದ್ದಳತೆ(l) + 1));
        ಆದರೆ (!r)
                ಹಿಂತಿರುಗಿ ಖಾಲಿ;

        ಆದರೆ (l)
                ಗಾಗಿ (; *l; k++, l++) {
                        *k = strdup(*l);
                        ಆದರೆ (!*k) {
                                ul_strv_free(r);
                                ಹಿಂತಿರುಗಿ ಖಾಲಿ;
                        }
                }

        *k = ಖಾಲಿ;
        ಹಿಂತಿರುಗಿ r;
}

ಚಿಹ್ನೆರಹಿತ ul_strv_ಉದ್ದಳತೆ(ಅಕ್ಷರ * ಸ್ಥಿರಾಂಕ *l) {
        ಚಿಹ್ನೆರಹಿತ n = 0;

        ಆದರೆ (!l)
                ಹಿಂತಿರುಗಿ 0;

        ಗಾಗಿ (; *l; l++)
                n++;

        ಹಿಂತಿರುಗಿ n;
}

ಸ್ಥಿರ ಅಕ್ಷರ **ul_strv_new_ap(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *x, va_list ap)
{
        ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s;
        ಅಕ್ಷರ **a;
        ಚಿಹ್ನೆರಹಿತ n = 0, i = 0;
        va_list aq;

        /* As a special trick we ignore all listed strings that equal
         * (const char*) -1. This is supposed to be used with the
         * UL_STRV_IFNOTNULL() macro to include possibly NULL strings in
         * the string list. */

        ಆದರೆ (x) {
                n = x == (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*) -1 ? 0 : 1;

                va_copy(aq, ap);
                ಆಗ ((s = va_arg(aq, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*))) {
                        ಆದರೆ (s == (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*) -1)
                                ಮುಂದುವರಿ;

                        n++;
                }

                va_end(aq);
        }

        a = ಸ್ಮರಣೆ_ಕೊಡು(ಗಾತ್ರ(ಅಕ್ಷರ *) * (n + 1));
        ಆದರೆ (!a)
                ಹಿಂತಿರುಗಿ ಖಾಲಿ;

        ಆದರೆ (x) {
                ಆದರೆ (x != (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*) -1) {
                        a[i] = strdup(x);
                        ಆದರೆ (!a[i])
                                ಹೋಗು fail;
                        i++;
                }

                ಆಗ ((s = va_arg(ap, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*))) {

                        ಆದರೆ (s == (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ*) -1)
                                ಮುಂದುವರಿ;

                        a[i] = strdup(s);
                        ಆದರೆ (!a[i])
                                ಹೋಗು fail;

                        i++;
                }
        }

        a[i] = ಖಾಲಿ;

        ಹಿಂತಿರುಗಿ a;

fail:
        ul_strv_free(a);
        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

ಅಕ್ಷರ **ul_strv_new(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *x, ...)
{
        ಅಕ್ಷರ **r;
        va_list ap;

        va_start(ap, x);
        r = ul_strv_new_ap(x, ap);
        va_end(ap);

        ಹಿಂತಿರುಗಿ r;
}

ಪೂರ್ಣಾಂಕ ul_strv_extend_strv(ಅಕ್ಷರ ***a, ಅಕ್ಷರ **b)
{
        ಪೂರ್ಣಾಂಕ r;
        ಅಕ್ಷರ **s;

        UL_STRV_FOREACH(s, b) {
                r = ul_strv_extend(a, *s);
                ಆದರೆ (r < 0)
                        ಹಿಂತಿರುಗಿ r;
        }

        ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ ul_strv_extend_strv_concat(ಅಕ್ಷರ ***a, ಅಕ್ಷರ **b, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *suffix)
{
        ಪೂರ್ಣಾಂಕ r;
        ಅಕ್ಷರ **s;

        UL_STRV_FOREACH(s, b) {
                ಅಕ್ಷರ *v;

                v = ul_strconcat(*s, suffix);
                ಆದರೆ (!v)
                        ಹಿಂತಿರುಗಿ -ENOMEM;

                r = ul_strv_ತಳ್ಳು(a, v);
                ಆದರೆ (r < 0) {
                        ಬಿಡುಗಡೆ(v);
                        ಹಿಂತಿರುಗಿ r;
                }
        }

        ಹಿಂತಿರುಗಿ 0;
}


#ವ್ಯಾಖ್ಯಾನಿಸು FOREACH_ಶಬ್ದ(ಶಬ್ದ, ಉದ್ದಳತೆ, s, separator, quoted, state)        \
        ಗಾಗಿ ((state) = (s), (ಶಬ್ದ) = ul_ವಿಭಜಿಸು(&(state), &(ಉದ್ದಳತೆ), (separator), (quoted)); (ಶಬ್ದ); (ಶಬ್ದ) = ul_ವಿಭಜಿಸು(&(state), &(ಉದ್ದಳತೆ), (separator), (quoted)))

#ವ್ಯಾಖ್ಯಾನಿಸು FOREACH_ಶಬ್ದ_SEPARATOR(ಶಬ್ದ, ಉದ್ದಳತೆ, s, separator, state)       \
        FOREACH_ಶಬ್ದ(ಶಬ್ದ, ಉದ್ದಳತೆ, s, separator, false, state)


ಅಕ್ಷರ **ul_strv_ವಿಭಜಿಸು(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *separator)
{
        ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಶಬ್ದ, *state;
        size_t l;
        ಚಿಹ್ನೆರಹಿತ n, i;
        ಅಕ್ಷರ **r;

        ಪ್ರತಿಜ್ಞಾ(s);

        n = 0;
        FOREACH_ಶಬ್ದ_SEPARATOR(ಶಬ್ದ, l, s, separator, state)
                n++;

        r = ಸ್ಮರಣೆ_ಕೊಡು(ಗಾತ್ರ(ಅಕ್ಷರ *) * (n + 1));
        ಆದರೆ (!r)
                ಹಿಂತಿರುಗಿ ಖಾಲಿ;

        i = 0;
        FOREACH_ಶಬ್ದ_SEPARATOR(ಶಬ್ದ, l, s, separator, state) {
                r[i] = strndup(ಶಬ್ದ, l);
                ಆದರೆ (!r[i]) {
                        ul_strv_free(r);
                        ಹಿಂತಿರುಗಿ ಖಾಲಿ;
                }

                i++;
        }

        r[i] = ಖಾಲಿ;
        ಹಿಂತಿರುಗಿ r;
}

ಅಕ್ಷರ *ul_strv_join(ಅಕ್ಷರ **l, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *separator)
{
        ಅಕ್ಷರ *r, *e;
        ಅಕ್ಷರ **s;
        size_t n, k;

        ಆದರೆ (!separator)
                separator = " ";

        k = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(separator);

        n = 0;
        UL_STRV_FOREACH(s, l) {
                ಆದರೆ (n != 0)
                        n += k;
                n += ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(*s);
        }

        r = ಸ್ಮರಣೆ_ಕೊಡು(n + 1);
        ಆದರೆ (!r)
                ಹಿಂತಿರುಗಿ ಖಾಲಿ;

        e = r;
        UL_STRV_FOREACH(s, l) {
                ಆದರೆ (e != r)
                        e = stpcpy(e, separator);

                e = stpcpy(e, *s);
        }

        *e = 0;

        ಹಿಂತಿರುಗಿ r;
}

ಪೂರ್ಣಾಂಕ ul_strv_ತಳ್ಳು(ಅಕ್ಷರ ***l, ಅಕ್ಷರ *ಮೌಲ್ಯ)
{
        ಅಕ್ಷರ **c;
        ಚಿಹ್ನೆರಹಿತ n, m;

        ಆದರೆ (!ಮೌಲ್ಯ)
                ಹಿಂತಿರುಗಿ 0;

        n = ul_strv_ಉದ್ದಳತೆ(*l);

        /* Increase and check for overflow */
        m = n + 2;
        ಆದರೆ (m < n)
                ಹಿಂತಿರುಗಿ -ENOMEM;

        c = reallocarray(*l, m, ಗಾತ್ರ(ಅಕ್ಷರ *));
        ಆದರೆ (!c)
                ಹಿಂತಿರುಗಿ -ENOMEM;

        c[n] = ಮೌಲ್ಯ;
        c[n+1] = ಖಾಲಿ;

        *l = c;
        ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ ul_strv_ತಳ್ಳು_prepend(ಅಕ್ಷರ ***l, ಅಕ್ಷರ *ಮೌಲ್ಯ)
{
        ಅಕ್ಷರ **c;
        ಚಿಹ್ನೆರಹಿತ n, m, i;

        ಆದರೆ (!ಮೌಲ್ಯ)
                ಹಿಂತಿರುಗಿ 0;

        n = ul_strv_ಉದ್ದಳತೆ(*l);

        /* increase and check for overflow */
        m = n + 2;
        ಆದರೆ (m < n)
                ಹಿಂತಿರುಗಿ -ENOMEM;

        c = ಸ್ಮರಣೆ_ಕೊಡು(ಗಾತ್ರ(ಅಕ್ಷರ *) * m);
        ಆದರೆ (!c)
                ಹಿಂತಿರುಗಿ -ENOMEM;

        ಗಾಗಿ (i = 0; i < n; i++)
                c[i+1] = (*l)[i];

        c[0] = ಮೌಲ್ಯ;
        c[n+1] = ಖಾಲಿ;

        ಬಿಡುಗಡೆ(*l);
        *l = c;

        ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ ul_strv_consume(ಅಕ್ಷರ ***l, ಅಕ್ಷರ *ಮೌಲ್ಯ)
{
        ಪೂರ್ಣಾಂಕ r;

        r = ul_strv_ತಳ್ಳು(l, ಮೌಲ್ಯ);
        ಆದರೆ (r < 0)
                ಬಿಡುಗಡೆ(ಮೌಲ್ಯ);

        ಹಿಂತಿರುಗಿ r;
}

ಪೂರ್ಣಾಂಕ ul_strv_consume_prepend(ಅಕ್ಷರ ***l, ಅಕ್ಷರ *ಮೌಲ್ಯ)
{
        ಪೂರ್ಣಾಂಕ r;

        r = ul_strv_ತಳ್ಳು_prepend(l, ಮೌಲ್ಯ);
        ಆದರೆ (r < 0)
                ಬಿಡುಗಡೆ(ಮೌಲ್ಯ);

        ಹಿಂತಿರುಗಿ r;
}

ಪೂರ್ಣಾಂಕ ul_strv_extend(ಅಕ್ಷರ ***l, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮೌಲ್ಯ)
{
        ಅಕ್ಷರ *v;

        ಆದರೆ (!ಮೌಲ್ಯ)
                ಹಿಂತಿರುಗಿ 0;

        v = strdup(ಮೌಲ್ಯ);
        ಆದರೆ (!v)
                ಹಿಂತಿರುಗಿ -ENOMEM;

        ಹಿಂತಿರುಗಿ ul_strv_consume(l, v);
}

ಅಕ್ಷರ **ul_strv_remove(ಅಕ್ಷರ **l, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s)
{
        ಅಕ್ಷರ **f, **t;

        ಆದರೆ (!l)
                ಹಿಂತಿರುಗಿ ಖಾಲಿ;

        ಪ್ರತಿಜ್ಞಾ(s);

        /* Drops every occurrence of s in the string list, edits
         * in-place. */

        ಗಾಗಿ (f = t = l; *f; f++)
                ಆದರೆ (strcmp(*f, s) == 0)
                        ಬಿಡುಗಡೆ(*f);
                ಇಲ್ಲದಿದ್ದರೆ
                        *(t++) = *f;

        *t = ಖಾಲಿ;
        ಹಿಂತಿರುಗಿ l;
}

ಪೂರ್ಣಾಂಕ ul_strv_extendf(ಅಕ್ಷರ ***l, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *format, ...)
{
        va_list ap;
        ಅಕ್ಷರ *x;
        ಪೂರ್ಣಾಂಕ r;

        va_start(ap, format);
        r = vasprintf(&x, format, ap);
        va_end(ap);

        ಆದರೆ (r < 0)
                ಹಿಂತಿರುಗಿ -ENOMEM;

        ಹಿಂತಿರುಗಿ ul_strv_consume(l, x);
}

ಪೂರ್ಣಾಂಕ ul_strv_extendv(ಅಕ್ಷರ ***l, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *format, va_list ap)
{
        ಅಕ್ಷರ *x;
        ಪೂರ್ಣಾಂಕ r;

        r = vasprintf(&x, format, ap);
        ಆದರೆ (r < 0)
                ಹಿಂತಿರುಗಿ -ENOMEM;

        ಹಿಂತಿರುಗಿ ul_strv_consume(l, x);
}

ಅಕ್ಷರ **ul_strv_ಹಿಮ್ಮುಖ(ಅಕ್ಷರ **l)
{
        ಚಿಹ್ನೆರಹಿತ n, i;

        n = ul_strv_ಉದ್ದಳತೆ(l);
        ಆದರೆ (n <= 1)
                ಹಿಂತಿರುಗಿ l;

        ಗಾಗಿ (i = 0; i < n / 2; i++) {
                ಅಕ್ಷರ *t;

                t = l[i];
                l[i] = l[n-1-i];
                l[n-1-i] = t;
        }

        ಹಿಂತಿರುಗಿ l;
}
