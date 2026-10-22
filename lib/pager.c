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

	पूर्णाङ्क header_पंक्तियाँ;
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

स्थिर चञ्चल संक_पूर्ण_प्रकार pager_caught_signal;
स्थिर चञ्चल संक_पूर्ण_प्रकार pager_caught_sigpipe;

स्थिर अन्तःस्थ शून्य close_pair(पूर्णाङ्क fd[2])
{
	बंद(fd[0]);
	बंद(fd[1]);
}

स्थिर शून्य pager_preexec(शून्य)
{
	ध्रुव वर्ण *less_env = दो_पर्या("LESS");
	ध्रुव वर्ण *base = less_env ? less_env : "FRSX";
	पूर्णाङ्क header_पंक्तियाँ = pager_process.header_पंक्तियाँ;
	पूर्णाङ्क header_width = pager_process.header_width;
	वर्ण *less_val = रिक्त;

	यदि (header_पंक्तियाँ > 0 && header_width > 0)
		less_val = ul_strfconcat(base, " --header %d,%d",
					 header_पंक्तियाँ, header_width);
	अन्यथा यदि (header_width > 0)
		less_val = ul_strfconcat(base, " --header 0,%d",
					 header_width);
	अन्यथा यदि (header_पंक्तियाँ > 0)
		less_val = ul_strfconcat(base, " --header %d",
					 header_पंक्तियाँ);

	यदि (less_val) {
		यदि (setenv("LESS", less_val, 1) != 0)
			warn(_("failed to set the %s environment variable"), "LESS");
		मुक्त(less_val);
	} अन्यथा यदि (less_env == रिक्त) {
		यदि (setenv("LESS", "FRSX", 0) != 0)
			warn(_("failed to set the %s environment variable"), "LESS");
	}

	यदि (दो_पर्या("LV") == रिक्त && setenv("LV", "-c", 0) != 0)
		warn(_("failed to set the %s environment variable"), "LV");
}

स्थिर पूर्णाङ्क start_command(संरचना child_process *cmd)
{
	पूर्णाङ्क fdin[2];

	यदि (pipe(fdin) < 0)
		प्रत्यागम -1;
	cmd->in = fdin[1];

	ख_साफ(रिक्त);
	cmd->pid = विभाजन();
	यदि (!cmd->pid) {
		dup2(fdin[0], STDIN_FILENO);
		close_pair(fdin);

		pager_preexec();
		execvp(cmd->argv[0], (वर्ण *ध्रुव*) cmd->argv);
		errexec(cmd->argv[0]);
	}

	यदि (cmd->pid < 0) {
		cmd->pid = 0;
		close_pair(fdin);
		प्रत्यागम -1;
	}

	बंद(fdin[0]);
	प्रत्यागम 0;
}

स्थिर पूर्णाङ्क wait_for_pager(शून्य)
{
	pid_t waiting;
	पूर्णाङ्क ret, status;

	कुरु {
		waiting = waitpid(pager_process.pid, &status, 0);
	} यावत् (waiting == -1 && त्रुटि_सं == EINTR);

	यदि (waiting == -1)
		ret = -1;
	अन्यथा यदि (waiting == pager_process.pid && WIFEXITED(status))
		ret = WEXITSTATUS(status);
	अन्यथा
		ret = 1;

	pager_process.pid = 0;

	प्रत्यागम ret;
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
	बंद(STDOUT_FILENO);
	बंद(STDERR_FILENO);

	/* async-signal safe: wait for all children, including pager */
	यावत् (रुको(रिक्त) != -1 || त्रुटि_सं == EINTR)
		;

	_exit(निकास_त्रुटि);
}

स्थिर पूर्णाङ्क has_command(ध्रुव वर्ण *cmd)
{
	ध्रुव वर्ण *path;
	वर्ण *b, *c, *p, *s;
	पूर्णाङ्क rc = 0;

	यदि (!cmd)
		गच्छ done;

	c = xstrdup(cmd);
	यदि (!c)
		गच्छ done;
	b = म_मोहर(c, " ");	/* cmd may contain options */
	यदि (!b)
		गच्छ cleanup;

	यदि (म_अक्षर(b, '/')) {
		rc = access(b, X_OK) == 0;
		गच्छ cleanup;
	}

	path = दो_पर्या("PATH");
	यदि (!path)
		गच्छ cleanup;
	p = xstrdup(path);
	यदि (!p)
		गच्छ cleanup;

	प्रति (s = म_मोहर(p, ":"); s; s = म_मोहर(रिक्त, ":")) {
		पूर्णाङ्क fd = खोलो(s, O_RDONLY|O_CLOEXEC);
		यदि (fd < 0)
			अनुवर्त;
		rc = faccessat(fd, b, X_OK, 0) == 0;
		बंद(fd);
		यदि (rc)
			भङ्ग;
	}
	मुक्त(p);
cleanup:
	मुक्त(c);
done:
	/*fprintf(stderr, "has PAGER '%s': rc=%d\n", cmd, rc);*/
	प्रत्यागम rc;
}

स्थिर शून्य __setup_pager(शून्य)
{
	ध्रुव वर्ण *pager = दो_पर्या("PAGER");
	संरचना sigaction sa;

	यदि (!isatty(STDOUT_FILENO))
		प्रत्यागम;

	यदि (!pager)
		pager = "less";
	अन्यथा यदि (!*pager || !म_भेद(pager, "cat"))
		प्रत्यागम;

	यदि (!has_command(pager))
		प्रत्यागम;

	/* spawn the pager */
	pager_argv[2] = pager;
	pager_process.argv = pager_argv;
	pager_process.in = -1;

	स_रखो(&sa, 0, परिमाण(sa));
	sa.sa_handler = संक_पूर्व;

	/* this makes sure that waitpid works as expected */
	sigaction(SIGCHLD, &sa, &pager_process.orig_sigchld);

	यदि (start_command(&pager_process)) {
		sigaction(SIGCHLD, &pager_process.orig_sigchld, रिक्त);
		प्रत्यागम;
	}

	/* original process continues, but writes to the pipe */
	dup2(pager_process.in, STDOUT_FILENO);
	रखो_भबफ(मानक_निकास, रिक्त, _IOLBF, 0);
	यदि (isatty(STDERR_FILENO)) {
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
शून्य pager_open_header(पूर्णाङ्क header_पंक्तियाँ, पूर्णाङ्क first_col_width)
{
	यदि (pager_process.pid)
		प्रत्यागम;		/* already running */

	pager_process.header_पंक्तियाँ = header_पंक्तियाँ;
	pager_process.header_width = first_col_width;

	pager_process.org_out = dup(STDOUT_FILENO);
	pager_process.org_err = dup(STDERR_FILENO);

	यदि (pager_process.org_out != -1 && pager_process.org_err != -1)
		__setup_pager();

	यदि (!pager_process.pid) {
		यदि (pager_process.org_out != -1)
			बंद(pager_process.org_out);
		यदि (pager_process.org_err != -1)
			बंद(pager_process.org_err);
		स_रखो(&pager_process, 0, परिमाण(pager_process));
	}
}

/* Close pager and restore original std{out,err}.
 */
शून्य pager_close(शून्य)
{
	संरचना sigaction sa;
	पूर्णाङ्क ret, safe_errno;

	यदि (pager_process.pid == 0)
		प्रत्यागम;

	स_रखो(&sa, 0, परिमाण(sa));
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

	यदि (ret == -1)
		err(निकास_त्रुटि, _("waitpid failed"));

	यदि (pager_caught_signal || (pager_caught_sigpipe && ret))
		निकास(निकास_त्रुटि);

	स_रखो(&pager_process, 0, परिमाण(pager_process));
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
		प्रत्यागम true;
	पक्ष UL_PAGER_NEVER:
		प्रत्यागम false;
	पक्ष UL_PAGER_AUTO: {
		ध्रुव वर्ण *s;
		bool val;

		यदि (!isatty(STDOUT_FILENO))
			प्रत्यागम false;
		s = safe_getenv("PAGER_ENABLE");
		यदि (!s || ul_strtobool(s, &val) != 0)
			प्रत्यागम false;
		प्रत्यागम val;
	}
	स्वभाव:
		प्रत्यागम false;
	}
}

#ifdef TEST_PROGRAM_PAGER

#define MAX 255

पूर्णाङ्क main(पूर्णाङ्क argc __attribute__ ((__unused__)),
	 वर्ण *argv[] __attribute__ ((__unused__)))
{
	पूर्णाङ्क i;

	pager_open();
	प्रति (i = 0; i < MAX; i++)
		म_लिखो("%d\n", i);
	pager_close();
	प्रत्यागम निकास_सफल;
}
#endif /* TEST_PROGRAM_PAGER */
