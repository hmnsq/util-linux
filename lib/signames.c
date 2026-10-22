/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.

 * Written by:
 *  Sami Kerola <kerolasa@iki.fi>
 *  Karel Zak <kzak@redhat.com>
 *  Niklas Hambüchen <mail@nh2.me>
 */

#ಅನ್ತರ್ಭಾವ <ctype.h>		/* for isdigit() */
#ಅನ್ತರ್ಭಾವ <signal.h>
#ಅನ್ತರ್ಭಾವ <unistd.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <errno.h>

#ಅನ್ತರ್ಭಾವ "c.h"
#ಅನ್ತರ್ಭಾವ "cctype.h"
#ಅನ್ತರ್ಭಾವ "strutils.h"
#ಅನ್ತರ್ಭಾವ "signames.h"

ಸ್ಥಿರ ಧ್ರುವ ಸಂರಚನಾ ul_signal_name {
	ಧ್ರುವ ವರ್ಣ *name;
	ಪೂರ್ಣಾಙ್ಕ val;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ rtsig_to_signum(ಧ್ರುವ ವರ್ಣ *sig)
{
	ಪೂರ್ಣಾಙ್ಕ ಅಂಕೆ, maxi = 0;
	ವರ್ಣ *ep = ರಿಕ್ತ;

	ಯದಿ (c_strncasecmp(sig, "min+", 4) == 0)
		sig += 4;
	ಅನ್ಯಥಾ ಯದಿ (c_strncasecmp(sig, "max-", 4) == 0) {
		sig += 4;
		maxi = 1;
	}
	ಯದಿ (!isdigit(*sig))
		ಪ್ರತ್ಯಾಗಮ -1;
	errno = 0;
	ಅಂಕೆ = strtol(sig, &ep, 10);
	ಯದಿ (!ep || sig == ep || errno || ಅಂಕೆ < 0)
		ಪ್ರತ್ಯಾಗಮ -1;
	ಅಂಕೆ = maxi ? SIGRTMAX - ಅಂಕೆ : SIGRTMIN + ಅಂಕೆ;
	ಯದಿ (ಅಂಕೆ < SIGRTMIN || SIGRTMAX < ಅಂಕೆ)
		ಪ್ರತ್ಯಾಗಮ -1;
	ಪ್ರತ್ಯಾಗಮ ಅಂಕೆ;
}
#endif

ಪೂರ್ಣಾಙ್ಕ signame_to_signum(ಧ್ರುವ ವರ್ಣ *sig)
{
	size_t n;

	ಯದಿ (!c_strncasecmp(sig, "sig", 3))
		sig += 3;
#ifdef SIGRTMIN
	/* RT signals */
	ಯದಿ (!c_strncasecmp(sig, "rt", 2))
		ಪ್ರತ್ಯಾಗಮ rtsig_to_signum(sig + 2);
#endif
	/* Normal signals */
	ಪ್ರತಿ (n = 0; n < ARRAY_SIZE(ul_signames); n++) {
		ಯದಿ (!c_strcasecmp(ul_signames[n].name, sig))
			ಪ್ರತ್ಯಾಗಮ ul_signames[n].val;
	}
	ಪ್ರತ್ಯಾಗಮ -1;
}

ಧ್ರುವ ವರ್ಣ *signum_to_signame(ಪೂರ್ಣಾಙ್ಕ signum)
{
	size_t n;

	ಪ್ರತಿ (n = 0; n < ARRAY_SIZE(ul_signames); n++) {
		ಯದಿ (ul_signames[n].val == signum) {
			ಪ್ರತ್ಯಾಗಮ ul_signames[n].name;
		}
	}

	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

ಪೂರ್ಣಾಙ್ಕ get_signame_by_idx(size_t idx, ಧ್ರುವ ವರ್ಣ **signame, ಪೂರ್ಣಾಙ್ಕ *signum)
{
	ಯದಿ (idx >= ARRAY_SIZE(ul_signames))
		ಪ್ರತ್ಯಾಗಮ -1;

	ಯದಿ (signame)
		*signame = ul_signames[idx].name;
	ಯದಿ (signum)
		*signum = ul_signames[idx].val;
	ಪ್ರತ್ಯಾಗಮ 0;

}

