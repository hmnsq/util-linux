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

static inline 整数 val_to_char(整数 v)
{
	もし (v >= 0 && v < 10)
		戻る '0' + v;
	もし (v >= 10 && v < 16)
		戻る ('a' - 10) + v;
	戻る -1;
}

static inline 整数 char_to_val(整数 c)
{
	整数 cl;

	もし (c >= '0' && c <= '9')
		戻る c - '0';
	cl = tolower(c);
	もし (cl >= 'a' && cl <= 'f')
		戻る cl + (10 - 'a');
	戻る -1;
}

static const char *nexttoken(const char *q,  整数 sep)
{
	もし (q)
		q = strchr(q, sep);
	もし (q)
		q++;
	戻る q;
}

/*
 * Number of bits in a CPU bitmask on current system
 */
整数 get_max_number_of_cpus(void)
{
#ifdef SYS_sched_getaffinity
	整数 n, cpus = 2048;
	size_t setsize;
	cpu_set_t *set = cpuset_alloc(cpus, &setsize, NULL);

	もし (!set)
		戻る -1;	/* error */

	ために (;;) {
		CPU_ZERO_S(setsize, set);

		/* the library version does not return size of cpumask_t */
		n = syscall(SYS_sched_getaffinity, 0, setsize, set);

		もし (n < 0 && errno == EINVAL && cpus < 1024 * 1024) {
			cpuset_free(set);
			cpus *= 2;
			set = cpuset_alloc(cpus, &setsize, NULL);
			もし (!set)
				戻る -1;	/* error */
			continue;
		}
		cpuset_free(set);
		戻る n * 8;
	}
#endif
	戻る -1;
}

/*
 * Allocates a new set for ncpus and returns size in bytes and size in bits
 */
cpu_set_t *cpuset_alloc(整数 ncpus, size_t *setsize, size_t *nbits)
{
	cpu_set_t *set = NULL;

	もし (ncpus)
		set = CPU_ALLOC(ncpus);
	もし (!set)
		戻る NULL;
	もし (setsize)
		*setsize = CPU_ALLOC_SIZE(ncpus);
	もし (nbits)
		*nbits = cpuset_nbits(CPU_ALLOC_SIZE(ncpus));
	戻る set;
}

void cpuset_free(cpu_set_t *set)
{
	CPU_FREE(set);
}

#もし !HAVE_DECL_CPU_ALLOC
/* Please, use CPU_COUNT_S() macro. This is fallback */
整数 __cpuset_count_s(size_t setsize, const cpu_set_t *set)
{
	整数 s = 0;
	const __cpu_mask *p = set->__bits;
	const __cpu_mask *end = &set->__bits[setsize / sizeof (__cpu_mask)];

	ながら (p < end) {
		__cpu_mask l = *p++;

		もし (l == 0)
			continue;
# もし LONG_BIT > 32
		l = (l & 0x5555555555555555ul) + ((l >> 1) & 0x5555555555555555ul);
		l = (l & 0x3333333333333333ul) + ((l >> 2) & 0x3333333333333333ul);
		l = (l & 0x0f0f0f0f0f0f0f0ful) + ((l >> 4) & 0x0f0f0f0f0f0f0f0ful);
		l = (l & 0x00ff00ff00ff00fful) + ((l >> 8) & 0x00ff00ff00ff00fful);
		l = (l & 0x0000ffff0000fffful) + ((l >> 16) & 0x0000ffff0000fffful);
		l = (l & 0x00000000fffffffful) + ((l >> 32) & 0x00000000fffffffful);
# 否则
		l = (l & 0x55555555ul) + ((l >> 1) & 0x55555555ul);
		l = (l & 0x33333333ul) + ((l >> 2) & 0x33333333ul);
		l = (l & 0x0f0f0f0ful) + ((l >> 4) & 0x0f0f0f0ful);
		l = (l & 0x00ff00fful) + ((l >> 8) & 0x00ff00fful);
		l = (l & 0x0000fffful) + ((l >> 16) & 0x0000fffful);
# endif
		s += l;
	}
	戻る s;
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
	ために (; start < nbits; start++)
		もし (CPU_ISSET_S(start, setsize, set))
			戻る start;
	戻る start;
}

/*
 * Returns human readable representation of the cpuset. The output format is
 * a list of CPUs with ranges (for example, "0,1,3-9:3").
 */
char *cpulist_create(char *str, size_t len,
			cpu_set_t *set, size_t setsize)
{
	char *ptr = str;
	整数 entry_made = 0;
	size_t max = cpuset_nbits(setsize);
	size_t a = 0;  /* min for cpu range */
	size_t next = 0;  /* where to start looking for next cpu */

	ながら ((a = find_next_cpu(next, setsize, set)) < max) {
		整数 rlen;
		next = find_next_cpu(a + 1, setsize, set);
		もし (next == max) {
			rlen = snprintf(ptr, len, "%zu,", a);
		} 否则 {
			/* Extend range as long as we have the same stride. */
			size_t b = next;
			size_t s = b - a;
			ながら (((next = find_next_cpu(b + 1, setsize, set)) <
				max) && next - b == s) {
				b = next;
			}
			もし (b - a == s) {
				/*
				 * Only print one CPU.  Hope the next one can
				 * be put in the next range.
				 */
				rlen = snprintf(ptr, len, "%zu,", a);
				next = b;
			} 否则 もし (s == 1) {
				rlen = snprintf(ptr, len, "%zu-%zu,", a, b);
			} 否则 {
				rlen = snprintf(ptr, len, "%zu-%zu:%zu,",
						a, b, s);
			}
		}
		もし (rlen < 0 || (size_t) rlen >= len)
			戻る NULL;
		ptr += rlen;
		len -= rlen;
		entry_made = 1;
	}
	ptr -= entry_made;
	*ptr = '\0';

	戻る str;
}

/*
 * cpulist_flat_create - Convert cpumask to flat comma-separated list
 * Output: ONLY single CPUs, NO ranges (e.g. "0,1,2,3,4")
 */
char *cpulist_flat_create(char *str, size_t len,
			    cpu_set_t *set, size_t setsize)
{
	char *ptr = str;
	整数 entry_made = 0;
	size_t max = cpuset_nbits(setsize);
	size_t cpu;
	整数 rlen;

	ために (cpu = 0; cpu < max; cpu++) {
		もし (!CPU_ISSET_S(cpu, setsize, set))
			continue;

		rlen = snprintf(ptr, len, "%zu,", cpu);
		もし (rlen < 0 || (size_t)rlen >= len)
			戻る NULL;

		ptr += rlen;
		len -= rlen;
		entry_made = 1;
	}
	ptr -= entry_made;
	*ptr = '\0';

	戻る str;
}


/*
 * Returns string with CPU mask.
 */
char *cpumask_create(char *str, size_t len,
			cpu_set_t *set, size_t setsize)
{
	char *ptr = str;
	char *ret = NULL;
	整数 cpu;

	ために (cpu = cpuset_nbits(setsize) - 4; cpu >= 0; cpu -= 4) {
		char val = 0;

		もし (len == (size_t) (ptr - str))
			break;

		もし (CPU_ISSET_S(cpu, setsize, set))
			val |= 1;
		もし (CPU_ISSET_S(cpu + 1, setsize, set))
			val |= 2;
		もし (CPU_ISSET_S(cpu + 2, setsize, set))
			val |= 4;
		もし (CPU_ISSET_S(cpu + 3, setsize, set))
			val |= 8;

		もし (!ret && val)
			ret = ptr;
		*ptr++ = val_to_char(val);
	}
	*ptr = '\0';
	戻る ret ? ret : ptr - 1;
}

/*
 * Parses string with CPUs mask.
 */
整数 cpumask_parse(const char *str, cpu_set_t *set, size_t setsize)
{
	整数 len = strlen(str);
	const char *ptr = str + len - 1;
	整数 cpu = 0;

	/* skip 0x, it's all hex anyway */
	もし (len > 1 && !memcmp(str, "0x", 2L))
		str += 2;

	CPU_ZERO_S(setsize, set);

	ながら (ptr >= str) {
		char val;

		/* cpu masks in /sys uses comma as a separator */
		もし (*ptr == ',')
			ptr--;

		val = char_to_val(*ptr);
		もし (val == (char) -1)
			戻る -1;
		もし (val & 1)
			CPU_SET_S(cpu, setsize, set);
		もし (val & 2)
			CPU_SET_S(cpu + 1, setsize, set);
		もし (val & 4)
			CPU_SET_S(cpu + 2, setsize, set);
		もし (val & 8)
			CPU_SET_S(cpu + 3, setsize, set);
		ptr--;
		cpu += 4;
	}

	戻る 0;
}

static 整数 nextnumber(const char *str, char **end, unsigned 整数 *result)
{
	errno = 0;
	もし (str == NULL || *str == '\0' || !isdigit(*str))
		戻る -EINVAL;
	*result = (unsigned 整数) strtoul(str, end, 10);
	もし (errno)
		戻る -errno;
	もし (str == *end)
		戻る -EINVAL;
	戻る 0;
}

/*
 * Parses string with list of CPU ranges.
 * Returns 0 on success.
 * Returns 1 on error.
 * Returns 2 if fail is set and a cpu number passed in the list doesn't fit
 * into the cpu_set. If fail is not set cpu numbers that do not fit are
 * ignored and 0 is returned instead.
 */
整数 cpulist_parse(const char *str, cpu_set_t *set, size_t setsize, 整数 fail)
{
	const size_t max = cpuset_nbits(setsize);
	const char *p, *q;
	char *end = NULL;

	q = str;
	CPU_ZERO_S(setsize, set);

	ながら (p = q, q = nexttoken(q, ','), p) {
		unsigned 整数 a;	/* beginning of range */
		unsigned 整数 b;	/* end of range */
		unsigned 整数 s;	/* stride */
		const char *c1, *c2;

		もし (nextnumber(p, &end, &a) != 0 ||
			(*end && *end != ',' && *end != '-'))
			戻る 1;
		b = a;
		s = 1;
		p = end;

		c1 = nexttoken(p, '-');
		c2 = nexttoken(p, ',');

		もし (c1 != NULL && (c2 == NULL || c1 < c2)) {
			もし (nextnumber(c1, &end, &b) != 0 ||
				(*end && *end != ',' && *end != ':'))
				戻る 1;

			c1 = nexttoken(end, ':');

			もし (c1 != NULL && (c2 == NULL || c1 < c2)) {
				もし (nextnumber(c1, &end, &s) != 0 ||
					(*end && *end != ','))
					戻る 1;
				もし (s == 0)
					戻る 1;
			}
		}

		もし (!(a <= b))
			戻る 1;
		ながら (a <= b) {
			もし (a >= max) {
				もし (fail)
					戻る 2;
				否则
					break;
			}
			CPU_SET_S(a, setsize, set);
			a += s;
		}
	}

	もし (end && *end)
		戻る 1;
	戻る 0;
}

#ifdef TEST_PROGRAM_CPUSET

#include <getopt.h>

整数 メインページ(整数 argc, char *argv[])
{
	cpu_set_t *set;
	size_t setsize, buflen, nbits;
	char *buf, *mask = NULL, *range = NULL;
	整数 ncpus = 2048, rc, c;

	static const struct option longopts[] = {
	    { "ncpus", 1, NULL, 'n' },
	    { "mask",  1, NULL, 'm' },
	    { "range", 1, NULL, 'r' },
	    { NULL,    0, NULL, 0 }
	};

	ながら ((c = getopt_long(argc, argv, "n:m:r:", longopts, NULL)) != -1) {
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

	もし (!mask && !range)
		goto usage_err;

	set = cpuset_alloc(ncpus, &setsize, &nbits);
	もし (!set)
		err(EXIT_FAILURE, "failed to allocate cpu set");

	/*
	fprintf(stderr, "ncpus: %d, cpuset bits: %zd, cpuset bytes: %zd\n",
			ncpus, nbits, setsize);
	*/

	buflen = 7 * nbits;
	buf = malloc(buflen);
	もし (!buf)
		err(EXIT_FAILURE, "failed to allocate cpu set buffer");

	もし (mask)
		rc = cpumask_parse(mask, set, setsize);
	否则
		rc = cpulist_parse(range, set, setsize, 0);

	もし (rc)
		errx(EXIT_FAILURE, "failed to parse string: %s", mask ? : range);

	printf("%-15s = %15s ", mask ? : range,
				cpumask_create(buf, buflen, set, setsize));
	printf("[%s]\n", cpulist_create(buf, buflen, set, setsize));

	free(buf);
	cpuset_free(set);

	戻る EXIT_SUCCESS;

usage_err:
	fprintf(stderr,
		"usage: %s [--ncpus <num>] --mask <mask> | --range <list>\n",
		program_invocation_short_name);
	exit(EXIT_FAILURE);
}
#endif
