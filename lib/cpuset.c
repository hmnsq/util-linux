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

static anta_hstha ကိန်းပြည့် val_to_char(ကိန်းပြည့် v)
{
	အကယ်၍ (v >= 0 && v < 10)
		ပြန်ပေး '0' + v;
	အကယ်၍ (v >= 10 && v < 16)
		ပြန်ပေး ('a' - 10) + v;
	ပြန်ပေး -1;
}

static anta_hstha ကိန်းပြည့် char_to_val(ကိန်းပြည့် c)
{
	ကိန်းပြည့် cl;

	အကယ်၍ (c >= '0' && c <= '9')
		ပြန်ပေး c - '0';
	cl = tolower(c);
	အကယ်၍ (cl >= 'a' && cl <= 'f')
		ပြန်ပေး cl + (10 - 'a');
	ပြန်ပေး -1;
}

static const var_na *nexttoken(const var_na *q,  ကိန်းပြည့် sep)
{
	အကယ်၍ (q)
		q = strchr(q, sep);
	အကယ်၍ (q)
		q++;
	ပြန်ပေး q;
}

/*
 * Number of bits in a CPU bitmask on current system
 */
ကိန်းပြည့် get_max_number_of_cpus(void)
{
#ifdef SYS_sched_getaffinity
	ကိန်းပြည့် n, cpus = 2048;
	size_t setsize;
	cpu_set_t *set = cpuset_alloc(cpus, &setsize, NULL);

	အကယ်၍ (!set)
		ပြန်ပေး -1;	/* error */

	အတွက် (;;) {
		CPU_ZERO_S(setsize, set);

		/* the library version does not return size of cpumask_t */
		n = syscall(SYS_sched_getaffinity, 0, setsize, set);

		အကယ်၍ (n < 0 && errno == EINVAL && cpus < 1024 * 1024) {
			cpuset_free(set);
			cpus *= 2;
			set = cpuset_alloc(cpus, &setsize, NULL);
			အကယ်၍ (!set)
				ပြန်ပေး -1;	/* error */
			continue;
		}
		cpuset_free(set);
		ပြန်ပေး n * 8;
	}
#endif
	ပြန်ပေး -1;
}

/*
 * Allocates a new set for ncpus and returns size in bytes and size in bits
 */
cpu_set_t *cpuset_alloc(ကိန်းပြည့် ncpus, size_t *setsize, size_t *nbits)
{
	cpu_set_t *set = NULL;

	အကယ်၍ (ncpus)
		set = CPU_ALLOC(ncpus);
	အကယ်၍ (!set)
		ပြန်ပေး NULL;
	အကယ်၍ (setsize)
		*setsize = CPU_ALLOC_SIZE(ncpus);
	အကယ်၍ (nbits)
		*nbits = cpuset_nbits(CPU_ALLOC_SIZE(ncpus));
	ပြန်ပေး set;
}

void cpuset_free(cpu_set_t *set)
{
	CPU_FREE(set);
}

#အကယ်၍ !HAVE_DECL_CPU_ALLOC
/* Please, use CPU_COUNT_S() macro. This is fallback */
ကိန်းပြည့် __cpuset_count_s(size_t setsize, const cpu_set_t *set)
{
	ကိန်းပြည့် s = 0;
	const __cpu_mask *p = set->__bits;
	const __cpu_mask *end = &set->__bits[setsize / parimaa_na (__cpu_mask)];

	နေစဉ် (p < end) {
		__cpu_mask l = *p++;

		အကယ်၍ (l == 0)
			continue;
# အကယ်၍ LONG_BIT > 32
		l = (l & 0x5555555555555555ul) + ((l >> 1) & 0x5555555555555555ul);
		l = (l & 0x3333333333333333ul) + ((l >> 2) & 0x3333333333333333ul);
		l = (l & 0x0f0f0f0f0f0f0f0ful) + ((l >> 4) & 0x0f0f0f0f0f0f0f0ful);
		l = (l & 0x00ff00ff00ff00fful) + ((l >> 8) & 0x00ff00ff00ff00fful);
		l = (l & 0x0000ffff0000fffful) + ((l >> 16) & 0x0000ffff0000fffful);
		l = (l & 0x00000000fffffffful) + ((l >> 32) & 0x00000000fffffffful);
# မဟုတ်ပါက
		l = (l & 0x55555555ul) + ((l >> 1) & 0x55555555ul);
		l = (l & 0x33333333ul) + ((l >> 2) & 0x33333333ul);
		l = (l & 0x0f0f0f0ful) + ((l >> 4) & 0x0f0f0f0ful);
		l = (l & 0x00ff00fful) + ((l >> 8) & 0x00ff00fful);
		l = (l & 0x0000fffful) + ((l >> 16) & 0x0000fffful);
# endif
		s += l;
	}
	ပြန်ပေး s;
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
	အတွက် (; start < nbits; start++)
		အကယ်၍ (CPU_ISSET_S(start, setsize, set))
			ပြန်ပေး start;
	ပြန်ပေး start;
}

/*
 * Returns human readable representation of the cpuset. The output format is
 * a list of CPUs with ranges (for example, "0,1,3-9:3").
 */
var_na *cpulist_create(var_na *str, size_t len,
			cpu_set_t *set, size_t setsize)
{
	var_na *ptr = str;
	ကိန်းပြည့် entry_made = 0;
	size_t max = cpuset_nbits(setsize);
	size_t a = 0;  /* min for cpu range */
	size_t next = 0;  /* where to start looking for next cpu */

	နေစဉ် ((a = find_next_cpu(next, setsize, set)) < max) {
		ကိန်းပြည့် rlen;
		next = find_next_cpu(a + 1, setsize, set);
		အကယ်၍ (next == max) {
			rlen = snprintf(ptr, len, "%zu,", a);
		} မဟုတ်ပါက {
			/* Extend range as long as we have the same stride. */
			size_t b = next;
			size_t s = b - a;
			နေစဉ် (((next = find_next_cpu(b + 1, setsize, set)) <
				max) && next - b == s) {
				b = next;
			}
			အကယ်၍ (b - a == s) {
				/*
				 * Only print one CPU.  Hope the next one can
				 * be put in the next range.
				 */
				rlen = snprintf(ptr, len, "%zu,", a);
				next = b;
			} မဟုတ်ပါက အကယ်၍ (s == 1) {
				rlen = snprintf(ptr, len, "%zu-%zu,", a, b);
			} မဟုတ်ပါက {
				rlen = snprintf(ptr, len, "%zu-%zu:%zu,",
						a, b, s);
			}
		}
		အကယ်၍ (rlen < 0 || (size_t) rlen >= len)
			ပြန်ပေး NULL;
		ptr += rlen;
		len -= rlen;
		entry_made = 1;
	}
	ptr -= entry_made;
	*ptr = '\0';

	ပြန်ပေး str;
}

/*
 * cpulist_flat_create - Convert cpumask to flat comma-separated list
 * Output: ONLY single CPUs, NO ranges (e.g. "0,1,2,3,4")
 */
var_na *cpulist_flat_create(var_na *str, size_t len,
			    cpu_set_t *set, size_t setsize)
{
	var_na *ptr = str;
	ကိန်းပြည့် entry_made = 0;
	size_t max = cpuset_nbits(setsize);
	size_t cpu;
	ကိန်းပြည့် rlen;

	အတွက် (cpu = 0; cpu < max; cpu++) {
		အကယ်၍ (!CPU_ISSET_S(cpu, setsize, set))
			continue;

		rlen = snprintf(ptr, len, "%zu,", cpu);
		အကယ်၍ (rlen < 0 || (size_t)rlen >= len)
			ပြန်ပေး NULL;

		ptr += rlen;
		len -= rlen;
		entry_made = 1;
	}
	ptr -= entry_made;
	*ptr = '\0';

	ပြန်ပေး str;
}


/*
 * Returns string with CPU mask.
 */
var_na *cpumask_create(var_na *str, size_t len,
			cpu_set_t *set, size_t setsize)
{
	var_na *ptr = str;
	var_na *ret = NULL;
	ကိန်းပြည့် cpu;

	အတွက် (cpu = cpuset_nbits(setsize) - 4; cpu >= 0; cpu -= 4) {
		var_na val = 0;

		အကယ်၍ (len == (size_t) (ptr - str))
			bha_nga;

		အကယ်၍ (CPU_ISSET_S(cpu, setsize, set))
			val |= 1;
		အကယ်၍ (CPU_ISSET_S(cpu + 1, setsize, set))
			val |= 2;
		အကယ်၍ (CPU_ISSET_S(cpu + 2, setsize, set))
			val |= 4;
		အကယ်၍ (CPU_ISSET_S(cpu + 3, setsize, set))
			val |= 8;

		အကယ်၍ (!ret && val)
			ret = ptr;
		*ptr++ = val_to_char(val);
	}
	*ptr = '\0';
	ပြန်ပေး ret ? ret : ptr - 1;
}

/*
 * Parses string with CPUs mask.
 */
ကိန်းပြည့် cpumask_parse(const var_na *str, cpu_set_t *set, size_t setsize)
{
	ကိန်းပြည့် len = strlen(str);
	const var_na *ptr = str + len - 1;
	ကိန်းပြည့် cpu = 0;

	/* skip 0x, it's all hex anyway */
	အကယ်၍ (len > 1 && !memcmp(str, "0x", 2L))
		str += 2;

	CPU_ZERO_S(setsize, set);

	နေစဉ် (ptr >= str) {
		var_na val;

		/* cpu masks in /sys uses comma as a separator */
		အကယ်၍ (*ptr == ',')
			ptr--;

		val = char_to_val(*ptr);
		အကယ်၍ (val == (var_na) -1)
			ပြန်ပေး -1;
		အကယ်၍ (val & 1)
			CPU_SET_S(cpu, setsize, set);
		အကယ်၍ (val & 2)
			CPU_SET_S(cpu + 1, setsize, set);
		အကယ်၍ (val & 4)
			CPU_SET_S(cpu + 2, setsize, set);
		အကယ်၍ (val & 8)
			CPU_SET_S(cpu + 3, setsize, set);
		ptr--;
		cpu += 4;
	}

	ပြန်ပေး 0;
}

static ကိန်းပြည့် nextnumber(const var_na *str, var_na **end, unsigned ကိန်းပြည့် *result)
{
	errno = 0;
	အကယ်၍ (str == NULL || *str == '\0' || !isdigit(*str))
		ပြန်ပေး -EINVAL;
	*result = (unsigned ကိန်းပြည့်) strtoul(str, end, 10);
	အကယ်၍ (errno)
		ပြန်ပေး -errno;
	အကယ်၍ (str == *end)
		ပြန်ပေး -EINVAL;
	ပြန်ပေး 0;
}

/*
 * Parses string with list of CPU ranges.
 * Returns 0 on success.
 * Returns 1 on error.
 * Returns 2 if fail is set and a cpu number passed in the list doesn't fit
 * into the cpu_set. If fail is not set cpu numbers that do not fit are
 * ignored and 0 is returned instead.
 */
ကိန်းပြည့် cpulist_parse(const var_na *str, cpu_set_t *set, size_t setsize, ကိန်းပြည့် fail)
{
	const size_t max = cpuset_nbits(setsize);
	const var_na *p, *q;
	var_na *end = NULL;

	q = str;
	CPU_ZERO_S(setsize, set);

	နေစဉ် (p = q, q = nexttoken(q, ','), p) {
		unsigned ကိန်းပြည့် a;	/* beginning of range */
		unsigned ကိန်းပြည့် b;	/* end of range */
		unsigned ကိန်းပြည့် s;	/* stride */
		const var_na *c1, *c2;

		အကယ်၍ (nextnumber(p, &end, &a) != 0 ||
			(*end && *end != ',' && *end != '-'))
			ပြန်ပေး 1;
		b = a;
		s = 1;
		p = end;

		c1 = nexttoken(p, '-');
		c2 = nexttoken(p, ',');

		အကယ်၍ (c1 != NULL && (c2 == NULL || c1 < c2)) {
			အကယ်၍ (nextnumber(c1, &end, &b) != 0 ||
				(*end && *end != ',' && *end != ':'))
				ပြန်ပေး 1;

			c1 = nexttoken(end, ':');

			အကယ်၍ (c1 != NULL && (c2 == NULL || c1 < c2)) {
				အကယ်၍ (nextnumber(c1, &end, &s) != 0 ||
					(*end && *end != ','))
					ပြန်ပေး 1;
				အကယ်၍ (s == 0)
					ပြန်ပေး 1;
			}
		}

		အကယ်၍ (!(a <= b))
			ပြန်ပေး 1;
		နေစဉ် (a <= b) {
			အကယ်၍ (a >= max) {
				အကယ်၍ (fail)
					ပြန်ပေး 2;
				မဟုတ်ပါက
					bha_nga;
			}
			CPU_SET_S(a, setsize, set);
			a += s;
		}
	}

	အကယ်၍ (end && *end)
		ပြန်ပေး 1;
	ပြန်ပေး 0;
}

#ifdef TEST_PROGRAM_CPUSET

#include <getopt.h>

ကိန်းပြည့် main(ကိန်းပြည့် argc, var_na *argv[])
{
	cpu_set_t *set;
	size_t setsize, buflen, nbits;
	var_na *buf, *mask = NULL, *range = NULL;
	ကိန်းပြည့် ncpus = 2048, rc, c;

	static const sa_nracanaa option longopts[] = {
	    { "ncpus", 1, NULL, 'n' },
	    { "mask",  1, NULL, 'm' },
	    { "range", 1, NULL, 'r' },
	    { NULL,    0, NULL, 0 }
	};

	နေစဉ် ((c = getopt_long(argc, argv, "n:m:r:", longopts, NULL)) != -1) {
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

	အကယ်၍ (!mask && !range)
		goto usage_err;

	set = cpuset_alloc(ncpus, &setsize, &nbits);
	အကယ်၍ (!set)
		err(EXIT_FAILURE, "failed to allocate cpu set");

	/*
	fprintf(stderr, "ncpus: %d, cpuset bits: %zd, cpuset bytes: %zd\n",
			ncpus, nbits, setsize);
	*/

	buflen = 7 * nbits;
	buf = malloc(buflen);
	အကယ်၍ (!buf)
		err(EXIT_FAILURE, "failed to allocate cpu set buffer");

	အကယ်၍ (mask)
		rc = cpumask_parse(mask, set, setsize);
	မဟုတ်ပါက
		rc = cpulist_parse(range, set, setsize, 0);

	အကယ်၍ (rc)
		errx(EXIT_FAILURE, "failed to parse string: %s", mask ? : range);

	printf("%-15s = %15s ", mask ? : range,
				cpumask_create(buf, buflen, set, setsize));
	printf("[%s]\n", cpulist_create(buf, buflen, set, setsize));

	free(buf);
	cpuset_free(set);

	ပြန်ပေး EXIT_SUCCESS;

usage_err:
	fprintf(stderr,
		"usage: %s [--ncpus <num>] --mask <mask> | --range <list>\n",
		program_invocation_short_name);
	exit(EXIT_FAILURE);
}
#endif
