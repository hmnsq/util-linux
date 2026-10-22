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

ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *pager_argv[] = { "sh", "-c", ರಿಕ್ತ, ರಿಕ್ತ };

ಸಂರಚನಾ child_process {
	ಧ್ರುವ ವರ್ಣ **argv;
	pid_t pid;
	ಪೂರ್ಣಾಙ್ಕ in;

	ಪೂರ್ಣಾಙ್ಕ header_ಸಾಲುಗಳು;
	ಪೂರ್ಣಾಙ್ಕ header_width;

	ಪೂರ್ಣಾಙ್ಕ org_err;
	ಪೂರ್ಣಾಙ್ಕ org_out;
	ಸಂರಚನಾ sigaction orig_sigchld;
	ಸಂರಚನಾ sigaction orig_sigint;
	ಸಂರಚನಾ sigaction orig_sighup;
	ಸಂರಚನಾ sigaction orig_sigterm;
	ಸಂರಚನಾ sigaction orig_sigquit;
	ಸಂರಚನಾ sigaction orig_sigpipe;
};
ಸ್ಥಿರ ಸಂರಚನಾ child_process pager_process;

ಸ್ಥಿರ ಚಞ್ಚಲ sig_atomic_t pager_caught_signal;
ಸ್ಥಿರ ಚಞ್ಚಲ sig_atomic_t pager_caught_sigpipe;

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ close_pair(ಪೂರ್ಣಾಙ್ಕ fd[2])
{
	close(fd[0]);
	close(fd[1]);
}

ಸ್ಥಿರ ಶೂನ್ಯ pager_preexec(ಶೂನ್ಯ)
{
	ಧ್ರುವ ವರ್ಣ *less_env = getenv("LESS");
	ಧ್ರುವ ವರ್ಣ *base = less_env ? less_env : "FRSX";
	ಪೂರ್ಣಾಙ್ಕ header_ಸಾಲುಗಳು = pager_process.header_ಸಾಲುಗಳು;
	ಪೂರ್ಣಾಙ್ಕ header_width = pager_process.header_width;
	ವರ್ಣ *less_val = ರಿಕ್ತ;

	ಯದಿ (header_ಸಾಲುಗಳು > 0 && header_width > 0)
		less_val = ul_strfconcat(base, " --header %d,%d",
					 header_ಸಾಲುಗಳು, header_width);
	ಅನ್ಯಥಾ ಯದಿ (header_width > 0)
		less_val = ul_strfconcat(base, " --header 0,%d",
					 header_width);
	ಅನ್ಯಥಾ ಯದಿ (header_ಸಾಲುಗಳು > 0)
		less_val = ul_strfconcat(base, " --header %d",
					 header_ಸಾಲುಗಳು);

	ಯದಿ (less_val) {
		ಯದಿ (setenv("LESS", less_val, 1) != 0)
			warn(_("failed to set the %s environment variable"), "LESS");
		ಮುಕ್ತ(less_val);
	} ಅನ್ಯಥಾ ಯದಿ (less_env == ರಿಕ್ತ) {
		ಯದಿ (setenv("LESS", "FRSX", 0) != 0)
			warn(_("failed to set the %s environment variable"), "LESS");
	}

	ಯದಿ (getenv("LV") == ರಿಕ್ತ && setenv("LV", "-c", 0) != 0)
		warn(_("failed to set the %s environment variable"), "LV");
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ start_command(ಸಂರಚನಾ child_process *cmd)
{
	ಪೂರ್ಣಾಙ್ಕ fdin[2];

	ಯದಿ (pipe(fdin) < 0)
		ಪ್ರತ್ಯಾಗಮ -1;
	cmd->in = fdin[1];

	fflush(ರಿಕ್ತ);
	cmd->pid = fork();
	ಯದಿ (!cmd->pid) {
		dup2(fdin[0], STDIN_FILENO);
		close_pair(fdin);

		pager_preexec();
		execvp(cmd->argv[0], (ವರ್ಣ *ಧ್ರುವ*) cmd->argv);
		errexec(cmd->argv[0]);
	}

	ಯದಿ (cmd->pid < 0) {
		cmd->pid = 0;
		close_pair(fdin);
		ಪ್ರತ್ಯಾಗಮ -1;
	}

	close(fdin[0]);
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ wait_for_pager(ಶೂನ್ಯ)
{
	pid_t waiting;
	ಪೂರ್ಣಾಙ್ಕ ret, status;

	ಕುರು {
		waiting = waitpid(pager_process.pid, &status, 0);
	} ಯಾವತ್ (waiting == -1 && errno == EINTR);

	ಯದಿ (waiting == -1)
		ret = -1;
	ಅನ್ಯಥಾ ಯದಿ (waiting == pager_process.pid && WIFEXITED(status))
		ret = WEXITSTATUS(status);
	ಅನ್ಯಥಾ
		ret = 1;

	pager_process.pid = 0;

	ಪ್ರತ್ಯಾಗಮ ret;
}

ಸ್ಥಿರ ಶೂನ್ಯ catch_signal(ಪೂರ್ಣಾಙ್ಕ signo)
{
	pager_caught_signal = signo;
}

ಸ್ಥಿರ ಶೂನ್ಯ catch_sigpipe(ಪೂರ್ಣಾಙ್ಕ signo)
{
	pager_caught_sigpipe = signo;
}

ಸ್ಥಿರ ಶೂನ್ಯ wait_for_pager_signal(ಪೂರ್ಣಾಙ್ಕ signo __attribute__ ((__unused__)))
{
	/* signal EOF to pager */
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	/* async-signal safe: wait for all children, including pager */
	ಯಾವತ್ (wait(ರಿಕ್ತ) != -1 || errno == EINTR)
		;

	_exit(EXIT_FAILURE);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ has_command(ಧ್ರುವ ವರ್ಣ *cmd)
{
	ಧ್ರುವ ವರ್ಣ *path;
	ವರ್ಣ *b, *c, *p, *s;
	ಪೂರ್ಣಾಙ್ಕ rc = 0;

	ಯದಿ (!cmd)
		ಗಚ್ಛ done;

	c = xstrdup(cmd);
	ಯದಿ (!c)
		ಗಚ್ಛ done;
	b = strtok(c, " ");	/* cmd may contain options */
	ಯದಿ (!b)
		ಗಚ್ಛ cleanup;

	ಯದಿ (strchr(b, '/')) {
		rc = access(b, X_OK) == 0;
		ಗಚ್ಛ cleanup;
	}

	path = getenv("PATH");
	ಯದಿ (!path)
		ಗಚ್ಛ cleanup;
	p = xstrdup(path);
	ಯದಿ (!p)
		ಗಚ್ಛ cleanup;

	ಪ್ರತಿ (s = strtok(p, ":"); s; s = strtok(ರಿಕ್ತ, ":")) {
		ಪೂರ್ಣಾಙ್ಕ fd = open(s, O_RDONLY|O_CLOEXEC);
		ಯದಿ (fd < 0)
			ಅನುವರ್ತ;
		rc = faccessat(fd, b, X_OK, 0) == 0;
		close(fd);
		ಯದಿ (rc)
			ಭಙ್ಗ;
	}
	ಮುಕ್ತ(p);
cleanup:
	ಮುಕ್ತ(c);
done:
	/*fprintf(stderr, "has PAGER '%s': rc=%d\n", cmd, rc);*/
	ಪ್ರತ್ಯಾಗಮ rc;
}

ಸ್ಥಿರ ಶೂನ್ಯ __setup_pager(ಶೂನ್ಯ)
{
	ಧ್ರುವ ವರ್ಣ *pager = getenv("PAGER");
	ಸಂರಚನಾ sigaction sa;

	ಯದಿ (!isatty(STDOUT_FILENO))
		ಪ್ರತ್ಯಾಗಮ;

	ಯದಿ (!pager)
		pager = "less";
	ಅನ್ಯಥಾ ಯದಿ (!*pager || !strcmp(pager, "cat"))
		ಪ್ರತ್ಯಾಗಮ;

	ಯದಿ (!has_command(pager))
		ಪ್ರತ್ಯಾಗಮ;

	/* spawn the pager */
	pager_argv[2] = pager;
	pager_process.argv = pager_argv;
	pager_process.in = -1;

	memset(&sa, 0, ಪರಿಮಾಣ(sa));
	sa.sa_handler = SIG_DFL;

	/* this makes sure that waitpid works as expected */
	sigaction(SIGCHLD, &sa, &pager_process.orig_sigchld);

	ಯದಿ (start_command(&pager_process)) {
		sigaction(SIGCHLD, &pager_process.orig_sigchld, ರಿಕ್ತ);
		ಪ್ರತ್ಯಾಗಮ;
	}

	/* original process continues, but writes to the pipe */
	dup2(pager_process.in, STDOUT_FILENO);
	setvbuf(stdout, ರಿಕ್ತ, _IOLBF, 0);
	ಯದಿ (isatty(STDERR_FILENO)) {
		dup2(pager_process.in, STDERR_FILENO);
		setvbuf(stderr, ರಿಕ್ತ, _IOLBF, 0);
	}
	close(pager_process.in);

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
	pager_open_header(0, 0);
}

/* Setup pager with "less --header" support to freeze header lines and
 * optionally freeze the first column. The @header_lines specifies the
 * number of header lines to freeze (typically 1 for table header).
 * The @first_col_width specifies the number of character columns to
 * freeze (width of first column including separator), or 0 to not
 * freeze any column. Either parameter can be 0 independently;
 * less supports --header 0,M to freeze columns without header lines.
 */
ಶೂನ್ಯ pager_open_header(ಪೂರ್ಣಾಙ್ಕ header_ಸಾಲುಗಳು, ಪೂರ್ಣಾಙ್ಕ first_col_width)
{
	ಯದಿ (pager_process.pid)
		ಪ್ರತ್ಯಾಗಮ;		/* already running */

	pager_process.header_ಸಾಲುಗಳು = header_ಸಾಲುಗಳು;
	pager_process.header_width = first_col_width;

	pager_process.org_out = dup(STDOUT_FILENO);
	pager_process.org_err = dup(STDERR_FILENO);

	ಯದಿ (pager_process.org_out != -1 && pager_process.org_err != -1)
		__setup_pager();

	ಯದಿ (!pager_process.pid) {
		ಯದಿ (pager_process.org_out != -1)
			close(pager_process.org_out);
		ಯದಿ (pager_process.org_err != -1)
			close(pager_process.org_err);
		memset(&pager_process, 0, ಪರಿಮಾಣ(pager_process));
	}
}

/* Close pager and restore original std{out,err}.
 */
ಶೂನ್ಯ pager_close(ಶೂನ್ಯ)
{
	ಸಂರಚನಾ sigaction sa;
	ಪೂರ್ಣಾಙ್ಕ ret, safe_errno;

	ಯದಿ (pager_process.pid == 0)
		ಪ್ರತ್ಯಾಗಮ;

	memset(&sa, 0, ಪರಿಮಾಣ(sa));
	sa.sa_handler = catch_signal;

	/* set flag instead of calling wait_for_pager again */
	sigaction(SIGINT,  &sa, ರಿಕ್ತ);
	sigaction(SIGHUP,  &sa, ರಿಕ್ತ);
	sigaction(SIGTERM, &sa, ರಿಕ್ತ);
	sigaction(SIGQUIT, &sa, ರಿಕ್ತ);

	fflush(ರಿಕ್ತ);

	/* signal EOF to pager */
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	/* restore original output */
	clearerr(stdout);
	dup2(pager_process.org_out, STDOUT_FILENO);
	clearerr(stderr);
	dup2(pager_process.org_err, STDERR_FILENO);

	close(pager_process.org_out);
	close(pager_process.org_err);

	ret = wait_for_pager();

	/* restore original signal settings */
	safe_errno = errno;
	sigaction(SIGCHLD, &pager_process.orig_sigchld, ರಿಕ್ತ);
	sigaction(SIGINT,  &pager_process.orig_sigint, ರಿಕ್ತ);
	sigaction(SIGHUP,  &pager_process.orig_sighup, ರಿಕ್ತ);
	sigaction(SIGTERM, &pager_process.orig_sigterm, ರಿಕ್ತ);
	sigaction(SIGQUIT, &pager_process.orig_sigquit, ರಿಕ್ತ);
	sigaction(SIGPIPE, &pager_process.orig_sigpipe, ರಿಕ್ತ);
	errno = safe_errno;

	ಯದಿ (ret == -1)
		err(EXIT_FAILURE, _("waitpid failed"));

	ಯದಿ (pager_caught_signal || (pager_caught_sigpipe && ret))
		exit(EXIT_FAILURE);

	memset(&pager_process, 0, ಪರಿಮಾಣ(pager_process));
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
bool pager_is_enabled(ಗಣನಾ ul_pagermode mode)
{
	ವಿಕಲ್ಪ (mode) {
	ಪಕ್ಷ UL_PAGER_ALWAYS:
		ಪ್ರತ್ಯಾಗಮ true;
	ಪಕ್ಷ UL_PAGER_NEVER:
		ಪ್ರತ್ಯಾಗಮ false;
	ಪಕ್ಷ UL_PAGER_AUTO: {
		ಧ್ರುವ ವರ್ಣ *s;
		bool val;

		ಯದಿ (!isatty(STDOUT_FILENO))
			ಪ್ರತ್ಯಾಗಮ false;
		s = safe_getenv("PAGER_ENABLE");
		ಯದಿ (!s || ul_strtobool(s, &val) != 0)
			ಪ್ರತ್ಯಾಗಮ false;
		ಪ್ರತ್ಯಾಗಮ val;
	}
	ಸ್ವಭಾವ:
		ಪ್ರತ್ಯಾಗಮ false;
	}
}

#ifdef TEST_PROGRAM_PAGER

#define MAX 255

ಪೂರ್ಣಾಙ್ಕ main(ಪೂರ್ಣಾಙ್ಕ argc __attribute__ ((__unused__)),
	 ವರ್ಣ *argv[] __attribute__ ((__unused__)))
{
	ಪೂರ್ಣಾಙ್ಕ i;

	pager_open();
	ಪ್ರತಿ (i = 0; i < MAX; i++)
		printf("%d\n", i);
	pager_close();
	ಪ್ರತ್ಯಾಗಮ EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_PAGER */
