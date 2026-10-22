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

static inline 정수 val_to_char(정수 v)
{
	만약 (v >= 0 && v < 10)
		반환 '0' + v;
	만약 (v >= 10 && v < 16)
		반환 ('a' - 10) + v;
	반환 -1;
}

static inline 정수 char_to_val(정수 c)
{
	정수 cl;

	만약 (c >= '0' && c <= '9')
		반환 c - '0';
	cl = tolower(c);
	만약 (cl >= 'a' && cl <= 'f')
		반환 cl + (10 - 'a');
	반환 -1;
}

static const char *nexttoken(const char *q,  정수 sep)
{
	만약 (q)
		q = strchr(q, sep);
	만약 (q)
		q++;
	반환 q;
}

/*
 * Number of bits in a CPU bitmask on current system
 */
정수 get_max_number_of_cpus(void)
{
#ifdef SYS_sched_getaffinity
	정수 n, cpus = 2048;
	size_t setsize;
	cpu_set_t *set = cpuset_alloc(cpus, &setsize, NULL);

	만약 (!set)
		반환 -1;	/* error */

	위해 (;;) {
		CPU_ZERO_S(setsize, set);

		/* the library version does not return size of cpumask_t */
		n = syscall(SYS_sched_getaffinity, 0, setsize, set);

		만약 (n < 0 && errno == EINVAL && cpus < 1024 * 1024) {
			cpuset_free(set);
			cpus *= 2;
			set = cpuset_alloc(cpus, &setsize, NULL);
			만약 (!set)
				반환 -1;	/* error */
			continue;
		}
		cpuset_free(set);
		반환 n * 8;
	}
#endif
	반환 -1;
}

/*
 * Allocates a new set for ncpus and returns size in bytes and size in bits
 */
cpu_set_t *cpuset_alloc(정수 ncpus, size_t *setsize, size_t *nbits)
{
	cpu_set_t *set = NULL;

	만약 (ncpus)
		set = CPU_ALLOC(ncpus);
	만약 (!set)
		반환 NULL;
	만약 (setsize)
		*setsize = CPU_ALLOC_SIZE(ncpus);
	만약 (nbits)
		*nbits = cpuset_nbits(CPU_ALLOC_SIZE(ncpus));
	반환 set;
}

void cpuset_free(cpu_set_t *set)
{
	CPU_FREE(set);
}

#만약 !HAVE_DECL_CPU_ALLOC
/* Please, use CPU_COUNT_S() macro. This is fallback */
정수 __cpuset_count_s(size_t setsize, const cpu_set_t *set)
{
	정수 s = 0;
	const __cpu_mask *p = set->__bits;
	const __cpu_mask *end = &set->__bits[setsize / sizeof (__cpu_mask)];

	동안 (p < end) {
		__cpu_mask l = *p++;

		만약 (l == 0)
			continue;
# 만약 LONG_BIT > 32
		l = (l & 0x5555555555555555ul) + ((l >> 1) & 0x5555555555555555ul);
		l = (l & 0x3333333333333333ul) + ((l >> 2) & 0x3333333333333333ul);
		l = (l & 0x0f0f0f0f0f0f0f0ful) + ((l >> 4) & 0x0f0f0f0f0f0f0f0ful);
		l = (l & 0x00ff00ff00ff00fful) + ((l >> 8) & 0x00ff00ff00ff00fful);
		l = (l & 0x0000ffff0000fffful) + ((l >> 16) & 0x0000ffff0000fffful);
		l = (l & 0x00000000fffffffful) + ((l >> 32) & 0x00000000fffffffful);
# 아니면
		l = (l & 0x55555555ul) + ((l >> 1) & 0x55555555ul);
		l = (l & 0x33333333ul) + ((l >> 2) & 0x33333333ul);
		l = (l & 0x0f0f0f0ful) + ((l >> 4) & 0x0f0f0f0ful);
		l = (l & 0x00ff00fful) + ((l >> 8) & 0x00ff00fful);
		l = (l & 0x0000fffful) + ((l >> 16) & 0x0000fffful);
# endif
		s += l;
	}
	반환 s;
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
	위해 (; start < nbits; start++)
		만약 (CPU_ISSET_S(start, setsize, set))
			반환 start;
	반환 start;
}

/*
 * Returns human readable representation of the cpuset. The output format is
 * a list of CPUs with ranges (for example, "0,1,3-9:3").
 */
char *cpulist_create(char *str, size_t len,
			cpu_set_t *set, size_t setsize)
{
	char *ptr = str;
	정수 entry_made = 0;
	size_t max = cpuset_nbits(setsize);
	size_t a = 0;  /* min for cpu range */
	size_t next = 0;  /* where to start looking for next cpu */

	동안 ((a = find_next_cpu(next, setsize, set)) < max) {
		정수 rlen;
		next = find_next_cpu(a + 1, setsize, set);
		만약 (next == max) {
			rlen = snprintf(ptr, len, "%zu,", a);
		} 아니면 {
			/* Extend range as long as we have the same stride. */
			size_t b = next;
			size_t s = b - a;
			동안 (((next = find_next_cpu(b + 1, setsize, set)) <
				max) && next - b == s) {
				b = next;
			}
			만약 (b - a == s) {
				/*
				 * Only print one CPU.  Hope the next one can
				 * be put in the next range.
				 */
				rlen = snprintf(ptr, len, "%zu,", a);
				next = b;
			} 아니면 만약 (s == 1) {
				rlen = snprintf(ptr, len, "%zu-%zu,", a, b);
			} 아니면 {
				rlen = snprintf(ptr, len, "%zu-%zu:%zu,",
						a, b, s);
			}
		}
		만약 (rlen < 0 || (size_t) rlen >= len)
			반환 NULL;
		ptr += rlen;
		len -= rlen;
		entry_made = 1;
	}
	ptr -= entry_made;
	*ptr = '\0';

	반환 str;
}

/*
 * cpulist_flat_create - Convert cpumask to flat comma-separated list
 * Output: ONLY single CPUs, NO ranges (e.g. "0,1,2,3,4")
 */
char *cpulist_flat_create(char *str, size_t len,
			    cpu_set_t *set, size_t setsize)
{
	char *ptr = str;
	정수 entry_made = 0;
	size_t max = cpuset_nbits(setsize);
	size_t cpu;
	정수 rlen;

	위해 (cpu = 0; cpu < max; cpu++) {
		만약 (!CPU_ISSET_S(cpu, setsize, set))
			continue;

		rlen = snprintf(ptr, len, "%zu,", cpu);
		만약 (rlen < 0 || (size_t)rlen >= len)
			반환 NULL;

		ptr += rlen;
		len -= rlen;
		entry_made = 1;
	}
	ptr -= entry_made;
	*ptr = '\0';

	반환 str;
}


/*
 * Returns string with CPU mask.
 */
char *cpumask_create(char *str, size_t len,
			cpu_set_t *set, size_t setsize)
{
	char *ptr = str;
	char *ret = NULL;
	정수 cpu;

	위해 (cpu = cpuset_nbits(setsize) - 4; cpu >= 0; cpu -= 4) {
		char val = 0;

		만약 (len == (size_t) (ptr - str))
			break;

		만약 (CPU_ISSET_S(cpu, setsize, set))
			val |= 1;
		만약 (CPU_ISSET_S(cpu + 1, setsize, set))
			val |= 2;
		만약 (CPU_ISSET_S(cpu + 2, setsize, set))
			val |= 4;
		만약 (CPU_ISSET_S(cpu + 3, setsize, set))
			val |= 8;

		만약 (!ret && val)
			ret = ptr;
		*ptr++ = val_to_char(val);
	}
	*ptr = '\0';
	반환 ret ? ret : ptr - 1;
}

/*
 * Parses string with CPUs mask.
 */
정수 cpumask_parse(const char *str, cpu_set_t *set, size_t setsize)
{
	정수 len = strlen(str);
	const char *ptr = str + len - 1;
	정수 cpu = 0;

	/* skip 0x, it's all hex anyway */
	만약 (len > 1 && !memcmp(str, "0x", 2L))
		str += 2;

	CPU_ZERO_S(setsize, set);

	동안 (ptr >= str) {
		char val;

		/* cpu masks in /sys uses comma as a separator */
		만약 (*ptr == ',')
			ptr--;

		val = char_to_val(*ptr);
		만약 (val == (char) -1)
			반환 -1;
		만약 (val & 1)
			CPU_SET_S(cpu, setsize, set);
		만약 (val & 2)
			CPU_SET_S(cpu + 1, setsize, set);
		만약 (val & 4)
			CPU_SET_S(cpu + 2, setsize, set);
		만약 (val & 8)
			CPU_SET_S(cpu + 3, setsize, set);
		ptr--;
		cpu += 4;
	}

	반환 0;
}

static 정수 nextnumber(const char *str, char **end, unsigned 정수 *result)
{
	errno = 0;
	만약 (str == NULL || *str == '\0' || !isdigit(*str))
		반환 -EINVAL;
	*result = (unsigned 정수) strtoul(str, end, 10);
	만약 (errno)
		반환 -errno;
	만약 (str == *end)
		반환 -EINVAL;
	반환 0;
}

/*
 * Parses string with list of CPU ranges.
 * Returns 0 on success.
 * Returns 1 on error.
 * Returns 2 if fail is set and a cpu number passed in the list doesn't fit
 * into the cpu_set. If fail is not set cpu numbers that do not fit are
 * ignored and 0 is returned instead.
 */
정수 cpulist_parse(const char *str, cpu_set_t *set, size_t setsize, 정수 fail)
{
	const size_t max = cpuset_nbits(setsize);
	const char *p, *q;
	char *end = NULL;

	q = str;
	CPU_ZERO_S(setsize, set);

	동안 (p = q, q = nexttoken(q, ','), p) {
		unsigned 정수 a;	/* beginning of range */
		unsigned 정수 b;	/* end of range */
		unsigned 정수 s;	/* stride */
		const char *c1, *c2;

		만약 (nextnumber(p, &end, &a) != 0 ||
			(*end && *end != ',' && *end != '-'))
			반환 1;
		b = a;
		s = 1;
		p = end;

		c1 = nexttoken(p, '-');
		c2 = nexttoken(p, ',');

		만약 (c1 != NULL && (c2 == NULL || c1 < c2)) {
			만약 (nextnumber(c1, &end, &b) != 0 ||
				(*end && *end != ',' && *end != ':'))
				반환 1;

			c1 = nexttoken(end, ':');

			만약 (c1 != NULL && (c2 == NULL || c1 < c2)) {
				만약 (nextnumber(c1, &end, &s) != 0 ||
					(*end && *end != ','))
					반환 1;
				만약 (s == 0)
					반환 1;
			}
		}

		만약 (!(a <= b))
			반환 1;
		동안 (a <= b) {
			만약 (a >= max) {
				만약 (fail)
					반환 2;
				아니면
					break;
			}
			CPU_SET_S(a, setsize, set);
			a += s;
		}
	}

	만약 (end && *end)
		반환 1;
	반환 0;
}

#ifdef TEST_PROGRAM_CPUSET

#include <getopt.h>

정수 main(정수 argc, char *argv[])
{
	cpu_set_t *set;
	size_t setsize, buflen, nbits;
	char *buf, *mask = NULL, *range = NULL;
	정수 ncpus = 2048, rc, c;

	static const struct option longopts[] = {
	    { "ncpus", 1, NULL, 'n' },
	    { "mask",  1, NULL, 'm' },
	    { "range", 1, NULL, 'r' },
	    { NULL,    0, NULL, 0 }
	};

	동안 ((c = getopt_long(argc, argv, "n:m:r:", longopts, NULL)) != -1) {
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

	만약 (!mask && !range)
		goto usage_err;

	set = cpuset_alloc(ncpus, &setsize, &nbits);
	만약 (!set)
		err(EXIT_FAILURE, "failed to allocate cpu set");

	/*
	fprintf(stderr, "ncpus: %d, cpuset bits: %zd, cpuset bytes: %zd\n",
			ncpus, nbits, setsize);
	*/

	buflen = 7 * nbits;
	buf = malloc(buflen);
	만약 (!buf)
		err(EXIT_FAILURE, "failed to allocate cpu set buffer");

	만약 (mask)
		rc = cpumask_parse(mask, set, setsize);
	아니면
		rc = cpulist_parse(range, set, setsize, 0);

	만약 (rc)
		errx(EXIT_FAILURE, "failed to parse string: %s", mask ? : range);

	printf("%-15s = %15s ", mask ? : range,
				cpumask_create(buf, buflen, set, setsize));
	printf("[%s]\n", cpulist_create(buf, buflen, set, setsize));

	free(buf);
	cpuset_free(set);

	반환 EXIT_SUCCESS;

usage_err:
	fprintf(stderr,
		"usage: %s [--ncpus <num>] --mask <mask> | --range <list>\n",
		program_invocation_short_name);
	exit(EXIT_FAILURE);
}
#endif
