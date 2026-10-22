/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.

 * Written by:
 *  Sami Kerola <kerolasa@iki.fi>
 *  Karel Zak <kzak@redhat.com>
 *  Niklas Hambüchen <mail@nh2.me>
 */

#ଅନ୍ତର୍ଭାଵ <ctype.h>		/* for isdigit() */
#ଅନ୍ତର୍ଭାଵ <signal.h>
#ଅନ୍ତର୍ଭାଵ <unistd.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <errno.h>

#ଅନ୍ତର୍ଭାଵ "c.h"
#ଅନ୍ତର୍ଭାଵ "cctype.h"
#ଅନ୍ତର୍ଭାଵ "strutils.h"
#ଅନ୍ତର୍ଭାଵ "signames.h"

ସ୍ଥିର ଅଚଳ ସଂରଚନା ul_signal_name {
	ଅଚଳ ଅକ୍ଷର *name;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା val;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା rtsig_to_signum(ଅଚଳ ଅକ୍ଷର *sig)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଅଙ୍କ, maxi = 0;
	ଅକ୍ଷର *ep = ଖାଲି;

	ଯଦି (c_strncasecmp(sig, "min+", 4) == 0)
		sig += 4;
	ଅନ୍ୟଥା ଯଦି (c_strncasecmp(sig, "max-", 4) == 0) {
		sig += 4;
		maxi = 1;
	}
	ଯଦି (!isdigit(*sig))
		ଫେରନ୍ତୁ -1;
	errno = 0;
	ଅଙ୍କ = strtol(sig, &ep, 10);
	ଯଦି (!ep || sig == ep || errno || ଅଙ୍କ < 0)
		ଫେରନ୍ତୁ -1;
	ଅଙ୍କ = maxi ? SIGRTMAX - ଅଙ୍କ : SIGRTMIN + ଅଙ୍କ;
	ଯଦି (ଅଙ୍କ < SIGRTMIN || SIGRTMAX < ଅଙ୍କ)
		ଫେରନ୍ତୁ -1;
	ଫେରନ୍ତୁ ଅଙ୍କ;
}
#endif

ପୂର୍ଣ୍ଣସଂଖ୍ୟା signame_to_signum(ଅଚଳ ଅକ୍ଷର *sig)
{
	size_t n;

	ଯଦି (!c_strncasecmp(sig, "sig", 3))
		sig += 3;
#ifdef SIGRTMIN
	/* RT signals */
	ଯଦି (!c_strncasecmp(sig, "rt", 2))
		ଫେରନ୍ତୁ rtsig_to_signum(sig + 2);
#endif
	/* Normal signals */
	ପାଇଁ (n = 0; n < ARRAY_SIZE(ul_signames); n++) {
		ଯଦି (!c_strcasecmp(ul_signames[n].name, sig))
			ଫେରନ୍ତୁ ul_signames[n].val;
	}
	ଫେରନ୍ତୁ -1;
}

ଅଚଳ ଅକ୍ଷର *signum_to_signame(ପୂର୍ଣ୍ଣସଂଖ୍ୟା signum)
{
	size_t n;

	ପାଇଁ (n = 0; n < ARRAY_SIZE(ul_signames); n++) {
		ଯଦି (ul_signames[n].val == signum) {
			ଫେରନ୍ତୁ ul_signames[n].name;
		}
	}

	ଫେରନ୍ତୁ ଖାଲି;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା get_signame_by_idx(size_t idx, ଅଚଳ ଅକ୍ଷର **signame, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *signum)
{
	ଯଦି (idx >= ARRAY_SIZE(ul_signames))
		ଫେରନ୍ତୁ -1;

	ଯଦି (signame)
		*signame = ul_signames[idx].name;
	ଯଦି (signum)
		*signum = ul_signames[idx].val;
	ଫେରନ୍ତୁ 0;

}

