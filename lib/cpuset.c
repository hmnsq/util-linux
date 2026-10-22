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

#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <unistd.h>
#ಅನ್ತರ್ಭಾವ <sched.h>
#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <ctype.h>
#ifdef HAVE_SYS_SYSCALL_H
#ಅನ್ತರ್ಭಾವ <sys/syscall.h>
#endif

#ಅನ್ತರ್ಭಾವ "cpuset.h"
#ಅನ್ತರ್ಭಾವ "c.h"

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಪೂರ್ಣಾಙ್ಕ val_to_char(ಪೂರ್ಣಾಙ್ಕ v)
{
	ಯದಿ (v >= 0 && v < 10)
		ಪ್ರತ್ಯಾಗಮ '0' + v;
	ಯದಿ (v >= 10 && v < 16)
		ಪ್ರತ್ಯಾಗಮ ('a' - 10) + v;
	ಪ್ರತ್ಯಾಗಮ -1;
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಪೂರ್ಣಾಙ್ಕ char_to_val(ಪೂರ್ಣಾಙ್ಕ c)
{
	ಪೂರ್ಣಾಙ್ಕ cl;

	ಯದಿ (c >= '0' && c <= '9')
		ಪ್ರತ್ಯಾಗಮ c - '0';
	cl = tolower(c);
	ಯದಿ (cl >= 'a' && cl <= 'f')
		ಪ್ರತ್ಯಾಗಮ cl + (10 - 'a');
	ಪ್ರತ್ಯಾಗಮ -1;
}

ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *nexttoken(ಧ್ರುವ ವರ್ಣ *q,  ಪೂರ್ಣಾಙ್ಕ sep)
{
	ಯದಿ (q)
		q = strchr(q, sep);
	ಯದಿ (q)
		q++;
	ಪ್ರತ್ಯಾಗಮ q;
}

/*
 * Number of bits in a CPU bitmask on current system
 */
ಪೂರ್ಣಾಙ್ಕ get_max_ಸಂಖ್ಯೆ_of_cpus(ಶೂನ್ಯ)
{
#ifdef SYS_sched_getaffinity
	ಪೂರ್ಣಾಙ್ಕ n, cpus = 2048;
	size_t setsize;
	cpu_set_t *set = cpuset_alloc(cpus, &setsize, ರಿಕ್ತ);

	ಯದಿ (!set)
		ಪ್ರತ್ಯಾಗಮ -1;	/* error */

	ಪ್ರತಿ (;;) {
		CPU_ZERO_S(setsize, set);

		/* the library version does not return size of cpumask_t */
		n = syscall(SYS_sched_getaffinity, 0, setsize, set);

		ಯದಿ (n < 0 && errno == EINVAL && cpus < 1024 * 1024) {
			cpuset_free(set);
			cpus *= 2;
			set = cpuset_alloc(cpus, &setsize, ರಿಕ್ತ);
			ಯದಿ (!set)
				ಪ್ರತ್ಯಾಗಮ -1;	/* error */
			ಅನುವರ್ತ;
		}
		cpuset_free(set);
		ಪ್ರತ್ಯಾಗಮ n * 8;
	}
#endif
	ಪ್ರತ್ಯಾಗಮ -1;
}

/*
 * Allocates a new set for ncpus and returns size in bytes and size in bits
 */
cpu_set_t *cpuset_alloc(ಪೂರ್ಣಾಙ್ಕ ncpus, size_t *setsize, size_t *nbits)
{
	cpu_set_t *set = ರಿಕ್ತ;

	ಯದಿ (ncpus)
		set = CPU_ALLOC(ncpus);
	ಯದಿ (!set)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	ಯದಿ (setsize)
		*setsize = CPU_ALLOC_SIZE(ncpus);
	ಯದಿ (nbits)
		*nbits = cpuset_nbits(CPU_ALLOC_SIZE(ncpus));
	ಪ್ರತ್ಯಾಗಮ set;
}

ಶೂನ್ಯ cpuset_free(cpu_set_t *set)
{
	CPU_FREE(set);
}

#ಯದಿ !HAVE_DECL_CPU_ALLOC
/* Please, use CPU_COUNT_S() macro. This is fallback */
ಪೂರ್ಣಾಙ್ಕ __cpuset_count_s(size_t setsize, ಧ್ರುವ cpu_set_t *set)
{
	ಪೂರ್ಣಾಙ್ಕ s = 0;
	ಧ್ರುವ __cpu_mask *p = set->__bits;
	ಧ್ರುವ __cpu_mask *end = &set->__bits[setsize / ಪರಿಮಾಣ (__cpu_mask)];

	ಯಾವತ್ (p < end) {
		__cpu_mask l = *p++;

		ಯದಿ (l == 0)
			ಅನುವರ್ತ;
# ಯದಿ LONG_BIT > 32
		l = (l & 0x5555555555555555ul) + ((l >> 1) & 0x5555555555555555ul);
		l = (l & 0x3333333333333333ul) + ((l >> 2) & 0x3333333333333333ul);
		l = (l & 0x0f0f0f0f0f0f0f0ful) + ((l >> 4) & 0x0f0f0f0f0f0f0f0ful);
		l = (l & 0x00ff00ff00ff00fful) + ((l >> 8) & 0x00ff00ff00ff00fful);
		l = (l & 0x0000ffff0000fffful) + ((l >> 16) & 0x0000ffff0000fffful);
		l = (l & 0x00000000fffffffful) + ((l >> 32) & 0x00000000fffffffful);
# ಅನ್ಯಥಾ
		l = (l & 0x55555555ul) + ((l >> 1) & 0x55555555ul);
		l = (l & 0x33333333ul) + ((l >> 2) & 0x33333333ul);
		l = (l & 0x0f0f0f0ful) + ((l >> 4) & 0x0f0f0f0ful);
		l = (l & 0x00ff00fful) + ((l >> 8) & 0x00ff00fful);
		l = (l & 0x0000fffful) + ((l >> 16) & 0x0000fffful);
# endif
		s += l;
	}
	ಪ್ರತ್ಯಾಗಮ s;
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
ಸ್ಥಿರ size_t find_next_cpu(size_t start, size_t setsize, cpu_set_t *set)
{
	size_t nbits = cpuset_nbits(setsize);
	ಪ್ರತಿ (; start < nbits; start++)
		ಯದಿ (CPU_ISSET_S(start, setsize, set))
			ಪ್ರತ್ಯಾಗಮ start;
	ಪ್ರತ್ಯಾಗಮ start;
}

/*
 * Returns human readable representation of the cpuset. The output format is
 * a list of CPUs with ranges (for example, "0,1,3-9:3").
 */
ವರ್ಣ *cpulist_create(ವರ್ಣ *str, size_t len,
			cpu_set_t *set, size_t setsize)
{
	ವರ್ಣ *ptr = str;
	ಪೂರ್ಣಾಙ್ಕ entry_made = 0;
	size_t max = cpuset_nbits(setsize);
	size_t a = 0;  /* min for cpu range */
	size_t next = 0;  /* where to start looking for next cpu */

	ಯಾವತ್ ((a = find_next_cpu(next, setsize, set)) < max) {
		ಪೂರ್ಣಾಙ್ಕ rlen;
		next = find_next_cpu(a + 1, setsize, set);
		ಯದಿ (next == max) {
			rlen = snprintf(ptr, len, "%zu,", a);
		} ಅನ್ಯಥಾ {
			/* Extend range as long as we have the same stride. */
			size_t b = next;
			size_t s = b - a;
			ಯಾವತ್ (((next = find_next_cpu(b + 1, setsize, set)) <
				max) && next - b == s) {
				b = next;
			}
			ಯದಿ (b - a == s) {
				/*
				 * Only print one CPU.  Hope the next one can
				 * be put in the next range.
				 */
				rlen = snprintf(ptr, len, "%zu,", a);
				next = b;
			} ಅನ್ಯಥಾ ಯದಿ (s == 1) {
				rlen = snprintf(ptr, len, "%zu-%zu,", a, b);
			} ಅನ್ಯಥಾ {
				rlen = snprintf(ptr, len, "%zu-%zu:%zu,",
						a, b, s);
			}
		}
		ಯದಿ (rlen < 0 || (size_t) rlen >= len)
			ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
		ptr += rlen;
		len -= rlen;
		entry_made = 1;
	}
	ptr -= entry_made;
	*ptr = '\0';

	ಪ್ರತ್ಯಾಗಮ str;
}

/*
 * cpulist_flat_create - Convert cpumask to flat comma-separated list
 * Output: ONLY single CPUs, NO ranges (e.g. "0,1,2,3,4")
 */
ವರ್ಣ *cpulist_flat_create(ವರ್ಣ *str, size_t len,
			    cpu_set_t *set, size_t setsize)
{
	ವರ್ಣ *ptr = str;
	ಪೂರ್ಣಾಙ್ಕ entry_made = 0;
	size_t max = cpuset_nbits(setsize);
	size_t cpu;
	ಪೂರ್ಣಾಙ್ಕ rlen;

	ಪ್ರತಿ (cpu = 0; cpu < max; cpu++) {
		ಯದಿ (!CPU_ISSET_S(cpu, setsize, set))
			ಅನುವರ್ತ;

		rlen = snprintf(ptr, len, "%zu,", cpu);
		ಯದಿ (rlen < 0 || (size_t)rlen >= len)
			ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

		ptr += rlen;
		len -= rlen;
		entry_made = 1;
	}
	ptr -= entry_made;
	*ptr = '\0';

	ಪ್ರತ್ಯಾಗಮ str;
}


/*
 * Returns string with CPU mask.
 */
ವರ್ಣ *cpumask_create(ವರ್ಣ *str, size_t len,
			cpu_set_t *set, size_t setsize)
{
	ವರ್ಣ *ptr = str;
	ವರ್ಣ *ret = ರಿಕ್ತ;
	ಪೂರ್ಣಾಙ್ಕ cpu;

	ಪ್ರತಿ (cpu = cpuset_nbits(setsize) - 4; cpu >= 0; cpu -= 4) {
		ವರ್ಣ val = 0;

		ಯದಿ (len == (size_t) (ptr - str))
			ಭಙ್ಗ;

		ಯದಿ (CPU_ISSET_S(cpu, setsize, set))
			val |= 1;
		ಯದಿ (CPU_ISSET_S(cpu + 1, setsize, set))
			val |= 2;
		ಯದಿ (CPU_ISSET_S(cpu + 2, setsize, set))
			val |= 4;
		ಯದಿ (CPU_ISSET_S(cpu + 3, setsize, set))
			val |= 8;

		ಯದಿ (!ret && val)
			ret = ptr;
		*ptr++ = val_to_char(val);
	}
	*ptr = '\0';
	ಪ್ರತ್ಯಾಗಮ ret ? ret : ptr - 1;
}

/*
 * Parses string with CPUs mask.
 */
ಪೂರ್ಣಾಙ್ಕ cpumask_parse(ಧ್ರುವ ವರ್ಣ *str, cpu_set_t *set, size_t setsize)
{
	ಪೂರ್ಣಾಙ್ಕ len = strlen(str);
	ಧ್ರುವ ವರ್ಣ *ptr = str + len - 1;
	ಪೂರ್ಣಾಙ್ಕ cpu = 0;

	/* skip 0x, it's all hex anyway */
	ಯದಿ (len > 1 && !memcmp(str, "0x", 2L))
		str += 2;

	CPU_ZERO_S(setsize, set);

	ಯಾವತ್ (ptr >= str) {
		ವರ್ಣ val;

		/* cpu masks in /sys uses comma as a separator */
		ಯದಿ (*ptr == ',')
			ptr--;

		val = char_to_val(*ptr);
		ಯದಿ (val == (ವರ್ಣ) -1)
			ಪ್ರತ್ಯಾಗಮ -1;
		ಯದಿ (val & 1)
			CPU_SET_S(cpu, setsize, set);
		ಯದಿ (val & 2)
			CPU_SET_S(cpu + 1, setsize, set);
		ಯದಿ (val & 4)
			CPU_SET_S(cpu + 2, setsize, set);
		ಯದಿ (val & 8)
			CPU_SET_S(cpu + 3, setsize, set);
		ptr--;
		cpu += 4;
	}

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ nextnumber(ಧ್ರುವ ವರ್ಣ *str, ವರ್ಣ **end, unsigned ಪೂರ್ಣಾಙ್ಕ *result)
{
	errno = 0;
	ಯದಿ (str == ರಿಕ್ತ || *str == '\0' || !isdigit(*str))
		ಪ್ರತ್ಯಾಗಮ -EINVAL;
	*result = (unsigned ಪೂರ್ಣಾಙ್ಕ) strtoul(str, end, 10);
	ಯದಿ (errno)
		ಪ್ರತ್ಯಾಗಮ -errno;
	ಯದಿ (str == *end)
		ಪ್ರತ್ಯಾಗಮ -EINVAL;
	ಪ್ರತ್ಯಾಗಮ 0;
}

/*
 * Parses string with list of CPU ranges.
 * Returns 0 on success.
 * Returns 1 on error.
 * Returns 2 if fail is set and a cpu number passed in the list doesn't fit
 * into the cpu_set. If fail is not set cpu numbers that do not fit are
 * ignored and 0 is returned instead.
 */
ಪೂರ್ಣಾಙ್ಕ cpulist_parse(ಧ್ರುವ ವರ್ಣ *str, cpu_set_t *set, size_t setsize, ಪೂರ್ಣಾಙ್ಕ fail)
{
	ಧ್ರುವ size_t max = cpuset_nbits(setsize);
	ಧ್ರುವ ವರ್ಣ *p, *q;
	ವರ್ಣ *end = ರಿಕ್ತ;

	q = str;
	CPU_ZERO_S(setsize, set);

	ಯಾವತ್ (p = q, q = nexttoken(q, ','), p) {
		unsigned ಪೂರ್ಣಾಙ್ಕ a;	/* beginning of range */
		unsigned ಪೂರ್ಣಾಙ್ಕ b;	/* end of range */
		unsigned ಪೂರ್ಣಾಙ್ಕ s;	/* stride */
		ಧ್ರುವ ವರ್ಣ *c1, *c2;

		ಯದಿ (nextnumber(p, &end, &a) != 0 ||
			(*end && *end != ',' && *end != '-'))
			ಪ್ರತ್ಯಾಗಮ 1;
		b = a;
		s = 1;
		p = end;

		c1 = nexttoken(p, '-');
		c2 = nexttoken(p, ',');

		ಯದಿ (c1 != ರಿಕ್ತ && (c2 == ರಿಕ್ತ || c1 < c2)) {
			ಯದಿ (nextnumber(c1, &end, &b) != 0 ||
				(*end && *end != ',' && *end != ':'))
				ಪ್ರತ್ಯಾಗಮ 1;

			c1 = nexttoken(end, ':');

			ಯದಿ (c1 != ರಿಕ್ತ && (c2 == ರಿಕ್ತ || c1 < c2)) {
				ಯದಿ (nextnumber(c1, &end, &s) != 0 ||
					(*end && *end != ','))
					ಪ್ರತ್ಯಾಗಮ 1;
				ಯದಿ (s == 0)
					ಪ್ರತ್ಯಾಗಮ 1;
			}
		}

		ಯದಿ (!(a <= b))
			ಪ್ರತ್ಯಾಗಮ 1;
		ಯಾವತ್ (a <= b) {
			ಯದಿ (a >= max) {
				ಯದಿ (fail)
					ಪ್ರತ್ಯಾಗಮ 2;
				ಅನ್ಯಥಾ
					ಭಙ್ಗ;
			}
			CPU_SET_S(a, setsize, set);
			a += s;
		}
	}

	ಯದಿ (end && *end)
		ಪ್ರತ್ಯಾಗಮ 1;
	ಪ್ರತ್ಯಾಗಮ 0;
}

#ifdef TEST_PROGRAM_CPUSET

#ಅನ್ತರ್ಭಾವ <getopt.h>

ಪೂರ್ಣಾಙ್ಕ main(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ *argv[])
{
	cpu_set_t *set;
	size_t setsize, buflen, nbits;
	ವರ್ಣ *buf, *mask = ರಿಕ್ತ, *range = ರಿಕ್ತ;
	ಪೂರ್ಣಾಙ್ಕ ncpus = 2048, rc, c;

	ಸ್ಥಿರ ಧ್ರುವ ಸಂರಚನಾ option longopts[] = {
	    { "ncpus", 1, ರಿಕ್ತ, 'n' },
	    { "mask",  1, ರಿಕ್ತ, 'm' },
	    { "range", 1, ರಿಕ್ತ, 'r' },
	    { ರಿಕ್ತ,    0, ರಿಕ್ತ, 0 }
	};

	ಯಾವತ್ ((c = getopt_long(argc, argv, "n:m:r:", longopts, ರಿಕ್ತ)) != -1) {
		ವಿಕಲ್ಪ(c) {
		ಪಕ್ಷ 'n':
			ncpus = atoi(optarg);
			ಭಙ್ಗ;
		ಪಕ್ಷ 'm':
			mask = optarg;
			ಭಙ್ಗ;
		ಪಕ್ಷ 'r':
			range = optarg;
			ಭಙ್ಗ;
		ಸ್ವಭಾವ:
			ಗಚ್ಛ usage_err;
		}
	}

	ಯದಿ (!mask && !range)
		ಗಚ್ಛ usage_err;

	set = cpuset_alloc(ncpus, &setsize, &nbits);
	ಯದಿ (!set)
		err(EXIT_FAILURE, "failed to allocate cpu set");

	/*
	fprintf(stderr, "ncpus: %d, cpuset bits: %zd, cpuset bytes: %zd\n",
			ncpus, nbits, setsize);
	*/

	buflen = 7 * nbits;
	buf = malloc(buflen);
	ಯದಿ (!buf)
		err(EXIT_FAILURE, "failed to allocate cpu set buffer");

	ಯದಿ (mask)
		rc = cpumask_parse(mask, set, setsize);
	ಅನ್ಯಥಾ
		rc = cpulist_parse(range, set, setsize, 0);

	ಯದಿ (rc)
		errx(EXIT_FAILURE, "failed to parse string: %s", mask ? : range);

	printf("%-15s = %15s ", mask ? : range,
				cpumask_create(buf, buflen, set, setsize));
	printf("[%s]\n", cpulist_create(buf, buflen, set, setsize));

	ಮುಕ್ತ(buf);
	cpuset_free(set);

	ಪ್ರತ್ಯಾಗಮ EXIT_SUCCESS;

usage_err:
	fprintf(stderr,
		"usage: %s [--ncpus <num>] --mask <mask> | --range <list>\n",
		program_invocation_short_name);
	exit(EXIT_FAILURE);
}
#endif
