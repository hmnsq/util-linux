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

#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <unistd.h>
#અન્તર્ભાવ <sched.h>
#અન્તર્ભાવ <errno.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <ctype.h>
#ifdef HAVE_SYS_SYSCALL_H
#અન્તર્ભાવ <sys/syscall.h>
#endif

#અન્તર્ભાવ "cpuset.h"
#અન્તર્ભાવ "c.h"

સ્થિર અંતઃરેખ પૂર્ણાંક val_to_char(પૂર્ણાંક v)
{
	જો (v >= 0 && v < 10)
		પાછા '0' + v;
	જો (v >= 10 && v < 16)
		પાછા ('a' - 10) + v;
	પાછા -1;
}

સ્થિર અંતઃરેખ પૂર્ણાંક char_to_val(પૂર્ણાંક c)
{
	પૂર્ણાંક cl;

	જો (c >= '0' && c <= '9')
		પાછા c - '0';
	cl = tolower(c);
	જો (cl >= 'a' && cl <= 'f')
		પાછા cl + (10 - 'a');
	પાછા -1;
}

સ્થિર અચલ અક્ષર *nexttoken(અચલ અક્ષર *q,  પૂર્ણાંક sep)
{
	જો (q)
		q = strchr(q, sep);
	જો (q)
		q++;
	પાછા q;
}

/*
 * Number of bits in a CPU bitmask on current system
 */
પૂર્ણાંક get_max_સંખ્યા_of_cpus(શૂન્ય)
{
#ifdef SYS_sched_getaffinity
	પૂર્ણાંક n, cpus = 2048;
	size_t setsize;
	cpu_set_t *set = cpuset_alloc(cpus, &setsize, ખાલી);

	જો (!set)
		પાછા -1;	/* error */

	માટે (;;) {
		CPU_ZERO_S(setsize, set);

		/* the library version does not return size of cpumask_t */
		n = syscall(SYS_sched_getaffinity, 0, setsize, set);

		જો (n < 0 && errno == EINVAL && cpus < 1024 * 1024) {
			cpuset_free(set);
			cpus *= 2;
			set = cpuset_alloc(cpus, &setsize, ખાલી);
			જો (!set)
				પાછા -1;	/* error */
			ચાલુ;
		}
		cpuset_free(set);
		પાછા n * 8;
	}
#endif
	પાછા -1;
}

/*
 * Allocates a new set for ncpus and returns size in bytes and size in bits
 */
cpu_set_t *cpuset_alloc(પૂર્ણાંક ncpus, size_t *setsize, size_t *nbits)
{
	cpu_set_t *set = ખાલી;

	જો (ncpus)
		set = CPU_ALLOC(ncpus);
	જો (!set)
		પાછા ખાલી;
	જો (setsize)
		*setsize = CPU_ALLOC_SIZE(ncpus);
	જો (nbits)
		*nbits = cpuset_nbits(CPU_ALLOC_SIZE(ncpus));
	પાછા set;
}

શૂન્ય cpuset_free(cpu_set_t *set)
{
	CPU_FREE(set);
}

#જો !HAVE_DECL_CPU_ALLOC
/* Please, use CPU_COUNT_S() macro. This is fallback */
પૂર્ણાંક __cpuset_count_s(size_t setsize, અચલ cpu_set_t *set)
{
	પૂર્ણાંક s = 0;
	અચલ __cpu_mask *p = set->__bits;
	અચલ __cpu_mask *end = &set->__bits[setsize / કદ (__cpu_mask)];

	જ્યારે (p < end) {
		__cpu_mask l = *p++;

		જો (l == 0)
			ચાલુ;
# જો LONG_BIT > 32
		l = (l & 0x5555555555555555ul) + ((l >> 1) & 0x5555555555555555ul);
		l = (l & 0x3333333333333333ul) + ((l >> 2) & 0x3333333333333333ul);
		l = (l & 0x0f0f0f0f0f0f0f0ful) + ((l >> 4) & 0x0f0f0f0f0f0f0f0ful);
		l = (l & 0x00ff00ff00ff00fful) + ((l >> 8) & 0x00ff00ff00ff00fful);
		l = (l & 0x0000ffff0000fffful) + ((l >> 16) & 0x0000ffff0000fffful);
		l = (l & 0x00000000fffffffful) + ((l >> 32) & 0x00000000fffffffful);
# નહીંતર
		l = (l & 0x55555555ul) + ((l >> 1) & 0x55555555ul);
		l = (l & 0x33333333ul) + ((l >> 2) & 0x33333333ul);
		l = (l & 0x0f0f0f0ful) + ((l >> 4) & 0x0f0f0f0ful);
		l = (l & 0x00ff00fful) + ((l >> 8) & 0x00ff00fful);
		l = (l & 0x0000fffful) + ((l >> 16) & 0x0000fffful);
# endif
		s += l;
	}
	પાછા s;
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
સ્થિર size_t find_next_cpu(size_t start, size_t setsize, cpu_set_t *set)
{
	size_t nbits = cpuset_nbits(setsize);
	માટે (; start < nbits; start++)
		જો (CPU_ISSET_S(start, setsize, set))
			પાછા start;
	પાછા start;
}

/*
 * Returns human readable representation of the cpuset. The output format is
 * a list of CPUs with ranges (for example, "0,1,3-9:3").
 */
અક્ષર *cpulist_create(અક્ષર *str, size_t len,
			cpu_set_t *set, size_t setsize)
{
	અક્ષર *ptr = str;
	પૂર્ણાંક entry_made = 0;
	size_t max = cpuset_nbits(setsize);
	size_t a = 0;  /* min for cpu range */
	size_t next = 0;  /* where to start looking for next cpu */

	જ્યારે ((a = find_next_cpu(next, setsize, set)) < max) {
		પૂર્ણાંક rlen;
		next = find_next_cpu(a + 1, setsize, set);
		જો (next == max) {
			rlen = snprintf(ptr, len, "%zu,", a);
		} નહીંતર {
			/* Extend range as long as we have the same stride. */
			size_t b = next;
			size_t s = b - a;
			જ્યારે (((next = find_next_cpu(b + 1, setsize, set)) <
				max) && next - b == s) {
				b = next;
			}
			જો (b - a == s) {
				/*
				 * Only print one CPU.  Hope the next one can
				 * be put in the next range.
				 */
				rlen = snprintf(ptr, len, "%zu,", a);
				next = b;
			} નહીંતર જો (s == 1) {
				rlen = snprintf(ptr, len, "%zu-%zu,", a, b);
			} નહીંતર {
				rlen = snprintf(ptr, len, "%zu-%zu:%zu,",
						a, b, s);
			}
		}
		જો (rlen < 0 || (size_t) rlen >= len)
			પાછા ખાલી;
		ptr += rlen;
		len -= rlen;
		entry_made = 1;
	}
	ptr -= entry_made;
	*ptr = '\0';

	પાછા str;
}

/*
 * cpulist_flat_create - Convert cpumask to flat comma-separated list
 * Output: ONLY single CPUs, NO ranges (e.g. "0,1,2,3,4")
 */
અક્ષર *cpulist_flat_create(અક્ષર *str, size_t len,
			    cpu_set_t *set, size_t setsize)
{
	અક્ષર *ptr = str;
	પૂર્ણાંક entry_made = 0;
	size_t max = cpuset_nbits(setsize);
	size_t cpu;
	પૂર્ણાંક rlen;

	માટે (cpu = 0; cpu < max; cpu++) {
		જો (!CPU_ISSET_S(cpu, setsize, set))
			ચાલુ;

		rlen = snprintf(ptr, len, "%zu,", cpu);
		જો (rlen < 0 || (size_t)rlen >= len)
			પાછા ખાલી;

		ptr += rlen;
		len -= rlen;
		entry_made = 1;
	}
	ptr -= entry_made;
	*ptr = '\0';

	પાછા str;
}


/*
 * Returns string with CPU mask.
 */
અક્ષર *cpumask_create(અક્ષર *str, size_t len,
			cpu_set_t *set, size_t setsize)
{
	અક્ષર *ptr = str;
	અક્ષર *ret = ખાલી;
	પૂર્ણાંક cpu;

	માટે (cpu = cpuset_nbits(setsize) - 4; cpu >= 0; cpu -= 4) {
		અક્ષર val = 0;

		જો (len == (size_t) (ptr - str))
			તોડો;

		જો (CPU_ISSET_S(cpu, setsize, set))
			val |= 1;
		જો (CPU_ISSET_S(cpu + 1, setsize, set))
			val |= 2;
		જો (CPU_ISSET_S(cpu + 2, setsize, set))
			val |= 4;
		જો (CPU_ISSET_S(cpu + 3, setsize, set))
			val |= 8;

		જો (!ret && val)
			ret = ptr;
		*ptr++ = val_to_char(val);
	}
	*ptr = '\0';
	પાછા ret ? ret : ptr - 1;
}

/*
 * Parses string with CPUs mask.
 */
પૂર્ણાંક cpumask_parse(અચલ અક્ષર *str, cpu_set_t *set, size_t setsize)
{
	પૂર્ણાંક len = strlen(str);
	અચલ અક્ષર *ptr = str + len - 1;
	પૂર્ણાંક cpu = 0;

	/* skip 0x, it's all hex anyway */
	જો (len > 1 && !memcmp(str, "0x", 2L))
		str += 2;

	CPU_ZERO_S(setsize, set);

	જ્યારે (ptr >= str) {
		અક્ષર val;

		/* cpu masks in /sys uses comma as a separator */
		જો (*ptr == ',')
			ptr--;

		val = char_to_val(*ptr);
		જો (val == (અક્ષર) -1)
			પાછા -1;
		જો (val & 1)
			CPU_SET_S(cpu, setsize, set);
		જો (val & 2)
			CPU_SET_S(cpu + 1, setsize, set);
		જો (val & 4)
			CPU_SET_S(cpu + 2, setsize, set);
		જો (val & 8)
			CPU_SET_S(cpu + 3, setsize, set);
		ptr--;
		cpu += 4;
	}

	પાછા 0;
}

સ્થિર પૂર્ણાંક nextnumber(અચલ અક્ષર *str, અક્ષર **end, unsigned પૂર્ણાંક *result)
{
	errno = 0;
	જો (str == ખાલી || *str == '\0' || !isdigit(*str))
		પાછા -EINVAL;
	*result = (unsigned પૂર્ણાંક) strtoul(str, end, 10);
	જો (errno)
		પાછા -errno;
	જો (str == *end)
		પાછા -EINVAL;
	પાછા 0;
}

/*
 * Parses string with list of CPU ranges.
 * Returns 0 on success.
 * Returns 1 on error.
 * Returns 2 if fail is set and a cpu number passed in the list doesn't fit
 * into the cpu_set. If fail is not set cpu numbers that do not fit are
 * ignored and 0 is returned instead.
 */
પૂર્ણાંક cpulist_parse(અચલ અક્ષર *str, cpu_set_t *set, size_t setsize, પૂર્ણાંક fail)
{
	અચલ size_t max = cpuset_nbits(setsize);
	અચલ અક્ષર *p, *q;
	અક્ષર *end = ખાલી;

	q = str;
	CPU_ZERO_S(setsize, set);

	જ્યારે (p = q, q = nexttoken(q, ','), p) {
		unsigned પૂર્ણાંક a;	/* beginning of range */
		unsigned પૂર્ણાંક b;	/* end of range */
		unsigned પૂર્ણાંક s;	/* stride */
		અચલ અક્ષર *c1, *c2;

		જો (nextnumber(p, &end, &a) != 0 ||
			(*end && *end != ',' && *end != '-'))
			પાછા 1;
		b = a;
		s = 1;
		p = end;

		c1 = nexttoken(p, '-');
		c2 = nexttoken(p, ',');

		જો (c1 != ખાલી && (c2 == ખાલી || c1 < c2)) {
			જો (nextnumber(c1, &end, &b) != 0 ||
				(*end && *end != ',' && *end != ':'))
				પાછા 1;

			c1 = nexttoken(end, ':');

			જો (c1 != ખાલી && (c2 == ખાલી || c1 < c2)) {
				જો (nextnumber(c1, &end, &s) != 0 ||
					(*end && *end != ','))
					પાછા 1;
				જો (s == 0)
					પાછા 1;
			}
		}

		જો (!(a <= b))
			પાછા 1;
		જ્યારે (a <= b) {
			જો (a >= max) {
				જો (fail)
					પાછા 2;
				નહીંતર
					તોડો;
			}
			CPU_SET_S(a, setsize, set);
			a += s;
		}
	}

	જો (end && *end)
		પાછા 1;
	પાછા 0;
}

#ifdef TEST_PROGRAM_CPUSET

#અન્તર્ભાવ <getopt.h>

પૂર્ણાંક મુખ્ય(પૂર્ણાંક argc, અક્ષર *argv[])
{
	cpu_set_t *set;
	size_t setsize, buflen, nbits;
	અક્ષર *buf, *mask = ખાલી, *range = ખાલી;
	પૂર્ણાંક ncpus = 2048, rc, c;

	સ્થિર અચલ સંરચના option longopts[] = {
	    { "ncpus", 1, ખાલી, 'n' },
	    { "mask",  1, ખાલી, 'm' },
	    { "range", 1, ખાલી, 'r' },
	    { ખાલી,    0, ખાલી, 0 }
	};

	જ્યારે ((c = getopt_long(argc, argv, "n:m:r:", longopts, ખાલી)) != -1) {
		પસંદગી(c) {
		સ્થિતિ 'n':
			ncpus = atoi(optarg);
			તોડો;
		સ્થિતિ 'm':
			mask = optarg;
			તોડો;
		સ્થિતિ 'r':
			range = optarg;
			તોડો;
		મૂળભૂત:
			જાઓ usage_err;
		}
	}

	જો (!mask && !range)
		જાઓ usage_err;

	set = cpuset_alloc(ncpus, &setsize, &nbits);
	જો (!set)
		err(EXIT_FAILURE, "failed to allocate cpu set");

	/*
	fprintf(stderr, "ncpus: %d, cpuset bits: %zd, cpuset bytes: %zd\n",
			ncpus, nbits, setsize);
	*/

	buflen = 7 * nbits;
	buf = malloc(buflen);
	જો (!buf)
		err(EXIT_FAILURE, "failed to allocate cpu set buffer");

	જો (mask)
		rc = cpumask_parse(mask, set, setsize);
	નહીંતર
		rc = cpulist_parse(range, set, setsize, 0);

	જો (rc)
		errx(EXIT_FAILURE, "failed to parse string: %s", mask ? : range);

	printf("%-15s = %15s ", mask ? : range,
				cpumask_create(buf, buflen, set, setsize));
	printf("[%s]\n", cpulist_create(buf, buflen, set, setsize));

	મુક્ત(buf);
	cpuset_free(set);

	પાછા EXIT_SUCCESS;

usage_err:
	fprintf(stderr,
		"usage: %s [--ncpus <num>] --mask <mask> | --range <list>\n",
		program_invocation_short_name);
	exit(EXIT_FAILURE);
}
#endif
