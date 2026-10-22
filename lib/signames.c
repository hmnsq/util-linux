/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.

 * Written by:
 *  Sami Kerola <kerolasa@iki.fi>
 *  Karel Zak <kzak@redhat.com>
 *  Niklas Hambüchen <mail@nh2.me>
 */

#ضمن <ctype.h>		/* for isdigit() */
#ضمن <signal.h>
#ضمن <unistd.h>
#ضمن <string.h>
#ضمن <errno.h>

#ضمن "c.h"
#ضمن "cctype.h"
#ضمن "strutils.h"
#ضمن "signames.h"

ساكن ثابت بنية ul_signal_name {
	ثابت حرف *name;
	صحيح val;
} ul_signames[] = {
	/* POSIX signals */
	{ "HUP",	SIGHUP },	/* 1 */
	{ "INT",	SIGINT },	/* 2 */
	{ "QUIT",	SIGQUIT },	/* 3 */
	{ "ILL",	SIGILL },	/* 4 */
#اگر_تعریف_شده SIGTRAP
	{ "TRAP",	SIGTRAP },	/* 5 */
#پایان_اگر
	{ "ABRT",	SIGABRT },	/* 6 */
#اگر_تعریف_شده SIGIOT
	{ "IOT",	SIGIOT },	/* 6, same as SIGABRT */
#پایان_اگر
#اگر_تعریف_شده SIGEMT
	{ "EMT",	SIGEMT },	/* 7 (mips,alpha,sparc*) */
#پایان_اگر
#اگر_تعریف_شده SIGBUS
	{ "BUS",	SIGBUS },	/* 7 (arm,i386,m68k,ppc), 10 (mips,alpha,sparc*) */
#پایان_اگر
	{ "FPE",	SIGFPE },	/* 8 */
	{ "KILL",	SIGKILL },	/* 9 */
	{ "USR1",	SIGUSR1 },	/* 10 (arm,i386,m68k,ppc), 30 (alpha,sparc*), 16 (mips) */
	{ "SEGV",	SIGSEGV },	/* 11 */
	{ "USR2",	SIGUSR2 },	/* 12 (arm,i386,m68k,ppc), 31 (alpha,sparc*), 17 (mips) */
	{ "PIPE",	SIGPIPE },	/* 13 */
	{ "ALRM",	SIGALRM },	/* 14 */
	{ "TERM",	SIGTERM },	/* 15 */
#اگر_تعریف_شده SIGSTKFLT
	{ "STKFLT",	SIGSTKFLT },	/* 16 (arm,i386,m68k,ppc) */
#پایان_اگر
	{ "CHLD",	SIGCHLD },	/* 17 (arm,i386,m68k,ppc), 20 (alpha,sparc*), 18 (mips) */
#اگر_تعریف_شده SIGCLD
	{ "CLD",	SIGCLD },	/* same as SIGCHLD (mips) */
#پایان_اگر
	{ "CONT",	SIGCONT },	/* 18 (arm,i386,m68k,ppc), 19 (alpha,sparc*), 25 (mips) */
	{ "STOP",	SIGSTOP },	/* 19 (arm,i386,m68k,ppc), 17 (alpha,sparc*), 23 (mips) */
	{ "TSTP",	SIGTSTP },	/* 20 (arm,i386,m68k,ppc), 18 (alpha,sparc*), 24 (mips) */
	{ "TTIN",	SIGTTIN },	/* 21 (arm,i386,m68k,ppc,alpha,sparc*), 26 (mips) */
	{ "TTOU",	SIGTTOU },	/* 22 (arm,i386,m68k,ppc,alpha,sparc*), 27 (mips) */
#اگر_تعریف_شده SIGURG
	{ "URG",	SIGURG },	/* 23 (arm,i386,m68k,ppc), 16 (alpha,sparc*), 21 (mips) */
#پایان_اگر
#اگر_تعریف_شده SIGXCPU
	{ "XCPU",	SIGXCPU },	/* 24 (arm,i386,m68k,ppc,alpha,sparc*), 30 (mips) */
#پایان_اگر
#اگر_تعریف_شده SIGXFSZ
	{ "XFSZ",	SIGXFSZ },	/* 25 (arm,i386,m68k,ppc,alpha,sparc*), 31 (mips) */
#پایان_اگر
#اگر_تعریف_شده SIGVTALRM
	{ "VTALRM",	SIGVTALRM },	/* 26 (arm,i386,m68k,ppc,alpha,sparc*), 28 (mips) */
#پایان_اگر
#اگر_تعریف_شده SIGPROF
	{ "PROF",	SIGPROF },	/* 27 (arm,i386,m68k,ppc,alpha,sparc*), 29 (mips) */
#پایان_اگر
#اگر_تعریف_شده SIGWINCH
	{ "WINCH",	SIGWINCH },	/* 28 (arm,i386,m68k,ppc,alpha,sparc*), 20 (mips) */
#پایان_اگر
#اگر_تعریف_شده SIGIO
	{ "IO",		SIGIO },	/* 29 (arm,i386,m68k,ppc), 23 (alpha,sparc*), 22 (mips) */
#پایان_اگر
#اگر_تعریف_شده SIGPOLL
	{ "POLL",	SIGPOLL },	/* same as SIGIO */
#پایان_اگر
#اگر_تعریف_شده SIGINFO
	{ "INFO",	SIGINFO },	/* 29 (alpha) */
#پایان_اگر
#اگر_تعریف_شده SIGLOST
	{ "LOST",	SIGLOST },	/* 29 (arm,i386,m68k,ppc,sparc*) */
#پایان_اگر
#اگر_تعریف_شده SIGPWR
	{ "PWR",	SIGPWR },	/* 30 (arm,i386,m68k,ppc), 29 (alpha,sparc*), 19 (mips) */
#پایان_اگر
#اگر_تعریف_شده SIGUNUSED
	{ "UNUSED",	SIGUNUSED },	/* 31 (arm,i386,m68k,ppc) */
#پایان_اگر
#اگر_تعریف_شده SIGSYS
	{ "SYS",	SIGSYS },	/* 31 (mips,alpha,sparc*) */
#پایان_اگر
};

#اگر_تعریف_شده SIGRTMIN
ساكن صحيح rtsig_مقصد_signum(ثابت حرف *sig)
{
	صحيح رقم, maxi = 0;
	حرف *ep = عدم;

	إذا (c_strncasecmp(sig, "min+", 4) == 0)
		sig += 4;
	وإلا إذا (c_strncasecmp(sig, "max-", 4) == 0) {
		sig += 4;
		maxi = 1;
	}
	إذا (!آیا_رقم(*sig))
		أرجع -1;
	errno = 0;
	رقم = strtol(sig, &ep, 10);
	إذا (!ep || sig == ep || errno || رقم < 0)
		أرجع -1;
	رقم = maxi ? SIGRTMAX - رقم : SIGRTMIN + رقم;
	إذا (رقم < SIGRTMIN || SIGRTMAX < رقم)
		أرجع -1;
	أرجع رقم;
}
#پایان_اگر

صحيح signame_مقصد_signum(ثابت حرف *sig)
{
	size_t n;

	إذا (!c_strncasecmp(sig, "sig", 3))
		sig += 3;
#اگر_تعریف_شده SIGRTMIN
	/* RT signals */
	إذا (!c_strncasecmp(sig, "rt", 2))
		أرجع rtsig_مقصد_signum(sig + 2);
#پایان_اگر
	/* Normal signals */
	لكل (n = 0; n < ARRAY_اندازه(ul_signames); n++) {
		إذا (!c_strcasecmp(ul_signames[n].name, sig))
			أرجع ul_signames[n].val;
	}
	أرجع -1;
}

ثابت حرف *signum_مقصد_signame(صحيح signum)
{
	size_t n;

	لكل (n = 0; n < ARRAY_اندازه(ul_signames); n++) {
		إذا (ul_signames[n].val == signum) {
			أرجع ul_signames[n].name;
		}
	}

	أرجع عدم;
}

صحيح get_signame_by_idx(size_t idx, ثابت حرف **signame, صحيح *signum)
{
	إذا (idx >= ARRAY_اندازه(ul_signames))
		أرجع -1;

	إذا (signame)
		*signame = ul_signames[idx].name;
	إذا (signum)
		*signum = ul_signames[idx].val;
	أرجع 0;

}

