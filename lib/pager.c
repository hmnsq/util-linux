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

#അന്തര്ഭാവ <unistd.h>
#അന്തര്ഭാവ <stdbool.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <sys/types.h>
#അന്തര്ഭാവ <sys/stat.h>
#അന്തര്ഭാവ <sys/wait.h>
#അന്തര്ഭാവ <signal.h>

#അന്തര്ഭാവ "c.h"
#അന്തര്ഭാവ "xalloc.h"
#അന്തര്ഭാവ "nls.h"
#അന്തര്ഭാവ "strutils.h"
#അന്തര്ഭാവ "ttyutils.h"
#അന്തര്ഭാവ "pager.h"
#അന്തര്ഭാവ "env.h"

സ്ഥിതം സ്ഥിരം അക്ഷരം *pager_argv[] = { "sh", "-c", ശൂന്യ, ശൂന്യ };

ഘടന child_process {
	സ്ഥിരം അക്ഷരം **argv;
	pid_t pid;
	പൂർണ്ണസംഖ്യ in;

	പൂർണ്ണസംഖ്യ header_വരികൾ;
	പൂർണ്ണസംഖ്യ header_width;

	പൂർണ്ണസംഖ്യ org_err;
	പൂർണ്ണസംഖ്യ org_out;
	ഘടന sigaction orig_sigchld;
	ഘടന sigaction orig_sigint;
	ഘടന sigaction orig_sighup;
	ഘടന sigaction orig_sigterm;
	ഘടന sigaction orig_sigquit;
	ഘടന sigaction orig_sigpipe;
};
സ്ഥിതം ഘടന child_process pager_process;

സ്ഥിതം അസ്ഥിര sig_atomic_t pager_caught_signal;
സ്ഥിതം അസ്ഥിര sig_atomic_t pager_caught_sigpipe;

സ്ഥിതം വരിയിൽ ശൂന്യം close_pair(പൂർണ്ണസംഖ്യ fd[2])
{
	close(fd[0]);
	close(fd[1]);
}

സ്ഥിതം ശൂന്യം pager_preexec(ശൂന്യം)
{
	സ്ഥിരം അക്ഷരം *less_env = getenv("LESS");
	സ്ഥിരം അക്ഷരം *base = less_env ? less_env : "FRSX";
	പൂർണ്ണസംഖ്യ header_വരികൾ = pager_process.header_വരികൾ;
	പൂർണ്ണസംഖ്യ header_width = pager_process.header_width;
	അക്ഷരം *less_val = ശൂന്യ;

	എങ്കിൽ (header_വരികൾ > 0 && header_width > 0)
		less_val = ul_strfconcat(base, " --header %d,%d",
					 header_വരികൾ, header_width);
	അല്ലെങ്കിൽ എങ്കിൽ (header_width > 0)
		less_val = ul_strfconcat(base, " --header 0,%d",
					 header_width);
	അല്ലെങ്കിൽ എങ്കിൽ (header_വരികൾ > 0)
		less_val = ul_strfconcat(base, " --header %d",
					 header_വരികൾ);

	എങ്കിൽ (less_val) {
		എങ്കിൽ (setenv("LESS", less_val, 1) != 0)
			warn(_("failed to set the %s environment variable"), "LESS");
		സ്വതന്ത്രം(less_val);
	} അല്ലെങ്കിൽ എങ്കിൽ (less_env == ശൂന്യ) {
		എങ്കിൽ (setenv("LESS", "FRSX", 0) != 0)
			warn(_("failed to set the %s environment variable"), "LESS");
	}

	എങ്കിൽ (getenv("LV") == ശൂന്യ && setenv("LV", "-c", 0) != 0)
		warn(_("failed to set the %s environment variable"), "LV");
}

സ്ഥിതം പൂർണ്ണസംഖ്യ start_command(ഘടന child_process *cmd)
{
	പൂർണ്ണസംഖ്യ fdin[2];

	എങ്കിൽ (pipe(fdin) < 0)
		തിരികെ -1;
	cmd->in = fdin[1];

	fflush(ശൂന്യ);
	cmd->pid = fork();
	എങ്കിൽ (!cmd->pid) {
		dup2(fdin[0], STDIN_FILENO);
		close_pair(fdin);

		pager_preexec();
		execvp(cmd->argv[0], (അക്ഷരം *സ്ഥിരം*) cmd->argv);
		errexec(cmd->argv[0]);
	}

	എങ്കിൽ (cmd->pid < 0) {
		cmd->pid = 0;
		close_pair(fdin);
		തിരികെ -1;
	}

	close(fdin[0]);
	തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ wait_for_pager(ശൂന്യം)
{
	pid_t waiting;
	പൂർണ്ണസംഖ്യ ret, status;

	ചെയ്യുക {
		waiting = waitpid(pager_process.pid, &status, 0);
	} ആകുമ്പോൾ (waiting == -1 && errno == EINTR);

	എങ്കിൽ (waiting == -1)
		ret = -1;
	അല്ലെങ്കിൽ എങ്കിൽ (waiting == pager_process.pid && WIFEXITED(status))
		ret = WEXITSTATUS(status);
	അല്ലെങ്കിൽ
		ret = 1;

	pager_process.pid = 0;

	തിരികെ ret;
}

സ്ഥിതം ശൂന്യം catch_signal(പൂർണ്ണസംഖ്യ signo)
{
	pager_caught_signal = signo;
}

സ്ഥിതം ശൂന്യം catch_sigpipe(പൂർണ്ണസംഖ്യ signo)
{
	pager_caught_sigpipe = signo;
}

സ്ഥിതം ശൂന്യം wait_for_pager_signal(പൂർണ്ണസംഖ്യ signo __attribute__ ((__unused__)))
{
	/* signal EOF to pager */
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	/* async-signal safe: wait for all children, including pager */
	ആകുമ്പോൾ (wait(ശൂന്യ) != -1 || errno == EINTR)
		;

	_exit(EXIT_FAILURE);
}

സ്ഥിതം പൂർണ്ണസംഖ്യ has_command(സ്ഥിരം അക്ഷരം *cmd)
{
	സ്ഥിരം അക്ഷരം *path;
	അക്ഷരം *b, *c, *p, *s;
	പൂർണ്ണസംഖ്യ rc = 0;

	എങ്കിൽ (!cmd)
		പോകുക done;

	c = xstrdup(cmd);
	എങ്കിൽ (!c)
		പോകുക done;
	b = strtok(c, " ");	/* cmd may contain options */
	എങ്കിൽ (!b)
		പോകുക cleanup;

	എങ്കിൽ (strchr(b, '/')) {
		rc = access(b, X_OK) == 0;
		പോകുക cleanup;
	}

	path = getenv("PATH");
	എങ്കിൽ (!path)
		പോകുക cleanup;
	p = xstrdup(path);
	എങ്കിൽ (!p)
		പോകുക cleanup;

	വേണ്ടി (s = strtok(p, ":"); s; s = strtok(ശൂന്യ, ":")) {
		പൂർണ്ണസംഖ്യ fd = open(s, O_RDONLY|O_CLOEXEC);
		എങ്കിൽ (fd < 0)
			തുടരുക;
		rc = faccessat(fd, b, X_OK, 0) == 0;
		close(fd);
		എങ്കിൽ (rc)
			നിർത്തുക;
	}
	സ്വതന്ത്രം(p);
cleanup:
	സ്വതന്ത്രം(c);
done:
	/*fprintf(stderr, "has PAGER '%s': rc=%d\n", cmd, rc);*/
	തിരികെ rc;
}

സ്ഥിതം ശൂന്യം __setup_pager(ശൂന്യം)
{
	സ്ഥിരം അക്ഷരം *pager = getenv("PAGER");
	ഘടന sigaction sa;

	എങ്കിൽ (!isatty(STDOUT_FILENO))
		തിരികെ;

	എങ്കിൽ (!pager)
		pager = "less";
	അല്ലെങ്കിൽ എങ്കിൽ (!*pager || !strcmp(pager, "cat"))
		തിരികെ;

	എങ്കിൽ (!has_command(pager))
		തിരികെ;

	/* spawn the pager */
	pager_argv[2] = pager;
	pager_process.argv = pager_argv;
	pager_process.in = -1;

	memset(&sa, 0, വലുപ്പം(sa));
	sa.sa_handler = SIG_DFL;

	/* this makes sure that waitpid works as expected */
	sigaction(SIGCHLD, &sa, &pager_process.orig_sigchld);

	എങ്കിൽ (start_command(&pager_process)) {
		sigaction(SIGCHLD, &pager_process.orig_sigchld, ശൂന്യ);
		തിരികെ;
	}

	/* original process continues, but writes to the pipe */
	dup2(pager_process.in, STDOUT_FILENO);
	setvbuf(stdout, ശൂന്യ, _IOLBF, 0);
	എങ്കിൽ (isatty(STDERR_FILENO)) {
		dup2(pager_process.in, STDERR_FILENO);
		setvbuf(stderr, ശൂന്യ, _IOLBF, 0);
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
ശൂന്യം pager_open(ശൂന്യം)
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
ശൂന്യം pager_open_header(പൂർണ്ണസംഖ്യ header_വരികൾ, പൂർണ്ണസംഖ്യ first_col_width)
{
	എങ്കിൽ (pager_process.pid)
		തിരികെ;		/* already running */

	pager_process.header_വരികൾ = header_വരികൾ;
	pager_process.header_width = first_col_width;

	pager_process.org_out = dup(STDOUT_FILENO);
	pager_process.org_err = dup(STDERR_FILENO);

	എങ്കിൽ (pager_process.org_out != -1 && pager_process.org_err != -1)
		__setup_pager();

	എങ്കിൽ (!pager_process.pid) {
		എങ്കിൽ (pager_process.org_out != -1)
			close(pager_process.org_out);
		എങ്കിൽ (pager_process.org_err != -1)
			close(pager_process.org_err);
		memset(&pager_process, 0, വലുപ്പം(pager_process));
	}
}

/* Close pager and restore original std{out,err}.
 */
ശൂന്യം pager_close(ശൂന്യം)
{
	ഘടന sigaction sa;
	പൂർണ്ണസംഖ്യ ret, safe_errno;

	എങ്കിൽ (pager_process.pid == 0)
		തിരികെ;

	memset(&sa, 0, വലുപ്പം(sa));
	sa.sa_handler = catch_signal;

	/* set flag instead of calling wait_for_pager again */
	sigaction(SIGINT,  &sa, ശൂന്യ);
	sigaction(SIGHUP,  &sa, ശൂന്യ);
	sigaction(SIGTERM, &sa, ശൂന്യ);
	sigaction(SIGQUIT, &sa, ശൂന്യ);

	fflush(ശൂന്യ);

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
	sigaction(SIGCHLD, &pager_process.orig_sigchld, ശൂന്യ);
	sigaction(SIGINT,  &pager_process.orig_sigint, ശൂന്യ);
	sigaction(SIGHUP,  &pager_process.orig_sighup, ശൂന്യ);
	sigaction(SIGTERM, &pager_process.orig_sigterm, ശൂന്യ);
	sigaction(SIGQUIT, &pager_process.orig_sigquit, ശൂന്യ);
	sigaction(SIGPIPE, &pager_process.orig_sigpipe, ശൂന്യ);
	errno = safe_errno;

	എങ്കിൽ (ret == -1)
		err(EXIT_FAILURE, _("waitpid failed"));

	എങ്കിൽ (pager_caught_signal || (pager_caught_sigpipe && ret))
		exit(EXIT_FAILURE);

	memset(&pager_process, 0, വലുപ്പം(pager_process));
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
bool pager_is_enabled(എണ്ണൽ ul_pagermode mode)
{
	തിരഞ്ഞെടുപ്പ് (mode) {
	അവസ്ഥ UL_PAGER_ALWAYS:
		തിരികെ true;
	അവസ്ഥ UL_PAGER_NEVER:
		തിരികെ false;
	അവസ്ഥ UL_PAGER_AUTO: {
		സ്ഥിരം അക്ഷരം *s;
		bool val;

		എങ്കിൽ (!isatty(STDOUT_FILENO))
			തിരികെ false;
		s = safe_getenv("PAGER_ENABLE");
		എങ്കിൽ (!s || ul_strtobool(s, &val) != 0)
			തിരികെ false;
		തിരികെ val;
	}
	സ്ഥിരസ്ഥിതി:
		തിരികെ false;
	}
}

#ifdef TEST_PROGRAM_PAGER

#define MAX 255

പൂർണ്ണസംഖ്യ പ്രധാന(പൂർണ്ണസംഖ്യ argc __attribute__ ((__unused__)),
	 അക്ഷരം *argv[] __attribute__ ((__unused__)))
{
	പൂർണ്ണസംഖ്യ i;

	pager_open();
	വേണ്ടി (i = 0; i < MAX; i++)
		printf("%d\n", i);
	pager_close();
	തിരികെ EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_PAGER */
