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
#اگر_تعریف_شدہ HAVE_SYS_SYSCALL_H
#شامل <sys/syscall.h>
#اگر_ختم

#شامل "cpuset.h"
#شامل "c.h"

ساکن اندرونی صحیح val_مقصد_char(صحیح v)
{
	اگر (v >= 0 && v < 10)
		واپس '0' + v;
	اگر (v >= 10 && v < 16)
		واپس ('a' - 10) + v;
	واپس -1;
}

ساکن اندرونی صحیح char_مقصد_val(صحیح c)
{
	صحیح cl;

	اگر (c >= '0' && c <= '9')
		واپس c - '0';
	cl = tolower(c);
	اگر (cl >= 'a' && cl <= 'f')
		واپس cl + (10 - 'a');
	واپس -1;
}

ساکن ثابت حرف *nexttoken(ثابت حرف *q,  صحیح sep)
{
	اگر (q)
		q = strchr(q, sep);
	اگر (q)
		q++;
	واپس q;
}

/*
 * Number of bits in a CPU bitmask on current system
 */
صحیح get_max_نمبر_کا_cpus(خالی)
{
#اگر_تعریف_شدہ SYS_sched_getaffinity
	صحیح n, cpus = 2048;
	size_t setsize;
	cpu_set_t *سیٹ = cpuset_alloc(cpus, &setsize, تہی);

	اگر (!سیٹ)
		واپس -1;	/* error */

	برائے (;;) {
		CPU_ZERO_S(setsize, سیٹ);

		/* the library version does not return size of cpumask_t */
		n = syscall(SYS_sched_getaffinity, 0, setsize, سیٹ);

		اگر (n < 0 && errno == EINVAL && cpus < 1024 * 1024) {
			cpuset_free(سیٹ);
			cpus *= 2;
			سیٹ = cpuset_alloc(cpus, &setsize, تہی);
			اگر (!سیٹ)
				واپس -1;	/* error */
			جاری;
		}
		cpuset_free(سیٹ);
		واپس n * 8;
	}
#اگر_ختم
	واپس -1;
}

/*
 * Allocates a new set for ncpus and returns size in bytes and size in bits
 */
cpu_set_t *cpuset_alloc(صحیح ncpus, size_t *setsize, size_t *nbits)
{
	cpu_set_t *سیٹ = تہی;

	اگر (ncpus)
		سیٹ = CPU_ALLOC(ncpus);
	اگر (!سیٹ)
		واپس تہی;
	اگر (setsize)
		*setsize = CPU_ALLOC_سائز(ncpus);
	اگر (nbits)
		*nbits = cpuset_nbits(CPU_ALLOC_سائز(ncpus));
	واپس سیٹ;
}

خالی cpuset_free(cpu_set_t *سیٹ)
{
	CPU_FREE(سیٹ);
}

#اگر !HAVE_DECL_CPU_ALLOC
/* Please, use CPU_COUNT_S() macro. This is fallback */
صحیح __cpuset_count_s(size_t setsize, ثابت cpu_set_t *سیٹ)
{
	صحیح s = 0;
	ثابت __cpu_mask *p = سیٹ->__bits;
	ثابت __cpu_mask *انتہا = &سیٹ->__bits[setsize / حجم (__cpu_mask)];

	جب (p < انتہا) {
		__cpu_mask l = *p++;

		اگر (l == 0)
			جاری;
# اگر LONG_ثنائی_ہندسہ > 32
		l = (l & 0x5555555555555555ul) + ((l >> 1) & 0x5555555555555555ul);
		l = (l & 0x3333333333333333ul) + ((l >> 2) & 0x3333333333333333ul);
		l = (l & 0x0f0f0f0f0f0f0f0ful) + ((l >> 4) & 0x0f0f0f0f0f0f0f0ful);
		l = (l & 0x00ff00ff00ff00fful) + ((l >> 8) & 0x00ff00ff00ff00fful);
		l = (l & 0x0000ffff0000fffful) + ((l >> 16) & 0x0000ffff0000fffful);
		l = (l & 0x00000000fffffffful) + ((l >> 32) & 0x00000000fffffffful);
# ورنہ
		l = (l & 0x55555555ul) + ((l >> 1) & 0x55555555ul);
		l = (l & 0x33333333ul) + ((l >> 2) & 0x33333333ul);
		l = (l & 0x0f0f0f0ful) + ((l >> 4) & 0x0f0f0f0ful);
		l = (l & 0x00ff00fful) + ((l >> 8) & 0x00ff00fful);
		l = (l & 0x0000fffful) + ((l >> 16) & 0x0000fffful);
# endif
		s += l;
	}
	واپس s;
}
#اگر_ختم

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
ساکن size_t ڈھونڈو_اگلا_cpu(size_t آغاز, size_t setsize, cpu_set_t *سیٹ)
{
	size_t nbits = cpuset_nbits(setsize);
	برائے (; آغاز < nbits; آغاز++)
		اگر (CPU_ISSET_S(آغاز, setsize, سیٹ))
			واپس آغاز;
	واپس آغاز;
}

/*
 * Returns human readable representation of the cpuset. The output format is
 * a list of CPUs with ranges (for example, "0,1,3-9:3").
 */
حرف *cpulist_create(حرف *str, size_t len,
			cpu_set_t *سیٹ, size_t setsize)
{
	حرف *ptr = str;
	صحیح اندراج_made = 0;
	size_t max = cpuset_nbits(setsize);
	size_t a = 0;  /* min for cpu range */
	size_t اگلا = 0;  /* where to start looking for next cpu */

	جب ((a = ڈھونڈو_اگلا_cpu(اگلا, setsize, سیٹ)) < max) {
		صحیح rlen;
		اگلا = ڈھونڈو_اگلا_cpu(a + 1, setsize, سیٹ);
		اگر (اگلا == max) {
			rlen = snprintf(ptr, len, "%zu,", a);
		} ورنہ {
			/* Extend range as long as we have the same stride. */
			size_t b = اگلا;
			size_t s = b - a;
			جب (((اگلا = ڈھونڈو_اگلا_cpu(b + 1, setsize, سیٹ)) <
				max) && اگلا - b == s) {
				b = اگلا;
			}
			اگر (b - a == s) {
				/*
				 * Only print one CPU.  Hope the next one can
				 * be put in the next range.
				 */
				rlen = snprintf(ptr, len, "%zu,", a);
				اگلا = b;
			} ورنہ اگر (s == 1) {
				rlen = snprintf(ptr, len, "%zu-%zu,", a, b);
			} ورنہ {
				rlen = snprintf(ptr, len, "%zu-%zu:%zu,",
						a, b, s);
			}
		}
		اگر (rlen < 0 || (size_t) rlen >= len)
			واپس تہی;
		ptr += rlen;
		len -= rlen;
		اندراج_made = 1;
	}
	ptr -= اندراج_made;
	*ptr = '\0';

	واپس str;
}

/*
 * cpulist_flat_create - Convert cpumask to flat comma-separated list
 * Output: ONLY single CPUs, NO ranges (e.g. "0,1,2,3,4")
 */
حرف *cpulist_flat_create(حرف *str, size_t len,
			    cpu_set_t *سیٹ, size_t setsize)
{
	حرف *ptr = str;
	صحیح اندراج_made = 0;
	size_t max = cpuset_nbits(setsize);
	size_t cpu;
	صحیح rlen;

	برائے (cpu = 0; cpu < max; cpu++) {
		اگر (!CPU_ISSET_S(cpu, setsize, سیٹ))
			جاری;

		rlen = snprintf(ptr, len, "%zu,", cpu);
		اگر (rlen < 0 || (size_t)rlen >= len)
			واپس تہی;

		ptr += rlen;
		len -= rlen;
		اندراج_made = 1;
	}
	ptr -= اندراج_made;
	*ptr = '\0';

	واپس str;
}


/*
 * Returns string with CPU mask.
 */
حرف *cpumask_create(حرف *str, size_t len,
			cpu_set_t *سیٹ, size_t setsize)
{
	حرف *ptr = str;
	حرف *ret = تہی;
	صحیح cpu;

	برائے (cpu = cpuset_nbits(setsize) - 4; cpu >= 0; cpu -= 4) {
		حرف val = 0;

		اگر (len == (size_t) (ptr - str))
			توڑو;

		اگر (CPU_ISSET_S(cpu, setsize, سیٹ))
			val |= 1;
		اگر (CPU_ISSET_S(cpu + 1, setsize, سیٹ))
			val |= 2;
		اگر (CPU_ISSET_S(cpu + 2, setsize, سیٹ))
			val |= 4;
		اگر (CPU_ISSET_S(cpu + 3, setsize, سیٹ))
			val |= 8;

		اگر (!ret && val)
			ret = ptr;
		*ptr++ = val_مقصد_char(val);
	}
	*ptr = '\0';
	واپس ret ? ret : ptr - 1;
}

/*
 * Parses string with CPUs mask.
 */
صحیح cpumask_parse(ثابت حرف *str, cpu_set_t *سیٹ, size_t setsize)
{
	صحیح len = لڑی_لمبائی(str);
	ثابت حرف *ptr = str + len - 1;
	صحیح cpu = 0;

	/* skip 0x, it's all hex anyway */
	اگر (len > 1 && !memcmp(str, "0x", 2L))
		str += 2;

	CPU_ZERO_S(setsize, سیٹ);

	جب (ptr >= str) {
		حرف val;

		/* cpu masks in /sys uses comma as a separator */
		اگر (*ptr == ',')
			ptr--;

		val = char_مقصد_val(*ptr);
		اگر (val == (حرف) -1)
			واپس -1;
		اگر (val & 1)
			CPU_سیٹ_S(cpu, setsize, سیٹ);
		اگر (val & 2)
			CPU_سیٹ_S(cpu + 1, setsize, سیٹ);
		اگر (val & 4)
			CPU_سیٹ_S(cpu + 2, setsize, سیٹ);
		اگر (val & 8)
			CPU_سیٹ_S(cpu + 3, setsize, سیٹ);
		ptr--;
		cpu += 4;
	}

	واپس 0;
}

ساکن صحیح nextnumber(ثابت حرف *str, حرف **انتہا, غیرعلامتی صحیح *نتیجہ)
{
	errno = 0;
	اگر (str == تہی || *str == '\0' || !ہندسہ_ہے(*str))
		واپس -EINVAL;
	*نتیجہ = (غیرعلامتی صحیح) strtoul(str, انتہا, 10);
	اگر (errno)
		واپس -errno;
	اگر (str == *انتہا)
		واپس -EINVAL;
	واپس 0;
}

/*
 * Parses string with list of CPU ranges.
 * Returns 0 on success.
 * Returns 1 on error.
 * Returns 2 if fail is set and a cpu number passed in the list doesn't fit
 * into the cpu_set. If fail is not set cpu numbers that do not fit are
 * ignored and 0 is returned instead.
 */
صحیح cpulist_parse(ثابت حرف *str, cpu_set_t *سیٹ, size_t setsize, صحیح fail)
{
	ثابت size_t max = cpuset_nbits(setsize);
	ثابت حرف *p, *q;
	حرف *انتہا = تہی;

	q = str;
	CPU_ZERO_S(setsize, سیٹ);

	جب (p = q, q = nexttoken(q, ','), p) {
		غیرعلامتی صحیح a;	/* beginning of range */
		غیرعلامتی صحیح b;	/* end of range */
		غیرعلامتی صحیح s;	/* stride */
		ثابت حرف *c1, *c2;

		اگر (nextnumber(p, &انتہا, &a) != 0 ||
			(*انتہا && *انتہا != ',' && *انتہا != '-'))
			واپس 1;
		b = a;
		s = 1;
		p = انتہا;

		c1 = nexttoken(p, '-');
		c2 = nexttoken(p, ',');

		اگر (c1 != تہی && (c2 == تہی || c1 < c2)) {
			اگر (nextnumber(c1, &انتہا, &b) != 0 ||
				(*انتہا && *انتہا != ',' && *انتہا != ':'))
				واپس 1;

			c1 = nexttoken(انتہا, ':');

			اگر (c1 != تہی && (c2 == تہی || c1 < c2)) {
				اگر (nextnumber(c1, &انتہا, &s) != 0 ||
					(*انتہا && *انتہا != ','))
					واپس 1;
				اگر (s == 0)
					واپس 1;
			}
		}

		اگر (!(a <= b))
			واپس 1;
		جب (a <= b) {
			اگر (a >= max) {
				اگر (fail)
					واپس 2;
				ورنہ
					توڑو;
			}
			CPU_سیٹ_S(a, setsize, سیٹ);
			a += s;
		}
	}

	اگر (انتہا && *انتہا)
		واپس 1;
	واپس 0;
}

#اگر_تعریف_شدہ TEST_پروگرام_CPUSET

#شامل <getopt.h>

صحیح مین(صحیح argc, حرف *argv[])
{
	cpu_set_t *سیٹ;
	size_t setsize, buflen, nbits;
	حرف *buf, *نقاب = تہی, *range = تہی;
	صحیح ncpus = 2048, rc, c;

	ساکن ثابت ساخت option longopts[] = {
	    { "ncpus", 1, تہی, 'n' },
	    { "mask",  1, تہی, 'm' },
	    { "range", 1, تہی, 'r' },
	    { تہی,    0, تہی, 0 }
	};

	جب ((c = getopt_long(argc, argv, "n:m:r:", longopts, تہی)) != -1) {
		انتخاب(c) {
		حالت 'n':
			ncpus = atoi(optarg);
			توڑو;
		حالت 'm':
			نقاب = optarg;
			توڑو;
		حالت 'r':
			range = optarg;
			توڑو;
		طےشدہ:
			جاؤ usage_err;
		}
	}

	اگر (!نقاب && !range)
		جاؤ usage_err;

	سیٹ = cpuset_alloc(ncpus, &setsize, &nbits);
	اگر (!سیٹ)
		err(EXIT_FAILURE, "failed to allocate cpu set");

	/*
	fprintf(stderr, "ncpus: %d, cpuset bits: %zd, cpuset bytes: %zd\n",
			ncpus, nbits, setsize);
	*/

	buflen = 7 * nbits;
	buf = حافظہ_دو(buflen);
	اگر (!buf)
		err(EXIT_FAILURE, "failed to allocate cpu set buffer");

	اگر (نقاب)
		rc = cpumask_parse(نقاب, سیٹ, setsize);
	ورنہ
		rc = cpulist_parse(range, سیٹ, setsize, 0);

	اگر (rc)
		errx(EXIT_FAILURE, "failed to parse string: %s", نقاب ? : range);

	لکھو("%-15s = %15s ", نقاب ? : range,
				cpumask_create(buf, buflen, سیٹ, setsize));
	لکھو("[%s]\n", cpulist_create(buf, buflen, سیٹ, setsize));

	آزاد(buf);
	cpuset_free(سیٹ);

	واپس EXIT_SUCCESS;

usage_err:
	fprintf(stderr,
		"usage: %s [--ncpus <num>] --mask <mask> | --range <list>\n",
		پروگرام_invocation_short_name);
	باہر_نکلو(EXIT_FAILURE);
}
#اگر_ختم
