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

static inline số_nguyên val_to_char(số_nguyên v)
{
	nếu (v >= 0 && v < 10)
		trả_về '0' + v;
	nếu (v >= 10 && v < 16)
		trả_về ('a' - 10) + v;
	trả_về -1;
}

static inline số_nguyên char_to_val(số_nguyên c)
{
	số_nguyên cl;

	nếu (c >= '0' && c <= '9')
		trả_về c - '0';
	cl = tolower(c);
	nếu (cl >= 'a' && cl <= 'f')
		trả_về cl + (10 - 'a');
	trả_về -1;
}

static const char *nexttoken(const char *q,  số_nguyên sep)
{
	nếu (q)
		q = strchr(q, sep);
	nếu (q)
		q++;
	trả_về q;
}

/*
 * Number of bits in a CPU bitmask on current system
 */
số_nguyên get_max_number_of_cpus(void)
{
#ifdef SYS_sched_getaffinity
	số_nguyên n, cpus = 2048;
	size_t setsize;
	cpu_set_t *set = cpuset_alloc(cpus, &setsize, NULL);

	nếu (!set)
		trả_về -1;	/* error */

	cho (;;) {
		CPU_ZERO_S(setsize, set);

		/* the library version does not return size of cpumask_t */
		n = syscall(SYS_sched_getaffinity, 0, setsize, set);

		nếu (n < 0 && errno == EINVAL && cpus < 1024 * 1024) {
			cpuset_free(set);
			cpus *= 2;
			set = cpuset_alloc(cpus, &setsize, NULL);
			nếu (!set)
				trả_về -1;	/* error */
			continue;
		}
		cpuset_free(set);
		trả_về n * 8;
	}
#endif
	trả_về -1;
}

/*
 * Allocates a new set for ncpus and returns size in bytes and size in bits
 */
cpu_set_t *cpuset_alloc(số_nguyên ncpus, size_t *setsize, size_t *nbits)
{
	cpu_set_t *set = NULL;

	nếu (ncpus)
		set = CPU_ALLOC(ncpus);
	nếu (!set)
		trả_về NULL;
	nếu (setsize)
		*setsize = CPU_ALLOC_SIZE(ncpus);
	nếu (nbits)
		*nbits = cpuset_nbits(CPU_ALLOC_SIZE(ncpus));
	trả_về set;
}

void cpuset_free(cpu_set_t *set)
{
	CPU_FREE(set);
}

#nếu !HAVE_DECL_CPU_ALLOC
/* Please, use CPU_COUNT_S() macro. This is fallback */
số_nguyên __cpuset_count_s(size_t setsize, const cpu_set_t *set)
{
	số_nguyên s = 0;
	const __cpu_mask *p = set->__bits;
	const __cpu_mask *end = &set->__bits[setsize / sizeof (__cpu_mask)];

	trong_khi (p < end) {
		__cpu_mask l = *p++;

		nếu (l == 0)
			continue;
# nếu LONG_BIT > 32
		l = (l & 0x5555555555555555ul) + ((l >> 1) & 0x5555555555555555ul);
		l = (l & 0x3333333333333333ul) + ((l >> 2) & 0x3333333333333333ul);
		l = (l & 0x0f0f0f0f0f0f0f0ful) + ((l >> 4) & 0x0f0f0f0f0f0f0f0ful);
		l = (l & 0x00ff00ff00ff00fful) + ((l >> 8) & 0x00ff00ff00ff00fful);
		l = (l & 0x0000ffff0000fffful) + ((l >> 16) & 0x0000ffff0000fffful);
		l = (l & 0x00000000fffffffful) + ((l >> 32) & 0x00000000fffffffful);
# không_thì
		l = (l & 0x55555555ul) + ((l >> 1) & 0x55555555ul);
		l = (l & 0x33333333ul) + ((l >> 2) & 0x33333333ul);
		l = (l & 0x0f0f0f0ful) + ((l >> 4) & 0x0f0f0f0ful);
		l = (l & 0x00ff00fful) + ((l >> 8) & 0x00ff00fful);
		l = (l & 0x0000fffful) + ((l >> 16) & 0x0000fffful);
# endif
		s += l;
	}
	trả_về s;
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
	cho (; start < nbits; start++)
		nếu (CPU_ISSET_S(start, setsize, set))
			trả_về start;
	trả_về start;
}

/*
 * Returns human readable representation of the cpuset. The output format is
 * a list of CPUs with ranges (for example, "0,1,3-9:3").
 */
char *cpulist_create(char *str, size_t len,
			cpu_set_t *set, size_t setsize)
{
	char *ptr = str;
	số_nguyên entry_made = 0;
	size_t max = cpuset_nbits(setsize);
	size_t a = 0;  /* min for cpu range */
	size_t next = 0;  /* where to start looking for next cpu */

	trong_khi ((a = find_next_cpu(next, setsize, set)) < max) {
		số_nguyên rlen;
		next = find_next_cpu(a + 1, setsize, set);
		nếu (next == max) {
			rlen = snprintf(ptr, len, "%zu,", a);
		} không_thì {
			/* Extend range as long as we have the same stride. */
			size_t b = next;
			size_t s = b - a;
			trong_khi (((next = find_next_cpu(b + 1, setsize, set)) <
				max) && next - b == s) {
				b = next;
			}
			nếu (b - a == s) {
				/*
				 * Only print one CPU.  Hope the next one can
				 * be put in the next range.
				 */
				rlen = snprintf(ptr, len, "%zu,", a);
				next = b;
			} không_thì nếu (s == 1) {
				rlen = snprintf(ptr, len, "%zu-%zu,", a, b);
			} không_thì {
				rlen = snprintf(ptr, len, "%zu-%zu:%zu,",
						a, b, s);
			}
		}
		nếu (rlen < 0 || (size_t) rlen >= len)
			trả_về NULL;
		ptr += rlen;
		len -= rlen;
		entry_made = 1;
	}
	ptr -= entry_made;
	*ptr = '\0';

	trả_về str;
}

/*
 * cpulist_flat_create - Convert cpumask to flat comma-separated list
 * Output: ONLY single CPUs, NO ranges (e.g. "0,1,2,3,4")
 */
char *cpulist_flat_create(char *str, size_t len,
			    cpu_set_t *set, size_t setsize)
{
	char *ptr = str;
	số_nguyên entry_made = 0;
	size_t max = cpuset_nbits(setsize);
	size_t cpu;
	số_nguyên rlen;

	cho (cpu = 0; cpu < max; cpu++) {
		nếu (!CPU_ISSET_S(cpu, setsize, set))
			continue;

		rlen = snprintf(ptr, len, "%zu,", cpu);
		nếu (rlen < 0 || (size_t)rlen >= len)
			trả_về NULL;

		ptr += rlen;
		len -= rlen;
		entry_made = 1;
	}
	ptr -= entry_made;
	*ptr = '\0';

	trả_về str;
}


/*
 * Returns string with CPU mask.
 */
char *cpumask_create(char *str, size_t len,
			cpu_set_t *set, size_t setsize)
{
	char *ptr = str;
	char *ret = NULL;
	số_nguyên cpu;

	cho (cpu = cpuset_nbits(setsize) - 4; cpu >= 0; cpu -= 4) {
		char val = 0;

		nếu (len == (size_t) (ptr - str))
			break;

		nếu (CPU_ISSET_S(cpu, setsize, set))
			val |= 1;
		nếu (CPU_ISSET_S(cpu + 1, setsize, set))
			val |= 2;
		nếu (CPU_ISSET_S(cpu + 2, setsize, set))
			val |= 4;
		nếu (CPU_ISSET_S(cpu + 3, setsize, set))
			val |= 8;

		nếu (!ret && val)
			ret = ptr;
		*ptr++ = val_to_char(val);
	}
	*ptr = '\0';
	trả_về ret ? ret : ptr - 1;
}

/*
 * Parses string with CPUs mask.
 */
số_nguyên cpumask_parse(const char *str, cpu_set_t *set, size_t setsize)
{
	số_nguyên len = strlen(str);
	const char *ptr = str + len - 1;
	số_nguyên cpu = 0;

	/* skip 0x, it's all hex anyway */
	nếu (len > 1 && !memcmp(str, "0x", 2L))
		str += 2;

	CPU_ZERO_S(setsize, set);

	trong_khi (ptr >= str) {
		char val;

		/* cpu masks in /sys uses comma as a separator */
		nếu (*ptr == ',')
			ptr--;

		val = char_to_val(*ptr);
		nếu (val == (char) -1)
			trả_về -1;
		nếu (val & 1)
			CPU_SET_S(cpu, setsize, set);
		nếu (val & 2)
			CPU_SET_S(cpu + 1, setsize, set);
		nếu (val & 4)
			CPU_SET_S(cpu + 2, setsize, set);
		nếu (val & 8)
			CPU_SET_S(cpu + 3, setsize, set);
		ptr--;
		cpu += 4;
	}

	trả_về 0;
}

static số_nguyên nextnumber(const char *str, char **end, unsigned số_nguyên *result)
{
	errno = 0;
	nếu (str == NULL || *str == '\0' || !isdigit(*str))
		trả_về -EINVAL;
	*result = (unsigned số_nguyên) strtoul(str, end, 10);
	nếu (errno)
		trả_về -errno;
	nếu (str == *end)
		trả_về -EINVAL;
	trả_về 0;
}

/*
 * Parses string with list of CPU ranges.
 * Returns 0 on success.
 * Returns 1 on error.
 * Returns 2 if fail is set and a cpu number passed in the list doesn't fit
 * into the cpu_set. If fail is not set cpu numbers that do not fit are
 * ignored and 0 is returned instead.
 */
số_nguyên cpulist_parse(const char *str, cpu_set_t *set, size_t setsize, số_nguyên fail)
{
	const size_t max = cpuset_nbits(setsize);
	const char *p, *q;
	char *end = NULL;

	q = str;
	CPU_ZERO_S(setsize, set);

	trong_khi (p = q, q = nexttoken(q, ','), p) {
		unsigned số_nguyên a;	/* beginning of range */
		unsigned số_nguyên b;	/* end of range */
		unsigned số_nguyên s;	/* stride */
		const char *c1, *c2;

		nếu (nextnumber(p, &end, &a) != 0 ||
			(*end && *end != ',' && *end != '-'))
			trả_về 1;
		b = a;
		s = 1;
		p = end;

		c1 = nexttoken(p, '-');
		c2 = nexttoken(p, ',');

		nếu (c1 != NULL && (c2 == NULL || c1 < c2)) {
			nếu (nextnumber(c1, &end, &b) != 0 ||
				(*end && *end != ',' && *end != ':'))
				trả_về 1;

			c1 = nexttoken(end, ':');

			nếu (c1 != NULL && (c2 == NULL || c1 < c2)) {
				nếu (nextnumber(c1, &end, &s) != 0 ||
					(*end && *end != ','))
					trả_về 1;
				nếu (s == 0)
					trả_về 1;
			}
		}

		nếu (!(a <= b))
			trả_về 1;
		trong_khi (a <= b) {
			nếu (a >= max) {
				nếu (fail)
					trả_về 2;
				không_thì
					break;
			}
			CPU_SET_S(a, setsize, set);
			a += s;
		}
	}

	nếu (end && *end)
		trả_về 1;
	trả_về 0;
}

#ifdef TEST_PROGRAM_CPUSET

#include <getopt.h>

số_nguyên main(số_nguyên argc, char *argv[])
{
	cpu_set_t *set;
	size_t setsize, buflen, nbits;
	char *buf, *mask = NULL, *range = NULL;
	số_nguyên ncpus = 2048, rc, c;

	static const struct option longopts[] = {
	    { "ncpus", 1, NULL, 'n' },
	    { "mask",  1, NULL, 'm' },
	    { "range", 1, NULL, 'r' },
	    { NULL,    0, NULL, 0 }
	};

	trong_khi ((c = getopt_long(argc, argv, "n:m:r:", longopts, NULL)) != -1) {
		switch(c) {
		case 'n':
			ncpus = atoi(optarg);
			break;
		case 'm':
			mask = optarg;
			break;
		case 'r':
			range = optarg;
			break;
		default:
			goto usage_err;
		}
	}

	nếu (!mask && !range)
		goto usage_err;

	set = cpuset_alloc(ncpus, &setsize, &nbits);
	nếu (!set)
		err(EXIT_FAILURE, "failed to allocate cpu set");

	/*
	fprintf(stderr, "ncpus: %d, cpuset bits: %zd, cpuset bytes: %zd\n",
			ncpus, nbits, setsize);
	*/

	buflen = 7 * nbits;
	buf = malloc(buflen);
	nếu (!buf)
		err(EXIT_FAILURE, "failed to allocate cpu set buffer");

	nếu (mask)
		rc = cpumask_parse(mask, set, setsize);
	không_thì
		rc = cpulist_parse(range, set, setsize, 0);

	nếu (rc)
		errx(EXIT_FAILURE, "failed to parse string: %s", mask ? : range);

	printf("%-15s = %15s ", mask ? : range,
				cpumask_create(buf, buflen, set, setsize));
	printf("[%s]\n", cpulist_create(buf, buflen, set, setsize));

	free(buf);
	cpuset_free(set);

	trả_về EXIT_SUCCESS;

usage_err:
	fprintf(stderr,
		"usage: %s [--ncpus <num>] --mask <mask> | --range <list>\n",
		program_invocation_short_name);
	exit(EXIT_FAILURE);
}
#endif
