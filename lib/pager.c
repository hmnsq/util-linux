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

#ضمن <unistd.h>
#ضمن <stdbool.h>
#ضمن <stdlib.h>
#ضمن <string.h>
#ضمن <sys/types.h>
#ضمن <sys/stat.h>
#ضمن <sys/wait.h>
#ضمن <signal.h>

#ضمن "c.h"
#ضمن "xalloc.h"
#ضمن "nls.h"
#ضمن "strutils.h"
#ضمن "ttyutils.h"
#ضمن "pager.h"
#ضمن "env.h"

ساكن ثابت حرف *pager_argv[] = { "sh", "-c", عدم, عدم };

بنية child_process {
	ثابت حرف **argv;
	pid_t pid;
	صحيح in;

	صحيح header_ليوك;
	صحيح header_width;

	صحيح org_err;
	صحيح org_out;
	بنية sigaction orig_sigchld;
	بنية sigaction orig_sigint;
	بنية sigaction orig_sighup;
	بنية sigaction orig_sigterm;
	بنية sigaction orig_sigquit;
	بنية sigaction orig_sigpipe;
};
ساكن بنية child_process pager_process;

ساكن متطاير sig_atomic_t pager_caught_signal;
ساكن متطاير sig_atomic_t pager_caught_sigpipe;

ساكن مضمن فراغ close_pair(صحيح fd[2])
{
	close(fd[0]);
	close(fd[1]);
}

ساكن فراغ pager_preexec(فراغ)
{
	ثابت حرف *less_env = getenv("LESS");
	ثابت حرف *base = less_env ? less_env : "FRSX";
	صحيح header_ليوك = pager_process.header_ليوك;
	صحيح header_width = pager_process.header_width;
	حرف *less_val = عدم;

	إذا (header_ليوك > 0 && header_width > 0)
		less_val = ul_strfconcat(base, " --header %d,%d",
					 header_ليوك, header_width);
	وإلا إذا (header_width > 0)
		less_val = ul_strfconcat(base, " --header 0,%d",
					 header_width);
	وإلا إذا (header_ليوك > 0)
		less_val = ul_strfconcat(base, " --header %d",
					 header_ليوك);

	إذا (less_val) {
		إذا (setenv("LESS", less_val, 1) != 0)
			warn(_("failed to set the %s environment variable"), "LESS");
		حرر(less_val);
	} وإلا إذا (less_env == عدم) {
		إذا (setenv("LESS", "FRSX", 0) != 0)
			warn(_("failed to set the %s environment variable"), "LESS");
	}

	إذا (getenv("LV") == عدم && setenv("LV", "-c", 0) != 0)
		warn(_("failed to set the %s environment variable"), "LV");
}

ساكن صحيح start_command(بنية child_process *cmd)
{
	صحيح fdin[2];

	إذا (pipe(fdin) < 0)
		أرجع -1;
	cmd->in = fdin[1];

	fflush(عدم);
	cmd->pid = fork();
	إذا (!cmd->pid) {
		dup2(fdin[0], STDIN_FILENO);
		close_pair(fdin);

		pager_preexec();
		execvp(cmd->argv[0], (حرف *ثابت*) cmd->argv);
		errexec(cmd->argv[0]);
	}

	إذا (cmd->pid < 0) {
		cmd->pid = 0;
		close_pair(fdin);
		أرجع -1;
	}

	close(fdin[0]);
	أرجع 0;
}

ساكن صحيح wait_for_pager(فراغ)
{
	pid_t waiting;
	صحيح ret, status;

	افعل {
		waiting = waitpid(pager_process.pid, &status, 0);
	} طالما (waiting == -1 && errno == EINTR);

	إذا (waiting == -1)
		ret = -1;
	وإلا إذا (waiting == pager_process.pid && WIFEXITED(status))
		ret = WEXITSTATUS(status);
	وإلا
		ret = 1;

	pager_process.pid = 0;

	أرجع ret;
}

ساكن فراغ catch_signal(صحيح signo)
{
	pager_caught_signal = signo;
}

ساكن فراغ catch_sigpipe(صحيح signo)
{
	pager_caught_sigpipe = signo;
}

ساكن فراغ wait_for_pager_signal(صحيح signo __attribute__ ((__unused__)))
{
	/* signal EOF to pager */
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	/* async-signal safe: wait for all children, including pager */
	طالما (wait(عدم) != -1 || errno == EINTR)
		;

	_exit(EXIT_FAILURE);
}

ساكن صحيح has_command(ثابت حرف *cmd)
{
	ثابت حرف *path;
	حرف *b, *c, *p, *s;
	صحيح rc = 0;

	إذا (!cmd)
		اذهب done;

	c = xstrdup(cmd);
	إذا (!c)
		اذهب done;
	b = strtok(c, " ");	/* cmd may contain options */
	إذا (!b)
		اذهب cleanup;

	إذا (strchr(b, '/')) {
		rc = access(b, X_OK) == 0;
		اذهب cleanup;
	}

	path = getenv("PATH");
	إذا (!path)
		اذهب cleanup;
	p = xstrdup(path);
	إذا (!p)
		اذهب cleanup;

	لكل (s = strtok(p, ":"); s; s = strtok(عدم, ":")) {
		صحيح fd = open(s, O_RDONLY|O_CLOEXEC);
		إذا (fd < 0)
			استمر;
		rc = faccessat(fd, b, X_OK, 0) == 0;
		close(fd);
		إذا (rc)
			اكسر;
	}
	حرر(p);
cleanup:
	حرر(c);
done:
	/*fprintf(stderr, "has PAGER '%s': rc=%d\n", cmd, rc);*/
	أرجع rc;
}

ساكن فراغ __setup_pager(فراغ)
{
	ثابت حرف *pager = getenv("PAGER");
	بنية sigaction sa;

	إذا (!isatty(STDOUT_FILENO))
		أرجع;

	إذا (!pager)
		pager = "less";
	وإلا إذا (!*pager || !strcmp(pager, "cat"))
		أرجع;

	إذا (!has_command(pager))
		أرجع;

	/* spawn the pager */
	pager_argv[2] = pager;
	pager_process.argv = pager_argv;
	pager_process.in = -1;

	memset(&sa, 0, حجم(sa));
	sa.sa_handler = SIG_DFL;

	/* this makes sure that waitpid works as expected */
	sigaction(SIGCHLD, &sa, &pager_process.orig_sigchld);

	إذا (start_command(&pager_process)) {
		sigaction(SIGCHLD, &pager_process.orig_sigchld, عدم);
		أرجع;
	}

	/* original process continues, but writes to the pipe */
	dup2(pager_process.in, STDOUT_FILENO);
	setvbuf(stdout, عدم, _IOLBF, 0);
	إذا (isatty(STDERR_FILENO)) {
		dup2(pager_process.in, STDERR_FILENO);
		setvbuf(stderr, عدم, _IOLBF, 0);
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
فراغ pager_open(فراغ)
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
فراغ pager_open_header(صحيح header_ليوك, صحيح first_col_width)
{
	إذا (pager_process.pid)
		أرجع;		/* already running */

	pager_process.header_ليوك = header_ليوك;
	pager_process.header_width = first_col_width;

	pager_process.org_out = dup(STDOUT_FILENO);
	pager_process.org_err = dup(STDERR_FILENO);

	إذا (pager_process.org_out != -1 && pager_process.org_err != -1)
		__setup_pager();

	إذا (!pager_process.pid) {
		إذا (pager_process.org_out != -1)
			close(pager_process.org_out);
		إذا (pager_process.org_err != -1)
			close(pager_process.org_err);
		memset(&pager_process, 0, حجم(pager_process));
	}
}

/* Close pager and restore original std{out,err}.
 */
فراغ pager_close(فراغ)
{
	بنية sigaction sa;
	صحيح ret, safe_errno;

	إذا (pager_process.pid == 0)
		أرجع;

	memset(&sa, 0, حجم(sa));
	sa.sa_handler = catch_signal;

	/* set flag instead of calling wait_for_pager again */
	sigaction(SIGINT,  &sa, عدم);
	sigaction(SIGHUP,  &sa, عدم);
	sigaction(SIGTERM, &sa, عدم);
	sigaction(SIGQUIT, &sa, عدم);

	fflush(عدم);

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
	sigaction(SIGCHLD, &pager_process.orig_sigchld, عدم);
	sigaction(SIGINT,  &pager_process.orig_sigint, عدم);
	sigaction(SIGHUP,  &pager_process.orig_sighup, عدم);
	sigaction(SIGTERM, &pager_process.orig_sigterm, عدم);
	sigaction(SIGQUIT, &pager_process.orig_sigquit, عدم);
	sigaction(SIGPIPE, &pager_process.orig_sigpipe, عدم);
	errno = safe_errno;

	إذا (ret == -1)
		err(EXIT_FAILURE, _("waitpid failed"));

	إذا (pager_caught_signal || (pager_caught_sigpipe && ret))
		exit(EXIT_FAILURE);

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
bool pager_is_enabled(تعداد ul_pagermode mode)
{
	بدل (mode) {
	حالة UL_PAGER_ALWAYS:
		أرجع true;
	حالة UL_PAGER_NEVER:
		أرجع false;
	حالة UL_PAGER_AUTO: {
		ثابت حرف *s;
		bool val;

		إذا (!isatty(STDOUT_FILENO))
			أرجع false;
		s = safe_getenv("PAGER_ENABLE");
		إذا (!s || ul_strtobool(s, &val) != 0)
			أرجع false;
		أرجع val;
	}
	افتراضي:
		أرجع false;
	}
}

#ifdef TEST_PROGRAM_PAGER

#define MAX 255

صحيح main(صحيح argc __attribute__ ((__unused__)),
	 حرف *argv[] __attribute__ ((__unused__)))
{
	صحيح i;

	pager_open();
	لكل (i = 0; i < MAX; i++)
		printf("%d\n", i);
	pager_close();
	أرجع EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_PAGER */
