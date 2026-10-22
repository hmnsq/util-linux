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

static const char *pager_argv[] = { "sh", "-c", NULL, NULL };

struct child_process {
	const char **argv;
	pid_t pid;
	цяло in;

	цяло header_lines;
	цяло header_width;

	цяло org_err;
	цяло org_out;
	struct sigaction orig_sigchld;
	struct sigaction orig_sigint;
	struct sigaction orig_sighup;
	struct sigaction orig_sigterm;
	struct sigaction orig_sigquit;
	struct sigaction orig_sigpipe;
};
static struct child_process pager_process;

static volatile sig_atomic_t pager_caught_signal;
static volatile sig_atomic_t pager_caught_sigpipe;

static inline void close_pair(цяло fd[2])
{
	close(fd[0]);
	close(fd[1]);
}

static void pager_preexec(void)
{
	const char *less_env = getenv("LESS");
	const char *base = less_env ? less_env : "FRSX";
	цяло header_lines = pager_process.header_lines;
	цяло header_width = pager_process.header_width;
	char *less_val = NULL;

	ако (header_lines > 0 && header_width > 0)
		less_val = ul_strfconcat(base, " --header %d,%d",
					 header_lines, header_width);
	иначе ако (header_width > 0)
		less_val = ul_strfconcat(base, " --header 0,%d",
					 header_width);
	иначе ако (header_lines > 0)
		less_val = ul_strfconcat(base, " --header %d",
					 header_lines);

	ако (less_val) {
		ако (setenv("LESS", less_val, 1) != 0)
			warn(_("failed to set the %s environment variable"), "LESS");
		free(less_val);
	} иначе ако (less_env == NULL) {
		ако (setenv("LESS", "FRSX", 0) != 0)
			warn(_("failed to set the %s environment variable"), "LESS");
	}

	ако (getenv("LV") == NULL && setenv("LV", "-c", 0) != 0)
		warn(_("failed to set the %s environment variable"), "LV");
}

static цяло start_command(struct child_process *cmd)
{
	цяло fdin[2];

	ако (pipe(fdin) < 0)
		върни -1;
	cmd->in = fdin[1];

	fflush(NULL);
	cmd->pid = fork();
	ако (!cmd->pid) {
		dup2(fdin[0], STDIN_FILENO);
		close_pair(fdin);

		pager_preexec();
		execvp(cmd->argv[0], (char *const*) cmd->argv);
		errexec(cmd->argv[0]);
	}

	ако (cmd->pid < 0) {
		cmd->pid = 0;
		close_pair(fdin);
		върни -1;
	}

	close(fdin[0]);
	върни 0;
}

static цяло wait_for_pager(void)
{
	pid_t waiting;
	цяло ret, status;

	do {
		waiting = waitpid(pager_process.pid, &status, 0);
	} докато (waiting == -1 && errno == EINTR);

	ако (waiting == -1)
		ret = -1;
	иначе ако (waiting == pager_process.pid && WIFEXITED(status))
		ret = WEXITSTATUS(status);
	иначе
		ret = 1;

	pager_process.pid = 0;

	върни ret;
}

static void catch_signal(цяло signo)
{
	pager_caught_signal = signo;
}

static void catch_sigpipe(цяло signo)
{
	pager_caught_sigpipe = signo;
}

static void wait_for_pager_signal(цяло signo __attribute__ ((__unused__)))
{
	/* signal EOF to pager */
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	/* async-signal safe: wait for all children, including pager */
	докато (wait(NULL) != -1 || errno == EINTR)
		;

	_exit(EXIT_FAILURE);
}

static цяло has_command(const char *cmd)
{
	const char *path;
	char *b, *c, *p, *s;
	цяло rc = 0;

	ако (!cmd)
		goto done;

	c = xstrdup(cmd);
	ако (!c)
		goto done;
	b = strtok(c, " ");	/* cmd may contain options */
	ако (!b)
		goto cleanup;

	ако (strchr(b, '/')) {
		rc = access(b, X_OK) == 0;
		goto cleanup;
	}

	path = getenv("PATH");
	ако (!path)
		goto cleanup;
	p = xstrdup(path);
	ако (!p)
		goto cleanup;

	за (s = strtok(p, ":"); s; s = strtok(NULL, ":")) {
		цяло fd = open(s, O_RDONLY|O_CLOEXEC);
		ако (fd < 0)
			continue;
		rc = faccessat(fd, b, X_OK, 0) == 0;
		close(fd);
		ако (rc)
			break;
	}
	free(p);
cleanup:
	free(c);
done:
	/*fprintf(stderr, "has PAGER '%s': rc=%d\n", cmd, rc);*/
	върни rc;
}

static void __setup_pager(void)
{
	const char *pager = getenv("PAGER");
	struct sigaction sa;

	ако (!isatty(STDOUT_FILENO))
		върни;

	ако (!pager)
		pager = "less";
	иначе ако (!*pager || !strcmp(pager, "cat"))
		върни;

	ако (!has_command(pager))
		върни;

	/* spawn the pager */
	pager_argv[2] = pager;
	pager_process.argv = pager_argv;
	pager_process.in = -1;

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = SIG_DFL;

	/* this makes sure that waitpid works as expected */
	sigaction(SIGCHLD, &sa, &pager_process.orig_sigchld);

	ако (start_command(&pager_process)) {
		sigaction(SIGCHLD, &pager_process.orig_sigchld, NULL);
		върни;
	}

	/* original process continues, but writes to the pipe */
	dup2(pager_process.in, STDOUT_FILENO);
	setvbuf(stdout, NULL, _IOLBF, 0);
	ако (isatty(STDERR_FILENO)) {
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
void pager_open_header(цяло header_lines, цяло first_col_width)
{
	ако (pager_process.pid)
		върни;		/* already running */

	pager_process.header_lines = header_lines;
	pager_process.header_width = first_col_width;

	pager_process.org_out = dup(STDOUT_FILENO);
	pager_process.org_err = dup(STDERR_FILENO);

	ако (pager_process.org_out != -1 && pager_process.org_err != -1)
		__setup_pager();

	ако (!pager_process.pid) {
		ако (pager_process.org_out != -1)
			close(pager_process.org_out);
		ако (pager_process.org_err != -1)
			close(pager_process.org_err);
		memset(&pager_process, 0, sizeof(pager_process));
	}
}

/* Close pager and restore original std{out,err}.
 */
void pager_close(void)
{
	struct sigaction sa;
	цяло ret, safe_errno;

	ако (pager_process.pid == 0)
		върни;

	memset(&sa, 0, sizeof(sa));
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

	ако (ret == -1)
		err(EXIT_FAILURE, _("waitpid failed"));

	ако (pager_caught_signal || (pager_caught_sigpipe && ret))
		exit(EXIT_FAILURE);

	memset(&pager_process, 0, sizeof(pager_process));
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
bool pager_is_enabled(enum ul_pagermode mode)
{
	switch (mode) {
	case UL_PAGER_ALWAYS:
		върни true;
	case UL_PAGER_NEVER:
		върни false;
	case UL_PAGER_AUTO: {
		const char *s;
		bool val;

		ако (!isatty(STDOUT_FILENO))
			върни false;
		s = safe_getenv("PAGER_ENABLE");
		ако (!s || ul_strtobool(s, &val) != 0)
			върни false;
		върни val;
	}
	default:
		върни false;
	}
}

#ifdef TEST_PROGRAM_PAGER

#define MAX 255

цяло main(цяло argc __attribute__ ((__unused__)),
	 char *argv[] __attribute__ ((__unused__)))
{
	цяло i;

	pager_open();
	за (i = 0; i < MAX; i++)
		printf("%d\n", i);
	pager_close();
	върни EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_PAGER */
