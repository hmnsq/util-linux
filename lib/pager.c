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

#અન્તર્ભાવ <unistd.h>
#અન્તર્ભાવ <stdbool.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <sys/types.h>
#અન્તર્ભાવ <sys/stat.h>
#અન્તર્ભાવ <sys/wait.h>
#અન્તર્ભાવ <signal.h>

#અન્તર્ભાવ "c.h"
#અન્તર્ભાવ "xalloc.h"
#અન્તર્ભાવ "nls.h"
#અન્તર્ભાવ "strutils.h"
#અન્તર્ભાવ "ttyutils.h"
#અન્તર્ભાવ "pager.h"
#અન્તર્ભાવ "env.h"

સ્થિર અચલ અક્ષર *pager_argv[] = { "sh", "-c", ખાલી, ખાલી };

સંરચના child_process {
	અચલ અક્ષર **argv;
	pid_t pid;
	પૂર્ણાંક in;

	પૂર્ણાંક header_લીટીઓ;
	પૂર્ણાંક header_width;

	પૂર્ણાંક org_err;
	પૂર્ણાંક org_out;
	સંરચના sigaction orig_sigchld;
	સંરચના sigaction orig_sigint;
	સંરચના sigaction orig_sighup;
	સંરચના sigaction orig_sigterm;
	સંરચના sigaction orig_sigquit;
	સંરચના sigaction orig_sigpipe;
};
સ્થિર સંરચના child_process pager_process;

સ્થિર અસ્થિર sig_atomic_t pager_caught_signal;
સ્થિર અસ્થિર sig_atomic_t pager_caught_sigpipe;

સ્થિર અંતઃરેખ શૂન્ય close_pair(પૂર્ણાંક fd[2])
{
	close(fd[0]);
	close(fd[1]);
}

સ્થિર શૂન્ય pager_preexec(શૂન્ય)
{
	અચલ અક્ષર *less_env = getenv("LESS");
	અચલ અક્ષર *base = less_env ? less_env : "FRSX";
	પૂર્ણાંક header_લીટીઓ = pager_process.header_લીટીઓ;
	પૂર્ણાંક header_width = pager_process.header_width;
	અક્ષર *less_val = ખાલી;

	જો (header_લીટીઓ > 0 && header_width > 0)
		less_val = ul_strfconcat(base, " --header %d,%d",
					 header_લીટીઓ, header_width);
	નહીંતર જો (header_width > 0)
		less_val = ul_strfconcat(base, " --header 0,%d",
					 header_width);
	નહીંતર જો (header_લીટીઓ > 0)
		less_val = ul_strfconcat(base, " --header %d",
					 header_લીટીઓ);

	જો (less_val) {
		જો (setenv("LESS", less_val, 1) != 0)
			warn(_("failed to set the %s environment variable"), "LESS");
		મુક્ત(less_val);
	} નહીંતર જો (less_env == ખાલી) {
		જો (setenv("LESS", "FRSX", 0) != 0)
			warn(_("failed to set the %s environment variable"), "LESS");
	}

	જો (getenv("LV") == ખાલી && setenv("LV", "-c", 0) != 0)
		warn(_("failed to set the %s environment variable"), "LV");
}

સ્થિર પૂર્ણાંક start_command(સંરચના child_process *cmd)
{
	પૂર્ણાંક fdin[2];

	જો (pipe(fdin) < 0)
		પાછા -1;
	cmd->in = fdin[1];

	fflush(ખાલી);
	cmd->pid = fork();
	જો (!cmd->pid) {
		dup2(fdin[0], STDIN_FILENO);
		close_pair(fdin);

		pager_preexec();
		execvp(cmd->argv[0], (અક્ષર *અચલ*) cmd->argv);
		errexec(cmd->argv[0]);
	}

	જો (cmd->pid < 0) {
		cmd->pid = 0;
		close_pair(fdin);
		પાછા -1;
	}

	close(fdin[0]);
	પાછા 0;
}

સ્થિર પૂર્ણાંક wait_for_pager(શૂન્ય)
{
	pid_t waiting;
	પૂર્ણાંક ret, status;

	કરો {
		waiting = waitpid(pager_process.pid, &status, 0);
	} જ્યારે (waiting == -1 && errno == EINTR);

	જો (waiting == -1)
		ret = -1;
	નહીંતર જો (waiting == pager_process.pid && WIFEXITED(status))
		ret = WEXITSTATUS(status);
	નહીંતર
		ret = 1;

	pager_process.pid = 0;

	પાછા ret;
}

સ્થિર શૂન્ય catch_signal(પૂર્ણાંક signo)
{
	pager_caught_signal = signo;
}

સ્થિર શૂન્ય catch_sigpipe(પૂર્ણાંક signo)
{
	pager_caught_sigpipe = signo;
}

સ્થિર શૂન્ય wait_for_pager_signal(પૂર્ણાંક signo __attribute__ ((__unused__)))
{
	/* signal EOF to pager */
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	/* async-signal safe: wait for all children, including pager */
	જ્યારે (wait(ખાલી) != -1 || errno == EINTR)
		;

	_exit(EXIT_FAILURE);
}

સ્થિર પૂર્ણાંક has_command(અચલ અક્ષર *cmd)
{
	અચલ અક્ષર *path;
	અક્ષર *b, *c, *p, *s;
	પૂર્ણાંક rc = 0;

	જો (!cmd)
		જાઓ done;

	c = xstrdup(cmd);
	જો (!c)
		જાઓ done;
	b = strtok(c, " ");	/* cmd may contain options */
	જો (!b)
		જાઓ cleanup;

	જો (strchr(b, '/')) {
		rc = access(b, X_OK) == 0;
		જાઓ cleanup;
	}

	path = getenv("PATH");
	જો (!path)
		જાઓ cleanup;
	p = xstrdup(path);
	જો (!p)
		જાઓ cleanup;

	માટે (s = strtok(p, ":"); s; s = strtok(ખાલી, ":")) {
		પૂર્ણાંક fd = open(s, O_RDONLY|O_CLOEXEC);
		જો (fd < 0)
			ચાલુ;
		rc = faccessat(fd, b, X_OK, 0) == 0;
		close(fd);
		જો (rc)
			તોડો;
	}
	મુક્ત(p);
cleanup:
	મુક્ત(c);
done:
	/*fprintf(stderr, "has PAGER '%s': rc=%d\n", cmd, rc);*/
	પાછા rc;
}

સ્થિર શૂન્ય __setup_pager(શૂન્ય)
{
	અચલ અક્ષર *pager = getenv("PAGER");
	સંરચના sigaction sa;

	જો (!isatty(STDOUT_FILENO))
		પાછા;

	જો (!pager)
		pager = "less";
	નહીંતર જો (!*pager || !strcmp(pager, "cat"))
		પાછા;

	જો (!has_command(pager))
		પાછા;

	/* spawn the pager */
	pager_argv[2] = pager;
	pager_process.argv = pager_argv;
	pager_process.in = -1;

	memset(&sa, 0, કદ(sa));
	sa.sa_handler = SIG_DFL;

	/* this makes sure that waitpid works as expected */
	sigaction(SIGCHLD, &sa, &pager_process.orig_sigchld);

	જો (start_command(&pager_process)) {
		sigaction(SIGCHLD, &pager_process.orig_sigchld, ખાલી);
		પાછા;
	}

	/* original process continues, but writes to the pipe */
	dup2(pager_process.in, STDOUT_FILENO);
	setvbuf(stdout, ખાલી, _IOLBF, 0);
	જો (isatty(STDERR_FILENO)) {
		dup2(pager_process.in, STDERR_FILENO);
		setvbuf(stderr, ખાલી, _IOLBF, 0);
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
શૂન્ય pager_open(શૂન્ય)
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
શૂન્ય pager_open_header(પૂર્ણાંક header_લીટીઓ, પૂર્ણાંક first_col_width)
{
	જો (pager_process.pid)
		પાછા;		/* already running */

	pager_process.header_લીટીઓ = header_લીટીઓ;
	pager_process.header_width = first_col_width;

	pager_process.org_out = dup(STDOUT_FILENO);
	pager_process.org_err = dup(STDERR_FILENO);

	જો (pager_process.org_out != -1 && pager_process.org_err != -1)
		__setup_pager();

	જો (!pager_process.pid) {
		જો (pager_process.org_out != -1)
			close(pager_process.org_out);
		જો (pager_process.org_err != -1)
			close(pager_process.org_err);
		memset(&pager_process, 0, કદ(pager_process));
	}
}

/* Close pager and restore original std{out,err}.
 */
શૂન્ય pager_close(શૂન્ય)
{
	સંરચના sigaction sa;
	પૂર્ણાંક ret, safe_errno;

	જો (pager_process.pid == 0)
		પાછા;

	memset(&sa, 0, કદ(sa));
	sa.sa_handler = catch_signal;

	/* set flag instead of calling wait_for_pager again */
	sigaction(SIGINT,  &sa, ખાલી);
	sigaction(SIGHUP,  &sa, ખાલી);
	sigaction(SIGTERM, &sa, ખાલી);
	sigaction(SIGQUIT, &sa, ખાલી);

	fflush(ખાલી);

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
	sigaction(SIGCHLD, &pager_process.orig_sigchld, ખાલી);
	sigaction(SIGINT,  &pager_process.orig_sigint, ખાલી);
	sigaction(SIGHUP,  &pager_process.orig_sighup, ખાલી);
	sigaction(SIGTERM, &pager_process.orig_sigterm, ખાલી);
	sigaction(SIGQUIT, &pager_process.orig_sigquit, ખાલી);
	sigaction(SIGPIPE, &pager_process.orig_sigpipe, ખાલી);
	errno = safe_errno;

	જો (ret == -1)
		err(EXIT_FAILURE, _("waitpid failed"));

	જો (pager_caught_signal || (pager_caught_sigpipe && ret))
		exit(EXIT_FAILURE);

	memset(&pager_process, 0, કદ(pager_process));
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
bool pager_is_enabled(ગણના ul_pagermode mode)
{
	પસંદગી (mode) {
	સ્થિતિ UL_PAGER_ALWAYS:
		પાછા true;
	સ્થિતિ UL_PAGER_NEVER:
		પાછા false;
	સ્થિતિ UL_PAGER_AUTO: {
		અચલ અક્ષર *s;
		bool val;

		જો (!isatty(STDOUT_FILENO))
			પાછા false;
		s = safe_getenv("PAGER_ENABLE");
		જો (!s || ul_strtobool(s, &val) != 0)
			પાછા false;
		પાછા val;
	}
	મૂળભૂત:
		પાછા false;
	}
}

#ifdef TEST_PROGRAM_PAGER

#define MAX 255

પૂર્ણાંક મુખ્ય(પૂર્ણાંક argc __attribute__ ((__unused__)),
	 અક્ષર *argv[] __attribute__ ((__unused__)))
{
	પૂર્ણાંક i;

	pager_open();
	માટે (i = 0; i < MAX; i++)
		printf("%d\n", i);
	pager_close();
	પાછા EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_PAGER */
