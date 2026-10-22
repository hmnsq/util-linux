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

static inline целое val_to_char(целое v)
{
	если (v >= 0 && v < 10)
		вернуть '0' + v;
	если (v >= 10 && v < 16)
		вернуть ('a' - 10) + v;
	вернуть -1;
}

static inline целое char_to_val(целое c)
{
	целое cl;

	если (c >= '0' && c <= '9')
		вернуть c - '0';
	cl = tolower(c);
	если (cl >= 'a' && cl <= 'f')
		вернуть cl + (10 - 'a');
	вернуть -1;
}

static const char *nexttoken(const char *q,  целое sep)
{
	если (q)
		q = strchr(q, sep);
	если (q)
		q++;
	вернуть q;
}

/*
 * Number of bits in a CPU bitmask on current system
 */
целое get_max_number_of_cpus(void)
{
#ifdef SYS_sched_getaffinity
	целое n, cpus = 2048;
	size_t setsize;
	cpu_set_t *set = cpuset_alloc(cpus, &setsize, NULL);

	если (!set)
		вернуть -1;	/* error */

	для (;;) {
		CPU_ZERO_S(setsize, set);

		/* the library version does not return size of cpumask_t */
		n = syscall(SYS_sched_getaffinity, 0, setsize, set);

		если (n < 0 && errno == EINVAL && cpus < 1024 * 1024) {
			cpuset_free(set);
			cpus *= 2;
			set = cpuset_alloc(cpus, &setsize, NULL);
			если (!set)
				вернуть -1;	/* error */
			continue;
		}
		cpuset_free(set);
		вернуть n * 8;
	}
#endif
	вернуть -1;
}

/*
 * Allocates a new set for ncpus and returns size in bytes and size in bits
 */
cpu_set_t *cpuset_alloc(целое ncpus, size_t *setsize, size_t *nbits)
{
	cpu_set_t *set = NULL;

	если (ncpus)
		set = CPU_ALLOC(ncpus);
	если (!set)
		вернуть NULL;
	если (setsize)
		*setsize = CPU_ALLOC_SIZE(ncpus);
	если (nbits)
		*nbits = cpuset_nbits(CPU_ALLOC_SIZE(ncpus));
	вернуть set;
}

void cpuset_free(cpu_set_t *set)
{
	CPU_FREE(set);
}

#если !HAVE_DECL_CPU_ALLOC
/* Please, use CPU_COUNT_S() macro. This is fallback */
целое __cpuset_count_s(size_t setsize, const cpu_set_t *set)
{
	целое s = 0;
	const __cpu_mask *p = set->__bits;
	const __cpu_mask *end = &set->__bits[setsize / sizeof (__cpu_mask)];

	пока (p < end) {
		__cpu_mask l = *p++;

		если (l == 0)
			continue;
# если LONG_BIT > 32
		l = (l & 0x5555555555555555ul) + ((l >> 1) & 0x5555555555555555ul);
		l = (l & 0x3333333333333333ul) + ((l >> 2) & 0x3333333333333333ul);
		l = (l & 0x0f0f0f0f0f0f0f0ful) + ((l >> 4) & 0x0f0f0f0f0f0f0f0ful);
		l = (l & 0x00ff00ff00ff00fful) + ((l >> 8) & 0x00ff00ff00ff00fful);
		l = (l & 0x0000ffff0000fffful) + ((l >> 16) & 0x0000ffff0000fffful);
		l = (l & 0x00000000fffffffful) + ((l >> 32) & 0x00000000fffffffful);
# иначе
		l = (l & 0x55555555ul) + ((l >> 1) & 0x55555555ul);
		l = (l & 0x33333333ul) + ((l >> 2) & 0x33333333ul);
		l = (l & 0x0f0f0f0ful) + ((l >> 4) & 0x0f0f0f0ful);
		l = (l & 0x00ff00fful) + ((l >> 8) & 0x00ff00fful);
		l = (l & 0x0000fffful) + ((l >> 16) & 0x0000fffful);
# endif
		s += l;
	}
	вернуть s;
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
		если (CPU_ISSET_S(start, setsize, set))
			вернуть start;
	вернуть start;
}

/*
 * Returns human readable representation of the cpuset. The output format is
 * a list of CPUs with ranges (for example, "0,1,3-9:3").
 */
char *cpulist_create(char *str, size_t len,
			cpu_set_t *set, size_t setsize)
{
	char *ptr = str;
	целое entry_made = 0;
	size_t max = cpuset_nbits(setsize);
	size_t a = 0;  /* min for cpu range */
	size_t next = 0;  /* where to start looking for next cpu */

	пока ((a = find_next_cpu(next, setsize, set)) < max) {
		целое rlen;
		next = find_next_cpu(a + 1, setsize, set);
		если (next == max) {
			rlen = snprintf(ptr, len, "%zu,", a);
		} иначе {
			/* Extend range as long as we have the same stride. */
			size_t b = next;
			size_t s = b - a;
			пока (((next = find_next_cpu(b + 1, setsize, set)) <
				max) && next - b == s) {
				b = next;
			}
			если (b - a == s) {
				/*
				 * Only print one CPU.  Hope the next one can
				 * be put in the next range.
				 */
				rlen = snprintf(ptr, len, "%zu,", a);
				next = b;
			} иначе если (s == 1) {
				rlen = snprintf(ptr, len, "%zu-%zu,", a, b);
			} иначе {
				rlen = snprintf(ptr, len, "%zu-%zu:%zu,",
						a, b, s);
			}
		}
		если (rlen < 0 || (size_t) rlen >= len)
			вернуть NULL;
		ptr += rlen;
		len -= rlen;
		entry_made = 1;
	}
	ptr -= entry_made;
	*ptr = '\0';

	вернуть str;
}

/*
 * cpulist_flat_create - Convert cpumask to flat comma-separated list
 * Output: ONLY single CPUs, NO ranges (e.g. "0,1,2,3,4")
 */
char *cpulist_flat_create(char *str, size_t len,
			    cpu_set_t *set, size_t setsize)
{
	char *ptr = str;
	целое entry_made = 0;
	size_t max = cpuset_nbits(setsize);
	size_t cpu;
	целое rlen;

	для (cpu = 0; cpu < max; cpu++) {
		если (!CPU_ISSET_S(cpu, setsize, set))
			continue;

		rlen = snprintf(ptr, len, "%zu,", cpu);
		если (rlen < 0 || (size_t)rlen >= len)
			вернуть NULL;

		ptr += rlen;
		len -= rlen;
		entry_made = 1;
	}
	ptr -= entry_made;
	*ptr = '\0';

	вернуть str;
}


/*
 * Returns string with CPU mask.
 */
char *cpumask_create(char *str, size_t len,
			cpu_set_t *set, size_t setsize)
{
	char *ptr = str;
	char *ret = NULL;
	целое cpu;

	для (cpu = cpuset_nbits(setsize) - 4; cpu >= 0; cpu -= 4) {
		char val = 0;

		если (len == (size_t) (ptr - str))
			break;

		если (CPU_ISSET_S(cpu, setsize, set))
			val |= 1;
		если (CPU_ISSET_S(cpu + 1, setsize, set))
			val |= 2;
		если (CPU_ISSET_S(cpu + 2, setsize, set))
			val |= 4;
		если (CPU_ISSET_S(cpu + 3, setsize, set))
			val |= 8;

		если (!ret && val)
			ret = ptr;
		*ptr++ = val_to_char(val);
	}
	*ptr = '\0';
	вернуть ret ? ret : ptr - 1;
}

/*
 * Parses string with CPUs mask.
 */
целое cpumask_parse(const char *str, cpu_set_t *set, size_t setsize)
{
	целое len = strlen(str);
	const char *ptr = str + len - 1;
	целое cpu = 0;

	/* skip 0x, it's all hex anyway */
	если (len > 1 && !memcmp(str, "0x", 2L))
		str += 2;

	CPU_ZERO_S(setsize, set);

	пока (ptr >= str) {
		char val;

		/* cpu masks in /sys uses comma as a separator */
		если (*ptr == ',')
			ptr--;

		val = char_to_val(*ptr);
		если (val == (char) -1)
			вернуть -1;
		если (val & 1)
			CPU_SET_S(cpu, setsize, set);
		если (val & 2)
			CPU_SET_S(cpu + 1, setsize, set);
		если (val & 4)
			CPU_SET_S(cpu + 2, setsize, set);
		если (val & 8)
			CPU_SET_S(cpu + 3, setsize, set);
		ptr--;
		cpu += 4;
	}

	вернуть 0;
}

static целое nextnumber(const char *str, char **end, unsigned целое *result)
{
	errno = 0;
	если (str == NULL || *str == '\0' || !isdigit(*str))
		вернуть -EINVAL;
	*result = (unsigned целое) strtoul(str, end, 10);
	если (errno)
		вернуть -errno;
	если (str == *end)
		вернуть -EINVAL;
	вернуть 0;
}

/*
 * Parses string with list of CPU ranges.
 * Returns 0 on success.
 * Returns 1 on error.
 * Returns 2 if fail is set and a cpu number passed in the list doesn't fit
 * into the cpu_set. If fail is not set cpu numbers that do not fit are
 * ignored and 0 is returned instead.
 */
целое cpulist_parse(const char *str, cpu_set_t *set, size_t setsize, целое fail)
{
	const size_t max = cpuset_nbits(setsize);
	const char *p, *q;
	char *end = NULL;

	q = str;
	CPU_ZERO_S(setsize, set);

	пока (p = q, q = nexttoken(q, ','), p) {
		unsigned целое a;	/* beginning of range */
		unsigned целое b;	/* end of range */
		unsigned целое s;	/* stride */
		const char *c1, *c2;

		если (nextnumber(p, &end, &a) != 0 ||
			(*end && *end != ',' && *end != '-'))
			вернуть 1;
		b = a;
		s = 1;
		p = end;

		c1 = nexttoken(p, '-');
		c2 = nexttoken(p, ',');

		если (c1 != NULL && (c2 == NULL || c1 < c2)) {
			если (nextnumber(c1, &end, &b) != 0 ||
				(*end && *end != ',' && *end != ':'))
				вернуть 1;

			c1 = nexttoken(end, ':');

			если (c1 != NULL && (c2 == NULL || c1 < c2)) {
				если (nextnumber(c1, &end, &s) != 0 ||
					(*end && *end != ','))
					вернуть 1;
				если (s == 0)
					вернуть 1;
			}
		}

		если (!(a <= b))
			вернуть 1;
		пока (a <= b) {
			если (a >= max) {
				если (fail)
					вернуть 2;
				иначе
					break;
			}
			CPU_SET_S(a, setsize, set);
			a += s;
		}
	}

	если (end && *end)
		вернуть 1;
	вернуть 0;
}

#ifdef TEST_PROGRAM_CPUSET

#include <getopt.h>

целое основной(целое argc, char *argv[])
{
	cpu_set_t *set;
	size_t setsize, buflen, nbits;
	char *buf, *mask = NULL, *range = NULL;
	целое ncpus = 2048, rc, c;

	static const struct option longopts[] = {
	    { "ncpus", 1, NULL, 'n' },
	    { "mask",  1, NULL, 'm' },
	    { "range", 1, NULL, 'r' },
	    { NULL,    0, NULL, 0 }
	};

	пока ((c = getopt_long(argc, argv, "n:m:r:", longopts, NULL)) != -1) {
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

	если (!mask && !range)
		goto usage_err;

	set = cpuset_alloc(ncpus, &setsize, &nbits);
	если (!set)
		err(EXIT_FAILURE, "failed to allocate cpu set");

	/*
	fprintf(stderr, "ncpus: %d, cpuset bits: %zd, cpuset bytes: %zd\n",
			ncpus, nbits, setsize);
	*/

	buflen = 7 * nbits;
	buf = malloc(buflen);
	если (!buf)
		err(EXIT_FAILURE, "failed to allocate cpu set buffer");

	если (mask)
		rc = cpumask_parse(mask, set, setsize);
	иначе
		rc = cpulist_parse(range, set, setsize, 0);

	если (rc)
		errx(EXIT_FAILURE, "failed to parse string: %s", mask ? : range);

	printf("%-15s = %15s ", mask ? : range,
				cpumask_create(buf, buflen, set, setsize));
	printf("[%s]\n", cpulist_create(buf, buflen, set, setsize));

	free(buf);
	cpuset_free(set);

	вернуть EXIT_SUCCESS;

usage_err:
	fprintf(stderr,
		"usage: %s [--ncpus <num>] --mask <mask> | --range <list>\n",
		program_invocation_short_name);
	exit(EXIT_FAILURE);
}
#endif
