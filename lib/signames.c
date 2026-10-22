/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.

 * Written by:
 *  Sami Kerola <kerolasa@iki.fi>
 *  Karel Zak <kzak@redhat.com>
 *  Niklas Hambüchen <mail@nh2.me>
 */

#شامل <ctype.h>		/* for isdigit() */
#شامل <signal.h>
#شامل <unistd.h>
#شامل <string.h>
#شامل <errno.h>

#شامل "c.h"
#شامل "cctype.h"
#شامل "strutils.h"
#شامل "signames.h"

ساکن ثابت ساخت ul_signal_name {
	ثابت حرف *name;
	صحیح val;
} ul_signames[] = {
	/* POSIX signals */
	{ "HUP",	SIGHUP },	/* 1 */
	{ "INT",	SIGINT },	/* 2 */
	{ "QUIT",	SIGQUIT },	/* 3 */
	{ "ILL",	SIGILL },	/* 4 */
#اگر_تعریف_شدہ SIGTRAP
	{ "TRAP",	SIGTRAP },	/* 5 */
#اگر_ختم
	{ "ABRT",	SIGABRT },	/* 6 */
#اگر_تعریف_شدہ SIGIOT
	{ "IOT",	SIGIOT },	/* 6, same as SIGABRT */
#اگر_ختم
#اگر_تعریف_شدہ SIGEMT
	{ "EMT",	SIGEMT },	/* 7 (mips,alpha,sparc*) */
#اگر_ختم
#اگر_تعریف_شدہ SIGBUS
	{ "BUS",	SIGBUS },	/* 7 (arm,i386,m68k,ppc), 10 (mips,alpha,sparc*) */
#اگر_ختم
	{ "FPE",	SIGFPE },	/* 8 */
	{ "KILL",	SIGKILL },	/* 9 */
	{ "USR1",	SIGUSR1 },	/* 10 (arm,i386,m68k,ppc), 30 (alpha,sparc*), 16 (mips) */
	{ "SEGV",	SIGSEGV },	/* 11 */
	{ "USR2",	SIGUSR2 },	/* 12 (arm,i386,m68k,ppc), 31 (alpha,sparc*), 17 (mips) */
	{ "PIPE",	SIGPIPE },	/* 13 */
	{ "ALRM",	SIGALRM },	/* 14 */
	{ "TERM",	SIGTERM },	/* 15 */
#اگر_تعریف_شدہ SIGSTKFLT
	{ "STKFLT",	SIGSTKFLT },	/* 16 (arm,i386,m68k,ppc) */
#اگر_ختم
	{ "CHLD",	SIGCHLD },	/* 17 (arm,i386,m68k,ppc), 20 (alpha,sparc*), 18 (mips) */
#اگر_تعریف_شدہ SIGCLD
	{ "CLD",	SIGCLD },	/* same as SIGCHLD (mips) */
#اگر_ختم
	{ "CONT",	SIGCONT },	/* 18 (arm,i386,m68k,ppc), 19 (alpha,sparc*), 25 (mips) */
	{ "STOP",	SIGSTOP },	/* 19 (arm,i386,m68k,ppc), 17 (alpha,sparc*), 23 (mips) */
	{ "TSTP",	SIGTSTP },	/* 20 (arm,i386,m68k,ppc), 18 (alpha,sparc*), 24 (mips) */
	{ "TTIN",	SIGTTIN },	/* 21 (arm,i386,m68k,ppc,alpha,sparc*), 26 (mips) */
	{ "TTOU",	SIGTTOU },	/* 22 (arm,i386,m68k,ppc,alpha,sparc*), 27 (mips) */
#اگر_تعریف_شدہ SIGURG
	{ "URG",	SIGURG },	/* 23 (arm,i386,m68k,ppc), 16 (alpha,sparc*), 21 (mips) */
#اگر_ختم
#اگر_تعریف_شدہ SIGXCPU
	{ "XCPU",	SIGXCPU },	/* 24 (arm,i386,m68k,ppc,alpha,sparc*), 30 (mips) */
#اگر_ختم
#اگر_تعریف_شدہ SIGXFSZ
	{ "XFSZ",	SIGXFSZ },	/* 25 (arm,i386,m68k,ppc,alpha,sparc*), 31 (mips) */
#اگر_ختم
#اگر_تعریف_شدہ SIGVTALRM
	{ "VTALRM",	SIGVTALRM },	/* 26 (arm,i386,m68k,ppc,alpha,sparc*), 28 (mips) */
#اگر_ختم
#اگر_تعریف_شدہ SIGPROF
	{ "PROF",	SIGPROF },	/* 27 (arm,i386,m68k,ppc,alpha,sparc*), 29 (mips) */
#اگر_ختم
#اگر_تعریف_شدہ SIGWINCH
	{ "WINCH",	SIGWINCH },	/* 28 (arm,i386,m68k,ppc,alpha,sparc*), 20 (mips) */
#اگر_ختم
#اگر_تعریف_شدہ SIGIO
	{ "IO",		SIGIO },	/* 29 (arm,i386,m68k,ppc), 23 (alpha,sparc*), 22 (mips) */
#اگر_ختم
#اگر_تعریف_شدہ SIGPOLL
	{ "POLL",	SIGPOLL },	/* same as SIGIO */
#اگر_ختم
#اگر_تعریف_شدہ SIGINFO
	{ "INFO",	SIGINFO },	/* 29 (alpha) */
#اگر_ختم
#اگر_تعریف_شدہ SIGLOST
	{ "LOST",	SIGLOST },	/* 29 (arm,i386,m68k,ppc,sparc*) */
#اگر_ختم
#اگر_تعریف_شدہ SIGPWR
	{ "PWR",	SIGPWR },	/* 30 (arm,i386,m68k,ppc), 29 (alpha,sparc*), 19 (mips) */
#اگر_ختم
#اگر_تعریف_شدہ SIGUNUSED
	{ "UNUSED",	SIGUNUSED },	/* 31 (arm,i386,m68k,ppc) */
#اگر_ختم
#اگر_تعریف_شدہ SIGSYS
	{ "SYS",	SIGSYS },	/* 31 (mips,alpha,sparc*) */
#اگر_ختم
};

#اگر_تعریف_شدہ SIGRTMIN
ساکن صحیح rtsig_مقصد_signum(ثابت حرف *sig)
{
	صحیح عدد, maxi = 0;
	حرف *ep = تہی;

	اگر (c_strncasecmp(sig, "min+", 4) == 0)
		sig += 4;
	ورنہ اگر (c_strncasecmp(sig, "max-", 4) == 0) {
		sig += 4;
		maxi = 1;
	}
	اگر (!ہندسہ_ہے(*sig))
		واپس -1;
	errno = 0;
	عدد = strtol(sig, &ep, 10);
	اگر (!ep || sig == ep || errno || عدد < 0)
		واپس -1;
	عدد = maxi ? SIGRTMAX - عدد : SIGRTMIN + عدد;
	اگر (عدد < SIGRTMIN || SIGRTMAX < عدد)
		واپس -1;
	واپس عدد;
}
#اگر_ختم

صحیح signame_مقصد_signum(ثابت حرف *sig)
{
	size_t n;

	اگر (!c_strncasecmp(sig, "sig", 3))
		sig += 3;
#اگر_تعریف_شدہ SIGRTMIN
	/* RT signals */
	اگر (!c_strncasecmp(sig, "rt", 2))
		واپس rtsig_مقصد_signum(sig + 2);
#اگر_ختم
	/* Normal signals */
	برائے (n = 0; n < ARRAY_سائز(ul_signames); n++) {
		اگر (!c_strcasecmp(ul_signames[n].name, sig))
			واپس ul_signames[n].val;
	}
	واپس -1;
}

ثابت حرف *signum_مقصد_signame(صحیح signum)
{
	size_t n;

	برائے (n = 0; n < ARRAY_سائز(ul_signames); n++) {
		اگر (ul_signames[n].val == signum) {
			واپس ul_signames[n].name;
		}
	}

	واپس تہی;
}

صحیح get_signame_by_idx(size_t idx, ثابت حرف **signame, صحیح *signum)
{
	اگر (idx >= ARRAY_سائز(ul_signames))
		واپس -1;

	اگر (signame)
		*signame = ul_signames[idx].name;
	اگر (signum)
		*signum = ul_signames[idx].val;
	واپس 0;

}

