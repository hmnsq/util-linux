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
#اگر_تعریف_شده HAVE_SYS_SYSCALL_H
#ضمن <sys/syscall.h>
#پایان_اگر

#ضمن "cpuset.h"
#ضمن "c.h"

ساكن مضمن صحيح val_مقصد_char(صحيح v)
{
	إذا (v >= 0 && v < 10)
		أرجع '0' + v;
	إذا (v >= 10 && v < 16)
		أرجع ('a' - 10) + v;
	أرجع -1;
}

ساكن مضمن صحيح char_مقصد_val(صحيح c)
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
صحيح get_max_عدد_از_cpus(فراغ)
{
#اگر_تعریف_شده SYS_sched_getaffinity
	صحيح n, cpus = 2048;
	size_t setsize;
	cpu_set_t *مجموعه = cpuset_alloc(cpus, &setsize, عدم);

	إذا (!مجموعه)
		أرجع -1;	/* error */

	لكل (;;) {
		CPU_ZERO_S(setsize, مجموعه);

		/* the library version does not return size of cpumask_t */
		n = syscall(SYS_sched_getaffinity, 0, setsize, مجموعه);

		إذا (n < 0 && errno == EINVAL && cpus < 1024 * 1024) {
			cpuset_free(مجموعه);
			cpus *= 2;
			مجموعه = cpuset_alloc(cpus, &setsize, عدم);
			إذا (!مجموعه)
				أرجع -1;	/* error */
			استمر;
		}
		cpuset_free(مجموعه);
		أرجع n * 8;
	}
#پایان_اگر
	أرجع -1;
}

/*
 * Allocates a new set for ncpus and returns size in bytes and size in bits
 */
cpu_set_t *cpuset_alloc(صحيح ncpus, size_t *setsize, size_t *nbits)
{
	cpu_set_t *مجموعه = عدم;

	إذا (ncpus)
		مجموعه = CPU_ALLOC(ncpus);
	إذا (!مجموعه)
		أرجع عدم;
	إذا (setsize)
		*setsize = CPU_ALLOC_اندازه(ncpus);
	إذا (nbits)
		*nbits = cpuset_nbits(CPU_ALLOC_اندازه(ncpus));
	أرجع مجموعه;
}

فراغ cpuset_free(cpu_set_t *مجموعه)
{
	CPU_FREE(مجموعه);
}

#إذا !HAVE_DECL_CPU_ALLOC
/* Please, use CPU_COUNT_S() macro. This is fallback */
صحيح __cpuset_count_s(size_t setsize, ثابت cpu_set_t *مجموعه)
{
	صحيح s = 0;
	ثابت __cpu_mask *p = مجموعه->__bits;
	ثابت __cpu_mask *انتها = &مجموعه->__bits[setsize / حجم (__cpu_mask)];

	طالما (p < انتها) {
		__cpu_mask l = *p++;

		إذا (l == 0)
			استمر;
# إذا LONG_بیت > 32
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
#پایان_اگر

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
ساكن size_t پیدا_کن_بعدی_cpu(size_t شروع, size_t setsize, cpu_set_t *مجموعه)
{
	size_t nbits = cpuset_nbits(setsize);
	لكل (; شروع < nbits; شروع++)
		إذا (CPU_ISSET_S(شروع, setsize, مجموعه))
			أرجع شروع;
	أرجع شروع;
}

/*
 * Returns human readable representation of the cpuset. The output format is
 * a list of CPUs with ranges (for example, "0,1,3-9:3").
 */
حرف *cpulist_create(حرف *str, size_t len,
			cpu_set_t *مجموعه, size_t setsize)
{
	حرف *ptr = str;
	صحيح مدخل_made = 0;
	size_t max = cpuset_nbits(setsize);
	size_t a = 0;  /* min for cpu range */
	size_t بعدی = 0;  /* where to start looking for next cpu */

	طالما ((a = پیدا_کن_بعدی_cpu(بعدی, setsize, مجموعه)) < max) {
		صحيح rlen;
		بعدی = پیدا_کن_بعدی_cpu(a + 1, setsize, مجموعه);
		إذا (بعدی == max) {
			rlen = snprintf(ptr, len, "%zu,", a);
		} وإلا {
			/* Extend range as long as we have the same stride. */
			size_t b = بعدی;
			size_t s = b - a;
			طالما (((بعدی = پیدا_کن_بعدی_cpu(b + 1, setsize, مجموعه)) <
				max) && بعدی - b == s) {
				b = بعدی;
			}
			إذا (b - a == s) {
				/*
				 * Only print one CPU.  Hope the next one can
				 * be put in the next range.
				 */
				rlen = snprintf(ptr, len, "%zu,", a);
				بعدی = b;
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
		مدخل_made = 1;
	}
	ptr -= مدخل_made;
	*ptr = '\0';

	أرجع str;
}

/*
 * cpulist_flat_create - Convert cpumask to flat comma-separated list
 * Output: ONLY single CPUs, NO ranges (e.g. "0,1,2,3,4")
 */
حرف *cpulist_flat_create(حرف *str, size_t len,
			    cpu_set_t *مجموعه, size_t setsize)
{
	حرف *ptr = str;
	صحيح مدخل_made = 0;
	size_t max = cpuset_nbits(setsize);
	size_t cpu;
	صحيح rlen;

	لكل (cpu = 0; cpu < max; cpu++) {
		إذا (!CPU_ISSET_S(cpu, setsize, مجموعه))
			استمر;

		rlen = snprintf(ptr, len, "%zu,", cpu);
		إذا (rlen < 0 || (size_t)rlen >= len)
			أرجع عدم;

		ptr += rlen;
		len -= rlen;
		مدخل_made = 1;
	}
	ptr -= مدخل_made;
	*ptr = '\0';

	أرجع str;
}


/*
 * Returns string with CPU mask.
 */
حرف *cpumask_create(حرف *str, size_t len,
			cpu_set_t *مجموعه, size_t setsize)
{
	حرف *ptr = str;
	حرف *ret = عدم;
	صحيح cpu;

	لكل (cpu = cpuset_nbits(setsize) - 4; cpu >= 0; cpu -= 4) {
		حرف val = 0;

		إذا (len == (size_t) (ptr - str))
			اكسر;

		إذا (CPU_ISSET_S(cpu, setsize, مجموعه))
			val |= 1;
		إذا (CPU_ISSET_S(cpu + 1, setsize, مجموعه))
			val |= 2;
		إذا (CPU_ISSET_S(cpu + 2, setsize, مجموعه))
			val |= 4;
		إذا (CPU_ISSET_S(cpu + 3, setsize, مجموعه))
			val |= 8;

		إذا (!ret && val)
			ret = ptr;
		*ptr++ = val_مقصد_char(val);
	}
	*ptr = '\0';
	أرجع ret ? ret : ptr - 1;
}

/*
 * Parses string with CPUs mask.
 */
صحيح cpumask_parse(ثابت حرف *str, cpu_set_t *مجموعه, size_t setsize)
{
	صحيح len = طول_رشته(str);
	ثابت حرف *ptr = str + len - 1;
	صحيح cpu = 0;

	/* skip 0x, it's all hex anyway */
	إذا (len > 1 && !memcmp(str, "0x", 2L))
		str += 2;

	CPU_ZERO_S(setsize, مجموعه);

	طالما (ptr >= str) {
		حرف val;

		/* cpu masks in /sys uses comma as a separator */
		إذا (*ptr == ',')
			ptr--;

		val = char_مقصد_val(*ptr);
		إذا (val == (حرف) -1)
			أرجع -1;
		إذا (val & 1)
			CPU_مجموعه_S(cpu, setsize, مجموعه);
		إذا (val & 2)
			CPU_مجموعه_S(cpu + 1, setsize, مجموعه);
		إذا (val & 4)
			CPU_مجموعه_S(cpu + 2, setsize, مجموعه);
		إذا (val & 8)
			CPU_مجموعه_S(cpu + 3, setsize, مجموعه);
		ptr--;
		cpu += 4;
	}

	أرجع 0;
}

ساكن صحيح nextnumber(ثابت حرف *str, حرف **انتها, بیعلامت صحيح *نتیجه)
{
	errno = 0;
	إذا (str == عدم || *str == '\0' || !آیا_رقم(*str))
		أرجع -EINVAL;
	*نتیجه = (بیعلامت صحيح) strtoul(str, انتها, 10);
	إذا (errno)
		أرجع -errno;
	إذا (str == *انتها)
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
صحيح cpulist_parse(ثابت حرف *str, cpu_set_t *مجموعه, size_t setsize, صحيح fail)
{
	ثابت size_t max = cpuset_nbits(setsize);
	ثابت حرف *p, *q;
	حرف *انتها = عدم;

	q = str;
	CPU_ZERO_S(setsize, مجموعه);

	طالما (p = q, q = nexttoken(q, ','), p) {
		بیعلامت صحيح a;	/* beginning of range */
		بیعلامت صحيح b;	/* end of range */
		بیعلامت صحيح s;	/* stride */
		ثابت حرف *c1, *c2;

		إذا (nextnumber(p, &انتها, &a) != 0 ||
			(*انتها && *انتها != ',' && *انتها != '-'))
			أرجع 1;
		b = a;
		s = 1;
		p = انتها;

		c1 = nexttoken(p, '-');
		c2 = nexttoken(p, ',');

		إذا (c1 != عدم && (c2 == عدم || c1 < c2)) {
			إذا (nextnumber(c1, &انتها, &b) != 0 ||
				(*انتها && *انتها != ',' && *انتها != ':'))
				أرجع 1;

			c1 = nexttoken(انتها, ':');

			إذا (c1 != عدم && (c2 == عدم || c1 < c2)) {
				إذا (nextnumber(c1, &انتها, &s) != 0 ||
					(*انتها && *انتها != ','))
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
			CPU_مجموعه_S(a, setsize, مجموعه);
			a += s;
		}
	}

	إذا (انتها && *انتها)
		أرجع 1;
	أرجع 0;
}

#اگر_تعریف_شده TEST_برنامه_CPUSET

#ضمن <getopt.h>

صحيح اصلی(صحيح argc, حرف *argv[])
{
	cpu_set_t *مجموعه;
	size_t setsize, buflen, nbits;
	حرف *buf, *ماسک = عدم, *range = عدم;
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
			ماسک = optarg;
			اكسر;
		حالة 'r':
			range = optarg;
			اكسر;
		افتراضي:
			اذهب usage_err;
		}
	}

	إذا (!ماسک && !range)
		اذهب usage_err;

	مجموعه = cpuset_alloc(ncpus, &setsize, &nbits);
	إذا (!مجموعه)
		err(EXIT_FAILURE, "failed to allocate cpu set");

	/*
	fprintf(stderr, "ncpus: %d, cpuset bits: %zd, cpuset bytes: %zd\n",
			ncpus, nbits, setsize);
	*/

	buflen = 7 * nbits;
	buf = حافظه_بده(buflen);
	إذا (!buf)
		err(EXIT_FAILURE, "failed to allocate cpu set buffer");

	إذا (ماسک)
		rc = cpumask_parse(ماسک, مجموعه, setsize);
	وإلا
		rc = cpulist_parse(range, مجموعه, setsize, 0);

	إذا (rc)
		errx(EXIT_FAILURE, "failed to parse string: %s", ماسک ? : range);

	چاپ_کن("%-15s = %15s ", ماسک ? : range,
				cpumask_create(buf, buflen, مجموعه, setsize));
	چاپ_کن("[%s]\n", cpulist_create(buf, buflen, مجموعه, setsize));

	حرر(buf);
	cpuset_free(مجموعه);

	أرجع EXIT_SUCCESS;

usage_err:
	fprintf(stderr,
		"usage: %s [--ncpus <num>] --mask <mask> | --range <list>\n",
		برنامه_invocation_short_name);
	خروج(EXIT_FAILURE);
}
#پایان_اگر
