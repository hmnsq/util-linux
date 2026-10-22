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

#অন্তর্ভাব <unistd.h>
#অন্তর্ভাব <stdbool.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <sys/types.h>
#অন্তর্ভাব <sys/stat.h>
#অন্তর্ভাব <sys/wait.h>
#অন্তর্ভাব <signal.h>

#অন্তর্ভাব "c.h"
#অন্তর্ভাব "xalloc.h"
#অন্তর্ভাব "nls.h"
#অন্তর্ভাব "strutils.h"
#অন্তর্ভাব "ttyutils.h"
#অন্তর্ভাব "pager.h"
#অন্তর্ভাব "env.h"

স্থির ধ্রুব বর্ণ *pager_argv[] = { "sh", "-c", রিক্ত, রিক্ত };

সংরচনা child_process {
	ধ্রুব বর্ণ **argv;
	pid_t pid;
	পূর্ণাঙ্ক in;

	পূর্ণাঙ্ক header_সারিগুলি;
	পূর্ণাঙ্ক header_width;

	পূর্ণাঙ্ক org_err;
	পূর্ণাঙ্ক org_out;
	সংরচনা sigaction orig_sigchld;
	সংরচনা sigaction orig_sigint;
	সংরচনা sigaction orig_sighup;
	সংরচনা sigaction orig_sigterm;
	সংরচনা sigaction orig_sigquit;
	সংরচনা sigaction orig_sigpipe;
};
স্থির সংরচনা child_process pager_process;

স্থির চঞ্চল sig_atomic_t pager_caught_signal;
স্থির চঞ্চল sig_atomic_t pager_caught_sigpipe;

স্থির অন্তঃস্থ শূন্য close_pair(পূর্ণাঙ্ক fd[2])
{
	close(fd[0]);
	close(fd[1]);
}

স্থির শূন্য pager_preexec(শূন্য)
{
	ধ্রুব বর্ণ *less_env = getenv("LESS");
	ধ্রুব বর্ণ *base = less_env ? less_env : "FRSX";
	পূর্ণাঙ্ক header_সারিগুলি = pager_process.header_সারিগুলি;
	পূর্ণাঙ্ক header_width = pager_process.header_width;
	বর্ণ *less_val = রিক্ত;

	যদি (header_সারিগুলি > 0 && header_width > 0)
		less_val = ul_strfconcat(base, " --header %d,%d",
					 header_সারিগুলি, header_width);
	অন্যথা যদি (header_width > 0)
		less_val = ul_strfconcat(base, " --header 0,%d",
					 header_width);
	অন্যথা যদি (header_সারিগুলি > 0)
		less_val = ul_strfconcat(base, " --header %d",
					 header_সারিগুলি);

	যদি (less_val) {
		যদি (setenv("LESS", less_val, 1) != 0)
			warn(_("failed to set the %s environment variable"), "LESS");
		মুক্ত(less_val);
	} অন্যথা যদি (less_env == রিক্ত) {
		যদি (setenv("LESS", "FRSX", 0) != 0)
			warn(_("failed to set the %s environment variable"), "LESS");
	}

	যদি (getenv("LV") == রিক্ত && setenv("LV", "-c", 0) != 0)
		warn(_("failed to set the %s environment variable"), "LV");
}

স্থির পূর্ণাঙ্ক start_command(সংরচনা child_process *cmd)
{
	পূর্ণাঙ্ক fdin[2];

	যদি (pipe(fdin) < 0)
		প্রত্যাগম -1;
	cmd->in = fdin[1];

	fflush(রিক্ত);
	cmd->pid = fork();
	যদি (!cmd->pid) {
		dup2(fdin[0], STDIN_FILENO);
		close_pair(fdin);

		pager_preexec();
		execvp(cmd->argv[0], (বর্ণ *ধ্রুব*) cmd->argv);
		errexec(cmd->argv[0]);
	}

	যদি (cmd->pid < 0) {
		cmd->pid = 0;
		close_pair(fdin);
		প্রত্যাগম -1;
	}

	close(fdin[0]);
	প্রত্যাগম 0;
}

স্থির পূর্ণাঙ্ক wait_for_pager(শূন্য)
{
	pid_t waiting;
	পূর্ণাঙ্ক ret, status;

	কুরু {
		waiting = waitpid(pager_process.pid, &status, 0);
	} যাবত্ (waiting == -1 && errno == EINTR);

	যদি (waiting == -1)
		ret = -1;
	অন্যথা যদি (waiting == pager_process.pid && WIFEXITED(status))
		ret = WEXITSTATUS(status);
	অন্যথা
		ret = 1;

	pager_process.pid = 0;

	প্রত্যাগম ret;
}

স্থির শূন্য catch_signal(পূর্ণাঙ্ক signo)
{
	pager_caught_signal = signo;
}

স্থির শূন্য catch_sigpipe(পূর্ণাঙ্ক signo)
{
	pager_caught_sigpipe = signo;
}

স্থির শূন্য wait_for_pager_signal(পূর্ণাঙ্ক signo __attribute__ ((__unused__)))
{
	/* signal EOF to pager */
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	/* async-signal safe: wait for all children, including pager */
	যাবত্ (wait(রিক্ত) != -1 || errno == EINTR)
		;

	_exit(EXIT_FAILURE);
}

স্থির পূর্ণাঙ্ক has_command(ধ্রুব বর্ণ *cmd)
{
	ধ্রুব বর্ণ *path;
	বর্ণ *b, *c, *p, *s;
	পূর্ণাঙ্ক rc = 0;

	যদি (!cmd)
		গচ্ছ done;

	c = xstrdup(cmd);
	যদি (!c)
		গচ্ছ done;
	b = strtok(c, " ");	/* cmd may contain options */
	যদি (!b)
		গচ্ছ cleanup;

	যদি (strchr(b, '/')) {
		rc = access(b, X_OK) == 0;
		গচ্ছ cleanup;
	}

	path = getenv("PATH");
	যদি (!path)
		গচ্ছ cleanup;
	p = xstrdup(path);
	যদি (!p)
		গচ্ছ cleanup;

	প্রতি (s = strtok(p, ":"); s; s = strtok(রিক্ত, ":")) {
		পূর্ণাঙ্ক fd = open(s, O_RDONLY|O_CLOEXEC);
		যদি (fd < 0)
			অনুবর্ত;
		rc = faccessat(fd, b, X_OK, 0) == 0;
		close(fd);
		যদি (rc)
			ভঙ্গ;
	}
	মুক্ত(p);
cleanup:
	মুক্ত(c);
done:
	/*fprintf(stderr, "has PAGER '%s': rc=%d\n", cmd, rc);*/
	প্রত্যাগম rc;
}

স্থির শূন্য __setup_pager(শূন্য)
{
	ধ্রুব বর্ণ *pager = getenv("PAGER");
	সংরচনা sigaction sa;

	যদি (!isatty(STDOUT_FILENO))
		প্রত্যাগম;

	যদি (!pager)
		pager = "less";
	অন্যথা যদি (!*pager || !strcmp(pager, "cat"))
		প্রত্যাগম;

	যদি (!has_command(pager))
		প্রত্যাগম;

	/* spawn the pager */
	pager_argv[2] = pager;
	pager_process.argv = pager_argv;
	pager_process.in = -1;

	memset(&sa, 0, পরিমাণ(sa));
	sa.sa_handler = SIG_DFL;

	/* this makes sure that waitpid works as expected */
	sigaction(SIGCHLD, &sa, &pager_process.orig_sigchld);

	যদি (start_command(&pager_process)) {
		sigaction(SIGCHLD, &pager_process.orig_sigchld, রিক্ত);
		প্রত্যাগম;
	}

	/* original process continues, but writes to the pipe */
	dup2(pager_process.in, STDOUT_FILENO);
	setvbuf(stdout, রিক্ত, _IOLBF, 0);
	যদি (isatty(STDERR_FILENO)) {
		dup2(pager_process.in, STDERR_FILENO);
		setvbuf(stderr, রিক্ত, _IOLBF, 0);
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
শূন্য pager_open(শূন্য)
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
শূন্য pager_open_header(পূর্ণাঙ্ক header_সারিগুলি, পূর্ণাঙ্ক first_col_width)
{
	যদি (pager_process.pid)
		প্রত্যাগম;		/* already running */

	pager_process.header_সারিগুলি = header_সারিগুলি;
	pager_process.header_width = first_col_width;

	pager_process.org_out = dup(STDOUT_FILENO);
	pager_process.org_err = dup(STDERR_FILENO);

	যদি (pager_process.org_out != -1 && pager_process.org_err != -1)
		__setup_pager();

	যদি (!pager_process.pid) {
		যদি (pager_process.org_out != -1)
			close(pager_process.org_out);
		যদি (pager_process.org_err != -1)
			close(pager_process.org_err);
		memset(&pager_process, 0, পরিমাণ(pager_process));
	}
}

/* Close pager and restore original std{out,err}.
 */
শূন্য pager_close(শূন্য)
{
	সংরচনা sigaction sa;
	পূর্ণাঙ্ক ret, safe_errno;

	যদি (pager_process.pid == 0)
		প্রত্যাগম;

	memset(&sa, 0, পরিমাণ(sa));
	sa.sa_handler = catch_signal;

	/* set flag instead of calling wait_for_pager again */
	sigaction(SIGINT,  &sa, রিক্ত);
	sigaction(SIGHUP,  &sa, রিক্ত);
	sigaction(SIGTERM, &sa, রিক্ত);
	sigaction(SIGQUIT, &sa, রিক্ত);

	fflush(রিক্ত);

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
	sigaction(SIGCHLD, &pager_process.orig_sigchld, রিক্ত);
	sigaction(SIGINT,  &pager_process.orig_sigint, রিক্ত);
	sigaction(SIGHUP,  &pager_process.orig_sighup, রিক্ত);
	sigaction(SIGTERM, &pager_process.orig_sigterm, রিক্ত);
	sigaction(SIGQUIT, &pager_process.orig_sigquit, রিক্ত);
	sigaction(SIGPIPE, &pager_process.orig_sigpipe, রিক্ত);
	errno = safe_errno;

	যদি (ret == -1)
		err(EXIT_FAILURE, _("waitpid failed"));

	যদি (pager_caught_signal || (pager_caught_sigpipe && ret))
		exit(EXIT_FAILURE);

	memset(&pager_process, 0, পরিমাণ(pager_process));
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
bool pager_is_enabled(গণনা ul_pagermode mode)
{
	বিকল্প (mode) {
	পক্ষ UL_PAGER_ALWAYS:
		প্রত্যাগম true;
	পক্ষ UL_PAGER_NEVER:
		প্রত্যাগম false;
	পক্ষ UL_PAGER_AUTO: {
		ধ্রুব বর্ণ *s;
		bool val;

		যদি (!isatty(STDOUT_FILENO))
			প্রত্যাগম false;
		s = safe_getenv("PAGER_ENABLE");
		যদি (!s || ul_strtobool(s, &val) != 0)
			প্রত্যাগম false;
		প্রত্যাগম val;
	}
	স্বভাব:
		প্রত্যাগম false;
	}
}

#ifdef TEST_PROGRAM_PAGER

#define MAX 255

পূর্ণাঙ্ক main(পূর্ণাঙ্ক argc __attribute__ ((__unused__)),
	 বর্ণ *argv[] __attribute__ ((__unused__)))
{
	পূর্ণাঙ্ক i;

	pager_open();
	প্রতি (i = 0; i < MAX; i++)
		printf("%d\n", i);
	pager_close();
	প্রত্যাগম EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_PAGER */
