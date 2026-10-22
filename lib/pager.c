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

ساکن ثابت حرف *pager_argv[] = { "sh", "-c", تہی, تہی };

ساخت بچہ_process {
	ثابت حرف **argv;
	pid_t pid;
	صحیح اندر;

	صحیح سرخی_سطریں;
	صحیح سرخی_چوڑائی;

	صحیح org_err;
	صحیح org_برآمد;
	ساخت sigaction orig_sigchld;
	ساخت sigaction orig_sigint;
	ساخت sigaction orig_sighup;
	ساخت sigaction orig_sigterm;
	ساخت sigaction orig_sigquit;
	ساخت sigaction orig_sigpipe;
};
ساکن ساخت بچہ_process pager_process;

ساکن متلون sig_atomic_t pager_caught_signal;
ساکن متلون sig_atomic_t pager_caught_sigpipe;

ساکن اندرونی خالی close_pair(صحیح fd[2])
{
	close(fd[0]);
	close(fd[1]);
}

ساکن خالی pager_preexec(خالی)
{
	ثابت حرف *less_env = getenv("LESS");
	ثابت حرف *بنیاد = less_env ? less_env : "FRSX";
	صحیح سرخی_سطریں = pager_process.سرخی_سطریں;
	صحیح سرخی_چوڑائی = pager_process.سرخی_چوڑائی;
	حرف *less_val = تہی;

	اگر (سرخی_سطریں > 0 && سرخی_چوڑائی > 0)
		less_val = ul_strfconcat(بنیاد, " --header %d,%d",
					 سرخی_سطریں, سرخی_چوڑائی);
	ورنہ اگر (سرخی_چوڑائی > 0)
		less_val = ul_strfconcat(بنیاد, " --header 0,%d",
					 سرخی_چوڑائی);
	ورنہ اگر (سرخی_سطریں > 0)
		less_val = ul_strfconcat(بنیاد, " --header %d",
					 سرخی_سطریں);

	اگر (less_val) {
		اگر (setenv("LESS", less_val, 1) != 0)
			warn(_("failed to set the %s environment variable"), "LESS");
		آزاد(less_val);
	} ورنہ اگر (less_env == تہی) {
		اگر (setenv("LESS", "FRSX", 0) != 0)
			warn(_("failed to set the %s environment variable"), "LESS");
	}

	اگر (getenv("LV") == تہی && setenv("LV", "-c", 0) != 0)
		warn(_("failed to set the %s environment variable"), "LV");
}

ساکن صحیح آغاز_command(ساخت بچہ_process *cmd)
{
	صحیح fdin[2];

	اگر (pipe(fdin) < 0)
		واپس -1;
	cmd->اندر = fdin[1];

	fflush(تہی);
	cmd->pid = fork();
	اگر (!cmd->pid) {
		dup2(fdin[0], STDIN_FILENO);
		close_pair(fdin);

		pager_preexec();
		execvp(cmd->argv[0], (حرف *ثابت*) cmd->argv);
		errexec(cmd->argv[0]);
	}

	اگر (cmd->pid < 0) {
		cmd->pid = 0;
		close_pair(fdin);
		واپس -1;
	}

	close(fdin[0]);
	واپس 0;
}

ساکن صحیح wait_for_pager(خالی)
{
	pid_t انتظار;
	صحیح ret, حالت_2;

	کرو {
		انتظار = waitpid(pager_process.pid, &حالت_2, 0);
	} جب (انتظار == -1 && errno == EINTR);

	اگر (انتظار == -1)
		ret = -1;
	ورنہ اگر (انتظار == pager_process.pid && WIFEXITED(حالت_2))
		ret = WEXITSTATUS(حالت_2);
	ورنہ
		ret = 1;

	pager_process.pid = 0;

	واپس ret;
}

ساکن خالی catch_signal(صحیح signo)
{
	pager_caught_signal = signo;
}

ساکن خالی catch_sigpipe(صحیح signo)
{
	pager_caught_sigpipe = signo;
}

ساکن خالی wait_for_pager_signal(صحیح signo __attribute__ ((__unused__)))
{
	/* signal EOF to pager */
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	/* async-signal safe: wait for all children, including pager */
	جب (wait(تہی) != -1 || errno == EINTR)
		;

	_exit(EXIT_FAILURE);
}

ساکن صحیح has_command(ثابت حرف *cmd)
{
	ثابت حرف *راستہ;
	حرف *b, *c, *p, *s;
	صحیح rc = 0;

	اگر (!cmd)
		جاؤ مکمل;

	c = xstrdup(cmd);
	اگر (!c)
		جاؤ مکمل;
	b = strtok(c, " ");	/* cmd may contain options */
	اگر (!b)
		جاؤ cleanup;

	اگر (strchr(b, '/')) {
		rc = access(b, X_OK) == 0;
		جاؤ cleanup;
	}

	راستہ = getenv("PATH");
	اگر (!راستہ)
		جاؤ cleanup;
	p = xstrdup(راستہ);
	اگر (!p)
		جاؤ cleanup;

	برائے (s = strtok(p, ":"); s; s = strtok(تہی, ":")) {
		صحیح fd = open(s, O_RDONLY|O_CLOEXEC);
		اگر (fd < 0)
			جاری;
		rc = faccessat(fd, b, X_OK, 0) == 0;
		close(fd);
		اگر (rc)
			توڑو;
	}
	آزاد(p);
cleanup:
	آزاد(c);
مکمل:
	/*fprintf(stderr, "has PAGER '%s': rc=%d\n", cmd, rc);*/
	واپس rc;
}

ساکن خالی __setup_pager(خالی)
{
	ثابت حرف *pager = getenv("PAGER");
	ساخت sigaction sa;

	اگر (!isatty(STDOUT_FILENO))
		واپس;

	اگر (!pager)
		pager = "less";
	ورنہ اگر (!*pager || !strcmp(pager, "cat"))
		واپس;

	اگر (!has_command(pager))
		واپس;

	/* spawn the pager */
	pager_argv[2] = pager;
	pager_process.argv = pager_argv;
	pager_process.اندر = -1;

	memset(&sa, 0, حجم(sa));
	sa.sa_handler = SIG_DFL;

	/* this makes sure that waitpid works as expected */
	sigaction(SIGCHLD, &sa, &pager_process.orig_sigchld);

	اگر (آغاز_command(&pager_process)) {
		sigaction(SIGCHLD, &pager_process.orig_sigchld, تہی);
		واپس;
	}

	/* original process continues, but writes to the pipe */
	dup2(pager_process.اندر, STDOUT_FILENO);
	setvbuf(stdout, تہی, _IOLBF, 0);
	اگر (isatty(STDERR_FILENO)) {
		dup2(pager_process.اندر, STDERR_FILENO);
		setvbuf(stderr, تہی, _IOLBF, 0);
	}
	close(pager_process.اندر);

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
خالی pager_open(خالی)
{
	pager_open_سرخی(0, 0);
}

/* Setup pager with "less --header" support to freeze header lines and
 * optionally freeze the first column. The @header_lines specifies the
 * number of header lines to freeze (typically 1 for table header).
 * The @first_col_width specifies the number of character columns to
 * freeze (width of first column including separator), or 0 to not
 * freeze any column. Either parameter can be 0 independently;
 * less supports --header 0,M to freeze columns without header lines.
 */
خالی pager_open_سرخی(صحیح سرخی_سطریں, صحیح پہلا_ستون_چوڑائی)
{
	اگر (pager_process.pid)
		واپس;		/* already running */

	pager_process.سرخی_سطریں = سرخی_سطریں;
	pager_process.سرخی_چوڑائی = پہلا_ستون_چوڑائی;

	pager_process.org_برآمد = dup(STDOUT_FILENO);
	pager_process.org_err = dup(STDERR_FILENO);

	اگر (pager_process.org_برآمد != -1 && pager_process.org_err != -1)
		__setup_pager();

	اگر (!pager_process.pid) {
		اگر (pager_process.org_برآمد != -1)
			close(pager_process.org_برآمد);
		اگر (pager_process.org_err != -1)
			close(pager_process.org_err);
		memset(&pager_process, 0, حجم(pager_process));
	}
}

/* Close pager and restore original std{out,err}.
 */
خالی pager_close(خالی)
{
	ساخت sigaction sa;
	صحیح ret, محفوظ_errno;

	اگر (pager_process.pid == 0)
		واپس;

	memset(&sa, 0, حجم(sa));
	sa.sa_handler = catch_signal;

	/* set flag instead of calling wait_for_pager again */
	sigaction(SIGINT,  &sa, تہی);
	sigaction(SIGHUP,  &sa, تہی);
	sigaction(SIGTERM, &sa, تہی);
	sigaction(SIGQUIT, &sa, تہی);

	fflush(تہی);

	/* signal EOF to pager */
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	/* restore original output */
	clearerr(stdout);
	dup2(pager_process.org_برآمد, STDOUT_FILENO);
	clearerr(stderr);
	dup2(pager_process.org_err, STDERR_FILENO);

	close(pager_process.org_برآمد);
	close(pager_process.org_err);

	ret = wait_for_pager();

	/* restore original signal settings */
	محفوظ_errno = errno;
	sigaction(SIGCHLD, &pager_process.orig_sigchld, تہی);
	sigaction(SIGINT,  &pager_process.orig_sigint, تہی);
	sigaction(SIGHUP,  &pager_process.orig_sighup, تہی);
	sigaction(SIGTERM, &pager_process.orig_sigterm, تہی);
	sigaction(SIGQUIT, &pager_process.orig_sigquit, تہی);
	sigaction(SIGPIPE, &pager_process.orig_sigpipe, تہی);
	errno = محفوظ_errno;

	اگر (ret == -1)
		err(EXIT_FAILURE, _("waitpid failed"));

	اگر (pager_caught_signal || (pager_caught_sigpipe && ret))
		باہر_نکلو(EXIT_FAILURE);

	memset(&pager_process, 0, حجم(pager_process));
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
bool pager_is_enabled(شمار ul_pagermode منوال)
{
	انتخاب (منوال) {
	حالت UL_PAGER_ALWAYS:
		واپس true;
	حالت UL_PAGER_NEVER:
		واپس false;
	حالت UL_PAGER_AUTO: {
		ثابت حرف *s;
		bool val;

		اگر (!isatty(STDOUT_FILENO))
			واپس false;
		s = محفوظ_getenv("PAGER_ENABLE");
		اگر (!s || ul_strtobool(s, &val) != 0)
			واپس false;
		واپس val;
	}
	طےشدہ:
		واپس false;
	}
}

#اگر_تعریف_شدہ TEST_پروگرام_PAGER

#تعریف_کرو MAX 255

صحیح مین(صحیح argc __attribute__ ((__unused__)),
	 حرف *argv[] __attribute__ ((__unused__)))
{
	صحیح i;

	pager_open();
	برائے (i = 0; i < MAX; i++)
		لکھو("%d\n", i);
	pager_close();
	واپس EXIT_SUCCESS;
}
#اگر_ختم /* TEST_PROGRAM_PAGER */
