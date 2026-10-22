/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.

 * Written by:
 *  Sami Kerola <kerolasa@iki.fi>
 *  Karel Zak <kzak@redhat.com>
 *  Niklas Hambüchen <mail@nh2.me>
 */

#அந்தர்பாவ <ctype.h>		/* for isdigit() */
#அந்தர்பாவ <signal.h>
#அந்தர்பாவ <unistd.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <errno.h>

#அந்தர்பாவ "c.h"
#அந்தர்பாவ "cctype.h"
#அந்தர்பாவ "strutils.h"
#அந்தர்பாவ "signames.h"

நிலையான மாறிலி கட்டமைப்பு ul_signal_name {
	மாறிலி எழுத்து *name;
	முழு val;
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
நிலையான முழு rtsig_to_signum(மாறிலி எழுத்து *sig)
{
	முழு இலக்கம், maxi = 0;
	எழுத்து *ep = வெற்று;

	எனில் (c_strncasecmp(sig, "min+", 4) == 0)
		sig += 4;
	இல்லையெனில் எனில் (c_strncasecmp(sig, "max-", 4) == 0) {
		sig += 4;
		maxi = 1;
	}
	எனில் (!isdigit(*sig))
		திரும்பு -1;
	errno = 0;
	இலக்கம் = strtol(sig, &ep, 10);
	எனில் (!ep || sig == ep || errno || இலக்கம் < 0)
		திரும்பு -1;
	இலக்கம் = maxi ? SIGRTMAX - இலக்கம் : SIGRTMIN + இலக்கம்;
	எனில் (இலக்கம் < SIGRTMIN || SIGRTMAX < இலக்கம்)
		திரும்பு -1;
	திரும்பு இலக்கம்;
}
#endif

முழு signame_to_signum(மாறிலி எழுத்து *sig)
{
	size_t n;

	எனில் (!c_strncasecmp(sig, "sig", 3))
		sig += 3;
#ifdef SIGRTMIN
	/* RT signals */
	எனில் (!c_strncasecmp(sig, "rt", 2))
		திரும்பு rtsig_to_signum(sig + 2);
#endif
	/* Normal signals */
	க்கு (n = 0; n < ARRAY_SIZE(ul_signames); n++) {
		எனில் (!c_strcasecmp(ul_signames[n].name, sig))
			திரும்பு ul_signames[n].val;
	}
	திரும்பு -1;
}

மாறிலி எழுத்து *signum_to_signame(முழு signum)
{
	size_t n;

	க்கு (n = 0; n < ARRAY_SIZE(ul_signames); n++) {
		எனில் (ul_signames[n].val == signum) {
			திரும்பு ul_signames[n].name;
		}
	}

	திரும்பு வெற்று;
}

முழு get_signame_by_idx(size_t idx, மாறிலி எழுத்து **signame, முழு *signum)
{
	எனில் (idx >= ARRAY_SIZE(ul_signames))
		திரும்பு -1;

	எனில் (signame)
		*signame = ul_signames[idx].name;
	எனில் (signum)
		*signum = ul_signames[idx].val;
	திரும்பு 0;

}

