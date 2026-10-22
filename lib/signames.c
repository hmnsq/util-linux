/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.

 * Written by:
 *  Sami Kerola <kerolasa@iki.fi>
 *  Karel Zak <kzak@redhat.com>
 *  Niklas Hambüchen <mail@nh2.me>
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <ctype.h>		/* for isdigit() */
#ਅਨ੍ਤਰ੍ਭਾਵ <signal.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "c.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "cctype.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "strutils.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "signames.h"

ਸਥਿਰ ਅਚਲ ਬਣਤਰ ul_signal_name {
	ਅਚਲ ਅੱਖਰ *name;
	ਪੂਰਨ val;
} ul_signames[] = {
	/* POSIX signals */
	{ "HUP",	SIGHUP },	/* 1 */
	{ "INT",	SIGINT },	/* 2 */
	{ "QUIT",	SIGQUIT },	/* 3 */
	{ "ILL",	SIGILL },	/* 4 */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SIGTRAP
	{ "TRAP",	SIGTRAP },	/* 5 */
#ਜੇ_ਅੰਤ
	{ "ABRT",	SIGABRT },	/* 6 */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SIGIOT
	{ "IOT",	SIGIOT },	/* 6, same as SIGABRT */
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SIGEMT
	{ "EMT",	SIGEMT },	/* 7 (mips,alpha,sparc*) */
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SIGBUS
	{ "BUS",	SIGBUS },	/* 7 (arm,i386,m68k,ppc), 10 (mips,alpha,sparc*) */
#ਜੇ_ਅੰਤ
	{ "FPE",	SIGFPE },	/* 8 */
	{ "KILL",	SIGKILL },	/* 9 */
	{ "USR1",	SIGUSR1 },	/* 10 (arm,i386,m68k,ppc), 30 (alpha,sparc*), 16 (mips) */
	{ "SEGV",	SIGSEGV },	/* 11 */
	{ "USR2",	SIGUSR2 },	/* 12 (arm,i386,m68k,ppc), 31 (alpha,sparc*), 17 (mips) */
	{ "PIPE",	SIGPIPE },	/* 13 */
	{ "ALRM",	SIGALRM },	/* 14 */
	{ "TERM",	SIGTERM },	/* 15 */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SIGSTKFLT
	{ "STKFLT",	SIGSTKFLT },	/* 16 (arm,i386,m68k,ppc) */
#ਜੇ_ਅੰਤ
	{ "CHLD",	SIGCHLD },	/* 17 (arm,i386,m68k,ppc), 20 (alpha,sparc*), 18 (mips) */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SIGCLD
	{ "CLD",	SIGCLD },	/* same as SIGCHLD (mips) */
#ਜੇ_ਅੰਤ
	{ "CONT",	SIGCONT },	/* 18 (arm,i386,m68k,ppc), 19 (alpha,sparc*), 25 (mips) */
	{ "STOP",	SIGSTOP },	/* 19 (arm,i386,m68k,ppc), 17 (alpha,sparc*), 23 (mips) */
	{ "TSTP",	SIGTSTP },	/* 20 (arm,i386,m68k,ppc), 18 (alpha,sparc*), 24 (mips) */
	{ "TTIN",	SIGTTIN },	/* 21 (arm,i386,m68k,ppc,alpha,sparc*), 26 (mips) */
	{ "TTOU",	SIGTTOU },	/* 22 (arm,i386,m68k,ppc,alpha,sparc*), 27 (mips) */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SIGURG
	{ "URG",	SIGURG },	/* 23 (arm,i386,m68k,ppc), 16 (alpha,sparc*), 21 (mips) */
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SIGXCPU
	{ "XCPU",	SIGXCPU },	/* 24 (arm,i386,m68k,ppc,alpha,sparc*), 30 (mips) */
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SIGXFSZ
	{ "XFSZ",	SIGXFSZ },	/* 25 (arm,i386,m68k,ppc,alpha,sparc*), 31 (mips) */
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SIGVTALRM
	{ "VTALRM",	SIGVTALRM },	/* 26 (arm,i386,m68k,ppc,alpha,sparc*), 28 (mips) */
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SIGPROF
	{ "PROF",	SIGPROF },	/* 27 (arm,i386,m68k,ppc,alpha,sparc*), 29 (mips) */
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SIGWINCH
	{ "WINCH",	SIGWINCH },	/* 28 (arm,i386,m68k,ppc,alpha,sparc*), 20 (mips) */
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SIGIO
	{ "IO",		SIGIO },	/* 29 (arm,i386,m68k,ppc), 23 (alpha,sparc*), 22 (mips) */
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SIGPOLL
	{ "POLL",	SIGPOLL },	/* same as SIGIO */
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SIGINFO
	{ "INFO",	SIGINFO },	/* 29 (alpha) */
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SIGLOST
	{ "LOST",	SIGLOST },	/* 29 (arm,i386,m68k,ppc,sparc*) */
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SIGPWR
	{ "PWR",	SIGPWR },	/* 30 (arm,i386,m68k,ppc), 29 (alpha,sparc*), 19 (mips) */
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SIGUNUSED
	{ "UNUSED",	SIGUNUSED },	/* 31 (arm,i386,m68k,ppc) */
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SIGSYS
	{ "SYS",	SIGSYS },	/* 31 (mips,alpha,sparc*) */
#ਜੇ_ਅੰਤ
};

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SIGRTMIN
ਸਥਿਰ ਪੂਰਨ rtsig_ਮੰਜ਼ਿਲ_signum(ਅਚਲ ਅੱਖਰ *sig)
{
	ਪੂਰਨ ਅੰਕ, maxi = 0;
	ਅੱਖਰ *ep = ਨਿਰਾ;

	ਜੇ (c_strncasecmp(sig, "min+", 4) == 0)
		sig += 4;
	ਨਹੀਂਤਾਂ ਜੇ (c_strncasecmp(sig, "max-", 4) == 0) {
		sig += 4;
		maxi = 1;
	}
	ਜੇ (!ਅੰਕ_ਹੈ(*sig))
		ਵਾਪਸ -1;
	errno = 0;
	ਅੰਕ = strtol(sig, &ep, 10);
	ਜੇ (!ep || sig == ep || errno || ਅੰਕ < 0)
		ਵਾਪਸ -1;
	ਅੰਕ = maxi ? SIGRTMAX - ਅੰਕ : SIGRTMIN + ਅੰਕ;
	ਜੇ (ਅੰਕ < SIGRTMIN || SIGRTMAX < ਅੰਕ)
		ਵਾਪਸ -1;
	ਵਾਪਸ ਅੰਕ;
}
#ਜੇ_ਅੰਤ

ਪੂਰਨ signame_ਮੰਜ਼ਿਲ_signum(ਅਚਲ ਅੱਖਰ *sig)
{
	size_t n;

	ਜੇ (!c_strncasecmp(sig, "sig", 3))
		sig += 3;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ SIGRTMIN
	/* RT signals */
	ਜੇ (!c_strncasecmp(sig, "rt", 2))
		ਵਾਪਸ rtsig_ਮੰਜ਼ਿਲ_signum(sig + 2);
#ਜੇ_ਅੰਤ
	/* Normal signals */
	ਲਈ (n = 0; n < ARRAY_ਮਾਪ(ul_signames); n++) {
		ਜੇ (!c_strcasecmp(ul_signames[n].name, sig))
			ਵਾਪਸ ul_signames[n].val;
	}
	ਵਾਪਸ -1;
}

ਅਚਲ ਅੱਖਰ *signum_ਮੰਜ਼ਿਲ_signame(ਪੂਰਨ signum)
{
	size_t n;

	ਲਈ (n = 0; n < ARRAY_ਮਾਪ(ul_signames); n++) {
		ਜੇ (ul_signames[n].val == signum) {
			ਵਾਪਸ ul_signames[n].name;
		}
	}

	ਵਾਪਸ ਨਿਰਾ;
}

ਪੂਰਨ get_signame_by_idx(size_t idx, ਅਚਲ ਅੱਖਰ **signame, ਪੂਰਨ *signum)
{
	ਜੇ (idx >= ARRAY_ਮਾਪ(ul_signames))
		ਵਾਪਸ -1;

	ਜੇ (signame)
		*signame = ul_signames[idx].name;
	ਜੇ (signum)
		*signum = ul_signames[idx].val;
	ਵਾਪਸ 0;

}

