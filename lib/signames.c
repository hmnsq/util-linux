/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.

 * Written by:
 *  Sami Kerola <kerolasa@iki.fi>
 *  Karel Zak <kzak@redhat.com>
 *  Niklas Hambüchen <mail@nh2.me>
 */

#అన్తర్భావ <ctype.h>		/* for isdigit() */
#అన్తర్భావ <signal.h>
#అన్తర్భావ <unistd.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <errno.h>

#అన్తర్భావ "c.h"
#అన్తర్భావ "cctype.h"
#అన్తర్భావ "strutils.h"
#అన్తర్భావ "signames.h"

స్థిర స్థిరరాశి నిర్మాణం ul_signal_name {
	స్థిరరాశి అక్షరం *name;
	పూర్ణాంకం val;
} ul_signames[] = {
	/* POSIX signals */
	{ "HUP",	SIGHUP },	/* 1 */
	{ "INT",	SIGINT },	/* 2 */
	{ "QUIT",	SIGQUIT },	/* 3 */
	{ "ILL",	SIGILL },	/* 4 */
#నిర్వచితమైతే SIGTRAP
	{ "TRAP",	SIGTRAP },	/* 5 */
#అయితే_ముగింపు
	{ "ABRT",	SIGABRT },	/* 6 */
#నిర్వచితమైతే SIGIOT
	{ "IOT",	SIGIOT },	/* 6, same as SIGABRT */
#అయితే_ముగింపు
#నిర్వచితమైతే SIGEMT
	{ "EMT",	SIGEMT },	/* 7 (mips,alpha,sparc*) */
#అయితే_ముగింపు
#నిర్వచితమైతే SIGBUS
	{ "BUS",	SIGBUS },	/* 7 (arm,i386,m68k,ppc), 10 (mips,alpha,sparc*) */
#అయితే_ముగింపు
	{ "FPE",	SIGFPE },	/* 8 */
	{ "KILL",	SIGKILL },	/* 9 */
	{ "USR1",	SIGUSR1 },	/* 10 (arm,i386,m68k,ppc), 30 (alpha,sparc*), 16 (mips) */
	{ "SEGV",	SIGSEGV },	/* 11 */
	{ "USR2",	SIGUSR2 },	/* 12 (arm,i386,m68k,ppc), 31 (alpha,sparc*), 17 (mips) */
	{ "PIPE",	SIGPIPE },	/* 13 */
	{ "ALRM",	SIGALRM },	/* 14 */
	{ "TERM",	SIGTERM },	/* 15 */
#నిర్వచితమైతే SIGSTKFLT
	{ "STKFLT",	SIGSTKFLT },	/* 16 (arm,i386,m68k,ppc) */
#అయితే_ముగింపు
	{ "CHLD",	SIGCHLD },	/* 17 (arm,i386,m68k,ppc), 20 (alpha,sparc*), 18 (mips) */
#నిర్వచితమైతే SIGCLD
	{ "CLD",	SIGCLD },	/* same as SIGCHLD (mips) */
#అయితే_ముగింపు
	{ "CONT",	SIGCONT },	/* 18 (arm,i386,m68k,ppc), 19 (alpha,sparc*), 25 (mips) */
	{ "STOP",	SIGSTOP },	/* 19 (arm,i386,m68k,ppc), 17 (alpha,sparc*), 23 (mips) */
	{ "TSTP",	SIGTSTP },	/* 20 (arm,i386,m68k,ppc), 18 (alpha,sparc*), 24 (mips) */
	{ "TTIN",	SIGTTIN },	/* 21 (arm,i386,m68k,ppc,alpha,sparc*), 26 (mips) */
	{ "TTOU",	SIGTTOU },	/* 22 (arm,i386,m68k,ppc,alpha,sparc*), 27 (mips) */
#నిర్వచితమైతే SIGURG
	{ "URG",	SIGURG },	/* 23 (arm,i386,m68k,ppc), 16 (alpha,sparc*), 21 (mips) */
#అయితే_ముగింపు
#నిర్వచితమైతే SIGXCPU
	{ "XCPU",	SIGXCPU },	/* 24 (arm,i386,m68k,ppc,alpha,sparc*), 30 (mips) */
#అయితే_ముగింపు
#నిర్వచితమైతే SIGXFSZ
	{ "XFSZ",	SIGXFSZ },	/* 25 (arm,i386,m68k,ppc,alpha,sparc*), 31 (mips) */
#అయితే_ముగింపు
#నిర్వచితమైతే SIGVTALRM
	{ "VTALRM",	SIGVTALRM },	/* 26 (arm,i386,m68k,ppc,alpha,sparc*), 28 (mips) */
#అయితే_ముగింపు
#నిర్వచితమైతే SIGPROF
	{ "PROF",	SIGPROF },	/* 27 (arm,i386,m68k,ppc,alpha,sparc*), 29 (mips) */
#అయితే_ముగింపు
#నిర్వచితమైతే SIGWINCH
	{ "WINCH",	SIGWINCH },	/* 28 (arm,i386,m68k,ppc,alpha,sparc*), 20 (mips) */
#అయితే_ముగింపు
#నిర్వచితమైతే SIGIO
	{ "IO",		SIGIO },	/* 29 (arm,i386,m68k,ppc), 23 (alpha,sparc*), 22 (mips) */
#అయితే_ముగింపు
#నిర్వచితమైతే SIGPOLL
	{ "POLL",	SIGPOLL },	/* same as SIGIO */
#అయితే_ముగింపు
#నిర్వచితమైతే SIGINFO
	{ "INFO",	SIGINFO },	/* 29 (alpha) */
#అయితే_ముగింపు
#నిర్వచితమైతే SIGLOST
	{ "LOST",	SIGLOST },	/* 29 (arm,i386,m68k,ppc,sparc*) */
#అయితే_ముగింపు
#నిర్వచితమైతే SIGPWR
	{ "PWR",	SIGPWR },	/* 30 (arm,i386,m68k,ppc), 29 (alpha,sparc*), 19 (mips) */
#అయితే_ముగింపు
#నిర్వచితమైతే SIGUNUSED
	{ "UNUSED",	SIGUNUSED },	/* 31 (arm,i386,m68k,ppc) */
#అయితే_ముగింపు
#నిర్వచితమైతే SIGSYS
	{ "SYS",	SIGSYS },	/* 31 (mips,alpha,sparc*) */
#అయితే_ముగింపు
};

#నిర్వచితమైతే SIGRTMIN
స్థిర పూర్ణాంకం rtsig_గమ్యం_signum(స్థిరరాశి అక్షరం *sig)
{
	పూర్ణాంకం అంకె, maxi = 0;
	అక్షరం *ep = ఖాళీ;

	ఒకవేళ (c_strncasecmp(sig, "min+", 4) == 0)
		sig += 4;
	లేకపోతే ఒకవేళ (c_strncasecmp(sig, "max-", 4) == 0) {
		sig += 4;
		maxi = 1;
	}
	ఒకవేళ (!అంకెనా(*sig))
		తిరిగి -1;
	errno = 0;
	అంకె = strtol(sig, &ep, 10);
	ఒకవేళ (!ep || sig == ep || errno || అంకె < 0)
		తిరిగి -1;
	అంకె = maxi ? SIGRTMAX - అంకె : SIGRTMIN + అంకె;
	ఒకవేళ (అంకె < SIGRTMIN || SIGRTMAX < అంకె)
		తిరిగి -1;
	తిరిగి అంకె;
}
#అయితే_ముగింపు

పూర్ణాంకం signame_గమ్యం_signum(స్థిరరాశి అక్షరం *sig)
{
	size_t n;

	ఒకవేళ (!c_strncasecmp(sig, "sig", 3))
		sig += 3;
#నిర్వచితమైతే SIGRTMIN
	/* RT signals */
	ఒకవేళ (!c_strncasecmp(sig, "rt", 2))
		తిరిగి rtsig_గమ్యం_signum(sig + 2);
#అయితే_ముగింపు
	/* Normal signals */
	కొరకు (n = 0; n < ARRAY_కొలత(ul_signames); n++) {
		ఒకవేళ (!c_strcasecmp(ul_signames[n].name, sig))
			తిరిగి ul_signames[n].val;
	}
	తిరిగి -1;
}

స్థిరరాశి అక్షరం *signum_గమ్యం_signame(పూర్ణాంకం signum)
{
	size_t n;

	కొరకు (n = 0; n < ARRAY_కొలత(ul_signames); n++) {
		ఒకవేళ (ul_signames[n].val == signum) {
			తిరిగి ul_signames[n].name;
		}
	}

	తిరిగి ఖాళీ;
}

పూర్ణాంకం get_signame_by_idx(size_t idx, స్థిరరాశి అక్షరం **signame, పూర్ణాంకం *signum)
{
	ఒకవేళ (idx >= ARRAY_కొలత(ul_signames))
		తిరిగి -1;

	ఒకవేళ (signame)
		*signame = ul_signames[idx].name;
	ఒకవేళ (signum)
		*signum = ul_signames[idx].val;
	తిరిగి 0;

}

