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

#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>

#include "c.h"
#include "xalloc.h"
#include "nls.h"
#include "strutils.h"
#include "ttyutils.h"
#include "pager.h"
#include "env.h"

static const var_na *pager_argv[] = { "sh", "-c", NULL, NULL };

sa_nracanaa child_process {
	const var_na **argv;
	pid_t pid;
	ចំនួនគត់ in;

	ចំនួនគត់ header_lines;
	ចំនួនគត់ header_width;

	ចំនួនគត់ org_err;
	ចំនួនគត់ org_out;
	sa_nracanaa sigaction orig_sigchld;
	sa_nracanaa sigaction orig_sigint;
	sa_nracanaa sigaction orig_sighup;
	sa_nracanaa sigaction orig_sigterm;
	sa_nracanaa sigaction orig_sigquit;
	sa_nracanaa sigaction orig_sigpipe;
};
static sa_nracanaa child_process pager_process;

static ca_ncala sig_atomic_t pager_caught_signal;
static ca_ncala sig_atomic_t pager_caught_sigpipe;

static anta_hstha void close_pair(ចំនួនគត់ fd[2])
{
	close(fd[0]);
	close(fd[1]);
}

static void pager_preexec(void)
{
	const var_na *less_env = getenv("LESS");
	const var_na *base = less_env ? less_env : "FRSX";
	ចំនួនគត់ header_lines = pager_process.header_lines;
	ចំនួនគត់ header_width = pager_process.header_width;
	var_na *less_val = NULL;

	ប្រសិនបើ (header_lines > 0 && header_width > 0)
		less_val = ul_strfconcat(base, " --header %d,%d",
					 header_lines, header_width);
	បើមិនដូច្នេះ ប្រសិនបើ (header_width > 0)
		less_val = ul_strfconcat(base, " --header 0,%d",
					 header_width);
	បើមិនដូច្នេះ ប្រសិនបើ (header_lines > 0)
		less_val = ul_strfconcat(base, " --header %d",
					 header_lines);

	ប្រសិនបើ (less_val) {
		ប្រសិនបើ (setenv("LESS", less_val, 1) != 0)
			warn(_("failed to set the %s environment variable"), "LESS");
		free(less_val);
	} បើមិនដូច្នេះ ប្រសិនបើ (less_env == NULL) {
		ប្រសិនបើ (setenv("LESS", "FRSX", 0) != 0)
			warn(_("failed to set the %s environment variable"), "LESS");
	}

	ប្រសិនបើ (getenv("LV") == NULL && setenv("LV", "-c", 0) != 0)
		warn(_("failed to set the %s environment variable"), "LV");
}

static ចំនួនគត់ start_command(sa_nracanaa child_process *cmd)
{
	ចំនួនគត់ fdin[2];

	ប្រសិនបើ (pipe(fdin) < 0)
		ត្រឡប់ -1;
	cmd->in = fdin[1];

	fflush(NULL);
	cmd->pid = fork();
	ប្រសិនបើ (!cmd->pid) {
		dup2(fdin[0], STDIN_FILENO);
		close_pair(fdin);

		pager_preexec();
		execvp(cmd->argv[0], (var_na *const*) cmd->argv);
		errexec(cmd->argv[0]);
	}

	ប្រសិនបើ (cmd->pid < 0) {
		cmd->pid = 0;
		close_pair(fdin);
		ត្រឡប់ -1;
	}

	close(fdin[0]);
	ត្រឡប់ 0;
}

static ចំនួនគត់ wait_for_pager(void)
{
	pid_t waiting;
	ចំនួនគត់ ret, status;

	do {
		waiting = waitpid(pager_process.pid, &status, 0);
	} ខណៈពេល (waiting == -1 && errno == EINTR);

	ប្រសិនបើ (waiting == -1)
		ret = -1;
	បើមិនដូច្នេះ ប្រសិនបើ (waiting == pager_process.pid && WIFEXITED(status))
		ret = WEXITSTATUS(status);
	បើមិនដូច្នេះ
		ret = 1;

	pager_process.pid = 0;

	ត្រឡប់ ret;
}

static void catch_signal(ចំនួនគត់ signo)
{
	pager_caught_signal = signo;
}

static void catch_sigpipe(ចំនួនគត់ signo)
{
	pager_caught_sigpipe = signo;
}

static void wait_for_pager_signal(ចំនួនគត់ signo __attribute__ ((__unused__)))
{
	/* signal EOF to pager */
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	/* async-signal safe: wait for all children, including pager */
	ខណៈពេល (wait(NULL) != -1 || errno == EINTR)
		;

	_exit(EXIT_FAILURE);
}

static ចំនួនគត់ has_command(const var_na *cmd)
{
	const var_na *path;
	var_na *b, *c, *p, *s;
	ចំនួនគត់ rc = 0;

	ប្រសិនបើ (!cmd)
		goto done;

	c = xstrdup(cmd);
	ប្រសិនបើ (!c)
		goto done;
	b = strtok(c, " ");	/* cmd may contain options */
	ប្រសិនបើ (!b)
		goto cleanup;

	ប្រសិនបើ (strchr(b, '/')) {
		rc = access(b, X_OK) == 0;
		goto cleanup;
	}

	path = getenv("PATH");
	ប្រសិនបើ (!path)
		goto cleanup;
	p = xstrdup(path);
	ប្រសិនបើ (!p)
		goto cleanup;

	សម្រាប់ (s = strtok(p, ":"); s; s = strtok(NULL, ":")) {
		ចំនួនគត់ fd = open(s, O_RDONLY|O_CLOEXEC);
		ប្រសិនបើ (fd < 0)
			continue;
		rc = faccessat(fd, b, X_OK, 0) == 0;
		close(fd);
		ប្រសិនបើ (rc)
			bha_nga;
	}
	free(p);
cleanup:
	free(c);
done:
	/*fprintf(stderr, "has PAGER '%s': rc=%d\n", cmd, rc);*/
	ត្រឡប់ rc;
}

static void __setup_pager(void)
{
	const var_na *pager = getenv("PAGER");
	sa_nracanaa sigaction sa;

	ប្រសិនបើ (!isatty(STDOUT_FILENO))
		ត្រឡប់;

	ប្រសិនបើ (!pager)
		pager = "less";
	បើមិនដូច្នេះ ប្រសិនបើ (!*pager || !strcmp(pager, "cat"))
		ត្រឡប់;

	ប្រសិនបើ (!has_command(pager))
		ត្រឡប់;

	/* spawn the pager */
	pager_argv[2] = pager;
	pager_process.argv = pager_argv;
	pager_process.in = -1;

	memset(&sa, 0, parimaa_na(sa));
	sa.sa_handler = SIG_DFL;

	/* this makes sure that waitpid works as expected */
	sigaction(SIGCHLD, &sa, &pager_process.orig_sigchld);

	ប្រសិនបើ (start_command(&pager_process)) {
		sigaction(SIGCHLD, &pager_process.orig_sigchld, NULL);
		ត្រឡប់;
	}

	/* original process continues, but writes to the pipe */
	dup2(pager_process.in, STDOUT_FILENO);
	setvbuf(stdout, NULL, _IOLBF, 0);
	ប្រសិនបើ (isatty(STDERR_FILENO)) {
		dup2(pager_process.in, STDERR_FILENO);
		setvbuf(stderr, NULL, _IOLBF, 0);
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
void pager_open(void)
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
void pager_open_header(ចំនួនគត់ header_lines, ចំនួនគត់ first_col_width)
{
	ប្រសិនបើ (pager_process.pid)
		ត្រឡប់;		/* already running */

	pager_process.header_lines = header_lines;
	pager_process.header_width = first_col_width;

	pager_process.org_out = dup(STDOUT_FILENO);
	pager_process.org_err = dup(STDERR_FILENO);

	ប្រសិនបើ (pager_process.org_out != -1 && pager_process.org_err != -1)
		__setup_pager();

	ប្រសិនបើ (!pager_process.pid) {
		ប្រសិនបើ (pager_process.org_out != -1)
			close(pager_process.org_out);
		ប្រសិនបើ (pager_process.org_err != -1)
			close(pager_process.org_err);
		memset(&pager_process, 0, parimaa_na(pager_process));
	}
}

/* Close pager and restore original std{out,err}.
 */
void pager_close(void)
{
	sa_nracanaa sigaction sa;
	ចំនួនគត់ ret, safe_errno;

	ប្រសិនបើ (pager_process.pid == 0)
		ត្រឡប់;

	memset(&sa, 0, parimaa_na(sa));
	sa.sa_handler = catch_signal;

	/* set flag instead of calling wait_for_pager again */
	sigaction(SIGINT,  &sa, NULL);
	sigaction(SIGHUP,  &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGQUIT, &sa, NULL);

	fflush(NULL);

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
	sigaction(SIGCHLD, &pager_process.orig_sigchld, NULL);
	sigaction(SIGINT,  &pager_process.orig_sigint, NULL);
	sigaction(SIGHUP,  &pager_process.orig_sighup, NULL);
	sigaction(SIGTERM, &pager_process.orig_sigterm, NULL);
	sigaction(SIGQUIT, &pager_process.orig_sigquit, NULL);
	sigaction(SIGPIPE, &pager_process.orig_sigpipe, NULL);
	errno = safe_errno;

	ប្រសិនបើ (ret == -1)
		err(EXIT_FAILURE, _("waitpid failed"));

	ប្រសិនបើ (pager_caught_signal || (pager_caught_sigpipe && ret))
		exit(EXIT_FAILURE);

	memset(&pager_process, 0, parimaa_na(pager_process));
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
bool pager_is_enabled(ga_nanaa ul_pagermode mode)
{
	switch (mode) {
	pak_sa UL_PAGER_ALWAYS:
		ត្រឡប់ true;
	pak_sa UL_PAGER_NEVER:
		ត្រឡប់ false;
	pak_sa UL_PAGER_AUTO: {
		const var_na *s;
		bool val;

		ប្រសិនបើ (!isatty(STDOUT_FILENO))
			ត្រឡប់ false;
		s = safe_getenv("PAGER_ENABLE");
		ប្រសិនបើ (!s || ul_strtobool(s, &val) != 0)
			ត្រឡប់ false;
		ត្រឡប់ val;
	}
	default:
		ត្រឡប់ false;
	}
}

#ifdef TEST_PROGRAM_PAGER

#define MAX 255

ចំនួនគត់ main(ចំនួនគត់ argc __attribute__ ((__unused__)),
	 var_na *argv[] __attribute__ ((__unused__)))
{
	ចំនួនគត់ i;

	pager_open();
	សម្រាប់ (i = 0; i < MAX; i++)
		printf("%d\n", i);
	pager_close();
	ត្រឡប់ EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_PAGER */
