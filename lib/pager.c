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

#అన్తర్భావ <unistd.h>
#అన్తర్భావ <stdbool.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <sys/types.h>
#అన్తర్భావ <sys/stat.h>
#అన్తర్భావ <sys/wait.h>
#అన్తర్భావ <signal.h>

#అన్తర్భావ "c.h"
#అన్తర్భావ "xalloc.h"
#అన్తర్భావ "nls.h"
#అన్తర్భావ "strutils.h"
#అన్తర్భావ "ttyutils.h"
#అన్తర్భావ "pager.h"
#అన్తర్భావ "env.h"

స్థిర స్థిరరాశి అక్షరం *pager_argv[] = { "sh", "-c", ఖాళీ, ఖాళీ };

నిర్మాణం బిడ్డ_process {
	స్థిరరాశి అక్షరం **argv;
	pid_t pid;
	పూర్ణాంకం లోపల;

	పూర్ణాంకం శీర్షిక_పంక్తులు;
	పూర్ణాంకం శీర్షిక_వెడల్పు;

	పూర్ణాంకం org_err;
	పూర్ణాంకం org_బహిర్గతం;
	నిర్మాణం sigaction orig_sigchld;
	నిర్మాణం sigaction orig_sigint;
	నిర్మాణం sigaction orig_sighup;
	నిర్మాణం sigaction orig_sigterm;
	నిర్మాణం sigaction orig_sigquit;
	నిర్మాణం sigaction orig_sigpipe;
};
స్థిర నిర్మాణం బిడ్డ_process pager_process;

స్థిర అస్థిర sig_atomic_t pager_caught_signal;
స్థిర అస్థిర sig_atomic_t pager_caught_sigpipe;

స్థిర అంతఃవరుస శూన్య close_pair(పూర్ణాంకం fd[2])
{
	close(fd[0]);
	close(fd[1]);
}

స్థిర శూన్య pager_preexec(శూన్య)
{
	స్థిరరాశి అక్షరం *less_env = getenv("LESS");
	స్థిరరాశి అక్షరం *పునాది = less_env ? less_env : "FRSX";
	పూర్ణాంకం శీర్షిక_పంక్తులు = pager_process.శీర్షిక_పంక్తులు;
	పూర్ణాంకం శీర్షిక_వెడల్పు = pager_process.శీర్షిక_వెడల్పు;
	అక్షరం *less_val = ఖాళీ;

	ఒకవేళ (శీర్షిక_పంక్తులు > 0 && శీర్షిక_వెడల్పు > 0)
		less_val = ul_strfconcat(పునాది, " --header %d,%d",
					 శీర్షిక_పంక్తులు, శీర్షిక_వెడల్పు);
	లేకపోతే ఒకవేళ (శీర్షిక_వెడల్పు > 0)
		less_val = ul_strfconcat(పునాది, " --header 0,%d",
					 శీర్షిక_వెడల్పు);
	లేకపోతే ఒకవేళ (శీర్షిక_పంక్తులు > 0)
		less_val = ul_strfconcat(పునాది, " --header %d",
					 శీర్షిక_పంక్తులు);

	ఒకవేళ (less_val) {
		ఒకవేళ (setenv("LESS", less_val, 1) != 0)
			warn(_("failed to set the %s environment variable"), "LESS");
		విడుదల(less_val);
	} లేకపోతే ఒకవేళ (less_env == ఖాళీ) {
		ఒకవేళ (setenv("LESS", "FRSX", 0) != 0)
			warn(_("failed to set the %s environment variable"), "LESS");
	}

	ఒకవేళ (getenv("LV") == ఖాళీ && setenv("LV", "-c", 0) != 0)
		warn(_("failed to set the %s environment variable"), "LV");
}

స్థిర పూర్ణాంకం ప్రారంభం_command(నిర్మాణం బిడ్డ_process *cmd)
{
	పూర్ణాంకం fdin[2];

	ఒకవేళ (pipe(fdin) < 0)
		తిరిగి -1;
	cmd->లోపల = fdin[1];

	fflush(ఖాళీ);
	cmd->pid = fork();
	ఒకవేళ (!cmd->pid) {
		dup2(fdin[0], STDIN_FILENO);
		close_pair(fdin);

		pager_preexec();
		execvp(cmd->argv[0], (అక్షరం *స్థిరరాశి*) cmd->argv);
		errexec(cmd->argv[0]);
	}

	ఒకవేళ (cmd->pid < 0) {
		cmd->pid = 0;
		close_pair(fdin);
		తిరిగి -1;
	}

	close(fdin[0]);
	తిరిగి 0;
}

స్థిర పూర్ణాంకం wait_for_pager(శూన్య)
{
	pid_t నిరీక్షణ;
	పూర్ణాంకం ret, స్థితి_2;

	చేయి {
		నిరీక్షణ = waitpid(pager_process.pid, &స్థితి_2, 0);
	} అయితే (నిరీక్షణ == -1 && errno == EINTR);

	ఒకవేళ (నిరీక్షణ == -1)
		ret = -1;
	లేకపోతే ఒకవేళ (నిరీక్షణ == pager_process.pid && WIFEXITED(స్థితి_2))
		ret = WEXITSTATUS(స్థితి_2);
	లేకపోతే
		ret = 1;

	pager_process.pid = 0;

	తిరిగి ret;
}

స్థిర శూన్య catch_signal(పూర్ణాంకం signo)
{
	pager_caught_signal = signo;
}

స్థిర శూన్య catch_sigpipe(పూర్ణాంకం signo)
{
	pager_caught_sigpipe = signo;
}

స్థిర శూన్య wait_for_pager_signal(పూర్ణాంకం signo __attribute__ ((__unused__)))
{
	/* signal EOF to pager */
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	/* async-signal safe: wait for all children, including pager */
	అయితే (wait(ఖాళీ) != -1 || errno == EINTR)
		;

	_exit(EXIT_FAILURE);
}

స్థిర పూర్ణాంకం has_command(స్థిరరాశి అక్షరం *cmd)
{
	స్థిరరాశి అక్షరం *మార్గం;
	అక్షరం *b, *c, *p, *s;
	పూర్ణాంకం rc = 0;

	ఒకవేళ (!cmd)
		వెళ్ళు పూర్తి;

	c = xstrdup(cmd);
	ఒకవేళ (!c)
		వెళ్ళు పూర్తి;
	b = strtok(c, " ");	/* cmd may contain options */
	ఒకవేళ (!b)
		వెళ్ళు cleanup;

	ఒకవేళ (strchr(b, '/')) {
		rc = access(b, X_OK) == 0;
		వెళ్ళు cleanup;
	}

	మార్గం = getenv("PATH");
	ఒకవేళ (!మార్గం)
		వెళ్ళు cleanup;
	p = xstrdup(మార్గం);
	ఒకవేళ (!p)
		వెళ్ళు cleanup;

	కొరకు (s = strtok(p, ":"); s; s = strtok(ఖాళీ, ":")) {
		పూర్ణాంకం fd = open(s, O_RDONLY|O_CLOEXEC);
		ఒకవేళ (fd < 0)
			కొనసాగు;
		rc = faccessat(fd, b, X_OK, 0) == 0;
		close(fd);
		ఒకవేళ (rc)
			ఆపు;
	}
	విడుదల(p);
cleanup:
	విడుదల(c);
పూర్తి:
	/*fprintf(stderr, "has PAGER '%s': rc=%d\n", cmd, rc);*/
	తిరిగి rc;
}

స్థిర శూన్య __setup_pager(శూన్య)
{
	స్థిరరాశి అక్షరం *pager = getenv("PAGER");
	నిర్మాణం sigaction sa;

	ఒకవేళ (!isatty(STDOUT_FILENO))
		తిరిగి;

	ఒకవేళ (!pager)
		pager = "less";
	లేకపోతే ఒకవేళ (!*pager || !strcmp(pager, "cat"))
		తిరిగి;

	ఒకవేళ (!has_command(pager))
		తిరిగి;

	/* spawn the pager */
	pager_argv[2] = pager;
	pager_process.argv = pager_argv;
	pager_process.లోపల = -1;

	memset(&sa, 0, పరిమాణం(sa));
	sa.sa_handler = SIG_DFL;

	/* this makes sure that waitpid works as expected */
	sigaction(SIGCHLD, &sa, &pager_process.orig_sigchld);

	ఒకవేళ (ప్రారంభం_command(&pager_process)) {
		sigaction(SIGCHLD, &pager_process.orig_sigchld, ఖాళీ);
		తిరిగి;
	}

	/* original process continues, but writes to the pipe */
	dup2(pager_process.లోపల, STDOUT_FILENO);
	setvbuf(stdout, ఖాళీ, _IOLBF, 0);
	ఒకవేళ (isatty(STDERR_FILENO)) {
		dup2(pager_process.లోపల, STDERR_FILENO);
		setvbuf(stderr, ఖాళీ, _IOLBF, 0);
	}
	close(pager_process.లోపల);

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
శూన్య pager_open(శూన్య)
{
	pager_open_శీర్షిక(0, 0);
}

/* Setup pager with "less --header" support to freeze header lines and
 * optionally freeze the first column. The @header_lines specifies the
 * number of header lines to freeze (typically 1 for table header).
 * The @first_col_width specifies the number of character columns to
 * freeze (width of first column including separator), or 0 to not
 * freeze any column. Either parameter can be 0 independently;
 * less supports --header 0,M to freeze columns without header lines.
 */
శూన్య pager_open_శీర్షిక(పూర్ణాంకం శీర్షిక_పంక్తులు, పూర్ణాంకం మొదటి_నిలువు_వెడల్పు)
{
	ఒకవేళ (pager_process.pid)
		తిరిగి;		/* already running */

	pager_process.శీర్షిక_పంక్తులు = శీర్షిక_పంక్తులు;
	pager_process.శీర్షిక_వెడల్పు = మొదటి_నిలువు_వెడల్పు;

	pager_process.org_బహిర్గతం = dup(STDOUT_FILENO);
	pager_process.org_err = dup(STDERR_FILENO);

	ఒకవేళ (pager_process.org_బహిర్గతం != -1 && pager_process.org_err != -1)
		__setup_pager();

	ఒకవేళ (!pager_process.pid) {
		ఒకవేళ (pager_process.org_బహిర్గతం != -1)
			close(pager_process.org_బహిర్గతం);
		ఒకవేళ (pager_process.org_err != -1)
			close(pager_process.org_err);
		memset(&pager_process, 0, పరిమాణం(pager_process));
	}
}

/* Close pager and restore original std{out,err}.
 */
శూన్య pager_close(శూన్య)
{
	నిర్మాణం sigaction sa;
	పూర్ణాంకం ret, సురక్షిత_errno;

	ఒకవేళ (pager_process.pid == 0)
		తిరిగి;

	memset(&sa, 0, పరిమాణం(sa));
	sa.sa_handler = catch_signal;

	/* set flag instead of calling wait_for_pager again */
	sigaction(SIGINT,  &sa, ఖాళీ);
	sigaction(SIGHUP,  &sa, ఖాళీ);
	sigaction(SIGTERM, &sa, ఖాళీ);
	sigaction(SIGQUIT, &sa, ఖాళీ);

	fflush(ఖాళీ);

	/* signal EOF to pager */
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	/* restore original output */
	clearerr(stdout);
	dup2(pager_process.org_బహిర్గతం, STDOUT_FILENO);
	clearerr(stderr);
	dup2(pager_process.org_err, STDERR_FILENO);

	close(pager_process.org_బహిర్గతం);
	close(pager_process.org_err);

	ret = wait_for_pager();

	/* restore original signal settings */
	సురక్షిత_errno = errno;
	sigaction(SIGCHLD, &pager_process.orig_sigchld, ఖాళీ);
	sigaction(SIGINT,  &pager_process.orig_sigint, ఖాళీ);
	sigaction(SIGHUP,  &pager_process.orig_sighup, ఖాళీ);
	sigaction(SIGTERM, &pager_process.orig_sigterm, ఖాళీ);
	sigaction(SIGQUIT, &pager_process.orig_sigquit, ఖాళీ);
	sigaction(SIGPIPE, &pager_process.orig_sigpipe, ఖాళీ);
	errno = సురక్షిత_errno;

	ఒకవేళ (ret == -1)
		err(EXIT_FAILURE, _("waitpid failed"));

	ఒకవేళ (pager_caught_signal || (pager_caught_sigpipe && ret))
		నిష్క్రమించు(EXIT_FAILURE);

	memset(&pager_process, 0, పరిమాణం(pager_process));
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
bool pager_is_enabled(గణన ul_pagermode బహుళకం)
{
	ఎంపిక (బహుళకం) {
	స్థితి UL_PAGER_ALWAYS:
		తిరిగి true;
	స్థితి UL_PAGER_NEVER:
		తిరిగి false;
	స్థితి UL_PAGER_AUTO: {
		స్థిరరాశి అక్షరం *s;
		bool val;

		ఒకవేళ (!isatty(STDOUT_FILENO))
			తిరిగి false;
		s = సురక్షిత_getenv("PAGER_ENABLE");
		ఒకవేళ (!s || ul_strtobool(s, &val) != 0)
			తిరిగి false;
		తిరిగి val;
	}
	అప్రమేయ:
		తిరిగి false;
	}
}

#నిర్వచితమైతే TEST_కార్యక్రమం_PAGER

#నిర్వచించు MAX 255

పూర్ణాంకం ప్రధాన(పూర్ణాంకం argc __attribute__ ((__unused__)),
	 అక్షరం *argv[] __attribute__ ((__unused__)))
{
	పూర్ణాంకం i;

	pager_open();
	కొరకు (i = 0; i < MAX; i++)
		రాయి("%d\n", i);
	pager_close();
	తిరిగి EXIT_SUCCESS;
}
#అయితే_ముగింపు /* TEST_PROGRAM_PAGER */
