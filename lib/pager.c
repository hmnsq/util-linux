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

स्थिर स्थिरांक अक्षर *pager_argv[] = { "sh", "-c", रिक्त, रिक्त };

संरचना child_process {
	स्थिरांक अक्षर **argv;
	pid_t pid;
	पूर्णांक in;

	पूर्णांक header_ओळी;
	पूर्णांक header_width;

	पूर्णांक org_err;
	पूर्णांक org_out;
	संरचना sigaction orig_sigchld;
	संरचना sigaction orig_sigint;
	संरचना sigaction orig_sighup;
	संरचना sigaction orig_sigterm;
	संरचना sigaction orig_sigquit;
	संरचना sigaction orig_sigpipe;
};
स्थिर संरचना child_process pager_process;

स्थिर अस्थिर संक_पूर्ण_प्रकार pager_caught_signal;
स्थिर अस्थिर संक_पूर्ण_प्रकार pager_caught_sigpipe;

स्थिर ओळीत शून्य close_pair(पूर्णांक fd[2])
{
	बंद(fd[0]);
	बंद(fd[1]);
}

स्थिर शून्य pager_preexec(शून्य)
{
	स्थिरांक अक्षर *less_env = दो_पर्या("LESS");
	स्थिरांक अक्षर *base = less_env ? less_env : "FRSX";
	पूर्णांक header_ओळी = pager_process.header_ओळी;
	पूर्णांक header_width = pager_process.header_width;
	अक्षर *less_val = रिक्त;

	जर (header_ओळी > 0 && header_width > 0)
		less_val = ul_strfconcat(base, " --header %d,%d",
					 header_ओळी, header_width);
	अन्यथा जर (header_width > 0)
		less_val = ul_strfconcat(base, " --header 0,%d",
					 header_width);
	अन्यथा जर (header_ओळी > 0)
		less_val = ul_strfconcat(base, " --header %d",
					 header_ओळी);

	जर (less_val) {
		जर (setenv("LESS", less_val, 1) != 0)
			warn(_("failed to set the %s environment variable"), "LESS");
		मुक्त(less_val);
	} अन्यथा जर (less_env == रिक्त) {
		जर (setenv("LESS", "FRSX", 0) != 0)
			warn(_("failed to set the %s environment variable"), "LESS");
	}

	जर (दो_पर्या("LV") == रिक्त && setenv("LV", "-c", 0) != 0)
		warn(_("failed to set the %s environment variable"), "LV");
}

स्थिर पूर्णांक start_command(संरचना child_process *cmd)
{
	पूर्णांक fdin[2];

	जर (pipe(fdin) < 0)
		परत -1;
	cmd->in = fdin[1];

	ख_साफ(रिक्त);
	cmd->pid = विभाजन();
	जर (!cmd->pid) {
		dup2(fdin[0], STDIN_FILENO);
		close_pair(fdin);

		pager_preexec();
		execvp(cmd->argv[0], (अक्षर *स्थिरांक*) cmd->argv);
		errexec(cmd->argv[0]);
	}

	जर (cmd->pid < 0) {
		cmd->pid = 0;
		close_pair(fdin);
		परत -1;
	}

	बंद(fdin[0]);
	परत 0;
}

स्थिर पूर्णांक wait_for_pager(शून्य)
{
	pid_t waiting;
	पूर्णांक ret, status;

	करा {
		waiting = waitpid(pager_process.pid, &status, 0);
	} पर्यंत (waiting == -1 && त्रुटि_सं == EINTR);

	जर (waiting == -1)
		ret = -1;
	अन्यथा जर (waiting == pager_process.pid && WIFEXITED(status))
		ret = WEXITSTATUS(status);
	अन्यथा
		ret = 1;

	pager_process.pid = 0;

	परत ret;
}

स्थिर शून्य catch_signal(पूर्णांक signo)
{
	pager_caught_signal = signo;
}

स्थिर शून्य catch_sigpipe(पूर्णांक signo)
{
	pager_caught_sigpipe = signo;
}

स्थिर शून्य wait_for_pager_signal(पूर्णांक signo __attribute__ ((__unused__)))
{
	/* signal EOF to pager */
	बंद(STDOUT_FILENO);
	बंद(STDERR_FILENO);

	/* async-signal safe: wait for all children, including pager */
	पर्यंत (रुको(रिक्त) != -1 || त्रुटि_सं == EINTR)
		;

	_exit(निकास_त्रुटि);
}

स्थिर पूर्णांक has_command(स्थिरांक अक्षर *cmd)
{
	स्थिरांक अक्षर *path;
	अक्षर *b, *c, *p, *s;
	पूर्णांक rc = 0;

	जर (!cmd)
		जा done;

	c = xstrdup(cmd);
	जर (!c)
		जा done;
	b = म_मोहर(c, " ");	/* cmd may contain options */
	जर (!b)
		जा cleanup;

	जर (म_अक्षर(b, '/')) {
		rc = access(b, X_OK) == 0;
		जा cleanup;
	}

	path = दो_पर्या("PATH");
	जर (!path)
		जा cleanup;
	p = xstrdup(path);
	जर (!p)
		जा cleanup;

	साठी (s = म_मोहर(p, ":"); s; s = म_मोहर(रिक्त, ":")) {
		पूर्णांक fd = खोलो(s, O_RDONLY|O_CLOEXEC);
		जर (fd < 0)
			सुरूठेवा;
		rc = faccessat(fd, b, X_OK, 0) == 0;
		बंद(fd);
		जर (rc)
			थांबा;
	}
	मुक्त(p);
cleanup:
	मुक्त(c);
done:
	/*fprintf(stderr, "has PAGER '%s': rc=%d\n", cmd, rc);*/
	परत rc;
}

स्थिर शून्य __setup_pager(शून्य)
{
	स्थिरांक अक्षर *pager = दो_पर्या("PAGER");
	संरचना sigaction sa;

	जर (!isatty(STDOUT_FILENO))
		परत;

	जर (!pager)
		pager = "less";
	अन्यथा जर (!*pager || !म_भेद(pager, "cat"))
		परत;

	जर (!has_command(pager))
		परत;

	/* spawn the pager */
	pager_argv[2] = pager;
	pager_process.argv = pager_argv;
	pager_process.in = -1;

	स_रखो(&sa, 0, आकार(sa));
	sa.sa_handler = संक_पूर्व;

	/* this makes sure that waitpid works as expected */
	sigaction(SIGCHLD, &sa, &pager_process.orig_sigchld);

	जर (start_command(&pager_process)) {
		sigaction(SIGCHLD, &pager_process.orig_sigchld, रिक्त);
		परत;
	}

	/* original process continues, but writes to the pipe */
	dup2(pager_process.in, STDOUT_FILENO);
	रखो_भबफ(मानक_निकास, रिक्त, _IOLBF, 0);
	जर (isatty(STDERR_FILENO)) {
		dup2(pager_process.in, STDERR_FILENO);
		रखो_भबफ(मानक_त्रुटि, रिक्त, _IOLBF, 0);
	}
	बंद(pager_process.in);

	sa.sa_handler = wait_for_pager_signal;

	/* this makes sure that the parent terminates after the pager */
	sigaction(संक_विघ्न,  &sa, &pager_process.orig_sigint);
	sigaction(SIGHUP,  &sa, &pager_process.orig_sighup);
	sigaction(संक_इति, &sa, &pager_process.orig_sigterm);
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
शून्य pager_open_header(पूर्णांक header_ओळी, पूर्णांक first_col_width)
{
	जर (pager_process.pid)
		परत;		/* already running */

	pager_process.header_ओळी = header_ओळी;
	pager_process.header_width = first_col_width;

	pager_process.org_out = dup(STDOUT_FILENO);
	pager_process.org_err = dup(STDERR_FILENO);

	जर (pager_process.org_out != -1 && pager_process.org_err != -1)
		__setup_pager();

	जर (!pager_process.pid) {
		जर (pager_process.org_out != -1)
			बंद(pager_process.org_out);
		जर (pager_process.org_err != -1)
			बंद(pager_process.org_err);
		स_रखो(&pager_process, 0, आकार(pager_process));
	}
}

/* Close pager and restore original std{out,err}.
 */
शून्य pager_close(शून्य)
{
	संरचना sigaction sa;
	पूर्णांक ret, safe_errno;

	जर (pager_process.pid == 0)
		परत;

	स_रखो(&sa, 0, आकार(sa));
	sa.sa_handler = catch_signal;

	/* set flag instead of calling wait_for_pager again */
	sigaction(संक_विघ्न,  &sa, रिक्त);
	sigaction(SIGHUP,  &sa, रिक्त);
	sigaction(संक_इति, &sa, रिक्त);
	sigaction(SIGQUIT, &sa, रिक्त);

	ख_साफ(रिक्त);

	/* signal EOF to pager */
	बंद(STDOUT_FILENO);
	बंद(STDERR_FILENO);

	/* restore original output */
	साफ_त्रुटि(मानक_निकास);
	dup2(pager_process.org_out, STDOUT_FILENO);
	साफ_त्रुटि(मानक_त्रुटि);
	dup2(pager_process.org_err, STDERR_FILENO);

	बंद(pager_process.org_out);
	बंद(pager_process.org_err);

	ret = wait_for_pager();

	/* restore original signal settings */
	safe_errno = त्रुटि_सं;
	sigaction(SIGCHLD, &pager_process.orig_sigchld, रिक्त);
	sigaction(संक_विघ्न,  &pager_process.orig_sigint, रिक्त);
	sigaction(SIGHUP,  &pager_process.orig_sighup, रिक्त);
	sigaction(संक_इति, &pager_process.orig_sigterm, रिक्त);
	sigaction(SIGQUIT, &pager_process.orig_sigquit, रिक्त);
	sigaction(SIGPIPE, &pager_process.orig_sigpipe, रिक्त);
	त्रुटि_सं = safe_errno;

	जर (ret == -1)
		err(निकास_त्रुटि, _("waitpid failed"));

	जर (pager_caught_signal || (pager_caught_sigpipe && ret))
		निकास(निकास_त्रुटि);

	स_रखो(&pager_process, 0, आकार(pager_process));
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
	निवड (mode) {
	स्थिती UL_PAGER_ALWAYS:
		परत true;
	स्थिती UL_PAGER_NEVER:
		परत false;
	स्थिती UL_PAGER_AUTO: {
		स्थिरांक अक्षर *s;
		bool val;

		जर (!isatty(STDOUT_FILENO))
			परत false;
		s = safe_getenv("PAGER_ENABLE");
		जर (!s || ul_strtobool(s, &val) != 0)
			परत false;
		परत val;
	}
	पूर्वनिर्धारित:
		परत false;
	}
}

#ifdef TEST_PROGRAM_PAGER

#define MAX 255

पूर्णांक main(पूर्णांक argc __attribute__ ((__unused__)),
	 अक्षर *argv[] __attribute__ ((__unused__)))
{
	पूर्णांक i;

	pager_open();
	साठी (i = 0; i < MAX; i++)
		म_लिखो("%d\n", i);
	pager_close();
	परत निकास_सफल;
}
#endif /* TEST_PROGRAM_PAGER */
