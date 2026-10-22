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
	정수 in;

	정수 header_lines;
	정수 header_width;

	정수 org_err;
	정수 org_out;
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

static inline void close_pair(정수 fd[2])
{
	close(fd[0]);
	close(fd[1]);
}

static void pager_preexec(void)
{
	const char *less_env = getenv("LESS");
	const char *base = less_env ? less_env : "FRSX";
	정수 header_lines = pager_process.header_lines;
	정수 header_width = pager_process.header_width;
	char *less_val = NULL;

	만약 (header_lines > 0 && header_width > 0)
		less_val = ul_strfconcat(base, " --header %d,%d",
					 header_lines, header_width);
	아니면 만약 (header_width > 0)
		less_val = ul_strfconcat(base, " --header 0,%d",
					 header_width);
	아니면 만약 (header_lines > 0)
		less_val = ul_strfconcat(base, " --header %d",
					 header_lines);

	만약 (less_val) {
		만약 (setenv("LESS", less_val, 1) != 0)
			warn(_("failed to set the %s environment variable"), "LESS");
		free(less_val);
	} 아니면 만약 (less_env == NULL) {
		만약 (setenv("LESS", "FRSX", 0) != 0)
			warn(_("failed to set the %s environment variable"), "LESS");
	}

	만약 (getenv("LV") == NULL && setenv("LV", "-c", 0) != 0)
		warn(_("failed to set the %s environment variable"), "LV");
}

static 정수 start_command(struct child_process *cmd)
{
	정수 fdin[2];

	만약 (pipe(fdin) < 0)
		반환 -1;
	cmd->in = fdin[1];

	fflush(NULL);
	cmd->pid = fork();
	만약 (!cmd->pid) {
		dup2(fdin[0], STDIN_FILENO);
		close_pair(fdin);

		pager_preexec();
		execvp(cmd->argv[0], (char *const*) cmd->argv);
		errexec(cmd->argv[0]);
	}

	만약 (cmd->pid < 0) {
		cmd->pid = 0;
		close_pair(fdin);
		반환 -1;
	}

	close(fdin[0]);
	반환 0;
}

static 정수 wait_for_pager(void)
{
	pid_t waiting;
	정수 ret, status;

	do {
		waiting = waitpid(pager_process.pid, &status, 0);
	} 동안 (waiting == -1 && errno == EINTR);

	만약 (waiting == -1)
		ret = -1;
	아니면 만약 (waiting == pager_process.pid && WIFEXITED(status))
		ret = WEXITSTATUS(status);
	아니면
		ret = 1;

	pager_process.pid = 0;

	반환 ret;
}

static void catch_signal(정수 signo)
{
	pager_caught_signal = signo;
}

static void catch_sigpipe(정수 signo)
{
	pager_caught_sigpipe = signo;
}

static void wait_for_pager_signal(정수 signo __attribute__ ((__unused__)))
{
	/* signal EOF to pager */
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	/* async-signal safe: wait for all children, including pager */
	동안 (wait(NULL) != -1 || errno == EINTR)
		;

	_exit(EXIT_FAILURE);
}

static 정수 has_command(const char *cmd)
{
	const char *path;
	char *b, *c, *p, *s;
	정수 rc = 0;

	만약 (!cmd)
		goto done;

	c = xstrdup(cmd);
	만약 (!c)
		goto done;
	b = strtok(c, " ");	/* cmd may contain options */
	만약 (!b)
		goto cleanup;

	만약 (strchr(b, '/')) {
		rc = access(b, X_OK) == 0;
		goto cleanup;
	}

	path = getenv("PATH");
	만약 (!path)
		goto cleanup;
	p = xstrdup(path);
	만약 (!p)
		goto cleanup;

	위해 (s = strtok(p, ":"); s; s = strtok(NULL, ":")) {
		정수 fd = open(s, O_RDONLY|O_CLOEXEC);
		만약 (fd < 0)
			continue;
		rc = faccessat(fd, b, X_OK, 0) == 0;
		close(fd);
		만약 (rc)
			break;
	}
	free(p);
cleanup:
	free(c);
done:
	/*fprintf(stderr, "has PAGER '%s': rc=%d\n", cmd, rc);*/
	반환 rc;
}

static void __setup_pager(void)
{
	const char *pager = getenv("PAGER");
	struct sigaction sa;

	만약 (!isatty(STDOUT_FILENO))
		반환;

	만약 (!pager)
		pager = "less";
	아니면 만약 (!*pager || !strcmp(pager, "cat"))
		반환;

	만약 (!has_command(pager))
		반환;

	/* spawn the pager */
	pager_argv[2] = pager;
	pager_process.argv = pager_argv;
	pager_process.in = -1;

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = SIG_DFL;

	/* this makes sure that waitpid works as expected */
	sigaction(SIGCHLD, &sa, &pager_process.orig_sigchld);

	만약 (start_command(&pager_process)) {
		sigaction(SIGCHLD, &pager_process.orig_sigchld, NULL);
		반환;
	}

	/* original process continues, but writes to the pipe */
	dup2(pager_process.in, STDOUT_FILENO);
	setvbuf(stdout, NULL, _IOLBF, 0);
	만약 (isatty(STDERR_FILENO)) {
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
void pager_open_header(정수 header_lines, 정수 first_col_width)
{
	만약 (pager_process.pid)
		반환;		/* already running */

	pager_process.header_lines = header_lines;
	pager_process.header_width = first_col_width;

	pager_process.org_out = dup(STDOUT_FILENO);
	pager_process.org_err = dup(STDERR_FILENO);

	만약 (pager_process.org_out != -1 && pager_process.org_err != -1)
		__setup_pager();

	만약 (!pager_process.pid) {
		만약 (pager_process.org_out != -1)
			close(pager_process.org_out);
		만약 (pager_process.org_err != -1)
			close(pager_process.org_err);
		memset(&pager_process, 0, sizeof(pager_process));
	}
}

/* Close pager and restore original std{out,err}.
 */
void pager_close(void)
{
	struct sigaction sa;
	정수 ret, safe_errno;

	만약 (pager_process.pid == 0)
		반환;

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

	만약 (ret == -1)
		err(EXIT_FAILURE, _("waitpid failed"));

	만약 (pager_caught_signal || (pager_caught_sigpipe && ret))
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
		반환 true;
	case UL_PAGER_NEVER:
		반환 false;
	case UL_PAGER_AUTO: {
		const char *s;
		bool val;

		만약 (!isatty(STDOUT_FILENO))
			반환 false;
		s = safe_getenv("PAGER_ENABLE");
		만약 (!s || ul_strtobool(s, &val) != 0)
			반환 false;
		반환 val;
	}
	default:
		반환 false;
	}
}

#ifdef TEST_PROGRAM_PAGER

#define MAX 255

정수 main(정수 argc __attribute__ ((__unused__)),
	 char *argv[] __attribute__ ((__unused__)))
{
	정수 i;

	pager_open();
	위해 (i = 0; i < MAX; i++)
		printf("%d\n", i);
	pager_close();
	반환 EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_PAGER */
