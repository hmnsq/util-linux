/*
 * No copyright is claimed.  This code is in the public domain; do with
 * it what you wish.

 * Written by:
 *  Sami Kerola <kerolasa@iki.fi>
 *  Karel Zak <kzak@redhat.com>
 *  Niklas Hambüchen <mail@nh2.me>
 */

#समावेश <ctype.h>		/* for isdigit() */
#समावेश <signal.h>
#समावेश <unistd.h>
#समावेश <string.h>
#समावेश <errno.h>

#समावेश "c.h"
#समावेश "cctype.h"
#समावेश "strutils.h"
#समावेश "signames.h"

अटल स्थिर काष्ठा ul_signal_name {
	स्थिर अक्षर *name;
	पूर्णांक val;
} ul_signames[] = {
	/* POSIX signals */
	{ "HUP",	SIGHUP },	/* 1 */
	{ "INT",	संक_विघ्न },	/* 2 */
	{ "QUIT",	SIGQUIT },	/* 3 */
	{ "ILL",	संक_अवैध },	/* 4 */
#अगर_घोषित SIGTRAP
	{ "TRAP",	SIGTRAP },	/* 5 */
#पूर्ण_अगर
	{ "ABRT",	SIGABRT },	/* 6 */
#अगर_घोषित SIGIOT
	{ "IOT",	SIGIOT },	/* 6, same as SIGABRT */
#पूर्ण_अगर
#अगर_घोषित SIGEMT
	{ "EMT",	SIGEMT },	/* 7 (mips,alpha,sparc*) */
#पूर्ण_अगर
#अगर_घोषित SIGBUS
	{ "BUS",	SIGBUS },	/* 7 (arm,i386,m68k,ppc), 10 (mips,alpha,sparc*) */
#पूर्ण_अगर
	{ "FPE",	संक_भ_त्रुटि },	/* 8 */
	{ "KILL",	SIGKILL },	/* 9 */
	{ "USR1",	SIGUSR1 },	/* 10 (arm,i386,m68k,ppc), 30 (alpha,sparc*), 16 (mips) */
	{ "SEGV",	संक_अंश },	/* 11 */
	{ "USR2",	SIGUSR2 },	/* 12 (arm,i386,m68k,ppc), 31 (alpha,sparc*), 17 (mips) */
	{ "PIPE",	SIGPIPE },	/* 13 */
	{ "ALRM",	SIGALRM },	/* 14 */
	{ "TERM",	संक_इति },	/* 15 */
#अगर_घोषित SIGSTKFLT
	{ "STKFLT",	SIGSTKFLT },	/* 16 (arm,i386,m68k,ppc) */
#पूर्ण_अगर
	{ "CHLD",	SIGCHLD },	/* 17 (arm,i386,m68k,ppc), 20 (alpha,sparc*), 18 (mips) */
#अगर_घोषित SIGCLD
	{ "CLD",	SIGCLD },	/* same as SIGCHLD (mips) */
#पूर्ण_अगर
	{ "CONT",	SIGCONT },	/* 18 (arm,i386,m68k,ppc), 19 (alpha,sparc*), 25 (mips) */
	{ "STOP",	SIGSTOP },	/* 19 (arm,i386,m68k,ppc), 17 (alpha,sparc*), 23 (mips) */
	{ "TSTP",	SIGTSTP },	/* 20 (arm,i386,m68k,ppc), 18 (alpha,sparc*), 24 (mips) */
	{ "TTIN",	SIGTTIN },	/* 21 (arm,i386,m68k,ppc,alpha,sparc*), 26 (mips) */
	{ "TTOU",	SIGTTOU },	/* 22 (arm,i386,m68k,ppc,alpha,sparc*), 27 (mips) */
#अगर_घोषित SIGURG
	{ "URG",	SIGURG },	/* 23 (arm,i386,m68k,ppc), 16 (alpha,sparc*), 21 (mips) */
#पूर्ण_अगर
#अगर_घोषित SIGXCPU
	{ "XCPU",	SIGXCPU },	/* 24 (arm,i386,m68k,ppc,alpha,sparc*), 30 (mips) */
#पूर्ण_अगर
#अगर_घोषित SIGXFSZ
	{ "XFSZ",	SIGXFSZ },	/* 25 (arm,i386,m68k,ppc,alpha,sparc*), 31 (mips) */
#पूर्ण_अगर
#अगर_घोषित SIGVTALRM
	{ "VTALRM",	SIGVTALRM },	/* 26 (arm,i386,m68k,ppc,alpha,sparc*), 28 (mips) */
#पूर्ण_अगर
#अगर_घोषित SIGPROF
	{ "PROF",	SIGPROF },	/* 27 (arm,i386,m68k,ppc,alpha,sparc*), 29 (mips) */
#पूर्ण_अगर
#अगर_घोषित SIGWINCH
	{ "WINCH",	SIGWINCH },	/* 28 (arm,i386,m68k,ppc,alpha,sparc*), 20 (mips) */
#पूर्ण_अगर
#अगर_घोषित SIGIO
	{ "IO",		SIGIO },	/* 29 (arm,i386,m68k,ppc), 23 (alpha,sparc*), 22 (mips) */
#पूर्ण_अगर
#अगर_घोषित SIGPOLL
	{ "POLL",	SIGPOLL },	/* same as SIGIO */
#पूर्ण_अगर
#अगर_घोषित SIGINFO
	{ "INFO",	SIGINFO },	/* 29 (alpha) */
#पूर्ण_अगर
#अगर_घोषित SIGLOST
	{ "LOST",	SIGLOST },	/* 29 (arm,i386,m68k,ppc,sparc*) */
#पूर्ण_अगर
#अगर_घोषित SIGPWR
	{ "PWR",	SIGPWR },	/* 30 (arm,i386,m68k,ppc), 29 (alpha,sparc*), 19 (mips) */
#पूर्ण_अगर
#अगर_घोषित SIGUNUSED
	{ "UNUSED",	SIGUNUSED },	/* 31 (arm,i386,m68k,ppc) */
#पूर्ण_अगर
#अगर_घोषित SIGSYS
	{ "SYS",	SIGSYS },	/* 31 (mips,alpha,sparc*) */
#पूर्ण_अगर
};

#अगर_घोषित SIGRTMIN
अटल पूर्णांक rtsig_to_signum(स्थिर अक्षर *sig)
{
	पूर्णांक अंक, maxi = 0;
	अक्षर *ep = शून्य;

	अगर (c_strncasecmp(sig, "min+", 4) == 0)
		sig += 4;
	अन्यथा अगर (c_strncasecmp(sig, "max-", 4) == 0) {
		sig += 4;
		maxi = 1;
	}
	अगर (!है_अंक(*sig))
		वापस -1;
	त्रुटि_सं = 0;
	अंक = म_से_दीर्घ(sig, &ep, 10);
	अगर (!ep || sig == ep || त्रुटि_सं || अंक < 0)
		वापस -1;
	अंक = maxi ? SIGRTMAX - अंक : SIGRTMIN + अंक;
	अगर (अंक < SIGRTMIN || SIGRTMAX < अंक)
		वापस -1;
	वापस अंक;
}
#पूर्ण_अगर

पूर्णांक signame_to_signum(स्थिर अक्षर *sig)
{
	माप_प्रकार n;

	अगर (!c_strncasecmp(sig, "sig", 3))
		sig += 3;
#अगर_घोषित SIGRTMIN
	/* RT signals */
	अगर (!c_strncasecmp(sig, "rt", 2))
		वापस rtsig_to_signum(sig + 2);
#पूर्ण_अगर
	/* Normal signals */
	क्रम (n = 0; n < ARRAY_आकार(ul_signames); n++) {
		अगर (!c_strcasecmp(ul_signames[n].name, sig))
			वापस ul_signames[n].val;
	}
	वापस -1;
}

स्थिर अक्षर *signum_to_signame(पूर्णांक signum)
{
	माप_प्रकार n;

	क्रम (n = 0; n < ARRAY_आकार(ul_signames); n++) {
		अगर (ul_signames[n].val == signum) {
			वापस ul_signames[n].name;
		}
	}

	वापस शून्य;
}

पूर्णांक get_signame_by_idx(माप_प्रकार idx, स्थिर अक्षर **signame, पूर्णांक *signum)
{
	अगर (idx >= ARRAY_आकार(ul_signames))
		वापस -1;

	अगर (signame)
		*signame = ul_signames[idx].name;
	अगर (signum)
		*signum = ul_signames[idx].val;
	वापस 0;

}

