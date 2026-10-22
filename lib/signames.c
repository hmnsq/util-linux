/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.

 * Written by:
 *  Sami Kerola <kerolasa@iki.fi>
 *  Karel Zak <kzak@redhat.com>
 *  Niklas Hambüchen <mail@nh2.me>
 */

#include <ctype.h>		/* for isdigit() */
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "c.h"
#include "cctype.h"
#include "strutils.h"
#include "signames.h"

static const sa_nracanaa ul_signal_name {
	const var_na *name;
	පූර්ණ_සංඛ්‍යා val;
} ul_signames[] = {
	/* POSIX signals */
	{ "HUP",	SIGHUP },	/* 1 */
	{ "INT",	SIGINT },	/* 2 */
	{ "QUIT",	SIGQUIT },	/* 3 */
	{ "ILL",	SIGILL },	/* 4 */
#ifdef SIGTRAP
	{ "TRAP",	SIGTRAP },	/* 5 */
#endif
	{ "ABRT",	SIGABRT },	/* 6 */
#ifdef SIGIOT
	{ "IOT",	SIGIOT },	/* 6, same as SIGABRT */
#endif
#ifdef SIGEMT
	{ "EMT",	SIGEMT },	/* 7 (mips,alpha,sparc*) */
#endif
#ifdef SIGBUS
	{ "BUS",	SIGBUS },	/* 7 (arm,i386,m68k,ppc), 10 (mips,alpha,sparc*) */
#endif
	{ "FPE",	SIGFPE },	/* 8 */
	{ "KILL",	SIGKILL },	/* 9 */
	{ "USR1",	SIGUSR1 },	/* 10 (arm,i386,m68k,ppc), 30 (alpha,sparc*), 16 (mips) */
	{ "SEGV",	SIGSEGV },	/* 11 */
	{ "USR2",	SIGUSR2 },	/* 12 (arm,i386,m68k,ppc), 31 (alpha,sparc*), 17 (mips) */
	{ "PIPE",	SIGPIPE },	/* 13 */
	{ "ALRM",	SIGALRM },	/* 14 */
	{ "TERM",	SIGTERM },	/* 15 */
#ifdef SIGSTKFLT
	{ "STKFLT",	SIGSTKFLT },	/* 16 (arm,i386,m68k,ppc) */
#endif
	{ "CHLD",	SIGCHLD },	/* 17 (arm,i386,m68k,ppc), 20 (alpha,sparc*), 18 (mips) */
#ifdef SIGCLD
	{ "CLD",	SIGCLD },	/* same as SIGCHLD (mips) */
#endif
	{ "CONT",	SIGCONT },	/* 18 (arm,i386,m68k,ppc), 19 (alpha,sparc*), 25 (mips) */
	{ "STOP",	SIGSTOP },	/* 19 (arm,i386,m68k,ppc), 17 (alpha,sparc*), 23 (mips) */
	{ "TSTP",	SIGTSTP },	/* 20 (arm,i386,m68k,ppc), 18 (alpha,sparc*), 24 (mips) */
	{ "TTIN",	SIGTTIN },	/* 21 (arm,i386,m68k,ppc,alpha,sparc*), 26 (mips) */
	{ "TTOU",	SIGTTOU },	/* 22 (arm,i386,m68k,ppc,alpha,sparc*), 27 (mips) */
#ifdef SIGURG
	{ "URG",	SIGURG },	/* 23 (arm,i386,m68k,ppc), 16 (alpha,sparc*), 21 (mips) */
#endif
#ifdef SIGXCPU
	{ "XCPU",	SIGXCPU },	/* 24 (arm,i386,m68k,ppc,alpha,sparc*), 30 (mips) */
#endif
#ifdef SIGXFSZ
	{ "XFSZ",	SIGXFSZ },	/* 25 (arm,i386,m68k,ppc,alpha,sparc*), 31 (mips) */
#endif
#ifdef SIGVTALRM
	{ "VTALRM",	SIGVTALRM },	/* 26 (arm,i386,m68k,ppc,alpha,sparc*), 28 (mips) */
#endif
#ifdef SIGPROF
	{ "PROF",	SIGPROF },	/* 27 (arm,i386,m68k,ppc,alpha,sparc*), 29 (mips) */
#endif
#ifdef SIGWINCH
	{ "WINCH",	SIGWINCH },	/* 28 (arm,i386,m68k,ppc,alpha,sparc*), 20 (mips) */
#endif
#ifdef SIGIO
	{ "IO",		SIGIO },	/* 29 (arm,i386,m68k,ppc), 23 (alpha,sparc*), 22 (mips) */
#endif
#ifdef SIGPOLL
	{ "POLL",	SIGPOLL },	/* same as SIGIO */
#endif
#ifdef SIGINFO
	{ "INFO",	SIGINFO },	/* 29 (alpha) */
#endif
#ifdef SIGLOST
	{ "LOST",	SIGLOST },	/* 29 (arm,i386,m68k,ppc,sparc*) */
#endif
#ifdef SIGPWR
	{ "PWR",	SIGPWR },	/* 30 (arm,i386,m68k,ppc), 29 (alpha,sparc*), 19 (mips) */
#endif
#ifdef SIGUNUSED
	{ "UNUSED",	SIGUNUSED },	/* 31 (arm,i386,m68k,ppc) */
#endif
#ifdef SIGSYS
	{ "SYS",	SIGSYS },	/* 31 (mips,alpha,sparc*) */
#endif
};

#ifdef SIGRTMIN
static පූර්ණ_සංඛ්‍යා rtsig_to_signum(const var_na *sig)
{
	පූර්ණ_සංඛ්‍යා num, maxi = 0;
	var_na *ep = NULL;

	නම් (c_strncasecmp(sig, "min+", 4) == 0)
		sig += 4;
	නැතහොත් නම් (c_strncasecmp(sig, "max-", 4) == 0) {
		sig += 4;
		maxi = 1;
	}
	නම් (!isdigit(*sig))
		ආපසු -1;
	errno = 0;
	num = strtol(sig, &ep, 10);
	නම් (!ep || sig == ep || errno || num < 0)
		ආපසු -1;
	num = maxi ? SIGRTMAX - num : SIGRTMIN + num;
	නම් (num < SIGRTMIN || SIGRTMAX < num)
		ආපසු -1;
	ආපසු num;
}
#endif

පූර්ණ_සංඛ්‍යා signame_to_signum(const var_na *sig)
{
	size_t n;

	නම් (!c_strncasecmp(sig, "sig", 3))
		sig += 3;
#ifdef SIGRTMIN
	/* RT signals */
	නම් (!c_strncasecmp(sig, "rt", 2))
		ආපසු rtsig_to_signum(sig + 2);
#endif
	/* Normal signals */
	සඳහා (n = 0; n < ARRAY_SIZE(ul_signames); n++) {
		නම් (!c_strcasecmp(ul_signames[n].name, sig))
			ආපසු ul_signames[n].val;
	}
	ආපසු -1;
}

const var_na *signum_to_signame(පූර්ණ_සංඛ්‍යා signum)
{
	size_t n;

	සඳහා (n = 0; n < ARRAY_SIZE(ul_signames); n++) {
		නම් (ul_signames[n].val == signum) {
			ආපසු ul_signames[n].name;
		}
	}

	ආපසු NULL;
}

පූර්ණ_සංඛ්‍යා get_signame_by_idx(size_t idx, const var_na **signame, පූර්ණ_සංඛ්‍යා *signum)
{
	නම් (idx >= ARRAY_SIZE(ul_signames))
		ආපසු -1;

	නම් (signame)
		*signame = ul_signames[idx].name;
	නම් (signum)
		*signum = ul_signames[idx].val;
	ආපසු 0;

}

