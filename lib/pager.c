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

#அந்தர்பாவ <unistd.h>
#அந்தர்பாவ <stdbool.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <sys/types.h>
#அந்தர்பாவ <sys/stat.h>
#அந்தர்பாவ <sys/wait.h>
#அந்தர்பாவ <signal.h>

#அந்தர்பாவ "c.h"
#அந்தர்பாவ "xalloc.h"
#அந்தர்பாவ "nls.h"
#அந்தர்பாவ "strutils.h"
#அந்தர்பாவ "ttyutils.h"
#அந்தர்பாவ "pager.h"
#அந்தர்பாவ "env.h"

நிலையான மாறிலி எழுத்து *pager_argv[] = { "sh", "-c", வெற்று, வெற்று };

கட்டமைப்பு child_process {
	மாறிலி எழுத்து **argv;
	pid_t pid;
	முழு in;

	முழு header_வரிகள்;
	முழு header_width;

	முழு org_err;
	முழு org_out;
	கட்டமைப்பு sigaction orig_sigchld;
	கட்டமைப்பு sigaction orig_sigint;
	கட்டமைப்பு sigaction orig_sighup;
	கட்டமைப்பு sigaction orig_sigterm;
	கட்டமைப்பு sigaction orig_sigquit;
	கட்டமைப்பு sigaction orig_sigpipe;
};
நிலையான கட்டமைப்பு child_process pager_process;

நிலையான நிலையற்ற sig_atomic_t pager_caught_signal;
நிலையான நிலையற்ற sig_atomic_t pager_caught_sigpipe;

நிலையான உள்வரி வெறுமை close_pair(முழு fd[2])
{
	close(fd[0]);
	close(fd[1]);
}

நிலையான வெறுமை pager_preexec(வெறுமை)
{
	மாறிலி எழுத்து *less_env = getenv("LESS");
	மாறிலி எழுத்து *base = less_env ? less_env : "FRSX";
	முழு header_வரிகள் = pager_process.header_வரிகள்;
	முழு header_width = pager_process.header_width;
	எழுத்து *less_val = வெற்று;

	எனில் (header_வரிகள் > 0 && header_width > 0)
		less_val = ul_strfconcat(base, " --header %d,%d",
					 header_வரிகள், header_width);
	இல்லையெனில் எனில் (header_width > 0)
		less_val = ul_strfconcat(base, " --header 0,%d",
					 header_width);
	இல்லையெனில் எனில் (header_வரிகள் > 0)
		less_val = ul_strfconcat(base, " --header %d",
					 header_வரிகள்);

	எனில் (less_val) {
		எனில் (setenv("LESS", less_val, 1) != 0)
			warn(_("failed to set the %s environment variable"), "LESS");
		விடுவி(less_val);
	} இல்லையெனில் எனில் (less_env == வெற்று) {
		எனில் (setenv("LESS", "FRSX", 0) != 0)
			warn(_("failed to set the %s environment variable"), "LESS");
	}

	எனில் (getenv("LV") == வெற்று && setenv("LV", "-c", 0) != 0)
		warn(_("failed to set the %s environment variable"), "LV");
}

நிலையான முழு start_command(கட்டமைப்பு child_process *cmd)
{
	முழு fdin[2];

	எனில் (pipe(fdin) < 0)
		திரும்பு -1;
	cmd->in = fdin[1];

	fflush(வெற்று);
	cmd->pid = fork();
	எனில் (!cmd->pid) {
		dup2(fdin[0], STDIN_FILENO);
		close_pair(fdin);

		pager_preexec();
		execvp(cmd->argv[0], (எழுத்து *மாறிலி*) cmd->argv);
		errexec(cmd->argv[0]);
	}

	எனில் (cmd->pid < 0) {
		cmd->pid = 0;
		close_pair(fdin);
		திரும்பு -1;
	}

	close(fdin[0]);
	திரும்பு 0;
}

நிலையான முழு wait_for_pager(வெறுமை)
{
	pid_t waiting;
	முழு ret, status;

	செய் {
		waiting = waitpid(pager_process.pid, &status, 0);
	} போது (waiting == -1 && errno == EINTR);

	எனில் (waiting == -1)
		ret = -1;
	இல்லையெனில் எனில் (waiting == pager_process.pid && WIFEXITED(status))
		ret = WEXITSTATUS(status);
	இல்லையெனில்
		ret = 1;

	pager_process.pid = 0;

	திரும்பு ret;
}

நிலையான வெறுமை catch_signal(முழு signo)
{
	pager_caught_signal = signo;
}

நிலையான வெறுமை catch_sigpipe(முழு signo)
{
	pager_caught_sigpipe = signo;
}

நிலையான வெறுமை wait_for_pager_signal(முழு signo __attribute__ ((__unused__)))
{
	/* signal EOF to pager */
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	/* async-signal safe: wait for all children, including pager */
	போது (wait(வெற்று) != -1 || errno == EINTR)
		;

	_exit(EXIT_FAILURE);
}

நிலையான முழு has_command(மாறிலி எழுத்து *cmd)
{
	மாறிலி எழுத்து *path;
	எழுத்து *b, *c, *p, *s;
	முழு rc = 0;

	எனில் (!cmd)
		செல் done;

	c = xstrdup(cmd);
	எனில் (!c)
		செல் done;
	b = strtok(c, " ");	/* cmd may contain options */
	எனில் (!b)
		செல் cleanup;

	எனில் (strchr(b, '/')) {
		rc = access(b, X_OK) == 0;
		செல் cleanup;
	}

	path = getenv("PATH");
	எனில் (!path)
		செல் cleanup;
	p = xstrdup(path);
	எனில் (!p)
		செல் cleanup;

	க்கு (s = strtok(p, ":"); s; s = strtok(வெற்று, ":")) {
		முழு fd = open(s, O_RDONLY|O_CLOEXEC);
		எனில் (fd < 0)
			தொடர்;
		rc = faccessat(fd, b, X_OK, 0) == 0;
		close(fd);
		எனில் (rc)
			நிறுத்து;
	}
	விடுவி(p);
cleanup:
	விடுவி(c);
done:
	/*fprintf(stderr, "has PAGER '%s': rc=%d\n", cmd, rc);*/
	திரும்பு rc;
}

நிலையான வெறுமை __setup_pager(வெறுமை)
{
	மாறிலி எழுத்து *pager = getenv("PAGER");
	கட்டமைப்பு sigaction sa;

	எனில் (!isatty(STDOUT_FILENO))
		திரும்பு;

	எனில் (!pager)
		pager = "less";
	இல்லையெனில் எனில் (!*pager || !strcmp(pager, "cat"))
		திரும்பு;

	எனில் (!has_command(pager))
		திரும்பு;

	/* spawn the pager */
	pager_argv[2] = pager;
	pager_process.argv = pager_argv;
	pager_process.in = -1;

	memset(&sa, 0, அளவு(sa));
	sa.sa_handler = SIG_DFL;

	/* this makes sure that waitpid works as expected */
	sigaction(SIGCHLD, &sa, &pager_process.orig_sigchld);

	எனில் (start_command(&pager_process)) {
		sigaction(SIGCHLD, &pager_process.orig_sigchld, வெற்று);
		திரும்பு;
	}

	/* original process continues, but writes to the pipe */
	dup2(pager_process.in, STDOUT_FILENO);
	setvbuf(stdout, வெற்று, _IOLBF, 0);
	எனில் (isatty(STDERR_FILENO)) {
		dup2(pager_process.in, STDERR_FILENO);
		setvbuf(stderr, வெற்று, _IOLBF, 0);
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
வெறுமை pager_open(வெறுமை)
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
வெறுமை pager_open_header(முழு header_வரிகள், முழு first_col_width)
{
	எனில் (pager_process.pid)
		திரும்பு;		/* already running */

	pager_process.header_வரிகள் = header_வரிகள்;
	pager_process.header_width = first_col_width;

	pager_process.org_out = dup(STDOUT_FILENO);
	pager_process.org_err = dup(STDERR_FILENO);

	எனில் (pager_process.org_out != -1 && pager_process.org_err != -1)
		__setup_pager();

	எனில் (!pager_process.pid) {
		எனில் (pager_process.org_out != -1)
			close(pager_process.org_out);
		எனில் (pager_process.org_err != -1)
			close(pager_process.org_err);
		memset(&pager_process, 0, அளவு(pager_process));
	}
}

/* Close pager and restore original std{out,err}.
 */
வெறுமை pager_close(வெறுமை)
{
	கட்டமைப்பு sigaction sa;
	முழு ret, safe_errno;

	எனில் (pager_process.pid == 0)
		திரும்பு;

	memset(&sa, 0, அளவு(sa));
	sa.sa_handler = catch_signal;

	/* set flag instead of calling wait_for_pager again */
	sigaction(SIGINT,  &sa, வெற்று);
	sigaction(SIGHUP,  &sa, வெற்று);
	sigaction(SIGTERM, &sa, வெற்று);
	sigaction(SIGQUIT, &sa, வெற்று);

	fflush(வெற்று);

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
	sigaction(SIGCHLD, &pager_process.orig_sigchld, வெற்று);
	sigaction(SIGINT,  &pager_process.orig_sigint, வெற்று);
	sigaction(SIGHUP,  &pager_process.orig_sighup, வெற்று);
	sigaction(SIGTERM, &pager_process.orig_sigterm, வெற்று);
	sigaction(SIGQUIT, &pager_process.orig_sigquit, வெற்று);
	sigaction(SIGPIPE, &pager_process.orig_sigpipe, வெற்று);
	errno = safe_errno;

	எனில் (ret == -1)
		err(EXIT_FAILURE, _("waitpid failed"));

	எனில் (pager_caught_signal || (pager_caught_sigpipe && ret))
		exit(EXIT_FAILURE);

	memset(&pager_process, 0, அளவு(pager_process));
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
bool pager_is_enabled(எண்ணிக்கை ul_pagermode mode)
{
	தேர்வு (mode) {
	நிலை UL_PAGER_ALWAYS:
		திரும்பு true;
	நிலை UL_PAGER_NEVER:
		திரும்பு false;
	நிலை UL_PAGER_AUTO: {
		மாறிலி எழுத்து *s;
		bool val;

		எனில் (!isatty(STDOUT_FILENO))
			திரும்பு false;
		s = safe_getenv("PAGER_ENABLE");
		எனில் (!s || ul_strtobool(s, &val) != 0)
			திரும்பு false;
		திரும்பு val;
	}
	இயல்பு:
		திரும்பு false;
	}
}

#ifdef TEST_PROGRAM_PAGER

#define MAX 255

முழு முதன்மை(முழு argc __attribute__ ((__unused__)),
	 எழுத்து *argv[] __attribute__ ((__unused__)))
{
	முழு i;

	pager_open();
	க்கு (i = 0; i < MAX; i++)
		printf("%d\n", i);
	pager_close();
	திரும்பு EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_PAGER */
