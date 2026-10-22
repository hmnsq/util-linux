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

#ضمن <stdio.h>
#ضمن <stdlib.h>
#ضمن <unistd.h>
#ضمن <sched.h>
#ضمن <errno.h>
#ضمن <string.h>
#ضمن <ctype.h>
#ifdef HAVE_SYS_SYSCALL_H
#ضمن <sys/syscall.h>
#endif

#ضمن "cpuset.h"
#ضمن "c.h"

ساكن مضمن صحيح val_to_char(صحيح v)
{
	إذا (v >= 0 && v < 10)
		أرجع '0' + v;
	إذا (v >= 10 && v < 16)
		أرجع ('a' - 10) + v;
	أرجع -1;
}

ساكن مضمن صحيح char_to_val(صحيح c)
{
	صحيح cl;

	إذا (c >= '0' && c <= '9')
		أرجع c - '0';
	cl = tolower(c);
	إذا (cl >= 'a' && cl <= 'f')
		أرجع cl + (10 - 'a');
	أرجع -1;
}

ساكن ثابت حرف *nexttoken(ثابت حرف *q,  صحيح sep)
{
	إذا (q)
		q = strchr(q, sep);
	إذا (q)
		q++;
	أرجع q;
}

/*
 * Number of bits in a CPU bitmask on current system
 */
صحيح get_max_سان_of_cpus(فراغ)
{
#ifdef SYS_sched_getaffinity
	صحيح n, cpus = 2048;
	size_t setsize;
	cpu_set_t *set = cpuset_alloc(cpus, &setsize, عدم);

	إذا (!set)
		أرجع -1;	/* error */

	لكل (;;) {
		CPU_ZERO_S(setsize, set);

		/* the library version does not return size of cpumask_t */
		n = syscall(SYS_sched_getaffinity, 0, setsize, set);

		إذا (n < 0 && errno == EINVAL && cpus < 1024 * 1024) {
			cpuset_free(set);
			cpus *= 2;
			set = cpuset_alloc(cpus, &setsize, عدم);
			إذا (!set)
				أرجع -1;	/* error */
			استمر;
		}
		cpuset_free(set);
		أرجع n * 8;
	}
#endif
	أرجع -1;
}

/*
 * Allocates a new set for ncpus and returns size in bytes and size in bits
 */
cpu_set_t *cpuset_alloc(صحيح ncpus, size_t *setsize, size_t *nbits)
{
	cpu_set_t *set = عدم;

	إذا (ncpus)
		set = CPU_ALLOC(ncpus);
	إذا (!set)
		أرجع عدم;
	إذا (setsize)
		*setsize = CPU_ALLOC_SIZE(ncpus);
	إذا (nbits)
		*nbits = cpuset_nbits(CPU_ALLOC_SIZE(ncpus));
	أرجع set;
}

فراغ cpuset_free(cpu_set_t *set)
{
	CPU_FREE(set);
}

#إذا !HAVE_DECL_CPU_ALLOC
/* Please, use CPU_COUNT_S() macro. This is fallback */
صحيح __cpuset_count_s(size_t setsize, ثابت cpu_set_t *set)
{
	صحيح s = 0;
	ثابت __cpu_mask *p = set->__bits;
	ثابت __cpu_mask *end = &set->__bits[setsize / حجم (__cpu_mask)];

	طالما (p < end) {
		__cpu_mask l = *p++;

		إذا (l == 0)
			استمر;
# إذا LONG_BIT > 32
		l = (l & 0x5555555555555555ul) + ((l >> 1) & 0x5555555555555555ul);
		l = (l & 0x3333333333333333ul) + ((l >> 2) & 0x3333333333333333ul);
		l = (l & 0x0f0f0f0f0f0f0f0ful) + ((l >> 4) & 0x0f0f0f0f0f0f0f0ful);
		l = (l & 0x00ff00ff00ff00fful) + ((l >> 8) & 0x00ff00ff00ff00fful);
		l = (l & 0x0000ffff0000fffful) + ((l >> 16) & 0x0000ffff0000fffful);
		l = (l & 0x00000000fffffffful) + ((l >> 32) & 0x00000000fffffffful);
# وإلا
		l = (l & 0x55555555ul) + ((l >> 1) & 0x55555555ul);
		l = (l & 0x33333333ul) + ((l >> 2) & 0x33333333ul);
		l = (l & 0x0f0f0f0ful) + ((l >> 4) & 0x0f0f0f0ful);
		l = (l & 0x00ff00fful) + ((l >> 8) & 0x00ff00fful);
		l = (l & 0x0000fffful) + ((l >> 16) & 0x0000fffful);
# endif
		s += l;
	}
	أرجع s;
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
ساكن size_t find_next_cpu(size_t start, size_t setsize, cpu_set_t *set)
{
	size_t nbits = cpuset_nbits(setsize);
	لكل (; start < nbits; start++)
		إذا (CPU_ISSET_S(start, setsize, set))
			أرجع start;
	أرجع start;
}

/*
 * Returns human readable representation of the cpuset. The output format is
 * a list of CPUs with ranges (for example, "0,1,3-9:3").
 */
حرف *cpulist_create(حرف *str, size_t len,
			cpu_set_t *set, size_t setsize)
{
	حرف *ptr = str;
	صحيح entry_made = 0;
	size_t max = cpuset_nbits(setsize);
	size_t a = 0;  /* min for cpu range */
	size_t next = 0;  /* where to start looking for next cpu */

	طالما ((a = find_next_cpu(next, setsize, set)) < max) {
		صحيح rlen;
		next = find_next_cpu(a + 1, setsize, set);
		إذا (next == max) {
			rlen = snprintf(ptr, len, "%zu,", a);
		} وإلا {
			/* Extend range as long as we have the same stride. */
			size_t b = next;
			size_t s = b - a;
			طالما (((next = find_next_cpu(b + 1, setsize, set)) <
				max) && next - b == s) {
				b = next;
			}
			إذا (b - a == s) {
				/*
				 * Only print one CPU.  Hope the next one can
				 * be put in the next range.
				 */
				rlen = snprintf(ptr, len, "%zu,", a);
				next = b;
			} وإلا إذا (s == 1) {
				rlen = snprintf(ptr, len, "%zu-%zu,", a, b);
			} وإلا {
				rlen = snprintf(ptr, len, "%zu-%zu:%zu,",
						a, b, s);
			}
		}
		إذا (rlen < 0 || (size_t) rlen >= len)
			أرجع عدم;
		ptr += rlen;
		len -= rlen;
		entry_made = 1;
	}
	ptr -= entry_made;
	*ptr = '\0';

	أرجع str;
}

/*
 * cpulist_flat_create - Convert cpumask to flat comma-separated list
 * Output: ONLY single CPUs, NO ranges (e.g. "0,1,2,3,4")
 */
حرف *cpulist_flat_create(حرف *str, size_t len,
			    cpu_set_t *set, size_t setsize)
{
	حرف *ptr = str;
	صحيح entry_made = 0;
	size_t max = cpuset_nbits(setsize);
	size_t cpu;
	صحيح rlen;

	لكل (cpu = 0; cpu < max; cpu++) {
		إذا (!CPU_ISSET_S(cpu, setsize, set))
			استمر;

		rlen = snprintf(ptr, len, "%zu,", cpu);
		إذا (rlen < 0 || (size_t)rlen >= len)
			أرجع عدم;

		ptr += rlen;
		len -= rlen;
		entry_made = 1;
	}
	ptr -= entry_made;
	*ptr = '\0';

	أرجع str;
}


/*
 * Returns string with CPU mask.
 */
حرف *cpumask_create(حرف *str, size_t len,
			cpu_set_t *set, size_t setsize)
{
	حرف *ptr = str;
	حرف *ret = عدم;
	صحيح cpu;

	لكل (cpu = cpuset_nbits(setsize) - 4; cpu >= 0; cpu -= 4) {
		حرف val = 0;

		إذا (len == (size_t) (ptr - str))
			اكسر;

		إذا (CPU_ISSET_S(cpu, setsize, set))
			val |= 1;
		إذا (CPU_ISSET_S(cpu + 1, setsize, set))
			val |= 2;
		إذا (CPU_ISSET_S(cpu + 2, setsize, set))
			val |= 4;
		إذا (CPU_ISSET_S(cpu + 3, setsize, set))
			val |= 8;

		إذا (!ret && val)
			ret = ptr;
		*ptr++ = val_to_char(val);
	}
	*ptr = '\0';
	أرجع ret ? ret : ptr - 1;
}

/*
 * Parses string with CPUs mask.
 */
صحيح cpumask_parse(ثابت حرف *str, cpu_set_t *set, size_t setsize)
{
	صحيح len = strlen(str);
	ثابت حرف *ptr = str + len - 1;
	صحيح cpu = 0;

	/* skip 0x, it's all hex anyway */
	إذا (len > 1 && !memcmp(str, "0x", 2L))
		str += 2;

	CPU_ZERO_S(setsize, set);

	طالما (ptr >= str) {
		حرف val;

		/* cpu masks in /sys uses comma as a separator */
		إذا (*ptr == ',')
			ptr--;

		val = char_to_val(*ptr);
		إذا (val == (حرف) -1)
			أرجع -1;
		إذا (val & 1)
			CPU_SET_S(cpu, setsize, set);
		إذا (val & 2)
			CPU_SET_S(cpu + 1, setsize, set);
		إذا (val & 4)
			CPU_SET_S(cpu + 2, setsize, set);
		إذا (val & 8)
			CPU_SET_S(cpu + 3, setsize, set);
		ptr--;
		cpu += 4;
	}

	أرجع 0;
}

ساكن صحيح nextnumber(ثابت حرف *str, حرف **end, unsigned صحيح *result)
{
	errno = 0;
	إذا (str == عدم || *str == '\0' || !isdigit(*str))
		أرجع -EINVAL;
	*result = (unsigned صحيح) strtoul(str, end, 10);
	إذا (errno)
		أرجع -errno;
	إذا (str == *end)
		أرجع -EINVAL;
	أرجع 0;
}

/*
 * Parses string with list of CPU ranges.
 * Returns 0 on success.
 * Returns 1 on error.
 * Returns 2 if fail is set and a cpu number passed in the list doesn't fit
 * into the cpu_set. If fail is not set cpu numbers that do not fit are
 * ignored and 0 is returned instead.
 */
صحيح cpulist_parse(ثابت حرف *str, cpu_set_t *set, size_t setsize, صحيح fail)
{
	ثابت size_t max = cpuset_nbits(setsize);
	ثابت حرف *p, *q;
	حرف *end = عدم;

	q = str;
	CPU_ZERO_S(setsize, set);

	طالما (p = q, q = nexttoken(q, ','), p) {
		unsigned صحيح a;	/* beginning of range */
		unsigned صحيح b;	/* end of range */
		unsigned صحيح s;	/* stride */
		ثابت حرف *c1, *c2;

		إذا (nextnumber(p, &end, &a) != 0 ||
			(*end && *end != ',' && *end != '-'))
			أرجع 1;
		b = a;
		s = 1;
		p = end;

		c1 = nexttoken(p, '-');
		c2 = nexttoken(p, ',');

		إذا (c1 != عدم && (c2 == عدم || c1 < c2)) {
			إذا (nextnumber(c1, &end, &b) != 0 ||
				(*end && *end != ',' && *end != ':'))
				أرجع 1;

			c1 = nexttoken(end, ':');

			إذا (c1 != عدم && (c2 == عدم || c1 < c2)) {
				إذا (nextnumber(c1, &end, &s) != 0 ||
					(*end && *end != ','))
					أرجع 1;
				إذا (s == 0)
					أرجع 1;
			}
		}

		إذا (!(a <= b))
			أرجع 1;
		طالما (a <= b) {
			إذا (a >= max) {
				إذا (fail)
					أرجع 2;
				وإلا
					اكسر;
			}
			CPU_SET_S(a, setsize, set);
			a += s;
		}
	}

	إذا (end && *end)
		أرجع 1;
	أرجع 0;
}

#ifdef TEST_PROGRAM_CPUSET

#ضمن <getopt.h>

صحيح main(صحيح argc, حرف *argv[])
{
	cpu_set_t *set;
	size_t setsize, buflen, nbits;
	حرف *buf, *mask = عدم, *range = عدم;
	صحيح ncpus = 2048, rc, c;

	ساكن ثابت بنية option longopts[] = {
	    { "ncpus", 1, عدم, 'n' },
	    { "mask",  1, عدم, 'm' },
	    { "range", 1, عدم, 'r' },
	    { عدم,    0, عدم, 0 }
	};

	طالما ((c = getopt_long(argc, argv, "n:m:r:", longopts, عدم)) != -1) {
		بدل(c) {
		حالة 'n':
			ncpus = atoi(optarg);
			اكسر;
		حالة 'm':
			mask = optarg;
			اكسر;
		حالة 'r':
			range = optarg;
			اكسر;
		افتراضي:
			اذهب usage_err;
		}
	}

	إذا (!mask && !range)
		اذهب usage_err;

	set = cpuset_alloc(ncpus, &setsize, &nbits);
	إذا (!set)
		err(EXIT_FAILURE, "failed to allocate cpu set");

	/*
	fprintf(stderr, "ncpus: %d, cpuset bits: %zd, cpuset bytes: %zd\n",
			ncpus, nbits, setsize);
	*/

	buflen = 7 * nbits;
	buf = malloc(buflen);
	إذا (!buf)
		err(EXIT_FAILURE, "failed to allocate cpu set buffer");

	إذا (mask)
		rc = cpumask_parse(mask, set, setsize);
	وإلا
		rc = cpulist_parse(range, set, setsize, 0);

	إذا (rc)
		errx(EXIT_FAILURE, "failed to parse string: %s", mask ? : range);

	printf("%-15s = %15s ", mask ? : range,
				cpumask_create(buf, buflen, set, setsize));
	printf("[%s]\n", cpulist_create(buf, buflen, set, setsize));

	حرر(buf);
	cpuset_free(set);

	أرجع EXIT_SUCCESS;

usage_err:
	fprintf(stderr,
		"usage: %s [--ncpus <num>] --mask <mask> | --range <list>\n",
		program_invocation_short_name);
	exit(EXIT_FAILURE);
}
#endif
