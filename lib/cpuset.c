/*
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * Terminology:
 *
 *	cpuset	- (libc) cpu_set_t data structure represents set of CPUs
 *	cpumask	- string with hex mask (e.g. "0x00000001")
 *	cpulist - string with CPU ranges (e.g. "0-3,5,7,8")
 *
 * Based on code from taskset.c and Linux kernel.
 *
 * This file may be redistributed under the terms of the
 * GNU Lesser General Public License.
 *
 * Copyright (C) 2010 Karel Zak <kzak@redhat.com>
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sched.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <ctype.h>
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HAVE_SYS_SYSCALL_H
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/syscall.h>
#ਜੇ_ਅੰਤ

#ਅਨ੍ਤਰ੍ਭਾਵ "cpuset.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "c.h"

ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ val_ਮੰਜ਼ਿਲ_char(ਪੂਰਨ v)
{
	ਜੇ (v >= 0 && v < 10)
		ਵਾਪਸ '0' + v;
	ਜੇ (v >= 10 && v < 16)
		ਵਾਪਸ ('a' - 10) + v;
	ਵਾਪਸ -1;
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ char_ਮੰਜ਼ਿਲ_val(ਪੂਰਨ c)
{
	ਪੂਰਨ cl;

	ਜੇ (c >= '0' && c <= '9')
		ਵਾਪਸ c - '0';
	cl = tolower(c);
	ਜੇ (cl >= 'a' && cl <= 'f')
		ਵਾਪਸ cl + (10 - 'a');
	ਵਾਪਸ -1;
}

ਸਥਿਰ ਅਚਲ ਅੱਖਰ *nexttoken(ਅਚਲ ਅੱਖਰ *q,  ਪੂਰਨ sep)
{
	ਜੇ (q)
		q = strchr(q, sep);
	ਜੇ (q)
		q++;
	ਵਾਪਸ q;
}

/*
 * Number of bits in a CPU bitmask on current system
 */
ਪੂਰਨ get_max_ਸੰਖਿਆ_ਦਾ_cpus(ਖਾਲੀ)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SYS_sched_getaffinity
	ਪੂਰਨ n, cpus = 2048;
	size_t setsize;
	cpu_set_t *ਸੈੱਟ = cpuset_alloc(cpus, &setsize, ਨਿਰਾ);

	ਜੇ (!ਸੈੱਟ)
		ਵਾਪਸ -1;	/* error */

	ਲਈ (;;) {
		CPU_ZERO_S(setsize, ਸੈੱਟ);

		/* the library version does not return size of cpumask_t */
		n = syscall(SYS_sched_getaffinity, 0, setsize, ਸੈੱਟ);

		ਜੇ (n < 0 && errno == EINVAL && cpus < 1024 * 1024) {
			cpuset_free(ਸੈੱਟ);
			cpus *= 2;
			ਸੈੱਟ = cpuset_alloc(cpus, &setsize, ਨਿਰਾ);
			ਜੇ (!ਸੈੱਟ)
				ਵਾਪਸ -1;	/* error */
			ਜਾਰੀ;
		}
		cpuset_free(ਸੈੱਟ);
		ਵਾਪਸ n * 8;
	}
#ਜੇ_ਅੰਤ
	ਵਾਪਸ -1;
}

/*
 * Allocates a new set for ncpus and returns size in bytes and size in bits
 */
cpu_set_t *cpuset_alloc(ਪੂਰਨ ncpus, size_t *setsize, size_t *nbits)
{
	cpu_set_t *ਸੈੱਟ = ਨਿਰਾ;

	ਜੇ (ncpus)
		ਸੈੱਟ = CPU_ALLOC(ncpus);
	ਜੇ (!ਸੈੱਟ)
		ਵਾਪਸ ਨਿਰਾ;
	ਜੇ (setsize)
		*setsize = CPU_ALLOC_ਮਾਪ(ncpus);
	ਜੇ (nbits)
		*nbits = cpuset_nbits(CPU_ALLOC_ਮਾਪ(ncpus));
	ਵਾਪਸ ਸੈੱਟ;
}

ਖਾਲੀ cpuset_free(cpu_set_t *ਸੈੱਟ)
{
	CPU_FREE(ਸੈੱਟ);
}

#ਜੇ !HAVE_DECL_CPU_ALLOC
/* Please, use CPU_COUNT_S() macro. This is fallback */
ਪੂਰਨ __cpuset_count_s(size_t setsize, ਅਚਲ cpu_set_t *ਸੈੱਟ)
{
	ਪੂਰਨ s = 0;
	ਅਚਲ __cpu_mask *p = ਸੈੱਟ->__bits;
	ਅਚਲ __cpu_mask *ਸਿਰਾ = &ਸੈੱਟ->__bits[setsize / ਆਕਾਰ (__cpu_mask)];

	ਜਦੋਂ (p < ਸਿਰਾ) {
		__cpu_mask l = *p++;

		ਜੇ (l == 0)
			ਜਾਰੀ;
# ਜੇ LONG_ਦੋਆਧਾਰੀ_ਅੰਕ > 32
		l = (l & 0x5555555555555555ul) + ((l >> 1) & 0x5555555555555555ul);
		l = (l & 0x3333333333333333ul) + ((l >> 2) & 0x3333333333333333ul);
		l = (l & 0x0f0f0f0f0f0f0f0ful) + ((l >> 4) & 0x0f0f0f0f0f0f0f0ful);
		l = (l & 0x00ff00ff00ff00fful) + ((l >> 8) & 0x00ff00ff00ff00fful);
		l = (l & 0x0000ffff0000fffful) + ((l >> 16) & 0x0000ffff0000fffful);
		l = (l & 0x00000000fffffffful) + ((l >> 32) & 0x00000000fffffffful);
# ਨਹੀਂਤਾਂ
		l = (l & 0x55555555ul) + ((l >> 1) & 0x55555555ul);
		l = (l & 0x33333333ul) + ((l >> 2) & 0x33333333ul);
		l = (l & 0x0f0f0f0ful) + ((l >> 4) & 0x0f0f0f0ful);
		l = (l & 0x00ff00fful) + ((l >> 8) & 0x00ff00fful);
		l = (l & 0x0000fffful) + ((l >> 16) & 0x0000fffful);
# endif
		s += l;
	}
	ਵਾਪਸ s;
}
#ਜੇ_ਅੰਤ

/*
 * Finds the first CPU present after the specified index.
 *
 * start: starting index, inclusive.
 * setsize: size of the set in *bytes*.
 * set: CPU set to search.
 *
 * Return: the index of the first CPU present in `set`, starting at `start`.
 * If no such CPU exists, returns the size of the set in *bits*.
 */
ਸਥਿਰ size_t ਲੱਭੋ_ਅਗਲਾ_cpu(size_t ਸ਼ੁਰੂ, size_t setsize, cpu_set_t *ਸੈੱਟ)
{
	size_t nbits = cpuset_nbits(setsize);
	ਲਈ (; ਸ਼ੁਰੂ < nbits; ਸ਼ੁਰੂ++)
		ਜੇ (CPU_ISSET_S(ਸ਼ੁਰੂ, setsize, ਸੈੱਟ))
			ਵਾਪਸ ਸ਼ੁਰੂ;
	ਵਾਪਸ ਸ਼ੁਰੂ;
}

/*
 * Returns human readable representation of the cpuset. The output format is
 * a list of CPUs with ranges (for example, "0,1,3-9:3").
 */
ਅੱਖਰ *cpulist_create(ਅੱਖਰ *str, size_t len,
			cpu_set_t *ਸੈੱਟ, size_t setsize)
{
	ਅੱਖਰ *ptr = str;
	ਪੂਰਨ ਇੰਦਰਾਜ_made = 0;
	size_t max = cpuset_nbits(setsize);
	size_t a = 0;  /* min for cpu range */
	size_t ਅਗਲਾ = 0;  /* where to start looking for next cpu */

	ਜਦੋਂ ((a = ਲੱਭੋ_ਅਗਲਾ_cpu(ਅਗਲਾ, setsize, ਸੈੱਟ)) < max) {
		ਪੂਰਨ rlen;
		ਅਗਲਾ = ਲੱਭੋ_ਅਗਲਾ_cpu(a + 1, setsize, ਸੈੱਟ);
		ਜੇ (ਅਗਲਾ == max) {
			rlen = snprintf(ptr, len, "%zu,", a);
		} ਨਹੀਂਤਾਂ {
			/* Extend range as long as we have the same stride. */
			size_t b = ਅਗਲਾ;
			size_t s = b - a;
			ਜਦੋਂ (((ਅਗਲਾ = ਲੱਭੋ_ਅਗਲਾ_cpu(b + 1, setsize, ਸੈੱਟ)) <
				max) && ਅਗਲਾ - b == s) {
				b = ਅਗਲਾ;
			}
			ਜੇ (b - a == s) {
				/*
				 * Only print one CPU.  Hope the next one can
				 * be put in the next range.
				 */
				rlen = snprintf(ptr, len, "%zu,", a);
				ਅਗਲਾ = b;
			} ਨਹੀਂਤਾਂ ਜੇ (s == 1) {
				rlen = snprintf(ptr, len, "%zu-%zu,", a, b);
			} ਨਹੀਂਤਾਂ {
				rlen = snprintf(ptr, len, "%zu-%zu:%zu,",
						a, b, s);
			}
		}
		ਜੇ (rlen < 0 || (size_t) rlen >= len)
			ਵਾਪਸ ਨਿਰਾ;
		ptr += rlen;
		len -= rlen;
		ਇੰਦਰਾਜ_made = 1;
	}
	ptr -= ਇੰਦਰਾਜ_made;
	*ptr = '\0';

	ਵਾਪਸ str;
}

/*
 * cpulist_flat_create - Convert cpumask to flat comma-separated list
 * Output: ONLY single CPUs, NO ranges (e.g. "0,1,2,3,4")
 */
ਅੱਖਰ *cpulist_flat_create(ਅੱਖਰ *str, size_t len,
			    cpu_set_t *ਸੈੱਟ, size_t setsize)
{
	ਅੱਖਰ *ptr = str;
	ਪੂਰਨ ਇੰਦਰਾਜ_made = 0;
	size_t max = cpuset_nbits(setsize);
	size_t cpu;
	ਪੂਰਨ rlen;

	ਲਈ (cpu = 0; cpu < max; cpu++) {
		ਜੇ (!CPU_ISSET_S(cpu, setsize, ਸੈੱਟ))
			ਜਾਰੀ;

		rlen = snprintf(ptr, len, "%zu,", cpu);
		ਜੇ (rlen < 0 || (size_t)rlen >= len)
			ਵਾਪਸ ਨਿਰਾ;

		ptr += rlen;
		len -= rlen;
		ਇੰਦਰਾਜ_made = 1;
	}
	ptr -= ਇੰਦਰਾਜ_made;
	*ptr = '\0';

	ਵਾਪਸ str;
}


/*
 * Returns string with CPU mask.
 */
ਅੱਖਰ *cpumask_create(ਅੱਖਰ *str, size_t len,
			cpu_set_t *ਸੈੱਟ, size_t setsize)
{
	ਅੱਖਰ *ptr = str;
	ਅੱਖਰ *ret = ਨਿਰਾ;
	ਪੂਰਨ cpu;

	ਲਈ (cpu = cpuset_nbits(setsize) - 4; cpu >= 0; cpu -= 4) {
		ਅੱਖਰ val = 0;

		ਜੇ (len == (size_t) (ptr - str))
			ਰੋਕੋ;

		ਜੇ (CPU_ISSET_S(cpu, setsize, ਸੈੱਟ))
			val |= 1;
		ਜੇ (CPU_ISSET_S(cpu + 1, setsize, ਸੈੱਟ))
			val |= 2;
		ਜੇ (CPU_ISSET_S(cpu + 2, setsize, ਸੈੱਟ))
			val |= 4;
		ਜੇ (CPU_ISSET_S(cpu + 3, setsize, ਸੈੱਟ))
			val |= 8;

		ਜੇ (!ret && val)
			ret = ptr;
		*ptr++ = val_ਮੰਜ਼ਿਲ_char(val);
	}
	*ptr = '\0';
	ਵਾਪਸ ret ? ret : ptr - 1;
}

/*
 * Parses string with CPUs mask.
 */
ਪੂਰਨ cpumask_parse(ਅਚਲ ਅੱਖਰ *str, cpu_set_t *ਸੈੱਟ, size_t setsize)
{
	ਪੂਰਨ len = ਲੜੀ_ਲੰਬਾਈ(str);
	ਅਚਲ ਅੱਖਰ *ptr = str + len - 1;
	ਪੂਰਨ cpu = 0;

	/* skip 0x, it's all hex anyway */
	ਜੇ (len > 1 && !memcmp(str, "0x", 2L))
		str += 2;

	CPU_ZERO_S(setsize, ਸੈੱਟ);

	ਜਦੋਂ (ptr >= str) {
		ਅੱਖਰ val;

		/* cpu masks in /sys uses comma as a separator */
		ਜੇ (*ptr == ',')
			ptr--;

		val = char_ਮੰਜ਼ਿਲ_val(*ptr);
		ਜੇ (val == (ਅੱਖਰ) -1)
			ਵਾਪਸ -1;
		ਜੇ (val & 1)
			CPU_ਸੈੱਟ_S(cpu, setsize, ਸੈੱਟ);
		ਜੇ (val & 2)
			CPU_ਸੈੱਟ_S(cpu + 1, setsize, ਸੈੱਟ);
		ਜੇ (val & 4)
			CPU_ਸੈੱਟ_S(cpu + 2, setsize, ਸੈੱਟ);
		ਜੇ (val & 8)
			CPU_ਸੈੱਟ_S(cpu + 3, setsize, ਸੈੱਟ);
		ptr--;
		cpu += 4;
	}

	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ nextnumber(ਅਚਲ ਅੱਖਰ *str, ਅੱਖਰ **ਸਿਰਾ, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *ਨਤੀਜਾ)
{
	errno = 0;
	ਜੇ (str == ਨਿਰਾ || *str == '\0' || !ਅੰਕ_ਹੈ(*str))
		ਵਾਪਸ -EINVAL;
	*ਨਤੀਜਾ = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ) strtoul(str, ਸਿਰਾ, 10);
	ਜੇ (errno)
		ਵਾਪਸ -errno;
	ਜੇ (str == *ਸਿਰਾ)
		ਵਾਪਸ -EINVAL;
	ਵਾਪਸ 0;
}

/*
 * Parses string with list of CPU ranges.
 * Returns 0 on success.
 * Returns 1 on error.
 * Returns 2 if fail is set and a cpu number passed in the list doesn't fit
 * into the cpu_set. If fail is not set cpu numbers that do not fit are
 * ignored and 0 is returned instead.
 */
ਪੂਰਨ cpulist_parse(ਅਚਲ ਅੱਖਰ *str, cpu_set_t *ਸੈੱਟ, size_t setsize, ਪੂਰਨ fail)
{
	ਅਚਲ size_t max = cpuset_nbits(setsize);
	ਅਚਲ ਅੱਖਰ *p, *q;
	ਅੱਖਰ *ਸਿਰਾ = ਨਿਰਾ;

	q = str;
	CPU_ZERO_S(setsize, ਸੈੱਟ);

	ਜਦੋਂ (p = q, q = nexttoken(q, ','), p) {
		ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ a;	/* beginning of range */
		ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ b;	/* end of range */
		ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ s;	/* stride */
		ਅਚਲ ਅੱਖਰ *c1, *c2;

		ਜੇ (nextnumber(p, &ਸਿਰਾ, &a) != 0 ||
			(*ਸਿਰਾ && *ਸਿਰਾ != ',' && *ਸਿਰਾ != '-'))
			ਵਾਪਸ 1;
		b = a;
		s = 1;
		p = ਸਿਰਾ;

		c1 = nexttoken(p, '-');
		c2 = nexttoken(p, ',');

		ਜੇ (c1 != ਨਿਰਾ && (c2 == ਨਿਰਾ || c1 < c2)) {
			ਜੇ (nextnumber(c1, &ਸਿਰਾ, &b) != 0 ||
				(*ਸਿਰਾ && *ਸਿਰਾ != ',' && *ਸਿਰਾ != ':'))
				ਵਾਪਸ 1;

			c1 = nexttoken(ਸਿਰਾ, ':');

			ਜੇ (c1 != ਨਿਰਾ && (c2 == ਨਿਰਾ || c1 < c2)) {
				ਜੇ (nextnumber(c1, &ਸਿਰਾ, &s) != 0 ||
					(*ਸਿਰਾ && *ਸਿਰਾ != ','))
					ਵਾਪਸ 1;
				ਜੇ (s == 0)
					ਵਾਪਸ 1;
			}
		}

		ਜੇ (!(a <= b))
			ਵਾਪਸ 1;
		ਜਦੋਂ (a <= b) {
			ਜੇ (a >= max) {
				ਜੇ (fail)
					ਵਾਪਸ 2;
				ਨਹੀਂਤਾਂ
					ਰੋਕੋ;
			}
			CPU_ਸੈੱਟ_S(a, setsize, ਸੈੱਟ);
			a += s;
		}
	}

	ਜੇ (ਸਿਰਾ && *ਸਿਰਾ)
		ਵਾਪਸ 1;
	ਵਾਪਸ 0;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ TEST_ਪ੍ਰੋਗਰਾਮ_CPUSET

#ਅਨ੍ਤਰ੍ਭਾਵ <getopt.h>

ਪੂਰਨ ਮੁੱਖ(ਪੂਰਨ argc, ਅੱਖਰ *argv[])
{
	cpu_set_t *ਸੈੱਟ;
	size_t setsize, buflen, nbits;
	ਅੱਖਰ *buf, *ਨਕਾਬ = ਨਿਰਾ, *range = ਨਿਰਾ;
	ਪੂਰਨ ncpus = 2048, rc, c;

	ਸਥਿਰ ਅਚਲ ਬਣਤਰ option longopts[] = {
	    { "ncpus", 1, ਨਿਰਾ, 'n' },
	    { "mask",  1, ਨਿਰਾ, 'm' },
	    { "range", 1, ਨਿਰਾ, 'r' },
	    { ਨਿਰਾ,    0, ਨਿਰਾ, 0 }
	};

	ਜਦੋਂ ((c = getopt_long(argc, argv, "n:m:r:", longopts, ਨਿਰਾ)) != -1) {
		ਚੋਣ(c) {
		ਹਾਲਤ 'n':
			ncpus = atoi(optarg);
			ਰੋਕੋ;
		ਹਾਲਤ 'm':
			ਨਕਾਬ = optarg;
			ਰੋਕੋ;
		ਹਾਲਤ 'r':
			range = optarg;
			ਰੋਕੋ;
		ਮੂਲ:
			ਜਾਓ usage_err;
		}
	}

	ਜੇ (!ਨਕਾਬ && !range)
		ਜਾਓ usage_err;

	ਸੈੱਟ = cpuset_alloc(ncpus, &setsize, &nbits);
	ਜੇ (!ਸੈੱਟ)
		err(EXIT_FAILURE, "failed to allocate cpu set");

	/*
	fprintf(stderr, "ncpus: %d, cpuset bits: %zd, cpuset bytes: %zd\n",
			ncpus, nbits, setsize);
	*/

	buflen = 7 * nbits;
	buf = ਸਮ੍ਰਿਤੀ_ਦਿਓ(buflen);
	ਜੇ (!buf)
		err(EXIT_FAILURE, "failed to allocate cpu set buffer");

	ਜੇ (ਨਕਾਬ)
		rc = cpumask_parse(ਨਕਾਬ, ਸੈੱਟ, setsize);
	ਨਹੀਂਤਾਂ
		rc = cpulist_parse(range, ਸੈੱਟ, setsize, 0);

	ਜੇ (rc)
		errx(EXIT_FAILURE, "failed to parse string: %s", ਨਕਾਬ ? : range);

	ਲਿਖੋ("%-15s = %15s ", ਨਕਾਬ ? : range,
				cpumask_create(buf, buflen, ਸੈੱਟ, setsize));
	ਲਿਖੋ("[%s]\n", cpulist_create(buf, buflen, ਸੈੱਟ, setsize));

	ਮੁਕਤ(buf);
	cpuset_free(ਸੈੱਟ);

	ਵਾਪਸ EXIT_SUCCESS;

usage_err:
	fprintf(stderr,
		"usage: %s [--ncpus <num>] --mask <mask> | --range <list>\n",
		ਪ੍ਰੋਗਰਾਮ_invocation_short_name);
	ਬਾਹਰ_ਨਿਕਲੋ(EXIT_FAILURE);
}
#ਜੇ_ਅੰਤ
