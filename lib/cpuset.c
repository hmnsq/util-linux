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
#جے_تعریف_شدہ HAVE_SYS_SYSCALL_H
#ضمن <sys/syscall.h>
#جے_انت

#ضمن "cpuset.h"
#ضمن "c.h"

ساكن مضمن صحيح val_منزل_char(صحيح v)
{
	إذا (v >= 0 && v < 10)
		أرجع '0' + v;
	إذا (v >= 10 && v < 16)
		أرجع ('a' - 10) + v;
	أرجع -1;
}

ساكن مضمن صحيح char_منزل_val(صحيح c)
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
صحيح get_max_نمبر_دا_cpus(فراغ)
{
#جے_تعریف_شدہ SYS_sched_getaffinity
	صحيح n, cpus = 2048;
	size_t setsize;
	cpu_set_t *سیٹ = cpuset_alloc(cpus, &setsize, عدم);

	إذا (!سیٹ)
		أرجع -1;	/* error */

	لكل (;;) {
		CPU_ZERO_S(setsize, سیٹ);

		/* the library version does not return size of cpumask_t */
		n = syscall(SYS_sched_getaffinity, 0, setsize, سیٹ);

		إذا (n < 0 && errno == EINVAL && cpus < 1024 * 1024) {
			cpuset_free(سیٹ);
			cpus *= 2;
			سیٹ = cpuset_alloc(cpus, &setsize, عدم);
			إذا (!سیٹ)
				أرجع -1;	/* error */
			استمر;
		}
		cpuset_free(سیٹ);
		أرجع n * 8;
	}
#جے_انت
	أرجع -1;
}

/*
 * Allocates a new set for ncpus and returns size in bytes and size in bits
 */
cpu_set_t *cpuset_alloc(صحيح ncpus, size_t *setsize, size_t *nbits)
{
	cpu_set_t *سیٹ = عدم;

	إذا (ncpus)
		سیٹ = CPU_ALLOC(ncpus);
	إذا (!سیٹ)
		أرجع عدم;
	إذا (setsize)
		*setsize = CPU_ALLOC_ناپ(ncpus);
	إذا (nbits)
		*nbits = cpuset_nbits(CPU_ALLOC_ناپ(ncpus));
	أرجع سیٹ;
}

فراغ cpuset_free(cpu_set_t *سیٹ)
{
	CPU_FREE(سیٹ);
}

#إذا !HAVE_DECL_CPU_ALLOC
/* Please, use CPU_COUNT_S() macro. This is fallback */
صحيح __cpuset_count_s(size_t setsize, ثابت cpu_set_t *سیٹ)
{
	صحيح s = 0;
	ثابت __cpu_mask *p = سیٹ->__bits;
	ثابت __cpu_mask *سرا = &سیٹ->__bits[setsize / حجم (__cpu_mask)];

	طالما (p < سرا) {
		__cpu_mask l = *p++;

		إذا (l == 0)
			استمر;
# إذا LONG_ثنائی_ہندسہ > 32
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
#جے_انت

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
ساكن size_t لبھو_اگلا_cpu(size_t شروع, size_t setsize, cpu_set_t *سیٹ)
{
	size_t nbits = cpuset_nbits(setsize);
	لكل (; شروع < nbits; شروع++)
		إذا (CPU_ISSET_S(شروع, setsize, سیٹ))
			أرجع شروع;
	أرجع شروع;
}

/*
 * Returns human readable representation of the cpuset. The output format is
 * a list of CPUs with ranges (for example, "0,1,3-9:3").
 */
حرف *cpulist_create(حرف *str, size_t len,
			cpu_set_t *سیٹ, size_t setsize)
{
	حرف *ptr = str;
	صحيح اندراج_made = 0;
	size_t max = cpuset_nbits(setsize);
	size_t a = 0;  /* min for cpu range */
	size_t اگلا = 0;  /* where to start looking for next cpu */

	طالما ((a = لبھو_اگلا_cpu(اگلا, setsize, سیٹ)) < max) {
		صحيح rlen;
		اگلا = لبھو_اگلا_cpu(a + 1, setsize, سیٹ);
		إذا (اگلا == max) {
			rlen = snprintf(ptr, len, "%zu,", a);
		} وإلا {
			/* Extend range as long as we have the same stride. */
			size_t b = اگلا;
			size_t s = b - a;
			طالما (((اگلا = لبھو_اگلا_cpu(b + 1, setsize, سیٹ)) <
				max) && اگلا - b == s) {
				b = اگلا;
			}
			إذا (b - a == s) {
				/*
				 * Only print one CPU.  Hope the next one can
				 * be put in the next range.
				 */
				rlen = snprintf(ptr, len, "%zu,", a);
				اگلا = b;
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
		اندراج_made = 1;
	}
	ptr -= اندراج_made;
	*ptr = '\0';

	أرجع str;
}

/*
 * cpulist_flat_create - Convert cpumask to flat comma-separated list
 * Output: ONLY single CPUs, NO ranges (e.g. "0,1,2,3,4")
 */
حرف *cpulist_flat_create(حرف *str, size_t len,
			    cpu_set_t *سیٹ, size_t setsize)
{
	حرف *ptr = str;
	صحيح اندراج_made = 0;
	size_t max = cpuset_nbits(setsize);
	size_t cpu;
	صحيح rlen;

	لكل (cpu = 0; cpu < max; cpu++) {
		إذا (!CPU_ISSET_S(cpu, setsize, سیٹ))
			استمر;

		rlen = snprintf(ptr, len, "%zu,", cpu);
		إذا (rlen < 0 || (size_t)rlen >= len)
			أرجع عدم;

		ptr += rlen;
		len -= rlen;
		اندراج_made = 1;
	}
	ptr -= اندراج_made;
	*ptr = '\0';

	أرجع str;
}


/*
 * Returns string with CPU mask.
 */
حرف *cpumask_create(حرف *str, size_t len,
			cpu_set_t *سیٹ, size_t setsize)
{
	حرف *ptr = str;
	حرف *ret = عدم;
	صحيح cpu;

	لكل (cpu = cpuset_nbits(setsize) - 4; cpu >= 0; cpu -= 4) {
		حرف val = 0;

		إذا (len == (size_t) (ptr - str))
			اكسر;

		إذا (CPU_ISSET_S(cpu, setsize, سیٹ))
			val |= 1;
		إذا (CPU_ISSET_S(cpu + 1, setsize, سیٹ))
			val |= 2;
		إذا (CPU_ISSET_S(cpu + 2, setsize, سیٹ))
			val |= 4;
		إذا (CPU_ISSET_S(cpu + 3, setsize, سیٹ))
			val |= 8;

		إذا (!ret && val)
			ret = ptr;
		*ptr++ = val_منزل_char(val);
	}
	*ptr = '\0';
	أرجع ret ? ret : ptr - 1;
}

/*
 * Parses string with CPUs mask.
 */
صحيح cpumask_parse(ثابت حرف *str, cpu_set_t *سیٹ, size_t setsize)
{
	صحيح len = لڑی_لمبائی(str);
	ثابت حرف *ptr = str + len - 1;
	صحيح cpu = 0;

	/* skip 0x, it's all hex anyway */
	إذا (len > 1 && !memcmp(str, "0x", 2L))
		str += 2;

	CPU_ZERO_S(setsize, سیٹ);

	طالما (ptr >= str) {
		حرف val;

		/* cpu masks in /sys uses comma as a separator */
		إذا (*ptr == ',')
			ptr--;

		val = char_منزل_val(*ptr);
		إذا (val == (حرف) -1)
			أرجع -1;
		إذا (val & 1)
			CPU_سیٹ_S(cpu, setsize, سیٹ);
		إذا (val & 2)
			CPU_سیٹ_S(cpu + 1, setsize, سیٹ);
		إذا (val & 4)
			CPU_سیٹ_S(cpu + 2, setsize, سیٹ);
		إذا (val & 8)
			CPU_سیٹ_S(cpu + 3, setsize, سیٹ);
		ptr--;
		cpu += 4;
	}

	أرجع 0;
}

ساكن صحيح nextnumber(ثابت حرف *str, حرف **سرا, بنانشانی صحيح *نتیجہ)
{
	errno = 0;
	إذا (str == عدم || *str == '\0' || !ہندسہ_اے(*str))
		أرجع -EINVAL;
	*نتیجہ = (بنانشانی صحيح) strtoul(str, سرا, 10);
	إذا (errno)
		أرجع -errno;
	إذا (str == *سرا)
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
صحيح cpulist_parse(ثابت حرف *str, cpu_set_t *سیٹ, size_t setsize, صحيح fail)
{
	ثابت size_t max = cpuset_nbits(setsize);
	ثابت حرف *p, *q;
	حرف *سرا = عدم;

	q = str;
	CPU_ZERO_S(setsize, سیٹ);

	طالما (p = q, q = nexttoken(q, ','), p) {
		بنانشانی صحيح a;	/* beginning of range */
		بنانشانی صحيح b;	/* end of range */
		بنانشانی صحيح s;	/* stride */
		ثابت حرف *c1, *c2;

		إذا (nextnumber(p, &سرا, &a) != 0 ||
			(*سرا && *سرا != ',' && *سرا != '-'))
			أرجع 1;
		b = a;
		s = 1;
		p = سرا;

		c1 = nexttoken(p, '-');
		c2 = nexttoken(p, ',');

		إذا (c1 != عدم && (c2 == عدم || c1 < c2)) {
			إذا (nextnumber(c1, &سرا, &b) != 0 ||
				(*سرا && *سرا != ',' && *سرا != ':'))
				أرجع 1;

			c1 = nexttoken(سرا, ':');

			إذا (c1 != عدم && (c2 == عدم || c1 < c2)) {
				إذا (nextnumber(c1, &سرا, &s) != 0 ||
					(*سرا && *سرا != ','))
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
			CPU_سیٹ_S(a, setsize, سیٹ);
			a += s;
		}
	}

	إذا (سرا && *سرا)
		أرجع 1;
	أرجع 0;
}

#جے_تعریف_شدہ TEST_پروگرام_CPUSET

#ضمن <getopt.h>

صحيح مکھ(صحيح argc, حرف *argv[])
{
	cpu_set_t *سیٹ;
	size_t setsize, buflen, nbits;
	حرف *buf, *نقاب = عدم, *range = عدم;
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
			نقاب = optarg;
			اكسر;
		حالة 'r':
			range = optarg;
			اكسر;
		افتراضي:
			اذهب usage_err;
		}
	}

	إذا (!نقاب && !range)
		اذهب usage_err;

	سیٹ = cpuset_alloc(ncpus, &setsize, &nbits);
	إذا (!سیٹ)
		err(EXIT_FAILURE, "failed to allocate cpu set");

	/*
	fprintf(stderr, "ncpus: %d, cpuset bits: %zd, cpuset bytes: %zd\n",
			ncpus, nbits, setsize);
	*/

	buflen = 7 * nbits;
	buf = حافظہ_دیو(buflen);
	إذا (!buf)
		err(EXIT_FAILURE, "failed to allocate cpu set buffer");

	إذا (نقاب)
		rc = cpumask_parse(نقاب, سیٹ, setsize);
	وإلا
		rc = cpulist_parse(range, سیٹ, setsize, 0);

	إذا (rc)
		errx(EXIT_FAILURE, "failed to parse string: %s", نقاب ? : range);

	لکھو("%-15s = %15s ", نقاب ? : range,
				cpumask_create(buf, buflen, سیٹ, setsize));
	لکھو("[%s]\n", cpulist_create(buf, buflen, سیٹ, setsize));

	حرر(buf);
	cpuset_free(سیٹ);

	أرجع EXIT_SUCCESS;

usage_err:
	fprintf(stderr,
		"usage: %s [--ncpus <num>] --mask <mask> | --range <list>\n",
		پروگرام_invocation_short_name);
	باہر_نکلو(EXIT_FAILURE);
}
#جے_انت
