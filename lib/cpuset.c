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

#شامل <stdio.h>
#شامل <stdlib.h>
#شامل <unistd.h>
#شامل <sched.h>
#شامل <errno.h>
#شامل <string.h>
#شامل <ctype.h>
#ifdef HAVE_SYS_SYSCALL_H
#شامل <sys/syscall.h>
#endif

#شامل "cpuset.h"
#شامل "c.h"

ایستا درونخطی صحیح val_to_char(صحیح v)
{
	اگر (v >= 0 && v < 10)
		بازگشت '0' + v;
	اگر (v >= 10 && v < 16)
		بازگشت ('a' - 10) + v;
	بازگشت -1;
}

ایستا درونخطی صحیح char_to_val(صحیح c)
{
	صحیح cl;

	اگر (c >= '0' && c <= '9')
		بازگشت c - '0';
	cl = tolower(c);
	اگر (cl >= 'a' && cl <= 'f')
		بازگشت cl + (10 - 'a');
	بازگشت -1;
}

ایستا ثابت نویسه *nexttoken(ثابت نویسه *q,  صحیح sep)
{
	اگر (q)
		q = strchr(q, sep);
	اگر (q)
		q++;
	بازگشت q;
}

/*
 * Number of bits in a CPU bitmask on current system
 */
صحیح get_max_عدد_of_cpus(تهی)
{
#ifdef SYS_sched_getaffinity
	صحیح n, cpus = 2048;
	size_t setsize;
	cpu_set_t *set = cpuset_alloc(cpus, &setsize, پوچ);

	اگر (!set)
		بازگشت -1;	/* error */

	برای (;;) {
		CPU_ZERO_S(setsize, set);

		/* the library version does not return size of cpumask_t */
		n = syscall(SYS_sched_getaffinity, 0, setsize, set);

		اگر (n < 0 && errno == EINVAL && cpus < 1024 * 1024) {
			cpuset_free(set);
			cpus *= 2;
			set = cpuset_alloc(cpus, &setsize, پوچ);
			اگر (!set)
				بازگشت -1;	/* error */
			ادامه;
		}
		cpuset_free(set);
		بازگشت n * 8;
	}
#endif
	بازگشت -1;
}

/*
 * Allocates a new set for ncpus and returns size in bytes and size in bits
 */
cpu_set_t *cpuset_alloc(صحیح ncpus, size_t *setsize, size_t *nbits)
{
	cpu_set_t *set = پوچ;

	اگر (ncpus)
		set = CPU_ALLOC(ncpus);
	اگر (!set)
		بازگشت پوچ;
	اگر (setsize)
		*setsize = CPU_ALLOC_SIZE(ncpus);
	اگر (nbits)
		*nbits = cpuset_nbits(CPU_ALLOC_SIZE(ncpus));
	بازگشت set;
}

تهی cpuset_free(cpu_set_t *set)
{
	CPU_FREE(set);
}

#اگر !HAVE_DECL_CPU_ALLOC
/* Please, use CPU_COUNT_S() macro. This is fallback */
صحیح __cpuset_count_s(size_t setsize, ثابت cpu_set_t *set)
{
	صحیح s = 0;
	ثابت __cpu_mask *p = set->__bits;
	ثابت __cpu_mask *end = &set->__bits[setsize / اندازه (__cpu_mask)];

	وقتی (p < end) {
		__cpu_mask l = *p++;

		اگر (l == 0)
			ادامه;
# اگر LONG_BIT > 32
		l = (l & 0x5555555555555555ul) + ((l >> 1) & 0x5555555555555555ul);
		l = (l & 0x3333333333333333ul) + ((l >> 2) & 0x3333333333333333ul);
		l = (l & 0x0f0f0f0f0f0f0f0ful) + ((l >> 4) & 0x0f0f0f0f0f0f0f0ful);
		l = (l & 0x00ff00ff00ff00fful) + ((l >> 8) & 0x00ff00ff00ff00fful);
		l = (l & 0x0000ffff0000fffful) + ((l >> 16) & 0x0000ffff0000fffful);
		l = (l & 0x00000000fffffffful) + ((l >> 32) & 0x00000000fffffffful);
# وگرنه
		l = (l & 0x55555555ul) + ((l >> 1) & 0x55555555ul);
		l = (l & 0x33333333ul) + ((l >> 2) & 0x33333333ul);
		l = (l & 0x0f0f0f0ful) + ((l >> 4) & 0x0f0f0f0ful);
		l = (l & 0x00ff00fful) + ((l >> 8) & 0x00ff00fful);
		l = (l & 0x0000fffful) + ((l >> 16) & 0x0000fffful);
# endif
		s += l;
	}
	بازگشت s;
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
ایستا size_t find_next_cpu(size_t start, size_t setsize, cpu_set_t *set)
{
	size_t nbits = cpuset_nbits(setsize);
	برای (; start < nbits; start++)
		اگر (CPU_ISSET_S(start, setsize, set))
			بازگشت start;
	بازگشت start;
}

/*
 * Returns human readable representation of the cpuset. The output format is
 * a list of CPUs with ranges (for example, "0,1,3-9:3").
 */
نویسه *cpulist_create(نویسه *str, size_t len,
			cpu_set_t *set, size_t setsize)
{
	نویسه *ptr = str;
	صحیح entry_made = 0;
	size_t max = cpuset_nbits(setsize);
	size_t a = 0;  /* min for cpu range */
	size_t next = 0;  /* where to start looking for next cpu */

	وقتی ((a = find_next_cpu(next, setsize, set)) < max) {
		صحیح rlen;
		next = find_next_cpu(a + 1, setsize, set);
		اگر (next == max) {
			rlen = snprintf(ptr, len, "%zu,", a);
		} وگرنه {
			/* Extend range as long as we have the same stride. */
			size_t b = next;
			size_t s = b - a;
			وقتی (((next = find_next_cpu(b + 1, setsize, set)) <
				max) && next - b == s) {
				b = next;
			}
			اگر (b - a == s) {
				/*
				 * Only print one CPU.  Hope the next one can
				 * be put in the next range.
				 */
				rlen = snprintf(ptr, len, "%zu,", a);
				next = b;
			} وگرنه اگر (s == 1) {
				rlen = snprintf(ptr, len, "%zu-%zu,", a, b);
			} وگرنه {
				rlen = snprintf(ptr, len, "%zu-%zu:%zu,",
						a, b, s);
			}
		}
		اگر (rlen < 0 || (size_t) rlen >= len)
			بازگشت پوچ;
		ptr += rlen;
		len -= rlen;
		entry_made = 1;
	}
	ptr -= entry_made;
	*ptr = '\0';

	بازگشت str;
}

/*
 * cpulist_flat_create - Convert cpumask to flat comma-separated list
 * Output: ONLY single CPUs, NO ranges (e.g. "0,1,2,3,4")
 */
نویسه *cpulist_flat_create(نویسه *str, size_t len,
			    cpu_set_t *set, size_t setsize)
{
	نویسه *ptr = str;
	صحیح entry_made = 0;
	size_t max = cpuset_nbits(setsize);
	size_t cpu;
	صحیح rlen;

	برای (cpu = 0; cpu < max; cpu++) {
		اگر (!CPU_ISSET_S(cpu, setsize, set))
			ادامه;

		rlen = snprintf(ptr, len, "%zu,", cpu);
		اگر (rlen < 0 || (size_t)rlen >= len)
			بازگشت پوچ;

		ptr += rlen;
		len -= rlen;
		entry_made = 1;
	}
	ptr -= entry_made;
	*ptr = '\0';

	بازگشت str;
}


/*
 * Returns string with CPU mask.
 */
نویسه *cpumask_create(نویسه *str, size_t len,
			cpu_set_t *set, size_t setsize)
{
	نویسه *ptr = str;
	نویسه *ret = پوچ;
	صحیح cpu;

	برای (cpu = cpuset_nbits(setsize) - 4; cpu >= 0; cpu -= 4) {
		نویسه val = 0;

		اگر (len == (size_t) (ptr - str))
			بشکن;

		اگر (CPU_ISSET_S(cpu, setsize, set))
			val |= 1;
		اگر (CPU_ISSET_S(cpu + 1, setsize, set))
			val |= 2;
		اگر (CPU_ISSET_S(cpu + 2, setsize, set))
			val |= 4;
		اگر (CPU_ISSET_S(cpu + 3, setsize, set))
			val |= 8;

		اگر (!ret && val)
			ret = ptr;
		*ptr++ = val_to_char(val);
	}
	*ptr = '\0';
	بازگشت ret ? ret : ptr - 1;
}

/*
 * Parses string with CPUs mask.
 */
صحیح cpumask_parse(ثابت نویسه *str, cpu_set_t *set, size_t setsize)
{
	صحیح len = strlen(str);
	ثابت نویسه *ptr = str + len - 1;
	صحیح cpu = 0;

	/* skip 0x, it's all hex anyway */
	اگر (len > 1 && !memcmp(str, "0x", 2L))
		str += 2;

	CPU_ZERO_S(setsize, set);

	وقتی (ptr >= str) {
		نویسه val;

		/* cpu masks in /sys uses comma as a separator */
		اگر (*ptr == ',')
			ptr--;

		val = char_to_val(*ptr);
		اگر (val == (نویسه) -1)
			بازگشت -1;
		اگر (val & 1)
			CPU_SET_S(cpu, setsize, set);
		اگر (val & 2)
			CPU_SET_S(cpu + 1, setsize, set);
		اگر (val & 4)
			CPU_SET_S(cpu + 2, setsize, set);
		اگر (val & 8)
			CPU_SET_S(cpu + 3, setsize, set);
		ptr--;
		cpu += 4;
	}

	بازگشت 0;
}

ایستا صحیح nextnumber(ثابت نویسه *str, نویسه **end, unsigned صحیح *result)
{
	errno = 0;
	اگر (str == پوچ || *str == '\0' || !isdigit(*str))
		بازگشت -EINVAL;
	*result = (unsigned صحیح) strtoul(str, end, 10);
	اگر (errno)
		بازگشت -errno;
	اگر (str == *end)
		بازگشت -EINVAL;
	بازگشت 0;
}

/*
 * Parses string with list of CPU ranges.
 * Returns 0 on success.
 * Returns 1 on error.
 * Returns 2 if fail is set and a cpu number passed in the list doesn't fit
 * into the cpu_set. If fail is not set cpu numbers that do not fit are
 * ignored and 0 is returned instead.
 */
صحیح cpulist_parse(ثابت نویسه *str, cpu_set_t *set, size_t setsize, صحیح fail)
{
	ثابت size_t max = cpuset_nbits(setsize);
	ثابت نویسه *p, *q;
	نویسه *end = پوچ;

	q = str;
	CPU_ZERO_S(setsize, set);

	وقتی (p = q, q = nexttoken(q, ','), p) {
		unsigned صحیح a;	/* beginning of range */
		unsigned صحیح b;	/* end of range */
		unsigned صحیح s;	/* stride */
		ثابت نویسه *c1, *c2;

		اگر (nextnumber(p, &end, &a) != 0 ||
			(*end && *end != ',' && *end != '-'))
			بازگشت 1;
		b = a;
		s = 1;
		p = end;

		c1 = nexttoken(p, '-');
		c2 = nexttoken(p, ',');

		اگر (c1 != پوچ && (c2 == پوچ || c1 < c2)) {
			اگر (nextnumber(c1, &end, &b) != 0 ||
				(*end && *end != ',' && *end != ':'))
				بازگشت 1;

			c1 = nexttoken(end, ':');

			اگر (c1 != پوچ && (c2 == پوچ || c1 < c2)) {
				اگر (nextnumber(c1, &end, &s) != 0 ||
					(*end && *end != ','))
					بازگشت 1;
				اگر (s == 0)
					بازگشت 1;
			}
		}

		اگر (!(a <= b))
			بازگشت 1;
		وقتی (a <= b) {
			اگر (a >= max) {
				اگر (fail)
					بازگشت 2;
				وگرنه
					بشکن;
			}
			CPU_SET_S(a, setsize, set);
			a += s;
		}
	}

	اگر (end && *end)
		بازگشت 1;
	بازگشت 0;
}

#ifdef TEST_PROGRAM_CPUSET

#شامل <getopt.h>

صحیح اصلی(صحیح argc, نویسه *argv[])
{
	cpu_set_t *set;
	size_t setsize, buflen, nbits;
	نویسه *buf, *mask = پوچ, *range = پوچ;
	صحیح ncpus = 2048, rc, c;

	ایستا ثابت ساختار option longopts[] = {
	    { "ncpus", 1, پوچ, 'n' },
	    { "mask",  1, پوچ, 'm' },
	    { "range", 1, پوچ, 'r' },
	    { پوچ,    0, پوچ, 0 }
	};

	وقتی ((c = getopt_long(argc, argv, "n:m:r:", longopts, پوچ)) != -1) {
		انتخاب(c) {
		حالت 'n':
			ncpus = atoi(optarg);
			بشکن;
		حالت 'm':
			mask = optarg;
			بشکن;
		حالت 'r':
			range = optarg;
			بشکن;
		پیشفرض:
			برو usage_err;
		}
	}

	اگر (!mask && !range)
		برو usage_err;

	set = cpuset_alloc(ncpus, &setsize, &nbits);
	اگر (!set)
		err(EXIT_FAILURE, "failed to allocate cpu set");

	/*
	fprintf(stderr, "ncpus: %d, cpuset bits: %zd, cpuset bytes: %zd\n",
			ncpus, nbits, setsize);
	*/

	buflen = 7 * nbits;
	buf = malloc(buflen);
	اگر (!buf)
		err(EXIT_FAILURE, "failed to allocate cpu set buffer");

	اگر (mask)
		rc = cpumask_parse(mask, set, setsize);
	وگرنه
		rc = cpulist_parse(range, set, setsize, 0);

	اگر (rc)
		errx(EXIT_FAILURE, "failed to parse string: %s", mask ? : range);

	printf("%-15s = %15s ", mask ? : range,
				cpumask_create(buf, buflen, set, setsize));
	printf("[%s]\n", cpulist_create(buf, buflen, set, setsize));

	آزاد(buf);
	cpuset_free(set);

	بازگشت EXIT_SUCCESS;

usage_err:
	fprintf(stderr,
		"usage: %s [--ncpus <num>] --mask <mask> | --range <list>\n",
		program_invocation_short_name);
	exit(EXIT_FAILURE);
}
#endif
