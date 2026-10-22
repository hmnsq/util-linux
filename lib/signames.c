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
#جے_تعریف_شدہ SIGTRAP
	{ "TRAP",	SIGTRAP },	/* 5 */
#جے_انت
	{ "ABRT",	SIGABRT },	/* 6 */
#جے_تعریف_شدہ SIGIOT
	{ "IOT",	SIGIOT },	/* 6, same as SIGABRT */
#جے_انت
#جے_تعریف_شدہ SIGEMT
	{ "EMT",	SIGEMT },	/* 7 (mips,alpha,sparc*) */
#جے_انت
#جے_تعریف_شدہ SIGBUS
	{ "BUS",	SIGBUS },	/* 7 (arm,i386,m68k,ppc), 10 (mips,alpha,sparc*) */
#جے_انت
	{ "FPE",	SIGFPE },	/* 8 */
	{ "KILL",	SIGKILL },	/* 9 */
	{ "USR1",	SIGUSR1 },	/* 10 (arm,i386,m68k,ppc), 30 (alpha,sparc*), 16 (mips) */
	{ "SEGV",	SIGSEGV },	/* 11 */
	{ "USR2",	SIGUSR2 },	/* 12 (arm,i386,m68k,ppc), 31 (alpha,sparc*), 17 (mips) */
	{ "PIPE",	SIGPIPE },	/* 13 */
	{ "ALRM",	SIGALRM },	/* 14 */
	{ "TERM",	SIGTERM },	/* 15 */
#جے_تعریف_شدہ SIGSTKFLT
	{ "STKFLT",	SIGSTKFLT },	/* 16 (arm,i386,m68k,ppc) */
#جے_انت
	{ "CHLD",	SIGCHLD },	/* 17 (arm,i386,m68k,ppc), 20 (alpha,sparc*), 18 (mips) */
#جے_تعریف_شدہ SIGCLD
	{ "CLD",	SIGCLD },	/* same as SIGCHLD (mips) */
#جے_انت
	{ "CONT",	SIGCONT },	/* 18 (arm,i386,m68k,ppc), 19 (alpha,sparc*), 25 (mips) */
	{ "STOP",	SIGSTOP },	/* 19 (arm,i386,m68k,ppc), 17 (alpha,sparc*), 23 (mips) */
	{ "TSTP",	SIGTSTP },	/* 20 (arm,i386,m68k,ppc), 18 (alpha,sparc*), 24 (mips) */
	{ "TTIN",	SIGTTIN },	/* 21 (arm,i386,m68k,ppc,alpha,sparc*), 26 (mips) */
	{ "TTOU",	SIGTTOU },	/* 22 (arm,i386,m68k,ppc,alpha,sparc*), 27 (mips) */
#جے_تعریف_شدہ SIGURG
	{ "URG",	SIGURG },	/* 23 (arm,i386,m68k,ppc), 16 (alpha,sparc*), 21 (mips) */
#جے_انت
#جے_تعریف_شدہ SIGXCPU
	{ "XCPU",	SIGXCPU },	/* 24 (arm,i386,m68k,ppc,alpha,sparc*), 30 (mips) */
#جے_انت
#جے_تعریف_شدہ SIGXFSZ
	{ "XFSZ",	SIGXFSZ },	/* 25 (arm,i386,m68k,ppc,alpha,sparc*), 31 (mips) */
#جے_انت
#جے_تعریف_شدہ SIGVTALRM
	{ "VTALRM",	SIGVTALRM },	/* 26 (arm,i386,m68k,ppc,alpha,sparc*), 28 (mips) */
#جے_انت
#جے_تعریف_شدہ SIGPROF
	{ "PROF",	SIGPROF },	/* 27 (arm,i386,m68k,ppc,alpha,sparc*), 29 (mips) */
#جے_انت
#جے_تعریف_شدہ SIGWINCH
	{ "WINCH",	SIGWINCH },	/* 28 (arm,i386,m68k,ppc,alpha,sparc*), 20 (mips) */
#جے_انت
#جے_تعریف_شدہ SIGIO
	{ "IO",		SIGIO },	/* 29 (arm,i386,m68k,ppc), 23 (alpha,sparc*), 22 (mips) */
#جے_انت
#جے_تعریف_شدہ SIGPOLL
	{ "POLL",	SIGPOLL },	/* same as SIGIO */
#جے_انت
#جے_تعریف_شدہ SIGINFO
	{ "INFO",	SIGINFO },	/* 29 (alpha) */
#جے_انت
#جے_تعریف_شدہ SIGLOST
	{ "LOST",	SIGLOST },	/* 29 (arm,i386,m68k,ppc,sparc*) */
#جے_انت
#جے_تعریف_شدہ SIGPWR
	{ "PWR",	SIGPWR },	/* 30 (arm,i386,m68k,ppc), 29 (alpha,sparc*), 19 (mips) */
#جے_انت
#جے_تعریف_شدہ SIGUNUSED
	{ "UNUSED",	SIGUNUSED },	/* 31 (arm,i386,m68k,ppc) */
#جے_انت
#جے_تعریف_شدہ SIGSYS
	{ "SYS",	SIGSYS },	/* 31 (mips,alpha,sparc*) */
#جے_انت
};

#جے_تعریف_شدہ SIGRTMIN
ساكن صحيح rtsig_منزل_signum(ثابت حرف *sig)
{
	صحيح عدد, maxi = 0;
	حرف *ep = عدم;

	إذا (c_strncasecmp(sig, "min+", 4) == 0)
		sig += 4;
	وإلا إذا (c_strncasecmp(sig, "max-", 4) == 0) {
		sig += 4;
		maxi = 1;
	}
	إذا (!ہندسہ_اے(*sig))
		أرجع -1;
	errno = 0;
	عدد = strtol(sig, &ep, 10);
	إذا (!ep || sig == ep || errno || عدد < 0)
		أرجع -1;
	عدد = maxi ? SIGRTMAX - عدد : SIGRTMIN + عدد;
	إذا (عدد < SIGRTMIN || SIGRTMAX < عدد)
		أرجع -1;
	أرجع عدد;
}
#جے_انت

صحيح signame_منزل_signum(ثابت حرف *sig)
{
	size_t n;

	إذا (!c_strncasecmp(sig, "sig", 3))
		sig += 3;
#جے_تعریف_شدہ SIGRTMIN
	/* RT signals */
	إذا (!c_strncasecmp(sig, "rt", 2))
		أرجع rtsig_منزل_signum(sig + 2);
#جے_انت
	/* Normal signals */
	لكل (n = 0; n < ARRAY_ناپ(ul_signames); n++) {
		إذا (!c_strcasecmp(ul_signames[n].name, sig))
			أرجع ul_signames[n].val;
	}
	أرجع -1;
}

ثابت حرف *signum_منزل_signame(صحيح signum)
{
	size_t n;

	لكل (n = 0; n < ARRAY_ناپ(ul_signames); n++) {
		إذا (ul_signames[n].val == signum) {
			أرجع ul_signames[n].name;
		}
	}

	أرجع عدم;
}

صحيح get_signame_by_idx(size_t idx, ثابت حرف **signame, صحيح *signum)
{
	إذا (idx >= ARRAY_ناپ(ul_signames))
		أرجع -1;

	إذا (signame)
		*signame = ul_signames[idx].name;
	إذا (signum)
		*signum = ul_signames[idx].val;
	أرجع 0;

}

