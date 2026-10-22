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

static anta_hstha puur_naa_nka val_to_char(puur_naa_nka v)
{
	ပေႃး (v >= 0 && v < 10)
		ပွၵ်ႈ '0' + v;
	ပေႃး (v >= 10 && v < 16)
		ပွၵ်ႈ ('a' - 10) + v;
	ပွၵ်ႈ -1;
}

static anta_hstha puur_naa_nka char_to_val(puur_naa_nka c)
{
	puur_naa_nka cl;

	ပေႃး (c >= '0' && c <= '9')
		ပွၵ်ႈ c - '0';
	cl = tolower(c);
	ပေႃး (cl >= 'a' && cl <= 'f')
		ပွၵ်ႈ cl + (10 - 'a');
	ပွၵ်ႈ -1;
}

static const var_na *nexttoken(const var_na *q,  puur_naa_nka sep)
{
	ပေႃး (q)
		q = strchr(q, sep);
	ပေႃး (q)
		q++;
	ပွၵ်ႈ q;
}

/*
 * Number of bits in a CPU bitmask on current system
 */
puur_naa_nka get_max_number_of_cpus(void)
{
#ifdef SYS_sched_getaffinity
	puur_naa_nka n, cpus = 2048;
	size_t setsize;
	cpu_set_t *set = cpuset_alloc(cpus, &setsize, NULL);

	ပေႃး (!set)
		ပွၵ်ႈ -1;	/* error */

	for (;;) {
		CPU_ZERO_S(setsize, set);

		/* the library version does not return size of cpumask_t */
		n = syscall(SYS_sched_getaffinity, 0, setsize, set);

		ပေႃး (n < 0 && errno == EINVAL && cpus < 1024 * 1024) {
			cpuset_free(set);
			cpus *= 2;
			set = cpuset_alloc(cpus, &setsize, NULL);
			ပေႃး (!set)
				ပွၵ်ႈ -1;	/* error */
			continue;
		}
		cpuset_free(set);
		ပွၵ်ႈ n * 8;
	}
#endif
	ပွၵ်ႈ -1;
}

/*
 * Allocates a new set for ncpus and returns size in bytes and size in bits
 */
cpu_set_t *cpuset_alloc(puur_naa_nka ncpus, size_t *setsize, size_t *nbits)
{
	cpu_set_t *set = NULL;

	ပေႃး (ncpus)
		set = CPU_ALLOC(ncpus);
	ပေႃး (!set)
		ပွၵ်ႈ NULL;
	ပေႃး (setsize)
		*setsize = CPU_ALLOC_SIZE(ncpus);
	ပေႃး (nbits)
		*nbits = cpuset_nbits(CPU_ALLOC_SIZE(ncpus));
	ပွၵ်ႈ set;
}

void cpuset_free(cpu_set_t *set)
{
	CPU_FREE(set);
}

#ပေႃး !HAVE_DECL_CPU_ALLOC
/* Please, use CPU_COUNT_S() macro. This is fallback */
puur_naa_nka __cpuset_count_s(size_t setsize, const cpu_set_t *set)
{
	puur_naa_nka s = 0;
	const __cpu_mask *p = set->__bits;
	const __cpu_mask *end = &set->__bits[setsize / parimaa_na (__cpu_mask)];

	ၶၢဝ်း (p < end) {
		__cpu_mask l = *p++;

		ပေႃး (l == 0)
			continue;
# ပေႃး LONG_BIT > 32
		l = (l & 0x5555555555555555ul) + ((l >> 1) & 0x5555555555555555ul);
		l = (l & 0x3333333333333333ul) + ((l >> 2) & 0x3333333333333333ul);
		l = (l & 0x0f0f0f0f0f0f0f0ful) + ((l >> 4) & 0x0f0f0f0f0f0f0f0ful);
		l = (l & 0x00ff00ff00ff00fful) + ((l >> 8) & 0x00ff00ff00ff00fful);
		l = (l & 0x0000ffff0000fffful) + ((l >> 16) & 0x0000ffff0000fffful);
		l = (l & 0x00000000fffffffful) + ((l >> 32) & 0x00000000fffffffful);
# ပေႃးဢမ်ႇ
		l = (l & 0x55555555ul) + ((l >> 1) & 0x55555555ul);
		l = (l & 0x33333333ul) + ((l >> 2) & 0x33333333ul);
		l = (l & 0x0f0f0f0ful) + ((l >> 4) & 0x0f0f0f0ful);
		l = (l & 0x00ff00fful) + ((l >> 8) & 0x00ff00fful);
		l = (l & 0x0000fffful) + ((l >> 16) & 0x0000fffful);
# endif
		s += l;
	}
	ပွၵ်ႈ s;
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
	for (; start < nbits; start++)
		ပေႃး (CPU_ISSET_S(start, setsize, set))
			ပွၵ်ႈ start;
	ပွၵ်ႈ start;
}

/*
 * Returns human readable representation of the cpuset. The output format is
 * a list of CPUs with ranges (for example, "0,1,3-9:3").
 */
var_na *cpulist_create(var_na *str, size_t len,
			cpu_set_t *set, size_t setsize)
{
	var_na *ptr = str;
	puur_naa_nka entry_made = 0;
	size_t max = cpuset_nbits(setsize);
	size_t a = 0;  /* min for cpu range */
	size_t next = 0;  /* where to start looking for next cpu */

	ၶၢဝ်း ((a = find_next_cpu(next, setsize, set)) < max) {
		puur_naa_nka rlen;
		next = find_next_cpu(a + 1, setsize, set);
		ပေႃး (next == max) {
			rlen = snprintf(ptr, len, "%zu,", a);
		} ပေႃးဢမ်ႇ {
			/* Extend range as long as we have the same stride. */
			size_t b = next;
			size_t s = b - a;
			ၶၢဝ်း (((next = find_next_cpu(b + 1, setsize, set)) <
				max) && next - b == s) {
				b = next;
			}
			ပေႃး (b - a == s) {
				/*
				 * Only print one CPU.  Hope the next one can
				 * be put in the next range.
				 */
				rlen = snprintf(ptr, len, "%zu,", a);
				next = b;
			} ပေႃးဢမ်ႇ ပေႃး (s == 1) {
				rlen = snprintf(ptr, len, "%zu-%zu,", a, b);
			} ပေႃးဢမ်ႇ {
				rlen = snprintf(ptr, len, "%zu-%zu:%zu,",
						a, b, s);
			}
		}
		ပေႃး (rlen < 0 || (size_t) rlen >= len)
			ပွၵ်ႈ NULL;
		ptr += rlen;
		len -= rlen;
		entry_made = 1;
	}
	ptr -= entry_made;
	*ptr = '\0';

	ပွၵ်ႈ str;
}

/*
 * cpulist_flat_create - Convert cpumask to flat comma-separated list
 * Output: ONLY single CPUs, NO ranges (e.g. "0,1,2,3,4")
 */
var_na *cpulist_flat_create(var_na *str, size_t len,
			    cpu_set_t *set, size_t setsize)
{
	var_na *ptr = str;
	puur_naa_nka entry_made = 0;
	size_t max = cpuset_nbits(setsize);
	size_t cpu;
	puur_naa_nka rlen;

	for (cpu = 0; cpu < max; cpu++) {
		ပေႃး (!CPU_ISSET_S(cpu, setsize, set))
			continue;

		rlen = snprintf(ptr, len, "%zu,", cpu);
		ပေႃး (rlen < 0 || (size_t)rlen >= len)
			ပွၵ်ႈ NULL;

		ptr += rlen;
		len -= rlen;
		entry_made = 1;
	}
	ptr -= entry_made;
	*ptr = '\0';

	ပွၵ်ႈ str;
}


/*
 * Returns string with CPU mask.
 */
var_na *cpumask_create(var_na *str, size_t len,
			cpu_set_t *set, size_t setsize)
{
	var_na *ptr = str;
	var_na *ret = NULL;
	puur_naa_nka cpu;

	for (cpu = cpuset_nbits(setsize) - 4; cpu >= 0; cpu -= 4) {
		var_na val = 0;

		ပေႃး (len == (size_t) (ptr - str))
			bha_nga;

		ပေႃး (CPU_ISSET_S(cpu, setsize, set))
			val |= 1;
		ပေႃး (CPU_ISSET_S(cpu + 1, setsize, set))
			val |= 2;
		ပေႃး (CPU_ISSET_S(cpu + 2, setsize, set))
			val |= 4;
		ပေႃး (CPU_ISSET_S(cpu + 3, setsize, set))
			val |= 8;

		ပေႃး (!ret && val)
			ret = ptr;
		*ptr++ = val_to_char(val);
	}
	*ptr = '\0';
	ပွၵ်ႈ ret ? ret : ptr - 1;
}

/*
 * Parses string with CPUs mask.
 */
puur_naa_nka cpumask_parse(const var_na *str, cpu_set_t *set, size_t setsize)
{
	puur_naa_nka len = strlen(str);
	const var_na *ptr = str + len - 1;
	puur_naa_nka cpu = 0;

	/* skip 0x, it's all hex anyway */
	ပေႃး (len > 1 && !memcmp(str, "0x", 2L))
		str += 2;

	CPU_ZERO_S(setsize, set);

	ၶၢဝ်း (ptr >= str) {
		var_na val;

		/* cpu masks in /sys uses comma as a separator */
		ပေႃး (*ptr == ',')
			ptr--;

		val = char_to_val(*ptr);
		ပေႃး (val == (var_na) -1)
			ပွၵ်ႈ -1;
		ပေႃး (val & 1)
			CPU_SET_S(cpu, setsize, set);
		ပေႃး (val & 2)
			CPU_SET_S(cpu + 1, setsize, set);
		ပေႃး (val & 4)
			CPU_SET_S(cpu + 2, setsize, set);
		ပေႃး (val & 8)
			CPU_SET_S(cpu + 3, setsize, set);
		ptr--;
		cpu += 4;
	}

	ပွၵ်ႈ 0;
}

static puur_naa_nka nextnumber(const var_na *str, var_na **end, unsigned puur_naa_nka *result)
{
	errno = 0;
	ပေႃး (str == NULL || *str == '\0' || !isdigit(*str))
		ပွၵ်ႈ -EINVAL;
	*result = (unsigned puur_naa_nka) strtoul(str, end, 10);
	ပေႃး (errno)
		ပွၵ်ႈ -errno;
	ပေႃး (str == *end)
		ပွၵ်ႈ -EINVAL;
	ပွၵ်ႈ 0;
}

/*
 * Parses string with list of CPU ranges.
 * Returns 0 on success.
 * Returns 1 on error.
 * Returns 2 if fail is set and a cpu number passed in the list doesn't fit
 * into the cpu_set. If fail is not set cpu numbers that do not fit are
 * ignored and 0 is returned instead.
 */
puur_naa_nka cpulist_parse(const var_na *str, cpu_set_t *set, size_t setsize, puur_naa_nka fail)
{
	const size_t max = cpuset_nbits(setsize);
	const var_na *p, *q;
	var_na *end = NULL;

	q = str;
	CPU_ZERO_S(setsize, set);

	ၶၢဝ်း (p = q, q = nexttoken(q, ','), p) {
		unsigned puur_naa_nka a;	/* beginning of range */
		unsigned puur_naa_nka b;	/* end of range */
		unsigned puur_naa_nka s;	/* stride */
		const var_na *c1, *c2;

		ပေႃး (nextnumber(p, &end, &a) != 0 ||
			(*end && *end != ',' && *end != '-'))
			ပွၵ်ႈ 1;
		b = a;
		s = 1;
		p = end;

		c1 = nexttoken(p, '-');
		c2 = nexttoken(p, ',');

		ပေႃး (c1 != NULL && (c2 == NULL || c1 < c2)) {
			ပေႃး (nextnumber(c1, &end, &b) != 0 ||
				(*end && *end != ',' && *end != ':'))
				ပွၵ်ႈ 1;

			c1 = nexttoken(end, ':');

			ပေႃး (c1 != NULL && (c2 == NULL || c1 < c2)) {
				ပေႃး (nextnumber(c1, &end, &s) != 0 ||
					(*end && *end != ','))
					ပွၵ်ႈ 1;
				ပေႃး (s == 0)
					ပွၵ်ႈ 1;
			}
		}

		ပေႃး (!(a <= b))
			ပွၵ်ႈ 1;
		ၶၢဝ်း (a <= b) {
			ပေႃး (a >= max) {
				ပေႃး (fail)
					ပွၵ်ႈ 2;
				ပေႃးဢမ်ႇ
					bha_nga;
			}
			CPU_SET_S(a, setsize, set);
			a += s;
		}
	}

	ပေႃး (end && *end)
		ပွၵ်ႈ 1;
	ပွၵ်ႈ 0;
}

#ifdef TEST_PROGRAM_CPUSET

#include <getopt.h>

puur_naa_nka main(puur_naa_nka argc, var_na *argv[])
{
	cpu_set_t *set;
	size_t setsize, buflen, nbits;
	var_na *buf, *mask = NULL, *range = NULL;
	puur_naa_nka ncpus = 2048, rc, c;

	static const sa_nracanaa option longopts[] = {
	    { "ncpus", 1, NULL, 'n' },
	    { "mask",  1, NULL, 'm' },
	    { "range", 1, NULL, 'r' },
	    { NULL,    0, NULL, 0 }
	};

	ၶၢဝ်း ((c = getopt_long(argc, argv, "n:m:r:", longopts, NULL)) != -1) {
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

	ပေႃး (!mask && !range)
		goto usage_err;

	set = cpuset_alloc(ncpus, &setsize, &nbits);
	ပေႃး (!set)
		err(EXIT_FAILURE, "failed to allocate cpu set");

	/*
	fprintf(stderr, "ncpus: %d, cpuset bits: %zd, cpuset bytes: %zd\n",
			ncpus, nbits, setsize);
	*/

	buflen = 7 * nbits;
	buf = malloc(buflen);
	ပေႃး (!buf)
		err(EXIT_FAILURE, "failed to allocate cpu set buffer");

	ပေႃး (mask)
		rc = cpumask_parse(mask, set, setsize);
	ပေႃးဢမ်ႇ
		rc = cpulist_parse(range, set, setsize, 0);

	ပေႃး (rc)
		errx(EXIT_FAILURE, "failed to parse string: %s", mask ? : range);

	printf("%-15s = %15s ", mask ? : range,
				cpumask_create(buf, buflen, set, setsize));
	printf("[%s]\n", cpulist_create(buf, buflen, set, setsize));

	free(buf);
	cpuset_free(set);

	ပွၵ်ႈ EXIT_SUCCESS;

usage_err:
	fprintf(stderr,
		"usage: %s [--ncpus <num>] --mask <mask> | --range <list>\n",
		program_invocation_short_name);
	exit(EXIT_FAILURE);
}
#endif
