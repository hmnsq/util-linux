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

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ ul_signal_name {
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name;
	ಪೂರ್ಣಾಂಕ val;
} ul_signames[] = {
	/* POSIX signals */
	{ "HUP",	SIGHUP },	/* 1 */
	{ "INT",	SIGINT },	/* 2 */
	{ "QUIT",	SIGQUIT },	/* 3 */
	{ "ILL",	SIGILL },	/* 4 */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SIGTRAP
	{ "TRAP",	SIGTRAP },	/* 5 */
#ಆದರೆ_ಅಂತ್ಯ
	{ "ABRT",	SIGABRT },	/* 6 */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SIGIOT
	{ "IOT",	SIGIOT },	/* 6, same as SIGABRT */
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SIGEMT
	{ "EMT",	SIGEMT },	/* 7 (mips,alpha,sparc*) */
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SIGBUS
	{ "BUS",	SIGBUS },	/* 7 (arm,i386,m68k,ppc), 10 (mips,alpha,sparc*) */
#ಆದರೆ_ಅಂತ್ಯ
	{ "FPE",	SIGFPE },	/* 8 */
	{ "KILL",	SIGKILL },	/* 9 */
	{ "USR1",	SIGUSR1 },	/* 10 (arm,i386,m68k,ppc), 30 (alpha,sparc*), 16 (mips) */
	{ "SEGV",	SIGSEGV },	/* 11 */
	{ "USR2",	SIGUSR2 },	/* 12 (arm,i386,m68k,ppc), 31 (alpha,sparc*), 17 (mips) */
	{ "PIPE",	SIGPIPE },	/* 13 */
	{ "ALRM",	SIGALRM },	/* 14 */
	{ "TERM",	SIGTERM },	/* 15 */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SIGSTKFLT
	{ "STKFLT",	SIGSTKFLT },	/* 16 (arm,i386,m68k,ppc) */
#ಆದರೆ_ಅಂತ್ಯ
	{ "CHLD",	SIGCHLD },	/* 17 (arm,i386,m68k,ppc), 20 (alpha,sparc*), 18 (mips) */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SIGCLD
	{ "CLD",	SIGCLD },	/* same as SIGCHLD (mips) */
#ಆದರೆ_ಅಂತ್ಯ
	{ "CONT",	SIGCONT },	/* 18 (arm,i386,m68k,ppc), 19 (alpha,sparc*), 25 (mips) */
	{ "STOP",	SIGSTOP },	/* 19 (arm,i386,m68k,ppc), 17 (alpha,sparc*), 23 (mips) */
	{ "TSTP",	SIGTSTP },	/* 20 (arm,i386,m68k,ppc), 18 (alpha,sparc*), 24 (mips) */
	{ "TTIN",	SIGTTIN },	/* 21 (arm,i386,m68k,ppc,alpha,sparc*), 26 (mips) */
	{ "TTOU",	SIGTTOU },	/* 22 (arm,i386,m68k,ppc,alpha,sparc*), 27 (mips) */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SIGURG
	{ "URG",	SIGURG },	/* 23 (arm,i386,m68k,ppc), 16 (alpha,sparc*), 21 (mips) */
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SIGXCPU
	{ "XCPU",	SIGXCPU },	/* 24 (arm,i386,m68k,ppc,alpha,sparc*), 30 (mips) */
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SIGXFSZ
	{ "XFSZ",	SIGXFSZ },	/* 25 (arm,i386,m68k,ppc,alpha,sparc*), 31 (mips) */
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SIGVTALRM
	{ "VTALRM",	SIGVTALRM },	/* 26 (arm,i386,m68k,ppc,alpha,sparc*), 28 (mips) */
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SIGPROF
	{ "PROF",	SIGPROF },	/* 27 (arm,i386,m68k,ppc,alpha,sparc*), 29 (mips) */
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SIGWINCH
	{ "WINCH",	SIGWINCH },	/* 28 (arm,i386,m68k,ppc,alpha,sparc*), 20 (mips) */
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SIGIO
	{ "IO",		SIGIO },	/* 29 (arm,i386,m68k,ppc), 23 (alpha,sparc*), 22 (mips) */
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SIGPOLL
	{ "POLL",	SIGPOLL },	/* same as SIGIO */
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SIGINFO
	{ "INFO",	SIGINFO },	/* 29 (alpha) */
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SIGLOST
	{ "LOST",	SIGLOST },	/* 29 (arm,i386,m68k,ppc,sparc*) */
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SIGPWR
	{ "PWR",	SIGPWR },	/* 30 (arm,i386,m68k,ppc), 29 (alpha,sparc*), 19 (mips) */
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SIGUNUSED
	{ "UNUSED",	SIGUNUSED },	/* 31 (arm,i386,m68k,ppc) */
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SIGSYS
	{ "SYS",	SIGSYS },	/* 31 (mips,alpha,sparc*) */
#ಆದರೆ_ಅಂತ್ಯ
};

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SIGRTMIN
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ rtsig_ಗಮ್ಯ_signum(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sig)
{
	ಪೂರ್ಣಾಂಕ ಅಂಕೆ, maxi = 0;
	ಅಕ್ಷರ *ep = ಖಾಲಿ;

	ಆದರೆ (c_strncasecmp(sig, "min+", 4) == 0)
		sig += 4;
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (c_strncasecmp(sig, "max-", 4) == 0) {
		sig += 4;
		maxi = 1;
	}
	ಆದರೆ (!ಅಂಕಿಯೇ(*sig))
		ಹಿಂತಿರುಗಿ -1;
	errno = 0;
	ಅಂಕೆ = strtol(sig, &ep, 10);
	ಆದರೆ (!ep || sig == ep || errno || ಅಂಕೆ < 0)
		ಹಿಂತಿರುಗಿ -1;
	ಅಂಕೆ = maxi ? SIGRTMAX - ಅಂಕೆ : SIGRTMIN + ಅಂಕೆ;
	ಆದರೆ (ಅಂಕೆ < SIGRTMIN || SIGRTMAX < ಅಂಕೆ)
		ಹಿಂತಿರುಗಿ -1;
	ಹಿಂತಿರುಗಿ ಅಂಕೆ;
}
#ಆದರೆ_ಅಂತ್ಯ

ಪೂರ್ಣಾಂಕ signame_ಗಮ್ಯ_signum(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sig)
{
	size_t n;

	ಆದರೆ (!c_strncasecmp(sig, "sig", 3))
		sig += 3;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ SIGRTMIN
	/* RT signals */
	ಆದರೆ (!c_strncasecmp(sig, "rt", 2))
		ಹಿಂತಿರುಗಿ rtsig_ಗಮ್ಯ_signum(sig + 2);
#ಆದರೆ_ಅಂತ್ಯ
	/* Normal signals */
	ಗಾಗಿ (n = 0; n < ARRAY_ಗಾತ್ರ(ul_signames); n++) {
		ಆದರೆ (!c_strcasecmp(ul_signames[n].name, sig))
			ಹಿಂತಿರುಗಿ ul_signames[n].val;
	}
	ಹಿಂತಿರುಗಿ -1;
}

ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *signum_ಗಮ್ಯ_signame(ಪೂರ್ಣಾಂಕ signum)
{
	size_t n;

	ಗಾಗಿ (n = 0; n < ARRAY_ಗಾತ್ರ(ul_signames); n++) {
		ಆದರೆ (ul_signames[n].val == signum) {
			ಹಿಂತಿರುಗಿ ul_signames[n].name;
		}
	}

	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

ಪೂರ್ಣಾಂಕ get_signame_by_idx(size_t idx, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **signame, ಪೂರ್ಣಾಂಕ *signum)
{
	ಆದರೆ (idx >= ARRAY_ಗಾತ್ರ(ul_signames))
		ಹಿಂತಿರುಗಿ -1;

	ಆದರೆ (signame)
		*signame = ul_signames[idx].name;
	ಆದರೆ (signum)
		*signum = ul_signames[idx].val;
	ಹಿಂತಿರುಗಿ 0;

}

