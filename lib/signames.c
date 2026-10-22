/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.

 * Written by:
 *  Sami Kerola <kerolasa@iki.fi>
 *  Karel Zak <kzak@redhat.com>
 *  Niklas Hambüchen <mail@nh2.me>
 */

#અન્તર્ભાવ <ctype.h>		/* for isdigit() */
#અન્તર્ભાવ <signal.h>
#અન્તર્ભાવ <unistd.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <errno.h>

#અન્તર્ભાવ "c.h"
#અન્તર્ભાવ "cctype.h"
#અન્તર્ભાવ "strutils.h"
#અન્તર્ભાવ "signames.h"

સ્થિર અચલ સંરચના ul_signal_name {
	અચલ અક્ષર *name;
	પૂર્ણાંક val;
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
સ્થિર પૂર્ણાંક rtsig_to_signum(અચલ અક્ષર *sig)
{
	પૂર્ણાંક અંક, maxi = 0;
	અક્ષર *ep = ખાલી;

	જો (c_strncasecmp(sig, "min+", 4) == 0)
		sig += 4;
	નહીંતર જો (c_strncasecmp(sig, "max-", 4) == 0) {
		sig += 4;
		maxi = 1;
	}
	જો (!isdigit(*sig))
		પાછા -1;
	errno = 0;
	અંક = strtol(sig, &ep, 10);
	જો (!ep || sig == ep || errno || અંક < 0)
		પાછા -1;
	અંક = maxi ? SIGRTMAX - અંક : SIGRTMIN + અંક;
	જો (અંક < SIGRTMIN || SIGRTMAX < અંક)
		પાછા -1;
	પાછા અંક;
}
#endif

પૂર્ણાંક signame_to_signum(અચલ અક્ષર *sig)
{
	size_t n;

	જો (!c_strncasecmp(sig, "sig", 3))
		sig += 3;
#ifdef SIGRTMIN
	/* RT signals */
	જો (!c_strncasecmp(sig, "rt", 2))
		પાછા rtsig_to_signum(sig + 2);
#endif
	/* Normal signals */
	માટે (n = 0; n < ARRAY_SIZE(ul_signames); n++) {
		જો (!c_strcasecmp(ul_signames[n].name, sig))
			પાછા ul_signames[n].val;
	}
	પાછા -1;
}

અચલ અક્ષર *signum_to_signame(પૂર્ણાંક signum)
{
	size_t n;

	માટે (n = 0; n < ARRAY_SIZE(ul_signames); n++) {
		જો (ul_signames[n].val == signum) {
			પાછા ul_signames[n].name;
		}
	}

	પાછા ખાલી;
}

પૂર્ણાંક get_signame_by_idx(size_t idx, અચલ અક્ષર **signame, પૂર્ણાંક *signum)
{
	જો (idx >= ARRAY_SIZE(ul_signames))
		પાછા -1;

	જો (signame)
		*signame = ul_signames[idx].name;
	જો (signum)
		*signum = ul_signames[idx].val;
	પાછા 0;

}

