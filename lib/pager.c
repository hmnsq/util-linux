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

#ଅନ୍ତର୍ଭାଵ <unistd.h>
#ଅନ୍ତର୍ଭାଵ <stdbool.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <sys/types.h>
#ଅନ୍ତର୍ଭାଵ <sys/stat.h>
#ଅନ୍ତର୍ଭାଵ <sys/wait.h>
#ଅନ୍ତର୍ଭାଵ <signal.h>

#ଅନ୍ତର୍ଭାଵ "c.h"
#ଅନ୍ତର୍ଭାଵ "xalloc.h"
#ଅନ୍ତର୍ଭାଵ "nls.h"
#ଅନ୍ତର୍ଭାଵ "strutils.h"
#ଅନ୍ତର୍ଭାଵ "ttyutils.h"
#ଅନ୍ତର୍ଭାଵ "pager.h"
#ଅନ୍ତର୍ଭାଵ "env.h"

ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *pager_argv[] = { "sh", "-c", ଖାଲି, ଖାଲି };

ସଂରଚନା child_process {
	ଅଚଳ ଅକ୍ଷର **argv;
	pid_t pid;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା in;

	ପୂର୍ଣ୍ଣସଂଖ୍ୟା header_ଧାଡ଼ିଗୁଡ଼ିକ;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା header_width;

	ପୂର୍ଣ୍ଣସଂଖ୍ୟା org_err;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା org_out;
	ସଂରଚନା sigaction orig_sigchld;
	ସଂରଚନା sigaction orig_sigint;
	ସଂରଚନା sigaction orig_sighup;
	ସଂରଚନା sigaction orig_sigterm;
	ସଂରଚନା sigaction orig_sigquit;
	ସଂରଚନା sigaction orig_sigpipe;
};
ସ୍ଥିର ସଂରଚନା child_process pager_process;

ସ୍ଥିର ଅସ୍ଥିର sig_atomic_t pager_caught_signal;
ସ୍ଥିର ଅସ୍ଥିର sig_atomic_t pager_caught_sigpipe;

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ close_pair(ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd[2])
{
	close(fd[0]);
	close(fd[1]);
}

ସ୍ଥିର ଶୂନ୍ୟ pager_preexec(ଶୂନ୍ୟ)
{
	ଅଚଳ ଅକ୍ଷର *less_env = getenv("LESS");
	ଅଚଳ ଅକ୍ଷର *base = less_env ? less_env : "FRSX";
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା header_ଧାଡ଼ିଗୁଡ଼ିକ = pager_process.header_ଧାଡ଼ିଗୁଡ଼ିକ;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା header_width = pager_process.header_width;
	ଅକ୍ଷର *less_val = ଖାଲି;

	ଯଦି (header_ଧାଡ଼ିଗୁଡ଼ିକ > 0 && header_width > 0)
		less_val = ul_strfconcat(base, " --header %d,%d",
					 header_ଧାଡ଼ିଗୁଡ଼ିକ, header_width);
	ଅନ୍ୟଥା ଯଦି (header_width > 0)
		less_val = ul_strfconcat(base, " --header 0,%d",
					 header_width);
	ଅନ୍ୟଥା ଯଦି (header_ଧାଡ଼ିଗୁଡ଼ିକ > 0)
		less_val = ul_strfconcat(base, " --header %d",
					 header_ଧାଡ଼ିଗୁଡ଼ିକ);

	ଯଦି (less_val) {
		ଯଦି (setenv("LESS", less_val, 1) != 0)
			warn(_("failed to set the %s environment variable"), "LESS");
		ମୁକ୍ତ(less_val);
	} ଅନ୍ୟଥା ଯଦି (less_env == ଖାଲି) {
		ଯଦି (setenv("LESS", "FRSX", 0) != 0)
			warn(_("failed to set the %s environment variable"), "LESS");
	}

	ଯଦି (getenv("LV") == ଖାଲି && setenv("LV", "-c", 0) != 0)
		warn(_("failed to set the %s environment variable"), "LV");
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା start_command(ସଂରଚନା child_process *cmd)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା fdin[2];

	ଯଦି (pipe(fdin) < 0)
		ଫେରନ୍ତୁ -1;
	cmd->in = fdin[1];

	fflush(ଖାଲି);
	cmd->pid = fork();
	ଯଦି (!cmd->pid) {
		dup2(fdin[0], STDIN_FILENO);
		close_pair(fdin);

		pager_preexec();
		execvp(cmd->argv[0], (ଅକ୍ଷର *ଅଚଳ*) cmd->argv);
		errexec(cmd->argv[0]);
	}

	ଯଦି (cmd->pid < 0) {
		cmd->pid = 0;
		close_pair(fdin);
		ଫେରନ୍ତୁ -1;
	}

	close(fdin[0]);
	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା wait_for_pager(ଶୂନ୍ୟ)
{
	pid_t waiting;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret, status;

	କର {
		waiting = waitpid(pager_process.pid, &status, 0);
	} ଯେତେବେଳେ (waiting == -1 && errno == EINTR);

	ଯଦି (waiting == -1)
		ret = -1;
	ଅନ୍ୟଥା ଯଦି (waiting == pager_process.pid && WIFEXITED(status))
		ret = WEXITSTATUS(status);
	ଅନ୍ୟଥା
		ret = 1;

	pager_process.pid = 0;

	ଫେରନ୍ତୁ ret;
}

ସ୍ଥିର ଶୂନ୍ୟ catch_signal(ପୂର୍ଣ୍ଣସଂଖ୍ୟା signo)
{
	pager_caught_signal = signo;
}

ସ୍ଥିର ଶୂନ୍ୟ catch_sigpipe(ପୂର୍ଣ୍ଣସଂଖ୍ୟା signo)
{
	pager_caught_sigpipe = signo;
}

ସ୍ଥିର ଶୂନ୍ୟ wait_for_pager_signal(ପୂର୍ଣ୍ଣସଂଖ୍ୟା signo __attribute__ ((__unused__)))
{
	/* signal EOF to pager */
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	/* async-signal safe: wait for all children, including pager */
	ଯେତେବେଳେ (wait(ଖାଲି) != -1 || errno == EINTR)
		;

	_exit(EXIT_FAILURE);
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା has_command(ଅଚଳ ଅକ୍ଷର *cmd)
{
	ଅଚଳ ଅକ୍ଷର *path;
	ଅକ୍ଷର *b, *c, *p, *s;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = 0;

	ଯଦି (!cmd)
		ଯାଅ done;

	c = xstrdup(cmd);
	ଯଦି (!c)
		ଯାଅ done;
	b = strtok(c, " ");	/* cmd may contain options */
	ଯଦି (!b)
		ଯାଅ cleanup;

	ଯଦି (strchr(b, '/')) {
		rc = access(b, X_OK) == 0;
		ଯାଅ cleanup;
	}

	path = getenv("PATH");
	ଯଦି (!path)
		ଯାଅ cleanup;
	p = xstrdup(path);
	ଯଦି (!p)
		ଯାଅ cleanup;

	ପାଇଁ (s = strtok(p, ":"); s; s = strtok(ଖାଲି, ":")) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd = open(s, O_RDONLY|O_CLOEXEC);
		ଯଦି (fd < 0)
			ଜାରି;
		rc = faccessat(fd, b, X_OK, 0) == 0;
		close(fd);
		ଯଦି (rc)
			ଭାଙ୍ଗ;
	}
	ମୁକ୍ତ(p);
cleanup:
	ମୁକ୍ତ(c);
done:
	/*fprintf(stderr, "has PAGER '%s': rc=%d\n", cmd, rc);*/
	ଫେରନ୍ତୁ rc;
}

ସ୍ଥିର ଶୂନ୍ୟ __setup_pager(ଶୂନ୍ୟ)
{
	ଅଚଳ ଅକ୍ଷର *pager = getenv("PAGER");
	ସଂରଚନା sigaction sa;

	ଯଦି (!isatty(STDOUT_FILENO))
		ଫେରନ୍ତୁ;

	ଯଦି (!pager)
		pager = "less";
	ଅନ୍ୟଥା ଯଦି (!*pager || !strcmp(pager, "cat"))
		ଫେରନ୍ତୁ;

	ଯଦି (!has_command(pager))
		ଫେରନ୍ତୁ;

	/* spawn the pager */
	pager_argv[2] = pager;
	pager_process.argv = pager_argv;
	pager_process.in = -1;

	memset(&sa, 0, ଆକାର(sa));
	sa.sa_handler = SIG_DFL;

	/* this makes sure that waitpid works as expected */
	sigaction(SIGCHLD, &sa, &pager_process.orig_sigchld);

	ଯଦି (start_command(&pager_process)) {
		sigaction(SIGCHLD, &pager_process.orig_sigchld, ଖାଲି);
		ଫେରନ୍ତୁ;
	}

	/* original process continues, but writes to the pipe */
	dup2(pager_process.in, STDOUT_FILENO);
	setvbuf(stdout, ଖାଲି, _IOLBF, 0);
	ଯଦି (isatty(STDERR_FILENO)) {
		dup2(pager_process.in, STDERR_FILENO);
		setvbuf(stderr, ଖାଲି, _IOLBF, 0);
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
ଶୂନ୍ୟ pager_open(ଶୂନ୍ୟ)
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
ଶୂନ୍ୟ pager_open_header(ପୂର୍ଣ୍ଣସଂଖ୍ୟା header_ଧାଡ଼ିଗୁଡ଼ିକ, ପୂର୍ଣ୍ଣସଂଖ୍ୟା first_col_width)
{
	ଯଦି (pager_process.pid)
		ଫେରନ୍ତୁ;		/* already running */

	pager_process.header_ଧାଡ଼ିଗୁଡ଼ିକ = header_ଧାଡ଼ିଗୁଡ଼ିକ;
	pager_process.header_width = first_col_width;

	pager_process.org_out = dup(STDOUT_FILENO);
	pager_process.org_err = dup(STDERR_FILENO);

	ଯଦି (pager_process.org_out != -1 && pager_process.org_err != -1)
		__setup_pager();

	ଯଦି (!pager_process.pid) {
		ଯଦି (pager_process.org_out != -1)
			close(pager_process.org_out);
		ଯଦି (pager_process.org_err != -1)
			close(pager_process.org_err);
		memset(&pager_process, 0, ଆକାର(pager_process));
	}
}

/* Close pager and restore original std{out,err}.
 */
ଶୂନ୍ୟ pager_close(ଶୂନ୍ୟ)
{
	ସଂରଚନା sigaction sa;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret, safe_errno;

	ଯଦି (pager_process.pid == 0)
		ଫେରନ୍ତୁ;

	memset(&sa, 0, ଆକାର(sa));
	sa.sa_handler = catch_signal;

	/* set flag instead of calling wait_for_pager again */
	sigaction(SIGINT,  &sa, ଖାଲି);
	sigaction(SIGHUP,  &sa, ଖାଲି);
	sigaction(SIGTERM, &sa, ଖାଲି);
	sigaction(SIGQUIT, &sa, ଖାଲି);

	fflush(ଖାଲି);

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
	sigaction(SIGCHLD, &pager_process.orig_sigchld, ଖାଲି);
	sigaction(SIGINT,  &pager_process.orig_sigint, ଖାଲି);
	sigaction(SIGHUP,  &pager_process.orig_sighup, ଖାଲି);
	sigaction(SIGTERM, &pager_process.orig_sigterm, ଖାଲି);
	sigaction(SIGQUIT, &pager_process.orig_sigquit, ଖାଲି);
	sigaction(SIGPIPE, &pager_process.orig_sigpipe, ଖାଲି);
	errno = safe_errno;

	ଯଦି (ret == -1)
		err(EXIT_FAILURE, _("waitpid failed"));

	ଯଦି (pager_caught_signal || (pager_caught_sigpipe && ret))
		exit(EXIT_FAILURE);

	memset(&pager_process, 0, ଆକାର(pager_process));
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
bool pager_is_enabled(ଗଣନା ul_pagermode mode)
{
	ବାଛ (mode) {
	ସ୍ଥିତି UL_PAGER_ALWAYS:
		ଫେରନ୍ତୁ true;
	ସ୍ଥିତି UL_PAGER_NEVER:
		ଫେରନ୍ତୁ false;
	ସ୍ଥିତି UL_PAGER_AUTO: {
		ଅଚଳ ଅକ୍ଷର *s;
		bool val;

		ଯଦି (!isatty(STDOUT_FILENO))
			ଫେରନ୍ତୁ false;
		s = safe_getenv("PAGER_ENABLE");
		ଯଦି (!s || ul_strtobool(s, &val) != 0)
			ଫେରନ୍ତୁ false;
		ଫେରନ୍ତୁ val;
	}
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଫେରନ୍ତୁ false;
	}
}

#ifdef TEST_PROGRAM_PAGER

#define MAX 255

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ମୁଖ୍ୟ(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc __attribute__ ((__unused__)),
	 ଅକ୍ଷର *argv[] __attribute__ ((__unused__)))
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

	pager_open();
	ପାଇଁ (i = 0; i < MAX; i++)
		printf("%d\n", i);
	pager_close();
	ଫେରନ୍ତୁ EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_PAGER */
