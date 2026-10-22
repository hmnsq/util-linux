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

static inline ціле val_to_char(ціле v)
{
	якщо (v >= 0 && v < 10)
		повернути '0' + v;
	якщо (v >= 10 && v < 16)
		повернути ('a' - 10) + v;
	повернути -1;
}

static inline ціле char_to_val(ціле c)
{
	ціле cl;

	якщо (c >= '0' && c <= '9')
		повернути c - '0';
	cl = tolower(c);
	якщо (cl >= 'a' && cl <= 'f')
		повернути cl + (10 - 'a');
	повернути -1;
}

static const char *nexttoken(const char *q,  ціле sep)
{
	якщо (q)
		q = strchr(q, sep);
	якщо (q)
		q++;
	повернути q;
}

/*
 * Number of bits in a CPU bitmask on current system
 */
ціле get_max_number_of_cpus(void)
{
#ifdef SYS_sched_getaffinity
	ціле n, cpus = 2048;
	size_t setsize;
	cpu_set_t *set = cpuset_alloc(cpus, &setsize, NULL);

	якщо (!set)
		повернути -1;	/* error */

	для (;;) {
		CPU_ZERO_S(setsize, set);

		/* the library version does not return size of cpumask_t */
		n = syscall(SYS_sched_getaffinity, 0, setsize, set);

		якщо (n < 0 && errno == EINVAL && cpus < 1024 * 1024) {
			cpuset_free(set);
			cpus *= 2;
			set = cpuset_alloc(cpus, &setsize, NULL);
			якщо (!set)
				повернути -1;	/* error */
			continue;
		}
		cpuset_free(set);
		повернути n * 8;
	}
#endif
	повернути -1;
}

/*
 * Allocates a new set for ncpus and returns size in bytes and size in bits
 */
cpu_set_t *cpuset_alloc(ціле ncpus, size_t *setsize, size_t *nbits)
{
	cpu_set_t *set = NULL;

	якщо (ncpus)
		set = CPU_ALLOC(ncpus);
	якщо (!set)
		повернути NULL;
	якщо (setsize)
		*setsize = CPU_ALLOC_SIZE(ncpus);
	якщо (nbits)
		*nbits = cpuset_nbits(CPU_ALLOC_SIZE(ncpus));
	повернути set;
}

void cpuset_free(cpu_set_t *set)
{
	CPU_FREE(set);
}

#якщо !HAVE_DECL_CPU_ALLOC
/* Please, use CPU_COUNT_S() macro. This is fallback */
ціле __cpuset_count_s(size_t setsize, const cpu_set_t *set)
{
	ціле s = 0;
	const __cpu_mask *p = set->__bits;
	const __cpu_mask *end = &set->__bits[setsize / sizeof (__cpu_mask)];

	поки (p < end) {
		__cpu_mask l = *p++;

		якщо (l == 0)
			continue;
# якщо LONG_BIT > 32
		l = (l & 0x5555555555555555ul) + ((l >> 1) & 0x5555555555555555ul);
		l = (l & 0x3333333333333333ul) + ((l >> 2) & 0x3333333333333333ul);
		l = (l & 0x0f0f0f0f0f0f0f0ful) + ((l >> 4) & 0x0f0f0f0f0f0f0f0ful);
		l = (l & 0x00ff00ff00ff00fful) + ((l >> 8) & 0x00ff00ff00ff00fful);
		l = (l & 0x0000ffff0000fffful) + ((l >> 16) & 0x0000ffff0000fffful);
		l = (l & 0x00000000fffffffful) + ((l >> 32) & 0x00000000fffffffful);
# інакше
		l = (l & 0x55555555ul) + ((l >> 1) & 0x55555555ul);
		l = (l & 0x33333333ul) + ((l >> 2) & 0x33333333ul);
		l = (l & 0x0f0f0f0ful) + ((l >> 4) & 0x0f0f0f0ful);
		l = (l & 0x00ff00fful) + ((l >> 8) & 0x00ff00fful);
		l = (l & 0x0000fffful) + ((l >> 16) & 0x0000fffful);
# endif
		s += l;
	}
	повернути s;
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
	для (; start < nbits; start++)
		якщо (CPU_ISSET_S(start, setsize, set))
			повернути start;
	повернути start;
}

/*
 * Returns human readable representation of the cpuset. The output format is
 * a list of CPUs with ranges (for example, "0,1,3-9:3").
 */
char *cpulist_create(char *str, size_t len,
			cpu_set_t *set, size_t setsize)
{
	char *ptr = str;
	ціле entry_made = 0;
	size_t max = cpuset_nbits(setsize);
	size_t a = 0;  /* min for cpu range */
	size_t next = 0;  /* where to start looking for next cpu */

	поки ((a = find_next_cpu(next, setsize, set)) < max) {
		ціле rlen;
		next = find_next_cpu(a + 1, setsize, set);
		якщо (next == max) {
			rlen = snprintf(ptr, len, "%zu,", a);
		} інакше {
			/* Extend range as long as we have the same stride. */
			size_t b = next;
			size_t s = b - a;
			поки (((next = find_next_cpu(b + 1, setsize, set)) <
				max) && next - b == s) {
				b = next;
			}
			якщо (b - a == s) {
				/*
				 * Only print one CPU.  Hope the next one can
				 * be put in the next range.
				 */
				rlen = snprintf(ptr, len, "%zu,", a);
				next = b;
			} інакше якщо (s == 1) {
				rlen = snprintf(ptr, len, "%zu-%zu,", a, b);
			} інакше {
				rlen = snprintf(ptr, len, "%zu-%zu:%zu,",
						a, b, s);
			}
		}
		якщо (rlen < 0 || (size_t) rlen >= len)
			повернути NULL;
		ptr += rlen;
		len -= rlen;
		entry_made = 1;
	}
	ptr -= entry_made;
	*ptr = '\0';

	повернути str;
}

/*
 * cpulist_flat_create - Convert cpumask to flat comma-separated list
 * Output: ONLY single CPUs, NO ranges (e.g. "0,1,2,3,4")
 */
char *cpulist_flat_create(char *str, size_t len,
			    cpu_set_t *set, size_t setsize)
{
	char *ptr = str;
	ціле entry_made = 0;
	size_t max = cpuset_nbits(setsize);
	size_t cpu;
	ціле rlen;

	для (cpu = 0; cpu < max; cpu++) {
		якщо (!CPU_ISSET_S(cpu, setsize, set))
			continue;

		rlen = snprintf(ptr, len, "%zu,", cpu);
		якщо (rlen < 0 || (size_t)rlen >= len)
			повернути NULL;

		ptr += rlen;
		len -= rlen;
		entry_made = 1;
	}
	ptr -= entry_made;
	*ptr = '\0';

	повернути str;
}


/*
 * Returns string with CPU mask.
 */
char *cpumask_create(char *str, size_t len,
			cpu_set_t *set, size_t setsize)
{
	char *ptr = str;
	char *ret = NULL;
	ціле cpu;

	для (cpu = cpuset_nbits(setsize) - 4; cpu >= 0; cpu -= 4) {
		char val = 0;

		якщо (len == (size_t) (ptr - str))
			break;

		якщо (CPU_ISSET_S(cpu, setsize, set))
			val |= 1;
		якщо (CPU_ISSET_S(cpu + 1, setsize, set))
			val |= 2;
		якщо (CPU_ISSET_S(cpu + 2, setsize, set))
			val |= 4;
		якщо (CPU_ISSET_S(cpu + 3, setsize, set))
			val |= 8;

		якщо (!ret && val)
			ret = ptr;
		*ptr++ = val_to_char(val);
	}
	*ptr = '\0';
	повернути ret ? ret : ptr - 1;
}

/*
 * Parses string with CPUs mask.
 */
ціле cpumask_parse(const char *str, cpu_set_t *set, size_t setsize)
{
	ціле len = strlen(str);
	const char *ptr = str + len - 1;
	ціле cpu = 0;

	/* skip 0x, it's all hex anyway */
	якщо (len > 1 && !memcmp(str, "0x", 2L))
		str += 2;

	CPU_ZERO_S(setsize, set);

	поки (ptr >= str) {
		char val;

		/* cpu masks in /sys uses comma as a separator */
		якщо (*ptr == ',')
			ptr--;

		val = char_to_val(*ptr);
		якщо (val == (char) -1)
			повернути -1;
		якщо (val & 1)
			CPU_SET_S(cpu, setsize, set);
		якщо (val & 2)
			CPU_SET_S(cpu + 1, setsize, set);
		якщо (val & 4)
			CPU_SET_S(cpu + 2, setsize, set);
		якщо (val & 8)
			CPU_SET_S(cpu + 3, setsize, set);
		ptr--;
		cpu += 4;
	}

	повернути 0;
}

static ціле nextnumber(const char *str, char **end, unsigned ціле *result)
{
	errno = 0;
	якщо (str == NULL || *str == '\0' || !isdigit(*str))
		повернути -EINVAL;
	*result = (unsigned ціле) strtoul(str, end, 10);
	якщо (errno)
		повернути -errno;
	якщо (str == *end)
		повернути -EINVAL;
	повернути 0;
}

/*
 * Parses string with list of CPU ranges.
 * Returns 0 on success.
 * Returns 1 on error.
 * Returns 2 if fail is set and a cpu number passed in the list doesn't fit
 * into the cpu_set. If fail is not set cpu numbers that do not fit are
 * ignored and 0 is returned instead.
 */
ціле cpulist_parse(const char *str, cpu_set_t *set, size_t setsize, ціле fail)
{
	const size_t max = cpuset_nbits(setsize);
	const char *p, *q;
	char *end = NULL;

	q = str;
	CPU_ZERO_S(setsize, set);

	поки (p = q, q = nexttoken(q, ','), p) {
		unsigned ціле a;	/* beginning of range */
		unsigned ціле b;	/* end of range */
		unsigned ціле s;	/* stride */
		const char *c1, *c2;

		якщо (nextnumber(p, &end, &a) != 0 ||
			(*end && *end != ',' && *end != '-'))
			повернути 1;
		b = a;
		s = 1;
		p = end;

		c1 = nexttoken(p, '-');
		c2 = nexttoken(p, ',');

		якщо (c1 != NULL && (c2 == NULL || c1 < c2)) {
			якщо (nextnumber(c1, &end, &b) != 0 ||
				(*end && *end != ',' && *end != ':'))
				повернути 1;

			c1 = nexttoken(end, ':');

			якщо (c1 != NULL && (c2 == NULL || c1 < c2)) {
				якщо (nextnumber(c1, &end, &s) != 0 ||
					(*end && *end != ','))
					повернути 1;
				якщо (s == 0)
					повернути 1;
			}
		}

		якщо (!(a <= b))
			повернути 1;
		поки (a <= b) {
			якщо (a >= max) {
				якщо (fail)
					повернути 2;
				інакше
					break;
			}
			CPU_SET_S(a, setsize, set);
			a += s;
		}
	}

	якщо (end && *end)
		повернути 1;
	повернути 0;
}

#ifdef TEST_PROGRAM_CPUSET

#include <getopt.h>

ціле main(ціле argc, char *argv[])
{
	cpu_set_t *set;
	size_t setsize, buflen, nbits;
	char *buf, *mask = NULL, *range = NULL;
	ціле ncpus = 2048, rc, c;

	static const struct option longopts[] = {
	    { "ncpus", 1, NULL, 'n' },
	    { "mask",  1, NULL, 'm' },
	    { "range", 1, NULL, 'r' },
	    { NULL,    0, NULL, 0 }
	};

	поки ((c = getopt_long(argc, argv, "n:m:r:", longopts, NULL)) != -1) {
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

	якщо (!mask && !range)
		goto usage_err;

	set = cpuset_alloc(ncpus, &setsize, &nbits);
	якщо (!set)
		err(EXIT_FAILURE, "failed to allocate cpu set");

	/*
	fprintf(stderr, "ncpus: %d, cpuset bits: %zd, cpuset bytes: %zd\n",
			ncpus, nbits, setsize);
	*/

	buflen = 7 * nbits;
	buf = malloc(buflen);
	якщо (!buf)
		err(EXIT_FAILURE, "failed to allocate cpu set buffer");

	якщо (mask)
		rc = cpumask_parse(mask, set, setsize);
	інакше
		rc = cpulist_parse(range, set, setsize, 0);

	якщо (rc)
		errx(EXIT_FAILURE, "failed to parse string: %s", mask ? : range);

	printf("%-15s = %15s ", mask ? : range,
				cpumask_create(buf, buflen, set, setsize));
	printf("[%s]\n", cpulist_create(buf, buflen, set, setsize));

	free(buf);
	cpuset_free(set);

	повернути EXIT_SUCCESS;

usage_err:
	fprintf(stderr,
		"usage: %s [--ncpus <num>] --mask <mask> | --range <list>\n",
		program_invocation_short_name);
	exit(EXIT_FAILURE);
}
#endif
