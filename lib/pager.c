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

স্থির ধ্রুবক অক্ষর *pager_argv[] = { "sh", "-c", ফাঁকা, ফাঁকা };

কাঠামো child_process {
	ধ্রুবক অক্ষর **argv;
	pid_t pid;
	পূর্ণসংখ্যা in;

	পূর্ণসংখ্যা header_সারিগুলি;
	পূর্ণসংখ্যা header_width;

	পূর্ণসংখ্যা org_err;
	পূর্ণসংখ্যা org_out;
	কাঠামো sigaction orig_sigchld;
	কাঠামো sigaction orig_sigint;
	কাঠামো sigaction orig_sighup;
	কাঠামো sigaction orig_sigterm;
	কাঠামো sigaction orig_sigquit;
	কাঠামো sigaction orig_sigpipe;
};
স্থির কাঠামো child_process pager_process;

স্থির অস্থির sig_atomic_t pager_caught_signal;
স্থির অস্থির sig_atomic_t pager_caught_sigpipe;

স্থির অন্তঃরেখ শূন্য close_pair(পূর্ণসংখ্যা fd[2])
{
	close(fd[0]);
	close(fd[1]);
}

স্থির শূন্য pager_preexec(শূন্য)
{
	ধ্রুবক অক্ষর *less_env = getenv("LESS");
	ধ্রুবক অক্ষর *base = less_env ? less_env : "FRSX";
	পূর্ণসংখ্যা header_সারিগুলি = pager_process.header_সারিগুলি;
	পূর্ণসংখ্যা header_width = pager_process.header_width;
	অক্ষর *less_val = ফাঁকা;

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
	} অন্যথা যদি (less_env == ফাঁকা) {
		যদি (setenv("LESS", "FRSX", 0) != 0)
			warn(_("failed to set the %s environment variable"), "LESS");
	}

	যদি (getenv("LV") == ফাঁকা && setenv("LV", "-c", 0) != 0)
		warn(_("failed to set the %s environment variable"), "LV");
}

স্থির পূর্ণসংখ্যা start_command(কাঠামো child_process *cmd)
{
	পূর্ণসংখ্যা fdin[2];

	যদি (pipe(fdin) < 0)
		ফেরত -1;
	cmd->in = fdin[1];

	fflush(ফাঁকা);
	cmd->pid = fork();
	যদি (!cmd->pid) {
		dup2(fdin[0], STDIN_FILENO);
		close_pair(fdin);

		pager_preexec();
		execvp(cmd->argv[0], (অক্ষর *ধ্রুবক*) cmd->argv);
		errexec(cmd->argv[0]);
	}

	যদি (cmd->pid < 0) {
		cmd->pid = 0;
		close_pair(fdin);
		ফেরত -1;
	}

	close(fdin[0]);
	ফেরত 0;
}

স্থির পূর্ণসংখ্যা wait_for_pager(শূন্য)
{
	pid_t waiting;
	পূর্ণসংখ্যা ret, status;

	করো {
		waiting = waitpid(pager_process.pid, &status, 0);
	} যখন (waiting == -1 && errno == EINTR);

	যদি (waiting == -1)
		ret = -1;
	অন্যথা যদি (waiting == pager_process.pid && WIFEXITED(status))
		ret = WEXITSTATUS(status);
	অন্যথা
		ret = 1;

	pager_process.pid = 0;

	ফেরত ret;
}

স্থির শূন্য catch_signal(পূর্ণসংখ্যা signo)
{
	pager_caught_signal = signo;
}

স্থির শূন্য catch_sigpipe(পূর্ণসংখ্যা signo)
{
	pager_caught_sigpipe = signo;
}

স্থির শূন্য wait_for_pager_signal(পূর্ণসংখ্যা signo __attribute__ ((__unused__)))
{
	/* signal EOF to pager */
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	/* async-signal safe: wait for all children, including pager */
	যখন (wait(ফাঁকা) != -1 || errno == EINTR)
		;

	_exit(EXIT_FAILURE);
}

স্থির পূর্ণসংখ্যা has_command(ধ্রুবক অক্ষর *cmd)
{
	ধ্রুবক অক্ষর *path;
	অক্ষর *b, *c, *p, *s;
	পূর্ণসংখ্যা rc = 0;

	যদি (!cmd)
		যাও done;

	c = xstrdup(cmd);
	যদি (!c)
		যাও done;
	b = strtok(c, " ");	/* cmd may contain options */
	যদি (!b)
		যাও cleanup;

	যদি (strchr(b, '/')) {
		rc = access(b, X_OK) == 0;
		যাও cleanup;
	}

	path = getenv("PATH");
	যদি (!path)
		যাও cleanup;
	p = xstrdup(path);
	যদি (!p)
		যাও cleanup;

	জন্য (s = strtok(p, ":"); s; s = strtok(ফাঁকা, ":")) {
		পূর্ণসংখ্যা fd = open(s, O_RDONLY|O_CLOEXEC);
		যদি (fd < 0)
			চালু;
		rc = faccessat(fd, b, X_OK, 0) == 0;
		close(fd);
		যদি (rc)
			বিরতি;
	}
	মুক্ত(p);
cleanup:
	মুক্ত(c);
done:
	/*fprintf(stderr, "has PAGER '%s': rc=%d\n", cmd, rc);*/
	ফেরত rc;
}

স্থির শূন্য __setup_pager(শূন্য)
{
	ধ্রুবক অক্ষর *pager = getenv("PAGER");
	কাঠামো sigaction sa;

	যদি (!isatty(STDOUT_FILENO))
		ফেরত;

	যদি (!pager)
		pager = "less";
	অন্যথা যদি (!*pager || !strcmp(pager, "cat"))
		ফেরত;

	যদি (!has_command(pager))
		ফেরত;

	/* spawn the pager */
	pager_argv[2] = pager;
	pager_process.argv = pager_argv;
	pager_process.in = -1;

	memset(&sa, 0, আকার(sa));
	sa.sa_handler = SIG_DFL;

	/* this makes sure that waitpid works as expected */
	sigaction(SIGCHLD, &sa, &pager_process.orig_sigchld);

	যদি (start_command(&pager_process)) {
		sigaction(SIGCHLD, &pager_process.orig_sigchld, ফাঁকা);
		ফেরত;
	}

	/* original process continues, but writes to the pipe */
	dup2(pager_process.in, STDOUT_FILENO);
	setvbuf(stdout, ফাঁকা, _IOLBF, 0);
	যদি (isatty(STDERR_FILENO)) {
		dup2(pager_process.in, STDERR_FILENO);
		setvbuf(stderr, ফাঁকা, _IOLBF, 0);
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
শূন্য pager_open_header(পূর্ণসংখ্যা header_সারিগুলি, পূর্ণসংখ্যা first_col_width)
{
	যদি (pager_process.pid)
		ফেরত;		/* already running */

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
		memset(&pager_process, 0, আকার(pager_process));
	}
}

/* Close pager and restore original std{out,err}.
 */
শূন্য pager_close(শূন্য)
{
	কাঠামো sigaction sa;
	পূর্ণসংখ্যা ret, safe_errno;

	যদি (pager_process.pid == 0)
		ফেরত;

	memset(&sa, 0, আকার(sa));
	sa.sa_handler = catch_signal;

	/* set flag instead of calling wait_for_pager again */
	sigaction(SIGINT,  &sa, ফাঁকা);
	sigaction(SIGHUP,  &sa, ফাঁকা);
	sigaction(SIGTERM, &sa, ফাঁকা);
	sigaction(SIGQUIT, &sa, ফাঁকা);

	fflush(ফাঁকা);

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
	sigaction(SIGCHLD, &pager_process.orig_sigchld, ফাঁকা);
	sigaction(SIGINT,  &pager_process.orig_sigint, ফাঁকা);
	sigaction(SIGHUP,  &pager_process.orig_sighup, ফাঁকা);
	sigaction(SIGTERM, &pager_process.orig_sigterm, ফাঁকা);
	sigaction(SIGQUIT, &pager_process.orig_sigquit, ফাঁকা);
	sigaction(SIGPIPE, &pager_process.orig_sigpipe, ফাঁকা);
	errno = safe_errno;

	যদি (ret == -1)
		err(EXIT_FAILURE, _("waitpid failed"));

	যদি (pager_caught_signal || (pager_caught_sigpipe && ret))
		exit(EXIT_FAILURE);

	memset(&pager_process, 0, আকার(pager_process));
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
	নির্বাচন (mode) {
	ক্ষেত্র UL_PAGER_ALWAYS:
		ফেরত true;
	ক্ষেত্র UL_PAGER_NEVER:
		ফেরত false;
	ক্ষেত্র UL_PAGER_AUTO: {
		ধ্রুবক অক্ষর *s;
		bool val;

		যদি (!isatty(STDOUT_FILENO))
			ফেরত false;
		s = safe_getenv("PAGER_ENABLE");
		যদি (!s || ul_strtobool(s, &val) != 0)
			ফেরত false;
		ফেরত val;
	}
	পূর্বনির্ধারিত:
		ফেরত false;
	}
}

#ifdef TEST_PROGRAM_PAGER

#define MAX 255

পূর্ণসংখ্যা প্রধান(পূর্ণসংখ্যা argc __attribute__ ((__unused__)),
	 অক্ষর *argv[] __attribute__ ((__unused__)))
{
	পূর্ণসংখ্যা i;

	pager_open();
	জন্য (i = 0; i < MAX; i++)
		printf("%d\n", i);
	pager_close();
	ফেরত EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_PAGER */
