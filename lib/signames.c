/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.

 * Written by:
 *  Sami Kerola <kerolasa@iki.fi>
 *  Karel Zak <kzak@redhat.com>
 *  Niklas Hambüchen <mail@nh2.me>
 */

#অন্তর্ভাব <ctype.h>		/* for isdigit() */
#অন্তর্ভাব <signal.h>
#অন্তর্ভাব <unistd.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <errno.h>

#অন্তর্ভাব "c.h"
#অন্তর্ভাব "cctype.h"
#অন্তর্ভাব "strutils.h"
#অন্তর্ভাব "signames.h"

স্থির ধ্রুব সংরচনা ul_signal_name {
	ধ্রুব বর্ণ *name;
	পূর্ণাঙ্ক val;
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
স্থির পূর্ণাঙ্ক rtsig_to_signum(ধ্রুব বর্ণ *sig)
{
	পূর্ণাঙ্ক অঙ্ক, maxi = 0;
	বর্ণ *ep = রিক্ত;

	যদি (c_strncasecmp(sig, "min+", 4) == 0)
		sig += 4;
	অন্যথা যদি (c_strncasecmp(sig, "max-", 4) == 0) {
		sig += 4;
		maxi = 1;
	}
	যদি (!isdigit(*sig))
		প্রত্যাগম -1;
	errno = 0;
	অঙ্ক = strtol(sig, &ep, 10);
	যদি (!ep || sig == ep || errno || অঙ্ক < 0)
		প্রত্যাগম -1;
	অঙ্ক = maxi ? SIGRTMAX - অঙ্ক : SIGRTMIN + অঙ্ক;
	যদি (অঙ্ক < SIGRTMIN || SIGRTMAX < অঙ্ক)
		প্রত্যাগম -1;
	প্রত্যাগম অঙ্ক;
}
#endif

পূর্ণাঙ্ক signame_to_signum(ধ্রুব বর্ণ *sig)
{
	size_t n;

	যদি (!c_strncasecmp(sig, "sig", 3))
		sig += 3;
#ifdef SIGRTMIN
	/* RT signals */
	যদি (!c_strncasecmp(sig, "rt", 2))
		প্রত্যাগম rtsig_to_signum(sig + 2);
#endif
	/* Normal signals */
	প্রতি (n = 0; n < ARRAY_SIZE(ul_signames); n++) {
		যদি (!c_strcasecmp(ul_signames[n].name, sig))
			প্রত্যাগম ul_signames[n].val;
	}
	প্রত্যাগম -1;
}

ধ্রুব বর্ণ *signum_to_signame(পূর্ণাঙ্ক signum)
{
	size_t n;

	প্রতি (n = 0; n < ARRAY_SIZE(ul_signames); n++) {
		যদি (ul_signames[n].val == signum) {
			প্রত্যাগম ul_signames[n].name;
		}
	}

	প্রত্যাগম রিক্ত;
}

পূর্ণাঙ্ক get_signame_by_idx(size_t idx, ধ্রুব বর্ণ **signame, পূর্ণাঙ্ক *signum)
{
	যদি (idx >= ARRAY_SIZE(ul_signames))
		প্রত্যাগম -1;

	যদি (signame)
		*signame = ul_signames[idx].name;
	যদি (signum)
		*signum = ul_signames[idx].val;
	প্রত্যাগম 0;

}

