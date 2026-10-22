/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.

 * Written by:
 *  Sami Kerola <kerolasa@iki.fi>
 *  Karel Zak <kzak@redhat.com>
 *  Niklas Hambüchen <mail@nh2.me>
 */

#अन्तर्भाव <ctype.h>		/* for isdigit() */
#अन्तर्भाव <signal.h>
#अन्तर्भाव <unistd.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <errno.h>

#अन्तर्भाव "c.h"
#अन्तर्भाव "cctype.h"
#अन्तर्भाव "strutils.h"
#अन्तर्भाव "signames.h"

स्थिर स्थिरांक संरचना ul_signal_name {
	स्थिरांक अक्षर *name;
	पूर्णांक val;
} ul_signames[] = {
	/* POSIX signals */
	{ "HUP",	SIGHUP },	/* 1 */
	{ "INT",	संक_विघ्न },	/* 2 */
	{ "QUIT",	SIGQUIT },	/* 3 */
	{ "ILL",	संक_अवैध },	/* 4 */
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
	{ "FPE",	संक_भ_त्रुटि },	/* 8 */
	{ "KILL",	SIGKILL },	/* 9 */
	{ "USR1",	SIGUSR1 },	/* 10 (arm,i386,m68k,ppc), 30 (alpha,sparc*), 16 (mips) */
	{ "SEGV",	संक_अंश },	/* 11 */
	{ "USR2",	SIGUSR2 },	/* 12 (arm,i386,m68k,ppc), 31 (alpha,sparc*), 17 (mips) */
	{ "PIPE",	SIGPIPE },	/* 13 */
	{ "ALRM",	SIGALRM },	/* 14 */
	{ "TERM",	संक_इति },	/* 15 */
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
स्थिर पूर्णांक rtsig_to_signum(स्थिरांक अक्षर *sig)
{
	पूर्णांक अंक, maxi = 0;
	अक्षर *ep = रिक्त;

	जर (c_strncasecmp(sig, "min+", 4) == 0)
		sig += 4;
	अन्यथा जर (c_strncasecmp(sig, "max-", 4) == 0) {
		sig += 4;
		maxi = 1;
	}
	जर (!है_अंक(*sig))
		परत -1;
	त्रुटि_सं = 0;
	अंक = म_से_दीर्घ(sig, &ep, 10);
	जर (!ep || sig == ep || त्रुटि_सं || अंक < 0)
		परत -1;
	अंक = maxi ? SIGRTMAX - अंक : SIGRTMIN + अंक;
	जर (अंक < SIGRTMIN || SIGRTMAX < अंक)
		परत -1;
	परत अंक;
}
#endif

पूर्णांक signame_to_signum(स्थिरांक अक्षर *sig)
{
	माप_प्रकार n;

	जर (!c_strncasecmp(sig, "sig", 3))
		sig += 3;
#ifdef SIGRTMIN
	/* RT signals */
	जर (!c_strncasecmp(sig, "rt", 2))
		परत rtsig_to_signum(sig + 2);
#endif
	/* Normal signals */
	साठी (n = 0; n < ARRAY_SIZE(ul_signames); n++) {
		जर (!c_strcasecmp(ul_signames[n].name, sig))
			परत ul_signames[n].val;
	}
	परत -1;
}

स्थिरांक अक्षर *signum_to_signame(पूर्णांक signum)
{
	माप_प्रकार n;

	साठी (n = 0; n < ARRAY_SIZE(ul_signames); n++) {
		जर (ul_signames[n].val == signum) {
			परत ul_signames[n].name;
		}
	}

	परत रिक्त;
}

पूर्णांक get_signame_by_idx(माप_प्रकार idx, स्थिरांक अक्षर **signame, पूर्णांक *signum)
{
	जर (idx >= ARRAY_SIZE(ul_signames))
		परत -1;

	जर (signame)
		*signame = ul_signames[idx].name;
	जर (signum)
		*signum = ul_signames[idx].val;
	परत 0;

}

