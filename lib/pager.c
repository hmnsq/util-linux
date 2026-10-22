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

#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdbool.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/types.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/stat.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/wait.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <signal.h>

#ਅਨ੍ਤਰ੍ਭਾਵ "c.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "xalloc.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "nls.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "strutils.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "ttyutils.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "pager.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "env.h"

ਸਥਿਰ ਅਚਲ ਅੱਖਰ *pager_argv[] = { "sh", "-c", ਨਿਰਾ, ਨਿਰਾ };

ਬਣਤਰ ਬੱਚਾ_process {
	ਅਚਲ ਅੱਖਰ **argv;
	pid_t pid;
	ਪੂਰਨ ਵਿੱਚ;

	ਪੂਰਨ ਸਿਰਲੇਖ_ਸਤਰਾਂ;
	ਪੂਰਨ ਸਿਰਲੇਖ_ਚੌੜਾਈ;

	ਪੂਰਨ org_err;
	ਪੂਰਨ org_ਬਾਹਰਲਾ;
	ਬਣਤਰ sigaction orig_sigchld;
	ਬਣਤਰ sigaction orig_sigint;
	ਬਣਤਰ sigaction orig_sighup;
	ਬਣਤਰ sigaction orig_sigterm;
	ਬਣਤਰ sigaction orig_sigquit;
	ਬਣਤਰ sigaction orig_sigpipe;
};
ਸਥਿਰ ਬਣਤਰ ਬੱਚਾ_process pager_process;

ਸਥਿਰ ਅਸਥਿਰ sig_atomic_t pager_caught_signal;
ਸਥਿਰ ਅਸਥਿਰ sig_atomic_t pager_caught_sigpipe;

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ close_pair(ਪੂਰਨ fd[2])
{
	close(fd[0]);
	close(fd[1]);
}

ਸਥਿਰ ਖਾਲੀ pager_preexec(ਖਾਲੀ)
{
	ਅਚਲ ਅੱਖਰ *less_env = getenv("LESS");
	ਅਚਲ ਅੱਖਰ *ਅਧਾਰ = less_env ? less_env : "FRSX";
	ਪੂਰਨ ਸਿਰਲੇਖ_ਸਤਰਾਂ = pager_process.ਸਿਰਲੇਖ_ਸਤਰਾਂ;
	ਪੂਰਨ ਸਿਰਲੇਖ_ਚੌੜਾਈ = pager_process.ਸਿਰਲੇਖ_ਚੌੜਾਈ;
	ਅੱਖਰ *less_val = ਨਿਰਾ;

	ਜੇ (ਸਿਰਲੇਖ_ਸਤਰਾਂ > 0 && ਸਿਰਲੇਖ_ਚੌੜਾਈ > 0)
		less_val = ul_strfconcat(ਅਧਾਰ, " --header %d,%d",
					 ਸਿਰਲੇਖ_ਸਤਰਾਂ, ਸਿਰਲੇਖ_ਚੌੜਾਈ);
	ਨਹੀਂਤਾਂ ਜੇ (ਸਿਰਲੇਖ_ਚੌੜਾਈ > 0)
		less_val = ul_strfconcat(ਅਧਾਰ, " --header 0,%d",
					 ਸਿਰਲੇਖ_ਚੌੜਾਈ);
	ਨਹੀਂਤਾਂ ਜੇ (ਸਿਰਲੇਖ_ਸਤਰਾਂ > 0)
		less_val = ul_strfconcat(ਅਧਾਰ, " --header %d",
					 ਸਿਰਲੇਖ_ਸਤਰਾਂ);

	ਜੇ (less_val) {
		ਜੇ (setenv("LESS", less_val, 1) != 0)
			warn(_("failed to set the %s environment variable"), "LESS");
		ਮੁਕਤ(less_val);
	} ਨਹੀਂਤਾਂ ਜੇ (less_env == ਨਿਰਾ) {
		ਜੇ (setenv("LESS", "FRSX", 0) != 0)
			warn(_("failed to set the %s environment variable"), "LESS");
	}

	ਜੇ (getenv("LV") == ਨਿਰਾ && setenv("LV", "-c", 0) != 0)
		warn(_("failed to set the %s environment variable"), "LV");
}

ਸਥਿਰ ਪੂਰਨ ਸ਼ੁਰੂ_command(ਬਣਤਰ ਬੱਚਾ_process *cmd)
{
	ਪੂਰਨ fdin[2];

	ਜੇ (pipe(fdin) < 0)
		ਵਾਪਸ -1;
	cmd->ਵਿੱਚ = fdin[1];

	fflush(ਨਿਰਾ);
	cmd->pid = fork();
	ਜੇ (!cmd->pid) {
		dup2(fdin[0], STDIN_FILENO);
		close_pair(fdin);

		pager_preexec();
		execvp(cmd->argv[0], (ਅੱਖਰ *ਅਚਲ*) cmd->argv);
		errexec(cmd->argv[0]);
	}

	ਜੇ (cmd->pid < 0) {
		cmd->pid = 0;
		close_pair(fdin);
		ਵਾਪਸ -1;
	}

	close(fdin[0]);
	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ wait_for_pager(ਖਾਲੀ)
{
	pid_t ਉਡੀਕ;
	ਪੂਰਨ ret, ਸਥਿਤੀ;

	ਕਰੋ {
		ਉਡੀਕ = waitpid(pager_process.pid, &ਸਥਿਤੀ, 0);
	} ਜਦੋਂ (ਉਡੀਕ == -1 && errno == EINTR);

	ਜੇ (ਉਡੀਕ == -1)
		ret = -1;
	ਨਹੀਂਤਾਂ ਜੇ (ਉਡੀਕ == pager_process.pid && WIFEXITED(ਸਥਿਤੀ))
		ret = WEXITSTATUS(ਸਥਿਤੀ);
	ਨਹੀਂਤਾਂ
		ret = 1;

	pager_process.pid = 0;

	ਵਾਪਸ ret;
}

ਸਥਿਰ ਖਾਲੀ catch_signal(ਪੂਰਨ signo)
{
	pager_caught_signal = signo;
}

ਸਥਿਰ ਖਾਲੀ catch_sigpipe(ਪੂਰਨ signo)
{
	pager_caught_sigpipe = signo;
}

ਸਥਿਰ ਖਾਲੀ wait_for_pager_signal(ਪੂਰਨ signo __attribute__ ((__unused__)))
{
	/* signal EOF to pager */
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	/* async-signal safe: wait for all children, including pager */
	ਜਦੋਂ (wait(ਨਿਰਾ) != -1 || errno == EINTR)
		;

	_exit(EXIT_FAILURE);
}

ਸਥਿਰ ਪੂਰਨ has_command(ਅਚਲ ਅੱਖਰ *cmd)
{
	ਅਚਲ ਅੱਖਰ *ਰਸਤਾ;
	ਅੱਖਰ *b, *c, *p, *s;
	ਪੂਰਨ rc = 0;

	ਜੇ (!cmd)
		ਜਾਓ ਪੂਰਾ;

	c = xstrdup(cmd);
	ਜੇ (!c)
		ਜਾਓ ਪੂਰਾ;
	b = strtok(c, " ");	/* cmd may contain options */
	ਜੇ (!b)
		ਜਾਓ cleanup;

	ਜੇ (strchr(b, '/')) {
		rc = access(b, X_OK) == 0;
		ਜਾਓ cleanup;
	}

	ਰਸਤਾ = getenv("PATH");
	ਜੇ (!ਰਸਤਾ)
		ਜਾਓ cleanup;
	p = xstrdup(ਰਸਤਾ);
	ਜੇ (!p)
		ਜਾਓ cleanup;

	ਲਈ (s = strtok(p, ":"); s; s = strtok(ਨਿਰਾ, ":")) {
		ਪੂਰਨ fd = open(s, O_RDONLY|O_CLOEXEC);
		ਜੇ (fd < 0)
			ਜਾਰੀ;
		rc = faccessat(fd, b, X_OK, 0) == 0;
		close(fd);
		ਜੇ (rc)
			ਰੋਕੋ;
	}
	ਮੁਕਤ(p);
cleanup:
	ਮੁਕਤ(c);
ਪੂਰਾ:
	/*fprintf(stderr, "has PAGER '%s': rc=%d\n", cmd, rc);*/
	ਵਾਪਸ rc;
}

ਸਥਿਰ ਖਾਲੀ __setup_pager(ਖਾਲੀ)
{
	ਅਚਲ ਅੱਖਰ *pager = getenv("PAGER");
	ਬਣਤਰ sigaction sa;

	ਜੇ (!isatty(STDOUT_FILENO))
		ਵਾਪਸ;

	ਜੇ (!pager)
		pager = "less";
	ਨਹੀਂਤਾਂ ਜੇ (!*pager || !strcmp(pager, "cat"))
		ਵਾਪਸ;

	ਜੇ (!has_command(pager))
		ਵਾਪਸ;

	/* spawn the pager */
	pager_argv[2] = pager;
	pager_process.argv = pager_argv;
	pager_process.ਵਿੱਚ = -1;

	memset(&sa, 0, ਆਕਾਰ(sa));
	sa.sa_handler = SIG_DFL;

	/* this makes sure that waitpid works as expected */
	sigaction(SIGCHLD, &sa, &pager_process.orig_sigchld);

	ਜੇ (ਸ਼ੁਰੂ_command(&pager_process)) {
		sigaction(SIGCHLD, &pager_process.orig_sigchld, ਨਿਰਾ);
		ਵਾਪਸ;
	}

	/* original process continues, but writes to the pipe */
	dup2(pager_process.ਵਿੱਚ, STDOUT_FILENO);
	setvbuf(stdout, ਨਿਰਾ, _IOLBF, 0);
	ਜੇ (isatty(STDERR_FILENO)) {
		dup2(pager_process.ਵਿੱਚ, STDERR_FILENO);
		setvbuf(stderr, ਨਿਰਾ, _IOLBF, 0);
	}
	close(pager_process.ਵਿੱਚ);

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
ਖਾਲੀ pager_open(ਖਾਲੀ)
{
	pager_open_ਸਿਰਲੇਖ(0, 0);
}

/* Setup pager with "less --header" support to freeze header lines and
 * optionally freeze the first column. The @header_lines specifies the
 * number of header lines to freeze (typically 1 for table header).
 * The @first_col_width specifies the number of character columns to
 * freeze (width of first column including separator), or 0 to not
 * freeze any column. Either parameter can be 0 independently;
 * less supports --header 0,M to freeze columns without header lines.
 */
ਖਾਲੀ pager_open_ਸਿਰਲੇਖ(ਪੂਰਨ ਸਿਰਲੇਖ_ਸਤਰਾਂ, ਪੂਰਨ ਪਹਿਲਾ_ਖਾਨਾ_ਚੌੜਾਈ)
{
	ਜੇ (pager_process.pid)
		ਵਾਪਸ;		/* already running */

	pager_process.ਸਿਰਲੇਖ_ਸਤਰਾਂ = ਸਿਰਲੇਖ_ਸਤਰਾਂ;
	pager_process.ਸਿਰਲੇਖ_ਚੌੜਾਈ = ਪਹਿਲਾ_ਖਾਨਾ_ਚੌੜਾਈ;

	pager_process.org_ਬਾਹਰਲਾ = dup(STDOUT_FILENO);
	pager_process.org_err = dup(STDERR_FILENO);

	ਜੇ (pager_process.org_ਬਾਹਰਲਾ != -1 && pager_process.org_err != -1)
		__setup_pager();

	ਜੇ (!pager_process.pid) {
		ਜੇ (pager_process.org_ਬਾਹਰਲਾ != -1)
			close(pager_process.org_ਬਾਹਰਲਾ);
		ਜੇ (pager_process.org_err != -1)
			close(pager_process.org_err);
		memset(&pager_process, 0, ਆਕਾਰ(pager_process));
	}
}

/* Close pager and restore original std{out,err}.
 */
ਖਾਲੀ pager_close(ਖਾਲੀ)
{
	ਬਣਤਰ sigaction sa;
	ਪੂਰਨ ret, ਸੁਰੱਖਿਅਤ_errno;

	ਜੇ (pager_process.pid == 0)
		ਵਾਪਸ;

	memset(&sa, 0, ਆਕਾਰ(sa));
	sa.sa_handler = catch_signal;

	/* set flag instead of calling wait_for_pager again */
	sigaction(SIGINT,  &sa, ਨਿਰਾ);
	sigaction(SIGHUP,  &sa, ਨਿਰਾ);
	sigaction(SIGTERM, &sa, ਨਿਰਾ);
	sigaction(SIGQUIT, &sa, ਨਿਰਾ);

	fflush(ਨਿਰਾ);

	/* signal EOF to pager */
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	/* restore original output */
	clearerr(stdout);
	dup2(pager_process.org_ਬਾਹਰਲਾ, STDOUT_FILENO);
	clearerr(stderr);
	dup2(pager_process.org_err, STDERR_FILENO);

	close(pager_process.org_ਬਾਹਰਲਾ);
	close(pager_process.org_err);

	ret = wait_for_pager();

	/* restore original signal settings */
	ਸੁਰੱਖਿਅਤ_errno = errno;
	sigaction(SIGCHLD, &pager_process.orig_sigchld, ਨਿਰਾ);
	sigaction(SIGINT,  &pager_process.orig_sigint, ਨਿਰਾ);
	sigaction(SIGHUP,  &pager_process.orig_sighup, ਨਿਰਾ);
	sigaction(SIGTERM, &pager_process.orig_sigterm, ਨਿਰਾ);
	sigaction(SIGQUIT, &pager_process.orig_sigquit, ਨਿਰਾ);
	sigaction(SIGPIPE, &pager_process.orig_sigpipe, ਨਿਰਾ);
	errno = ਸੁਰੱਖਿਅਤ_errno;

	ਜੇ (ret == -1)
		err(EXIT_FAILURE, _("waitpid failed"));

	ਜੇ (pager_caught_signal || (pager_caught_sigpipe && ret))
		ਬਾਹਰ_ਨਿਕਲੋ(EXIT_FAILURE);

	memset(&pager_process, 0, ਆਕਾਰ(pager_process));
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
bool pager_is_enabled(ਗਿਣਤੀ ul_pagermode ਬਹੁਲਕ)
{
	ਚੋਣ (ਬਹੁਲਕ) {
	ਹਾਲਤ UL_PAGER_ALWAYS:
		ਵਾਪਸ true;
	ਹਾਲਤ UL_PAGER_NEVER:
		ਵਾਪਸ false;
	ਹਾਲਤ UL_PAGER_AUTO: {
		ਅਚਲ ਅੱਖਰ *s;
		bool val;

		ਜੇ (!isatty(STDOUT_FILENO))
			ਵਾਪਸ false;
		s = ਸੁਰੱਖਿਅਤ_getenv("PAGER_ENABLE");
		ਜੇ (!s || ul_strtobool(s, &val) != 0)
			ਵਾਪਸ false;
		ਵਾਪਸ val;
	}
	ਮੂਲ:
		ਵਾਪਸ false;
	}
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ TEST_ਪ੍ਰੋਗਰਾਮ_PAGER

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAX 255

ਪੂਰਨ ਮੁੱਖ(ਪੂਰਨ argc __attribute__ ((__unused__)),
	 ਅੱਖਰ *argv[] __attribute__ ((__unused__)))
{
	ਪੂਰਨ i;

	pager_open();
	ਲਈ (i = 0; i < MAX; i++)
		ਲਿਖੋ("%d\n", i);
	pager_close();
	ਵਾਪਸ EXIT_SUCCESS;
}
#ਜੇ_ਅੰਤ /* TEST_PROGRAM_PAGER */
