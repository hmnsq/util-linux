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

#समावेश <unistd.h>
#समावेश <stdbool.h>
#समावेश <stdlib.h>
#समावेश <string.h>
#समावेश <sys/types.h>
#समावेश <sys/stat.h>
#समावेश <sys/wait.h>
#समावेश <signal.h>

#समावेश "c.h"
#समावेश "xalloc.h"
#समावेश "nls.h"
#समावेश "strutils.h"
#समावेश "ttyutils.h"
#समावेश "pager.h"
#समावेश "env.h"

अटल स्थिर अक्षर *pager_argv[] = { "sh", "-c", शून्य, शून्य };

काष्ठा child_process {
	स्थिर अक्षर **argv;
	pid_t pid;
	पूर्णांक in;

	पूर्णांक header_पंक्तियाँ;
	पूर्णांक header_width;

	पूर्णांक org_err;
	पूर्णांक org_out;
	काष्ठा sigaction orig_sigchld;
	काष्ठा sigaction orig_sigint;
	काष्ठा sigaction orig_sighup;
	काष्ठा sigaction orig_sigterm;
	काष्ठा sigaction orig_sigquit;
	काष्ठा sigaction orig_sigpipe;
};
अटल काष्ठा child_process pager_process;

अटल अस्थिर संक_पूर्ण_प्रकार pager_caught_signal;
अटल अस्थिर संक_पूर्ण_प्रकार pager_caught_sigpipe;

अटल अंतरभूत व्योम close_pair(पूर्णांक fd[2])
{
	बंद(fd[0]);
	बंद(fd[1]);
}

अटल व्योम pager_preexec(व्योम)
{
	स्थिर अक्षर *less_env = दो_पर्या("LESS");
	स्थिर अक्षर *base = less_env ? less_env : "FRSX";
	पूर्णांक header_पंक्तियाँ = pager_process.header_पंक्तियाँ;
	पूर्णांक header_width = pager_process.header_width;
	अक्षर *less_val = शून्य;

	अगर (header_पंक्तियाँ > 0 && header_width > 0)
		less_val = ul_strfconcat(base, " --header %d,%d",
					 header_पंक्तियाँ, header_width);
	अन्यथा अगर (header_width > 0)
		less_val = ul_strfconcat(base, " --header 0,%d",
					 header_width);
	अन्यथा अगर (header_पंक्तियाँ > 0)
		less_val = ul_strfconcat(base, " --header %d",
					 header_पंक्तियाँ);

	अगर (less_val) {
		अगर (setenv("LESS", less_val, 1) != 0)
			warn(_("failed to set the %s environment variable"), "LESS");
		मुक्त(less_val);
	} अन्यथा अगर (less_env == शून्य) {
		अगर (setenv("LESS", "FRSX", 0) != 0)
			warn(_("failed to set the %s environment variable"), "LESS");
	}

	अगर (दो_पर्या("LV") == शून्य && setenv("LV", "-c", 0) != 0)
		warn(_("failed to set the %s environment variable"), "LV");
}

अटल पूर्णांक start_command(काष्ठा child_process *cmd)
{
	पूर्णांक fdin[2];

	अगर (pipe(fdin) < 0)
		वापस -1;
	cmd->in = fdin[1];

	ख_साफ(शून्य);
	cmd->pid = विभाजन();
	अगर (!cmd->pid) {
		dup2(fdin[0], STDIN_FILENO);
		close_pair(fdin);

		pager_preexec();
		execvp(cmd->argv[0], (अक्षर *स्थिर*) cmd->argv);
		errexec(cmd->argv[0]);
	}

	अगर (cmd->pid < 0) {
		cmd->pid = 0;
		close_pair(fdin);
		वापस -1;
	}

	बंद(fdin[0]);
	वापस 0;
}

अटल पूर्णांक wait_for_pager(व्योम)
{
	pid_t waiting;
	पूर्णांक ret, status;

	करो {
		waiting = waitpid(pager_process.pid, &status, 0);
	} जबतक (waiting == -1 && त्रुटि_सं == EINTR);

	अगर (waiting == -1)
		ret = -1;
	अन्यथा अगर (waiting == pager_process.pid && WIFEXITED(status))
		ret = WEXITSTATUS(status);
	अन्यथा
		ret = 1;

	pager_process.pid = 0;

	वापस ret;
}

अटल व्योम catch_signal(पूर्णांक signo)
{
	pager_caught_signal = signo;
}

अटल व्योम catch_sigpipe(पूर्णांक signo)
{
	pager_caught_sigpipe = signo;
}

अटल व्योम wait_for_pager_signal(पूर्णांक signo __attribute__ ((__unused__)))
{
	/* signal EOF to pager */
	बंद(STDOUT_FILENO);
	बंद(STDERR_FILENO);

	/* async-signal safe: wait for all children, including pager */
	जबतक (रुको(शून्य) != -1 || त्रुटि_सं == EINTR)
		;

	_निकास(निकास_त्रुटि);
}

अटल पूर्णांक has_command(स्थिर अक्षर *cmd)
{
	स्थिर अक्षर *path;
	अक्षर *b, *c, *p, *s;
	पूर्णांक rc = 0;

	अगर (!cmd)
		जाओ done;

	c = xstrdup(cmd);
	अगर (!c)
		जाओ done;
	b = म_मोहर(c, " ");	/* cmd may contain options */
	अगर (!b)
		जाओ cleanup;

	अगर (म_अक्षर(b, '/')) {
		rc = access(b, X_OK) == 0;
		जाओ cleanup;
	}

	path = दो_पर्या("PATH");
	अगर (!path)
		जाओ cleanup;
	p = xstrdup(path);
	अगर (!p)
		जाओ cleanup;

	क्रम (s = म_मोहर(p, ":"); s; s = म_मोहर(शून्य, ":")) {
		पूर्णांक fd = खोलो(s, O_RDONLY|O_CLOEXEC);
		अगर (fd < 0)
			जारी;
		rc = faccessat(fd, b, X_OK, 0) == 0;
		बंद(fd);
		अगर (rc)
			अवरोध;
	}
	मुक्त(p);
cleanup:
	मुक्त(c);
done:
	/*fprintf(stderr, "has PAGER '%s': rc=%d\n", cmd, rc);*/
	वापस rc;
}

अटल व्योम __setup_pager(व्योम)
{
	स्थिर अक्षर *pager = दो_पर्या("PAGER");
	काष्ठा sigaction sa;

	अगर (!isatty(STDOUT_FILENO))
		वापस;

	अगर (!pager)
		pager = "less";
	अन्यथा अगर (!*pager || !म_भेद(pager, "cat"))
		वापस;

	अगर (!has_command(pager))
		वापस;

	/* spawn the pager */
	pager_argv[2] = pager;
	pager_process.argv = pager_argv;
	pager_process.in = -1;

	स_रखो(&sa, 0, माप(sa));
	sa.sa_handler = संक_पूर्व;

	/* this makes sure that waitpid works as expected */
	sigaction(SIGCHLD, &sa, &pager_process.orig_sigchld);

	अगर (start_command(&pager_process)) {
		sigaction(SIGCHLD, &pager_process.orig_sigchld, शून्य);
		वापस;
	}

	/* original process continues, but writes to the pipe */
	dup2(pager_process.in, STDOUT_FILENO);
	रखो_भबफ(मानक_निकास, शून्य, _IOLBF, 0);
	अगर (isatty(STDERR_FILENO)) {
		dup2(pager_process.in, STDERR_FILENO);
		रखो_भबफ(मानक_त्रुटि, शून्य, _IOLBF, 0);
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
व्योम pager_open(व्योम)
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
व्योम pager_open_header(पूर्णांक header_पंक्तियाँ, पूर्णांक first_col_width)
{
	अगर (pager_process.pid)
		वापस;		/* already running */

	pager_process.header_पंक्तियाँ = header_पंक्तियाँ;
	pager_process.header_width = first_col_width;

	pager_process.org_out = dup(STDOUT_FILENO);
	pager_process.org_err = dup(STDERR_FILENO);

	अगर (pager_process.org_out != -1 && pager_process.org_err != -1)
		__setup_pager();

	अगर (!pager_process.pid) {
		अगर (pager_process.org_out != -1)
			बंद(pager_process.org_out);
		अगर (pager_process.org_err != -1)
			बंद(pager_process.org_err);
		स_रखो(&pager_process, 0, माप(pager_process));
	}
}

/* Close pager and restore original std{out,err}.
 */
व्योम pager_close(व्योम)
{
	काष्ठा sigaction sa;
	पूर्णांक ret, safe_errno;

	अगर (pager_process.pid == 0)
		वापस;

	स_रखो(&sa, 0, माप(sa));
	sa.sa_handler = catch_signal;

	/* set flag instead of calling wait_for_pager again */
	sigaction(संक_विघ्न,  &sa, शून्य);
	sigaction(SIGHUP,  &sa, शून्य);
	sigaction(संक_इति, &sa, शून्य);
	sigaction(SIGQUIT, &sa, शून्य);

	ख_साफ(शून्य);

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
	sigaction(SIGCHLD, &pager_process.orig_sigchld, शून्य);
	sigaction(संक_विघ्न,  &pager_process.orig_sigint, शून्य);
	sigaction(SIGHUP,  &pager_process.orig_sighup, शून्य);
	sigaction(संक_इति, &pager_process.orig_sigterm, शून्य);
	sigaction(SIGQUIT, &pager_process.orig_sigquit, शून्य);
	sigaction(SIGPIPE, &pager_process.orig_sigpipe, शून्य);
	त्रुटि_सं = safe_errno;

	अगर (ret == -1)
		err(निकास_त्रुटि, _("waitpid failed"));

	अगर (pager_caught_signal || (pager_caught_sigpipe && ret))
		निकास(निकास_त्रुटि);

	स_रखो(&pager_process, 0, माप(pager_process));
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
bool pager_is_enabled(क्रमागत ul_pagermode mode)
{
	चयन (mode) {
	हाल UL_PAGER_ALWAYS:
		वापस true;
	हाल UL_PAGER_NEVER:
		वापस false;
	हाल UL_PAGER_AUTO: {
		स्थिर अक्षर *s;
		bool val;

		अगर (!isatty(STDOUT_FILENO))
			वापस false;
		s = safe_getenv("PAGER_ENABLE");
		अगर (!s || ul_strtobool(s, &val) != 0)
			वापस false;
		वापस val;
	}
	शेष:
		वापस false;
	}
}

#अगर_घोषित TEST_PROGRAM_PAGER

#घोषणा MAX 255

पूर्णांक मुख्य(पूर्णांक argc __attribute__ ((__unused__)),
	 अक्षर *argv[] __attribute__ ((__unused__)))
{
	पूर्णांक i;

	pager_open();
	क्रम (i = 0; i < MAX; i++)
		म_लिखो("%d\n", i);
	pager_close();
	वापस निकास_सफल;
}
#पूर्ण_अगर /* TEST_PROGRAM_PAGER */
