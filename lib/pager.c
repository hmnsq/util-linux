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

#شامل <unistd.h>
#شامل <stdbool.h>
#شامل <stdlib.h>
#شامل <string.h>
#شامل <sys/types.h>
#شامل <sys/stat.h>
#شامل <sys/wait.h>
#شامل <signal.h>

#شامل "c.h"
#شامل "xalloc.h"
#شامل "nls.h"
#شامل "strutils.h"
#شامل "ttyutils.h"
#شامل "pager.h"
#شامل "env.h"

ایستا ثابت نویسه *pager_argv[] = { "sh", "-c", پوچ, پوچ };

ساختار child_process {
	ثابت نویسه **argv;
	pid_t pid;
	صحیح in;

	صحیح header_سطرها;
	صحیح header_width;

	صحیح org_err;
	صحیح org_out;
	ساختار sigaction orig_sigchld;
	ساختار sigaction orig_sigint;
	ساختار sigaction orig_sighup;
	ساختار sigaction orig_sigterm;
	ساختار sigaction orig_sigquit;
	ساختار sigaction orig_sigpipe;
};
ایستا ساختار child_process pager_process;

ایستا فرار sig_atomic_t pager_caught_signal;
ایستا فرار sig_atomic_t pager_caught_sigpipe;

ایستا درونخطی تهی close_pair(صحیح fd[2])
{
	close(fd[0]);
	close(fd[1]);
}

ایستا تهی pager_preexec(تهی)
{
	ثابت نویسه *less_env = getenv("LESS");
	ثابت نویسه *base = less_env ? less_env : "FRSX";
	صحیح header_سطرها = pager_process.header_سطرها;
	صحیح header_width = pager_process.header_width;
	نویسه *less_val = پوچ;

	اگر (header_سطرها > 0 && header_width > 0)
		less_val = ul_strfconcat(base, " --header %d,%d",
					 header_سطرها, header_width);
	وگرنه اگر (header_width > 0)
		less_val = ul_strfconcat(base, " --header 0,%d",
					 header_width);
	وگرنه اگر (header_سطرها > 0)
		less_val = ul_strfconcat(base, " --header %d",
					 header_سطرها);

	اگر (less_val) {
		اگر (setenv("LESS", less_val, 1) != 0)
			warn(_("failed to set the %s environment variable"), "LESS");
		آزاد(less_val);
	} وگرنه اگر (less_env == پوچ) {
		اگر (setenv("LESS", "FRSX", 0) != 0)
			warn(_("failed to set the %s environment variable"), "LESS");
	}

	اگر (getenv("LV") == پوچ && setenv("LV", "-c", 0) != 0)
		warn(_("failed to set the %s environment variable"), "LV");
}

ایستا صحیح start_command(ساختار child_process *cmd)
{
	صحیح fdin[2];

	اگر (pipe(fdin) < 0)
		بازگشت -1;
	cmd->in = fdin[1];

	fflush(پوچ);
	cmd->pid = fork();
	اگر (!cmd->pid) {
		dup2(fdin[0], STDIN_FILENO);
		close_pair(fdin);

		pager_preexec();
		execvp(cmd->argv[0], (نویسه *ثابت*) cmd->argv);
		errexec(cmd->argv[0]);
	}

	اگر (cmd->pid < 0) {
		cmd->pid = 0;
		close_pair(fdin);
		بازگشت -1;
	}

	close(fdin[0]);
	بازگشت 0;
}

ایستا صحیح wait_for_pager(تهی)
{
	pid_t waiting;
	صحیح ret, status;

	انجامبده {
		waiting = waitpid(pager_process.pid, &status, 0);
	} وقتی (waiting == -1 && errno == EINTR);

	اگر (waiting == -1)
		ret = -1;
	وگرنه اگر (waiting == pager_process.pid && WIFEXITED(status))
		ret = WEXITSTATUS(status);
	وگرنه
		ret = 1;

	pager_process.pid = 0;

	بازگشت ret;
}

ایستا تهی catch_signal(صحیح signo)
{
	pager_caught_signal = signo;
}

ایستا تهی catch_sigpipe(صحیح signo)
{
	pager_caught_sigpipe = signo;
}

ایستا تهی wait_for_pager_signal(صحیح signo __attribute__ ((__unused__)))
{
	/* signal EOF to pager */
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	/* async-signal safe: wait for all children, including pager */
	وقتی (wait(پوچ) != -1 || errno == EINTR)
		;

	_exit(EXIT_FAILURE);
}

ایستا صحیح has_command(ثابت نویسه *cmd)
{
	ثابت نویسه *path;
	نویسه *b, *c, *p, *s;
	صحیح rc = 0;

	اگر (!cmd)
		برو done;

	c = xstrdup(cmd);
	اگر (!c)
		برو done;
	b = strtok(c, " ");	/* cmd may contain options */
	اگر (!b)
		برو cleanup;

	اگر (strchr(b, '/')) {
		rc = access(b, X_OK) == 0;
		برو cleanup;
	}

	path = getenv("PATH");
	اگر (!path)
		برو cleanup;
	p = xstrdup(path);
	اگر (!p)
		برو cleanup;

	برای (s = strtok(p, ":"); s; s = strtok(پوچ, ":")) {
		صحیح fd = open(s, O_RDONLY|O_CLOEXEC);
		اگر (fd < 0)
			ادامه;
		rc = faccessat(fd, b, X_OK, 0) == 0;
		close(fd);
		اگر (rc)
			بشکن;
	}
	آزاد(p);
cleanup:
	آزاد(c);
done:
	/*fprintf(stderr, "has PAGER '%s': rc=%d\n", cmd, rc);*/
	بازگشت rc;
}

ایستا تهی __setup_pager(تهی)
{
	ثابت نویسه *pager = getenv("PAGER");
	ساختار sigaction sa;

	اگر (!isatty(STDOUT_FILENO))
		بازگشت;

	اگر (!pager)
		pager = "less";
	وگرنه اگر (!*pager || !strcmp(pager, "cat"))
		بازگشت;

	اگر (!has_command(pager))
		بازگشت;

	/* spawn the pager */
	pager_argv[2] = pager;
	pager_process.argv = pager_argv;
	pager_process.in = -1;

	memset(&sa, 0, اندازه(sa));
	sa.sa_handler = SIG_DFL;

	/* this makes sure that waitpid works as expected */
	sigaction(SIGCHLD, &sa, &pager_process.orig_sigchld);

	اگر (start_command(&pager_process)) {
		sigaction(SIGCHLD, &pager_process.orig_sigchld, پوچ);
		بازگشت;
	}

	/* original process continues, but writes to the pipe */
	dup2(pager_process.in, STDOUT_FILENO);
	setvbuf(stdout, پوچ, _IOLBF, 0);
	اگر (isatty(STDERR_FILENO)) {
		dup2(pager_process.in, STDERR_FILENO);
		setvbuf(stderr, پوچ, _IOLBF, 0);
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
تهی pager_open(تهی)
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
تهی pager_open_header(صحیح header_سطرها, صحیح first_col_width)
{
	اگر (pager_process.pid)
		بازگشت;		/* already running */

	pager_process.header_سطرها = header_سطرها;
	pager_process.header_width = first_col_width;

	pager_process.org_out = dup(STDOUT_FILENO);
	pager_process.org_err = dup(STDERR_FILENO);

	اگر (pager_process.org_out != -1 && pager_process.org_err != -1)
		__setup_pager();

	اگر (!pager_process.pid) {
		اگر (pager_process.org_out != -1)
			close(pager_process.org_out);
		اگر (pager_process.org_err != -1)
			close(pager_process.org_err);
		memset(&pager_process, 0, اندازه(pager_process));
	}
}

/* Close pager and restore original std{out,err}.
 */
تهی pager_close(تهی)
{
	ساختار sigaction sa;
	صحیح ret, safe_errno;

	اگر (pager_process.pid == 0)
		بازگشت;

	memset(&sa, 0, اندازه(sa));
	sa.sa_handler = catch_signal;

	/* set flag instead of calling wait_for_pager again */
	sigaction(SIGINT,  &sa, پوچ);
	sigaction(SIGHUP,  &sa, پوچ);
	sigaction(SIGTERM, &sa, پوچ);
	sigaction(SIGQUIT, &sa, پوچ);

	fflush(پوچ);

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
	sigaction(SIGCHLD, &pager_process.orig_sigchld, پوچ);
	sigaction(SIGINT,  &pager_process.orig_sigint, پوچ);
	sigaction(SIGHUP,  &pager_process.orig_sighup, پوچ);
	sigaction(SIGTERM, &pager_process.orig_sigterm, پوچ);
	sigaction(SIGQUIT, &pager_process.orig_sigquit, پوچ);
	sigaction(SIGPIPE, &pager_process.orig_sigpipe, پوچ);
	errno = safe_errno;

	اگر (ret == -1)
		err(EXIT_FAILURE, _("waitpid failed"));

	اگر (pager_caught_signal || (pager_caught_sigpipe && ret))
		exit(EXIT_FAILURE);

	memset(&pager_process, 0, اندازه(pager_process));
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
bool pager_is_enabled(شمارش ul_pagermode mode)
{
	انتخاب (mode) {
	حالت UL_PAGER_ALWAYS:
		بازگشت true;
	حالت UL_PAGER_NEVER:
		بازگشت false;
	حالت UL_PAGER_AUTO: {
		ثابت نویسه *s;
		bool val;

		اگر (!isatty(STDOUT_FILENO))
			بازگشت false;
		s = safe_getenv("PAGER_ENABLE");
		اگر (!s || ul_strtobool(s, &val) != 0)
			بازگشت false;
		بازگشت val;
	}
	پیشفرض:
		بازگشت false;
	}
}

#ifdef TEST_PROGRAM_PAGER

#define MAX 255

صحیح اصلی(صحیح argc __attribute__ ((__unused__)),
	 نویسه *argv[] __attribute__ ((__unused__)))
{
	صحیح i;

	pager_open();
	برای (i = 0; i < MAX; i++)
		printf("%d\n", i);
	pager_close();
	بازگشت EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_PAGER */
