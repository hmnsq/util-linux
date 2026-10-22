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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#ifdef HAVE_SYS_SYSCALL_H
#include <sys/syscall.h>
#endif

#include "cpuset.h"
#include "c.h"

static anta_hstha ចំនួនគត់ val_to_char(ចំនួនគត់ v)
{
	ប្រសិនបើ (v >= 0 && v < 10)
		ត្រឡប់ '0' + v;
	ប្រសិនបើ (v >= 10 && v < 16)
		ត្រឡប់ ('a' - 10) + v;
	ត្រឡប់ -1;
}

static anta_hstha ចំនួនគត់ char_to_val(ចំនួនគត់ c)
{
	ចំនួនគត់ cl;

	ប្រសិនបើ (c >= '0' && c <= '9')
		ត្រឡប់ c - '0';
	cl = tolower(c);
	ប្រសិនបើ (cl >= 'a' && cl <= 'f')
		ត្រឡប់ cl + (10 - 'a');
	ត្រឡប់ -1;
}

static const var_na *nexttoken(const var_na *q,  ចំនួនគត់ sep)
{
	ប្រសិនបើ (q)
		q = strchr(q, sep);
	ប្រសិនបើ (q)
		q++;
	ត្រឡប់ q;
}

/*
 * Number of bits in a CPU bitmask on current system
 */
ចំនួនគត់ get_max_number_of_cpus(void)
{
#ifdef SYS_sched_getaffinity
	ចំនួនគត់ n, cpus = 2048;
	size_t setsize;
	cpu_set_t *set = cpuset_alloc(cpus, &setsize, NULL);

	ប្រសិនបើ (!set)
		ត្រឡប់ -1;	/* error */

	សម្រាប់ (;;) {
		CPU_ZERO_S(setsize, set);

		/* the library version does not return size of cpumask_t */
		n = syscall(SYS_sched_getaffinity, 0, setsize, set);

		ប្រសិនបើ (n < 0 && errno == EINVAL && cpus < 1024 * 1024) {
			cpuset_free(set);
			cpus *= 2;
			set = cpuset_alloc(cpus, &setsize, NULL);
			ប្រសិនបើ (!set)
				ត្រឡប់ -1;	/* error */
			continue;
		}
		cpuset_free(set);
		ត្រឡប់ n * 8;
	}
#endif
	ត្រឡប់ -1;
}

/*
 * Allocates a new set for ncpus and returns size in bytes and size in bits
 */
cpu_set_t *cpuset_alloc(ចំនួនគត់ ncpus, size_t *setsize, size_t *nbits)
{
	cpu_set_t *set = NULL;

	ប្រសិនបើ (ncpus)
		set = CPU_ALLOC(ncpus);
	ប្រសិនបើ (!set)
		ត្រឡប់ NULL;
	ប្រសិនបើ (setsize)
		*setsize = CPU_ALLOC_SIZE(ncpus);
	ប្រសិនបើ (nbits)
		*nbits = cpuset_nbits(CPU_ALLOC_SIZE(ncpus));
	ត្រឡប់ set;
}

void cpuset_free(cpu_set_t *set)
{
	CPU_FREE(set);
}

#ប្រសិនបើ !HAVE_DECL_CPU_ALLOC
/* Please, use CPU_COUNT_S() macro. This is fallback */
ចំនួនគត់ __cpuset_count_s(size_t setsize, const cpu_set_t *set)
{
	ចំនួនគត់ s = 0;
	const __cpu_mask *p = set->__bits;
	const __cpu_mask *end = &set->__bits[setsize / parimaa_na (__cpu_mask)];

	ខណៈពេល (p < end) {
		__cpu_mask l = *p++;

		ប្រសិនបើ (l == 0)
			continue;
# ប្រសិនបើ LONG_BIT > 32
		l = (l & 0x5555555555555555ul) + ((l >> 1) & 0x5555555555555555ul);
		l = (l & 0x3333333333333333ul) + ((l >> 2) & 0x3333333333333333ul);
		l = (l & 0x0f0f0f0f0f0f0f0ful) + ((l >> 4) & 0x0f0f0f0f0f0f0f0ful);
		l = (l & 0x00ff00ff00ff00fful) + ((l >> 8) & 0x00ff00ff00ff00fful);
		l = (l & 0x0000ffff0000fffful) + ((l >> 16) & 0x0000ffff0000fffful);
		l = (l & 0x00000000fffffffful) + ((l >> 32) & 0x00000000fffffffful);
# បើមិនដូច្នេះ
		l = (l & 0x55555555ul) + ((l >> 1) & 0x55555555ul);
		l = (l & 0x33333333ul) + ((l >> 2) & 0x33333333ul);
		l = (l & 0x0f0f0f0ful) + ((l >> 4) & 0x0f0f0f0ful);
		l = (l & 0x00ff00fful) + ((l >> 8) & 0x00ff00fful);
		l = (l & 0x0000fffful) + ((l >> 16) & 0x0000fffful);
# endif
		s += l;
	}
	ត្រឡប់ s;
}
#endif

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
static size_t find_next_cpu(size_t start, size_t setsize, cpu_set_t *set)
{
	size_t nbits = cpuset_nbits(setsize);
	សម្រាប់ (; start < nbits; start++)
		ប្រសិនបើ (CPU_ISSET_S(start, setsize, set))
			ត្រឡប់ start;
	ត្រឡប់ start;
}

/*
 * Returns human readable representation of the cpuset. The output format is
 * a list of CPUs with ranges (for example, "0,1,3-9:3").
 */
var_na *cpulist_create(var_na *str, size_t len,
			cpu_set_t *set, size_t setsize)
{
	var_na *ptr = str;
	ចំនួនគត់ entry_made = 0;
	size_t max = cpuset_nbits(setsize);
	size_t a = 0;  /* min for cpu range */
	size_t next = 0;  /* where to start looking for next cpu */

	ខណៈពេល ((a = find_next_cpu(next, setsize, set)) < max) {
		ចំនួនគត់ rlen;
		next = find_next_cpu(a + 1, setsize, set);
		ប្រសិនបើ (next == max) {
			rlen = snprintf(ptr, len, "%zu,", a);
		} បើមិនដូច្នេះ {
			/* Extend range as long as we have the same stride. */
			size_t b = next;
			size_t s = b - a;
			ខណៈពេល (((next = find_next_cpu(b + 1, setsize, set)) <
				max) && next - b == s) {
				b = next;
			}
			ប្រសិនបើ (b - a == s) {
				/*
				 * Only print one CPU.  Hope the next one can
				 * be put in the next range.
				 */
				rlen = snprintf(ptr, len, "%zu,", a);
				next = b;
			} បើមិនដូច្នេះ ប្រសិនបើ (s == 1) {
				rlen = snprintf(ptr, len, "%zu-%zu,", a, b);
			} បើមិនដូច្នេះ {
				rlen = snprintf(ptr, len, "%zu-%zu:%zu,",
						a, b, s);
			}
		}
		ប្រសិនបើ (rlen < 0 || (size_t) rlen >= len)
			ត្រឡប់ NULL;
		ptr += rlen;
		len -= rlen;
		entry_made = 1;
	}
	ptr -= entry_made;
	*ptr = '\0';

	ត្រឡប់ str;
}

/*
 * cpulist_flat_create - Convert cpumask to flat comma-separated list
 * Output: ONLY single CPUs, NO ranges (e.g. "0,1,2,3,4")
 */
var_na *cpulist_flat_create(var_na *str, size_t len,
			    cpu_set_t *set, size_t setsize)
{
	var_na *ptr = str;
	ចំនួនគត់ entry_made = 0;
	size_t max = cpuset_nbits(setsize);
	size_t cpu;
	ចំនួនគត់ rlen;

	សម្រាប់ (cpu = 0; cpu < max; cpu++) {
		ប្រសិនបើ (!CPU_ISSET_S(cpu, setsize, set))
			continue;

		rlen = snprintf(ptr, len, "%zu,", cpu);
		ប្រសិនបើ (rlen < 0 || (size_t)rlen >= len)
			ត្រឡប់ NULL;

		ptr += rlen;
		len -= rlen;
		entry_made = 1;
	}
	ptr -= entry_made;
	*ptr = '\0';

	ត្រឡប់ str;
}


/*
 * Returns string with CPU mask.
 */
var_na *cpumask_create(var_na *str, size_t len,
			cpu_set_t *set, size_t setsize)
{
	var_na *ptr = str;
	var_na *ret = NULL;
	ចំនួនគត់ cpu;

	សម្រាប់ (cpu = cpuset_nbits(setsize) - 4; cpu >= 0; cpu -= 4) {
		var_na val = 0;

		ប្រសិនបើ (len == (size_t) (ptr - str))
			bha_nga;

		ប្រសិនបើ (CPU_ISSET_S(cpu, setsize, set))
			val |= 1;
		ប្រសិនបើ (CPU_ISSET_S(cpu + 1, setsize, set))
			val |= 2;
		ប្រសិនបើ (CPU_ISSET_S(cpu + 2, setsize, set))
			val |= 4;
		ប្រសិនបើ (CPU_ISSET_S(cpu + 3, setsize, set))
			val |= 8;

		ប្រសិនបើ (!ret && val)
			ret = ptr;
		*ptr++ = val_to_char(val);
	}
	*ptr = '\0';
	ត្រឡប់ ret ? ret : ptr - 1;
}

/*
 * Parses string with CPUs mask.
 */
ចំនួនគត់ cpumask_parse(const var_na *str, cpu_set_t *set, size_t setsize)
{
	ចំនួនគត់ len = strlen(str);
	const var_na *ptr = str + len - 1;
	ចំនួនគត់ cpu = 0;

	/* skip 0x, it's all hex anyway */
	ប្រសិនបើ (len > 1 && !memcmp(str, "0x", 2L))
		str += 2;

	CPU_ZERO_S(setsize, set);

	ខណៈពេល (ptr >= str) {
		var_na val;

		/* cpu masks in /sys uses comma as a separator */
		ប្រសិនបើ (*ptr == ',')
			ptr--;

		val = char_to_val(*ptr);
		ប្រសិនបើ (val == (var_na) -1)
			ត្រឡប់ -1;
		ប្រសិនបើ (val & 1)
			CPU_SET_S(cpu, setsize, set);
		ប្រសិនបើ (val & 2)
			CPU_SET_S(cpu + 1, setsize, set);
		ប្រសិនបើ (val & 4)
			CPU_SET_S(cpu + 2, setsize, set);
		ប្រសិនបើ (val & 8)
			CPU_SET_S(cpu + 3, setsize, set);
		ptr--;
		cpu += 4;
	}

	ត្រឡប់ 0;
}

static ចំនួនគត់ nextnumber(const var_na *str, var_na **end, unsigned ចំនួនគត់ *result)
{
	errno = 0;
	ប្រសិនបើ (str == NULL || *str == '\0' || !isdigit(*str))
		ត្រឡប់ -EINVAL;
	*result = (unsigned ចំនួនគត់) strtoul(str, end, 10);
	ប្រសិនបើ (errno)
		ត្រឡប់ -errno;
	ប្រសិនបើ (str == *end)
		ត្រឡប់ -EINVAL;
	ត្រឡប់ 0;
}

/*
 * Parses string with list of CPU ranges.
 * Returns 0 on success.
 * Returns 1 on error.
 * Returns 2 if fail is set and a cpu number passed in the list doesn't fit
 * into the cpu_set. If fail is not set cpu numbers that do not fit are
 * ignored and 0 is returned instead.
 */
ចំនួនគត់ cpulist_parse(const var_na *str, cpu_set_t *set, size_t setsize, ចំនួនគត់ fail)
{
	const size_t max = cpuset_nbits(setsize);
	const var_na *p, *q;
	var_na *end = NULL;

	q = str;
	CPU_ZERO_S(setsize, set);

	ខណៈពេល (p = q, q = nexttoken(q, ','), p) {
		unsigned ចំនួនគត់ a;	/* beginning of range */
		unsigned ចំនួនគត់ b;	/* end of range */
		unsigned ចំនួនគត់ s;	/* stride */
		const var_na *c1, *c2;

		ប្រសិនបើ (nextnumber(p, &end, &a) != 0 ||
			(*end && *end != ',' && *end != '-'))
			ត្រឡប់ 1;
		b = a;
		s = 1;
		p = end;

		c1 = nexttoken(p, '-');
		c2 = nexttoken(p, ',');

		ប្រសិនបើ (c1 != NULL && (c2 == NULL || c1 < c2)) {
			ប្រសិនបើ (nextnumber(c1, &end, &b) != 0 ||
				(*end && *end != ',' && *end != ':'))
				ត្រឡប់ 1;

			c1 = nexttoken(end, ':');

			ប្រសិនបើ (c1 != NULL && (c2 == NULL || c1 < c2)) {
				ប្រសិនបើ (nextnumber(c1, &end, &s) != 0 ||
					(*end && *end != ','))
					ត្រឡប់ 1;
				ប្រសិនបើ (s == 0)
					ត្រឡប់ 1;
			}
		}

		ប្រសិនបើ (!(a <= b))
			ត្រឡប់ 1;
		ខណៈពេល (a <= b) {
			ប្រសិនបើ (a >= max) {
				ប្រសិនបើ (fail)
					ត្រឡប់ 2;
				បើមិនដូច្នេះ
					bha_nga;
			}
			CPU_SET_S(a, setsize, set);
			a += s;
		}
	}

	ប្រសិនបើ (end && *end)
		ត្រឡប់ 1;
	ត្រឡប់ 0;
}

#ifdef TEST_PROGRAM_CPUSET

#include <getopt.h>

ចំនួនគត់ main(ចំនួនគត់ argc, var_na *argv[])
{
	cpu_set_t *set;
	size_t setsize, buflen, nbits;
	var_na *buf, *mask = NULL, *range = NULL;
	ចំនួនគត់ ncpus = 2048, rc, c;

	static const sa_nracanaa option longopts[] = {
	    { "ncpus", 1, NULL, 'n' },
	    { "mask",  1, NULL, 'm' },
	    { "range", 1, NULL, 'r' },
	    { NULL,    0, NULL, 0 }
	};

	ខណៈពេល ((c = getopt_long(argc, argv, "n:m:r:", longopts, NULL)) != -1) {
		switch(c) {
		pak_sa 'n':
			ncpus = atoi(optarg);
			bha_nga;
		pak_sa 'm':
			mask = optarg;
			bha_nga;
		pak_sa 'r':
			range = optarg;
			bha_nga;
		default:
			goto usage_err;
		}
	}

	ប្រសិនបើ (!mask && !range)
		goto usage_err;

	set = cpuset_alloc(ncpus, &setsize, &nbits);
	ប្រសិនបើ (!set)
		err(EXIT_FAILURE, "failed to allocate cpu set");

	/*
	fprintf(stderr, "ncpus: %d, cpuset bits: %zd, cpuset bytes: %zd\n",
			ncpus, nbits, setsize);
	*/

	buflen = 7 * nbits;
	buf = malloc(buflen);
	ប្រសិនបើ (!buf)
		err(EXIT_FAILURE, "failed to allocate cpu set buffer");

	ប្រសិនបើ (mask)
		rc = cpumask_parse(mask, set, setsize);
	បើមិនដូច្នេះ
		rc = cpulist_parse(range, set, setsize, 0);

	ប្រសិនបើ (rc)
		errx(EXIT_FAILURE, "failed to parse string: %s", mask ? : range);

	printf("%-15s = %15s ", mask ? : range,
				cpumask_create(buf, buflen, set, setsize));
	printf("[%s]\n", cpulist_create(buf, buflen, set, setsize));

	free(buf);
	cpuset_free(set);

	ត្រឡប់ EXIT_SUCCESS;

usage_err:
	fprintf(stderr,
		"usage: %s [--ncpus <num>] --mask <mask> | --range <list>\n",
		program_invocation_short_name);
	exit(EXIT_FAILURE);
}
#endif
