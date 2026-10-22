/*
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * This file may be redistributed under the terms of the
 * GNU General Public License.
 *
 * Based on linux-perf/git scm
 *
 * Some modifications and simplifications for util-linux
 * by Davidlohr Bueso <dave@xxxxxxx> - March 2012.
 */

#ಅನ್ತರ್ಭಾವ <unistd.h>
#ಅನ್ತರ್ಭಾವ <stdbool.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <sys/types.h>
#ಅನ್ತರ್ಭಾವ <sys/stat.h>
#ಅನ್ತರ್ಭಾವ <sys/wait.h>
#ಅನ್ತರ್ಭಾವ <signal.h>

#ಅನ್ತರ್ಭಾವ "c.h"
#ಅನ್ತರ್ಭಾವ "xalloc.h"
#ಅನ್ತರ್ಭಾವ "nls.h"
#ಅನ್ತರ್ಭಾವ "strutils.h"
#ಅನ್ತರ್ಭಾವ "ttyutils.h"
#ಅನ್ತರ್ಭಾವ "pager.h"
#ಅನ್ತರ್ಭಾವ "env.h"

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *pager_argv[] = { "sh", "-c", ಖಾಲಿ, ಖಾಲಿ };

ರಚನೆ ಮಗು_process {
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **argv;
	pid_t pid;
	ಪೂರ್ಣಾಂಕ ಒಳ;

	ಪೂರ್ಣಾಂಕ ಶೀರ್ಷಿಕೆ_ಸಾಲುಗಳು;
	ಪೂರ್ಣಾಂಕ ಶೀರ್ಷಿಕೆ_ಅಗಲ;

	ಪೂರ್ಣಾಂಕ org_err;
	ಪೂರ್ಣಾಂಕ org_ಹೊರಹೋಗು;
	ರಚನೆ sigaction orig_sigchld;
	ರಚನೆ sigaction orig_sigint;
	ರಚನೆ sigaction orig_sighup;
	ರಚನೆ sigaction orig_sigterm;
	ರಚನೆ sigaction orig_sigquit;
	ರಚನೆ sigaction orig_sigpipe;
};
ಸ್ಥಿರ ರಚನೆ ಮಗು_process pager_process;

ಸ್ಥಿರ ಅಸ್ಥಿರ sig_atomic_t pager_caught_signal;
ಸ್ಥಿರ ಅಸ್ಥಿರ sig_atomic_t pager_caught_sigpipe;

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ close_pair(ಪೂರ್ಣಾಂಕ fd[2])
{
	close(fd[0]);
	close(fd[1]);
}

ಸ್ಥಿರ ಶೂನ್ಯ pager_preexec(ಶೂನ್ಯ)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *less_env = getenv("LESS");
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ತಳ = less_env ? less_env : "FRSX";
	ಪೂರ್ಣಾಂಕ ಶೀರ್ಷಿಕೆ_ಸಾಲುಗಳು = pager_process.ಶೀರ್ಷಿಕೆ_ಸಾಲುಗಳು;
	ಪೂರ್ಣಾಂಕ ಶೀರ್ಷಿಕೆ_ಅಗಲ = pager_process.ಶೀರ್ಷಿಕೆ_ಅಗಲ;
	ಅಕ್ಷರ *less_val = ಖಾಲಿ;

	ಆದರೆ (ಶೀರ್ಷಿಕೆ_ಸಾಲುಗಳು > 0 && ಶೀರ್ಷಿಕೆ_ಅಗಲ > 0)
		less_val = ul_strfconcat(ತಳ, " --header %d,%d",
					 ಶೀರ್ಷಿಕೆ_ಸಾಲುಗಳು, ಶೀರ್ಷಿಕೆ_ಅಗಲ);
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಶೀರ್ಷಿಕೆ_ಅಗಲ > 0)
		less_val = ul_strfconcat(ತಳ, " --header 0,%d",
					 ಶೀರ್ಷಿಕೆ_ಅಗಲ);
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಶೀರ್ಷಿಕೆ_ಸಾಲುಗಳು > 0)
		less_val = ul_strfconcat(ತಳ, " --header %d",
					 ಶೀರ್ಷಿಕೆ_ಸಾಲುಗಳು);

	ಆದರೆ (less_val) {
		ಆದರೆ (setenv("LESS", less_val, 1) != 0)
			warn(_("failed to set the %s environment variable"), "LESS");
		ಬಿಡುಗಡೆ(less_val);
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (less_env == ಖಾಲಿ) {
		ಆದರೆ (setenv("LESS", "FRSX", 0) != 0)
			warn(_("failed to set the %s environment variable"), "LESS");
	}

	ಆದರೆ (getenv("LV") == ಖಾಲಿ && setenv("LV", "-c", 0) != 0)
		warn(_("failed to set the %s environment variable"), "LV");
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಆರಂಭ_command(ರಚನೆ ಮಗು_process *cmd)
{
	ಪೂರ್ಣಾಂಕ fdin[2];

	ಆದರೆ (pipe(fdin) < 0)
		ಹಿಂತಿರುಗಿ -1;
	cmd->ಒಳ = fdin[1];

	fflush(ಖಾಲಿ);
	cmd->pid = fork();
	ಆದರೆ (!cmd->pid) {
		dup2(fdin[0], STDIN_FILENO);
		close_pair(fdin);

		pager_preexec();
		execvp(cmd->argv[0], (ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ*) cmd->argv);
		errexec(cmd->argv[0]);
	}

	ಆದರೆ (cmd->pid < 0) {
		cmd->pid = 0;
		close_pair(fdin);
		ಹಿಂತಿರುಗಿ -1;
	}

	close(fdin[0]);
	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ wait_for_pager(ಶೂನ್ಯ)
{
	pid_t ಕಾಯುವಿಕೆ;
	ಪೂರ್ಣಾಂಕ ret, ಸ್ಥಿತಿ_2;

	ಮಾಡು {
		ಕಾಯುವಿಕೆ = waitpid(pager_process.pid, &ಸ್ಥಿತಿ_2, 0);
	} ಆಗ (ಕಾಯುವಿಕೆ == -1 && errno == EINTR);

	ಆದರೆ (ಕಾಯುವಿಕೆ == -1)
		ret = -1;
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಕಾಯುವಿಕೆ == pager_process.pid && WIFEXITED(ಸ್ಥಿತಿ_2))
		ret = WEXITSTATUS(ಸ್ಥಿತಿ_2);
	ಇಲ್ಲದಿದ್ದರೆ
		ret = 1;

	pager_process.pid = 0;

	ಹಿಂತಿರುಗಿ ret;
}

ಸ್ಥಿರ ಶೂನ್ಯ catch_signal(ಪೂರ್ಣಾಂಕ signo)
{
	pager_caught_signal = signo;
}

ಸ್ಥಿರ ಶೂನ್ಯ catch_sigpipe(ಪೂರ್ಣಾಂಕ signo)
{
	pager_caught_sigpipe = signo;
}

ಸ್ಥಿರ ಶೂನ್ಯ wait_for_pager_signal(ಪೂರ್ಣಾಂಕ signo __attribute__ ((__unused__)))
{
	/* signal EOF to pager */
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	/* async-signal safe: wait for all children, including pager */
	ಆಗ (wait(ಖಾಲಿ) != -1 || errno == EINTR)
		;

	_exit(EXIT_FAILURE);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ has_command(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *cmd)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ;
	ಅಕ್ಷರ *b, *c, *p, *s;
	ಪೂರ್ಣಾಂಕ rc = 0;

	ಆದರೆ (!cmd)
		ಹೋಗು ಮುಗಿದ;

	c = xstrdup(cmd);
	ಆದರೆ (!c)
		ಹೋಗು ಮುಗಿದ;
	b = strtok(c, " ");	/* cmd may contain options */
	ಆದರೆ (!b)
		ಹೋಗು cleanup;

	ಆದರೆ (strchr(b, '/')) {
		rc = access(b, X_OK) == 0;
		ಹೋಗು cleanup;
	}

	ಮಾರ್ಗ = getenv("PATH");
	ಆದರೆ (!ಮಾರ್ಗ)
		ಹೋಗು cleanup;
	p = xstrdup(ಮಾರ್ಗ);
	ಆದರೆ (!p)
		ಹೋಗು cleanup;

	ಗಾಗಿ (s = strtok(p, ":"); s; s = strtok(ಖಾಲಿ, ":")) {
		ಪೂರ್ಣಾಂಕ fd = open(s, O_RDONLY|O_CLOEXEC);
		ಆದರೆ (fd < 0)
			ಮುಂದುವರಿ;
		rc = faccessat(fd, b, X_OK, 0) == 0;
		close(fd);
		ಆದರೆ (rc)
			ನಿಲ್ಲಿಸು;
	}
	ಬಿಡುಗಡೆ(p);
cleanup:
	ಬಿಡುಗಡೆ(c);
ಮುಗಿದ:
	/*fprintf(stderr, "has PAGER '%s': rc=%d\n", cmd, rc);*/
	ಹಿಂತಿರುಗಿ rc;
}

ಸ್ಥಿರ ಶೂನ್ಯ __setup_pager(ಶೂನ್ಯ)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *pager = getenv("PAGER");
	ರಚನೆ sigaction sa;

	ಆದರೆ (!isatty(STDOUT_FILENO))
		ಹಿಂತಿರುಗಿ;

	ಆದರೆ (!pager)
		pager = "less";
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (!*pager || !strcmp(pager, "cat"))
		ಹಿಂತಿರುಗಿ;

	ಆದರೆ (!has_command(pager))
		ಹಿಂತಿರುಗಿ;

	/* spawn the pager */
	pager_argv[2] = pager;
	pager_process.argv = pager_argv;
	pager_process.ಒಳ = -1;

	memset(&sa, 0, ಗಾತ್ರ(sa));
	sa.sa_handler = SIG_DFL;

	/* this makes sure that waitpid works as expected */
	sigaction(SIGCHLD, &sa, &pager_process.orig_sigchld);

	ಆದರೆ (ಆರಂಭ_command(&pager_process)) {
		sigaction(SIGCHLD, &pager_process.orig_sigchld, ಖಾಲಿ);
		ಹಿಂತಿರುಗಿ;
	}

	/* original process continues, but writes to the pipe */
	dup2(pager_process.ಒಳ, STDOUT_FILENO);
	setvbuf(stdout, ಖಾಲಿ, _IOLBF, 0);
	ಆದರೆ (isatty(STDERR_FILENO)) {
		dup2(pager_process.ಒಳ, STDERR_FILENO);
		setvbuf(stderr, ಖಾಲಿ, _IOLBF, 0);
	}
	close(pager_process.ಒಳ);

	sa.sa_handler = wait_for_pager_signal;

	/* this makes sure that the parent terminates after the pager */
	sigaction(SIGINT,  &sa, &pager_process.orig_sigint);
	sigaction(SIGHUP,  &sa, &pager_process.orig_sighup);
	sigaction(SIGTERM, &sa, &pager_process.orig_sigterm);
	sigaction(SIGQUIT, &sa, &pager_process.orig_sigquit);

	sa.sa_handler = catch_sigpipe;

	/* this allows graceful handling of premature termination of pager */
	sigaction(SIGPIPE, &sa, &pager_process.orig_sigpipe);
}

/* Setup pager and redirect output, the pager may be closed by pager_close().
 */
ಶೂನ್ಯ pager_open(ಶೂನ್ಯ)
{
	pager_open_ಶೀರ್ಷಿಕೆ(0, 0);
}

/* Setup pager with "less --header" support to freeze header lines and
 * optionally freeze the first column. The @header_lines specifies the
 * number of header lines to freeze (typically 1 for table header).
 * The @first_col_width specifies the number of character columns to
 * freeze (width of first column including separator), or 0 to not
 * freeze any column. Either parameter can be 0 independently;
 * less supports --header 0,M to freeze columns without header lines.
 */
ಶೂನ್ಯ pager_open_ಶೀರ್ಷಿಕೆ(ಪೂರ್ಣಾಂಕ ಶೀರ್ಷಿಕೆ_ಸಾಲುಗಳು, ಪೂರ್ಣಾಂಕ ಮೊದಲ_ಲಂಬಸಾಲು_ಅಗಲ)
{
	ಆದರೆ (pager_process.pid)
		ಹಿಂತಿರುಗಿ;		/* already running */

	pager_process.ಶೀರ್ಷಿಕೆ_ಸಾಲುಗಳು = ಶೀರ್ಷಿಕೆ_ಸಾಲುಗಳು;
	pager_process.ಶೀರ್ಷಿಕೆ_ಅಗಲ = ಮೊದಲ_ಲಂಬಸಾಲು_ಅಗಲ;

	pager_process.org_ಹೊರಹೋಗು = dup(STDOUT_FILENO);
	pager_process.org_err = dup(STDERR_FILENO);

	ಆದರೆ (pager_process.org_ಹೊರಹೋಗು != -1 && pager_process.org_err != -1)
		__setup_pager();

	ಆದರೆ (!pager_process.pid) {
		ಆದರೆ (pager_process.org_ಹೊರಹೋಗು != -1)
			close(pager_process.org_ಹೊರಹೋಗು);
		ಆದರೆ (pager_process.org_err != -1)
			close(pager_process.org_err);
		memset(&pager_process, 0, ಗಾತ್ರ(pager_process));
	}
}

/* Close pager and restore original std{out,err}.
 */
ಶೂನ್ಯ pager_close(ಶೂನ್ಯ)
{
	ರಚನೆ sigaction sa;
	ಪೂರ್ಣಾಂಕ ret, ಸುರಕ್ಷಿತ_errno;

	ಆದರೆ (pager_process.pid == 0)
		ಹಿಂತಿರುಗಿ;

	memset(&sa, 0, ಗಾತ್ರ(sa));
	sa.sa_handler = catch_signal;

	/* set flag instead of calling wait_for_pager again */
	sigaction(SIGINT,  &sa, ಖಾಲಿ);
	sigaction(SIGHUP,  &sa, ಖಾಲಿ);
	sigaction(SIGTERM, &sa, ಖಾಲಿ);
	sigaction(SIGQUIT, &sa, ಖಾಲಿ);

	fflush(ಖಾಲಿ);

	/* signal EOF to pager */
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	/* restore original output */
	clearerr(stdout);
	dup2(pager_process.org_ಹೊರಹೋಗು, STDOUT_FILENO);
	clearerr(stderr);
	dup2(pager_process.org_err, STDERR_FILENO);

	close(pager_process.org_ಹೊರಹೋಗು);
	close(pager_process.org_err);

	ret = wait_for_pager();

	/* restore original signal settings */
	ಸುರಕ್ಷಿತ_errno = errno;
	sigaction(SIGCHLD, &pager_process.orig_sigchld, ಖಾಲಿ);
	sigaction(SIGINT,  &pager_process.orig_sigint, ಖಾಲಿ);
	sigaction(SIGHUP,  &pager_process.orig_sighup, ಖಾಲಿ);
	sigaction(SIGTERM, &pager_process.orig_sigterm, ಖಾಲಿ);
	sigaction(SIGQUIT, &pager_process.orig_sigquit, ಖಾಲಿ);
	sigaction(SIGPIPE, &pager_process.orig_sigpipe, ಖಾಲಿ);
	errno = ಸುರಕ್ಷಿತ_errno;

	ಆದರೆ (ret == -1)
		err(EXIT_FAILURE, _("waitpid failed"));

	ಆದರೆ (pager_caught_signal || (pager_caught_sigpipe && ret))
		ನಿರ್ಗಮಿಸು(EXIT_FAILURE);

	memset(&pager_process, 0, ಗಾತ್ರ(pager_process));
	pager_caught_signal = 0;
	pager_caught_sigpipe = 0;
}

/* Decide whether the pager should run for the given @mode.
 *
 * UL_PAGER_ALWAYS and UL_PAGER_NEVER reflect an explicit user choice
 * (--pager / --nopager) and are honored unconditionally.
 *
 * UL_PAGER_AUTO consults the PAGER_ENABLE environment variable (accepted
 * values parsed by ul_strtobool()). PAGER_ENABLE is ignored when stdout
 * is not a terminal, so pipelines like `lslocks | grep foo` do not spawn
 * a pager. The variable is also suppressed in privileged (SUID/SGID)
 * contexts via safe_getenv().
 */
bool pager_is_enabled(ಎಣಿಕೆ ul_pagermode ಬಹುಲಕ)
{
	ಆಯ್ಕೆ (ಬಹುಲಕ) {
	ಸ್ಥಿತಿ UL_PAGER_ALWAYS:
		ಹಿಂತಿರುಗಿ true;
	ಸ್ಥಿತಿ UL_PAGER_NEVER:
		ಹಿಂತಿರುಗಿ false;
	ಸ್ಥಿತಿ UL_PAGER_AUTO: {
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s;
		bool val;

		ಆದರೆ (!isatty(STDOUT_FILENO))
			ಹಿಂತಿರುಗಿ false;
		s = ಸುರಕ್ಷಿತ_getenv("PAGER_ENABLE");
		ಆದರೆ (!s || ul_strtobool(s, &val) != 0)
			ಹಿಂತಿರುಗಿ false;
		ಹಿಂತಿರುಗಿ val;
	}
	ಪೂರ್ವನಿಯೋಜಿತ:
		ಹಿಂತಿರುಗಿ false;
	}
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ TEST_ಕಾರ್ಯಕ್ರಮ_PAGER

#ವ್ಯಾಖ್ಯಾನಿಸು MAX 255

ಪೂರ್ಣಾಂಕ ಮುಖ್ಯ(ಪೂರ್ಣಾಂಕ argc __attribute__ ((__unused__)),
	 ಅಕ್ಷರ *argv[] __attribute__ ((__unused__)))
{
	ಪೂರ್ಣಾಂಕ i;

	pager_open();
	ಗಾಗಿ (i = 0; i < MAX; i++)
		ಬರೆ("%d\n", i);
	pager_close();
	ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
}
#ಆದರೆ_ಅಂತ್ಯ /* TEST_PROGRAM_PAGER */
