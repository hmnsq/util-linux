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

#अन्तर्भाव <unistd.h>
#अन्तर्भाव <stdbool.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <sys/types.h>
#अन्तर्भाव <sys/stat.h>
#अन्तर्भाव <sys/wait.h>
#अन्तर्भाव <signal.h>

#अन्तर्भाव "c.h"
#अन्तर्भाव "xalloc.h"
#अन्तर्भाव "nls.h"
#अन्तर्भाव "strutils.h"
#अन्तर्भाव "ttyutils.h"
#अन्तर्भाव "pager.h"
#अन्तर्भाव "env.h"

स्थिर ध्रुव वर्ण *pager_argv[] = { "sh", "-c", रिक्त, रिक्त };

संरचना child_process {
	ध्रुव वर्ण **argv;
	pid_t pid;
	पूर्णाङ्क in;

	पूर्णाङ्क header_पङ्क्तयः;
	पूर्णाङ्क header_width;

	पूर्णाङ्क org_err;
	पूर्णाङ्क org_out;
	संरचना sigaction orig_sigchld;
	संरचना sigaction orig_sigint;
	संरचना sigaction orig_sighup;
	संरचना sigaction orig_sigterm;
	संरचना sigaction orig_sigquit;
	संरचना sigaction orig_sigpipe;
};
स्थिर संरचना child_process pager_process;

स्थिर चञ्चल sig_atomic_t pager_caught_signal;
स्थिर चञ्चल sig_atomic_t pager_caught_sigpipe;

स्थिर अन्तःस्थ शून्य close_pair(पूर्णाङ्क fd[2])
{
	close(fd[0]);
	close(fd[1]);
}

स्थिर शून्य pager_preexec(शून्य)
{
	ध्रुव वर्ण *less_env = getenv("LESS");
	ध्रुव वर्ण *base = less_env ? less_env : "FRSX";
	पूर्णाङ्क header_पङ्क्तयः = pager_process.header_पङ्क्तयः;
	पूर्णाङ्क header_width = pager_process.header_width;
	वर्ण *less_val = रिक्त;

	सचे (header_पङ्क्तयः > 0 && header_width > 0)
		less_val = ul_strfconcat(base, " --header %d,%d",
					 header_पङ्क्तयः, header_width);
	अञ्ञथा सचे (header_width > 0)
		less_val = ul_strfconcat(base, " --header 0,%d",
					 header_width);
	अञ्ञथा सचे (header_पङ्क्तयः > 0)
		less_val = ul_strfconcat(base, " --header %d",
					 header_पङ्क्तयः);

	सचे (less_val) {
		सचे (setenv("LESS", less_val, 1) != 0)
			warn(_("failed to set the %s environment variable"), "LESS");
		मुक्त(less_val);
	} अञ्ञथा सचे (less_env == रिक्त) {
		सचे (setenv("LESS", "FRSX", 0) != 0)
			warn(_("failed to set the %s environment variable"), "LESS");
	}

	सचे (getenv("LV") == रिक्त && setenv("LV", "-c", 0) != 0)
		warn(_("failed to set the %s environment variable"), "LV");
}

स्थिर पूर्णाङ्क start_command(संरचना child_process *cmd)
{
	पूर्णाङ्क fdin[2];

	सचे (pipe(fdin) < 0)
		पटिनिवत्त -1;
	cmd->in = fdin[1];

	fflush(रिक्त);
	cmd->pid = fork();
	सचे (!cmd->pid) {
		dup2(fdin[0], STDIN_FILENO);
		close_pair(fdin);

		pager_preexec();
		execvp(cmd->argv[0], (वर्ण *ध्रुव*) cmd->argv);
		errexec(cmd->argv[0]);
	}

	सचे (cmd->pid < 0) {
		cmd->pid = 0;
		close_pair(fdin);
		पटिनिवत्त -1;
	}

	close(fdin[0]);
	पटिनिवत्त 0;
}

स्थिर पूर्णाङ्क wait_for_pager(शून्य)
{
	pid_t waiting;
	पूर्णाङ्क ret, status;

	करोहि {
		waiting = waitpid(pager_process.pid, &status, 0);
	} याव (waiting == -1 && errno == EINTR);

	सचे (waiting == -1)
		ret = -1;
	अञ्ञथा सचे (waiting == pager_process.pid && WIFEXITED(status))
		ret = WEXITSTATUS(status);
	अञ्ञथा
		ret = 1;

	pager_process.pid = 0;

	पटिनिवत्त ret;
}

स्थिर शून्य catch_signal(पूर्णाङ्क signo)
{
	pager_caught_signal = signo;
}

स्थिर शून्य catch_sigpipe(पूर्णाङ्क signo)
{
	pager_caught_sigpipe = signo;
}

स्थिर शून्य wait_for_pager_signal(पूर्णाङ्क signo __attribute__ ((__unused__)))
{
	/* signal EOF to pager */
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	/* async-signal safe: wait for all children, including pager */
	याव (wait(रिक्त) != -1 || errno == EINTR)
		;

	_exit(EXIT_FAILURE);
}

स्थिर पूर्णाङ्क has_command(ध्रुव वर्ण *cmd)
{
	ध्रुव वर्ण *path;
	वर्ण *b, *c, *p, *s;
	पूर्णाङ्क rc = 0;

	सचे (!cmd)
		गच्छ done;

	c = xstrdup(cmd);
	सचे (!c)
		गच्छ done;
	b = strtok(c, " ");	/* cmd may contain options */
	सचे (!b)
		गच्छ cleanup;

	सचे (strchr(b, '/')) {
		rc = access(b, X_OK) == 0;
		गच्छ cleanup;
	}

	path = getenv("PATH");
	सचे (!path)
		गच्छ cleanup;
	p = xstrdup(path);
	सचे (!p)
		गच्छ cleanup;

	अत्थाय (s = strtok(p, ":"); s; s = strtok(रिक्त, ":")) {
		पूर्णाङ्क fd = open(s, O_RDONLY|O_CLOEXEC);
		सचे (fd < 0)
			अनुवर्त;
		rc = faccessat(fd, b, X_OK, 0) == 0;
		close(fd);
		सचे (rc)
			भङ्ग;
	}
	मुक्त(p);
cleanup:
	मुक्त(c);
done:
	/*fprintf(stderr, "has PAGER '%s': rc=%d\n", cmd, rc);*/
	पटिनिवत्त rc;
}

स्थिर शून्य __setup_pager(शून्य)
{
	ध्रुव वर्ण *pager = getenv("PAGER");
	संरचना sigaction sa;

	सचे (!isatty(STDOUT_FILENO))
		पटिनिवत्त;

	सचे (!pager)
		pager = "less";
	अञ्ञथा सचे (!*pager || !strcmp(pager, "cat"))
		पटिनिवत्त;

	सचे (!has_command(pager))
		पटिनिवत्त;

	/* spawn the pager */
	pager_argv[2] = pager;
	pager_process.argv = pager_argv;
	pager_process.in = -1;

	memset(&sa, 0, परिमाण(sa));
	sa.sa_handler = SIG_DFL;

	/* this makes sure that waitpid works as expected */
	sigaction(SIGCHLD, &sa, &pager_process.orig_sigchld);

	सचे (start_command(&pager_process)) {
		sigaction(SIGCHLD, &pager_process.orig_sigchld, रिक्त);
		पटिनिवत्त;
	}

	/* original process continues, but writes to the pipe */
	dup2(pager_process.in, STDOUT_FILENO);
	setvbuf(stdout, रिक्त, _IOLBF, 0);
	सचे (isatty(STDERR_FILENO)) {
		dup2(pager_process.in, STDERR_FILENO);
		setvbuf(stderr, रिक्त, _IOLBF, 0);
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
शून्य pager_open(शून्य)
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
शून्य pager_open_header(पूर्णाङ्क header_पङ्क्तयः, पूर्णाङ्क first_col_width)
{
	सचे (pager_process.pid)
		पटिनिवत्त;		/* already running */

	pager_process.header_पङ्क्तयः = header_पङ्क्तयः;
	pager_process.header_width = first_col_width;

	pager_process.org_out = dup(STDOUT_FILENO);
	pager_process.org_err = dup(STDERR_FILENO);

	सचे (pager_process.org_out != -1 && pager_process.org_err != -1)
		__setup_pager();

	सचे (!pager_process.pid) {
		सचे (pager_process.org_out != -1)
			close(pager_process.org_out);
		सचे (pager_process.org_err != -1)
			close(pager_process.org_err);
		memset(&pager_process, 0, परिमाण(pager_process));
	}
}

/* Close pager and restore original std{out,err}.
 */
शून्य pager_close(शून्य)
{
	संरचना sigaction sa;
	पूर्णाङ्क ret, safe_errno;

	सचे (pager_process.pid == 0)
		पटिनिवत्त;

	memset(&sa, 0, परिमाण(sa));
	sa.sa_handler = catch_signal;

	/* set flag instead of calling wait_for_pager again */
	sigaction(SIGINT,  &sa, रिक्त);
	sigaction(SIGHUP,  &sa, रिक्त);
	sigaction(SIGTERM, &sa, रिक्त);
	sigaction(SIGQUIT, &sa, रिक्त);

	fflush(रिक्त);

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
	sigaction(SIGCHLD, &pager_process.orig_sigchld, रिक्त);
	sigaction(SIGINT,  &pager_process.orig_sigint, रिक्त);
	sigaction(SIGHUP,  &pager_process.orig_sighup, रिक्त);
	sigaction(SIGTERM, &pager_process.orig_sigterm, रिक्त);
	sigaction(SIGQUIT, &pager_process.orig_sigquit, रिक्त);
	sigaction(SIGPIPE, &pager_process.orig_sigpipe, रिक्त);
	errno = safe_errno;

	सचे (ret == -1)
		err(EXIT_FAILURE, _("waitpid failed"));

	सचे (pager_caught_signal || (pager_caught_sigpipe && ret))
		exit(EXIT_FAILURE);

	memset(&pager_process, 0, परिमाण(pager_process));
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
bool pager_is_enabled(गणना ul_pagermode mode)
{
	विकल्प (mode) {
	पक्ष UL_PAGER_ALWAYS:
		पटिनिवत्त true;
	पक्ष UL_PAGER_NEVER:
		पटिनिवत्त false;
	पक्ष UL_PAGER_AUTO: {
		ध्रुव वर्ण *s;
		bool val;

		सचे (!isatty(STDOUT_FILENO))
			पटिनिवत्त false;
		s = safe_getenv("PAGER_ENABLE");
		सचे (!s || ul_strtobool(s, &val) != 0)
			पटिनिवत्त false;
		पटिनिवत्त val;
	}
	स्वभाव:
		पटिनिवत्त false;
	}
}

#ifdef TEST_PROGRAM_PAGER

#define MAX 255

पूर्णाङ्क main(पूर्णाङ्क argc __attribute__ ((__unused__)),
	 वर्ण *argv[] __attribute__ ((__unused__)))
{
	पूर्णाङ्क i;

	pager_open();
	अत्थाय (i = 0; i < MAX; i++)
		printf("%d\n", i);
	pager_close();
	पटिनिवत्त EXIT_SUCCESS;
}
#endif /* TEST_PROGRAM_PAGER */
