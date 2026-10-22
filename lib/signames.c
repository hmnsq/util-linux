/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.

 * Written by:
 *  Sami Kerola <kerolasa@iki.fi>
 *  Karel Zak <kzak@redhat.com>
 *  Niklas Hambüchen <mail@nh2.me>
 */

#അന്തര്ഭാവ <ctype.h>		/* for isdigit() */
#അന്തര്ഭാവ <signal.h>
#അന്തര്ഭാവ <unistd.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <errno.h>

#അന്തര്ഭാവ "c.h"
#അന്തര്ഭാവ "cctype.h"
#അന്തര്ഭാവ "strutils.h"
#അന്തര്ഭാവ "signames.h"

സ്ഥിതം സ്ഥിരം ഘടന ul_signal_name {
	സ്ഥിരം അക്ഷരം *name;
	പൂർണ്ണസംഖ്യ val;
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
സ്ഥിതം പൂർണ്ണസംഖ്യ rtsig_to_signum(സ്ഥിരം അക്ഷരം *sig)
{
	പൂർണ്ണസംഖ്യ അക്കം, maxi = 0;
	അക്ഷരം *ep = ശൂന്യ;

	എങ്കിൽ (c_strncasecmp(sig, "min+", 4) == 0)
		sig += 4;
	അല്ലെങ്കിൽ എങ്കിൽ (c_strncasecmp(sig, "max-", 4) == 0) {
		sig += 4;
		maxi = 1;
	}
	എങ്കിൽ (!isdigit(*sig))
		തിരികെ -1;
	errno = 0;
	അക്കം = strtol(sig, &ep, 10);
	എങ്കിൽ (!ep || sig == ep || errno || അക്കം < 0)
		തിരികെ -1;
	അക്കം = maxi ? SIGRTMAX - അക്കം : SIGRTMIN + അക്കം;
	എങ്കിൽ (അക്കം < SIGRTMIN || SIGRTMAX < അക്കം)
		തിരികെ -1;
	തിരികെ അക്കം;
}
#endif

പൂർണ്ണസംഖ്യ signame_to_signum(സ്ഥിരം അക്ഷരം *sig)
{
	size_t n;

	എങ്കിൽ (!c_strncasecmp(sig, "sig", 3))
		sig += 3;
#ifdef SIGRTMIN
	/* RT signals */
	എങ്കിൽ (!c_strncasecmp(sig, "rt", 2))
		തിരികെ rtsig_to_signum(sig + 2);
#endif
	/* Normal signals */
	വേണ്ടി (n = 0; n < ARRAY_SIZE(ul_signames); n++) {
		എങ്കിൽ (!c_strcasecmp(ul_signames[n].name, sig))
			തിരികെ ul_signames[n].val;
	}
	തിരികെ -1;
}

സ്ഥിരം അക്ഷരം *signum_to_signame(പൂർണ്ണസംഖ്യ signum)
{
	size_t n;

	വേണ്ടി (n = 0; n < ARRAY_SIZE(ul_signames); n++) {
		എങ്കിൽ (ul_signames[n].val == signum) {
			തിരികെ ul_signames[n].name;
		}
	}

	തിരികെ ശൂന്യ;
}

പൂർണ്ണസംഖ്യ get_signame_by_idx(size_t idx, സ്ഥിരം അക്ഷരം **signame, പൂർണ്ണസംഖ്യ *signum)
{
	എങ്കിൽ (idx >= ARRAY_SIZE(ul_signames))
		തിരികെ -1;

	എങ്കിൽ (signame)
		*signame = ul_signames[idx].name;
	എങ്കിൽ (signum)
		*signum = ul_signames[idx].val;
	തിരികെ 0;

}

