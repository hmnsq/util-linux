/*
 * This is pseudo-terminal container for child process where parent creates a
 * proxy between the current std{in,out,etrr} and the child's pty. Advantages:
 *
 * - child has no access to parent's terminal (e.g. su --pty)
 * - parent can log all traffic between user and child's terminal (e.g. script(1))
 * - it's possible to start commands on terminal although parent has no terminal
 *
 * This code is in the public domain; do with it what you wish.
 *
 * Written by Karel Zak <kzak@redhat.com> in Jul 2019
 */
#समावेश <stdio.h>
#समावेश <stdlib.h>
#समावेश <pty.h>
#समावेश <poll.h>
#समावेश <sys/signalfd.h>
#समावेश <paths.h>
#समावेश <sys/types.h>
#समावेश <sys/wait.h>
#समावेश <inttypes.h>

#समावेश "c.h"
#समावेश "all-io.h"
#समावेश "ttyutils.h"
#समावेश "pty-session.h"
#समावेश "monotonic.h"
#समावेश "debug.h"

अटल UL_DEBUG_DEFINE_MASK(ulpty);
UL_DEBUG_DEFINE_MASKNAMES(ulpty) = UL_DEBUG_EMPTY_MASKNAMES;

#घोषणा ULPTY_DEBUG_INIT	(1 << 1)
#घोषणा ULPTY_DEBUG_SETUP	(1 << 2)
#घोषणा ULPTY_DEBUG_SIG		(1 << 3)
#घोषणा ULPTY_DEBUG_IO		(1 << 4)
#घोषणा ULPTY_DEBUG_DONE	(1 << 5)

#घोषणा DBG(m, x)		__UL_DBG(ulpty, ULPTY_DEBUG_, m, x)
#घोषणा DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulpty, ULPTY_DEBUG_, m, h, x)
#घोषणा ON_DBG(m, x)		__UL_DBG_CALL(ulpty, ULPTY_DEBUG_, m, x)

व्योम ul_pty_init_debug(पूर्णांक mask)
{
	अगर (ulpty_debug_mask)
		वापस;
	__UL_INIT_DEBUG_FROM_ENV(ulpty, ULPTY_DEBUG_, mask, ULPTY_DEBUG);
}

काष्ठा ul_pty *ul_new_pty(पूर्णांक is_stdin_tty)
{
	काष्ठा ul_pty *pty = सुस्मृति(1, माप(*pty));

	अगर (!pty)
		वापस शून्य;

	DBG_OBJ(SETUP, pty, ul_debug("alloc handler"));
	pty->isterm = is_stdin_tty;
	pty->master = -1;
	pty->slave = -1;
	pty->sigfd = -1;
	pty->child = (pid_t) -1;

	वापस pty;
}

व्योम ul_free_pty(काष्ठा ul_pty *pty)
{
	काष्ठा ul_pty_child_बफर *hd;

	जबतक ((hd = pty->child_बफर_शीर्ष)) {
		pty->child_बफर_शीर्ष = hd->अग्र;
		मुक्त(hd);
	}

	जबतक ((hd = pty->free_buffers)) {
		pty->free_buffers = hd->अग्र;
		मुक्त(hd);
	}
	मुक्त(pty);
}

व्योम ul_pty_slave_echo(काष्ठा ul_pty *pty, पूर्णांक enable)
{
	निश्चित(pty);
	pty->slave_echo = enable ? 1 : 0;
}

पूर्णांक ul_pty_get_delivered_signal(काष्ठा ul_pty *pty)
{
	निश्चित(pty);
	वापस pty->delivered_signal;
}

काष्ठा ul_pty_callbacks *ul_pty_get_callbacks(काष्ठा ul_pty *pty)
{
	निश्चित(pty);
	वापस &pty->callbacks;
}

व्योम ul_pty_set_callback_तथ्य(काष्ठा ul_pty *pty, व्योम *तथ्य)
{
	निश्चित(pty);
	pty->callback_तथ्य = तथ्य;
}

व्योम ul_pty_set_child(काष्ठा ul_pty *pty, pid_t child)
{
	निश्चित(pty);
	pty->child = child;
}

पूर्णांक ul_pty_get_childfd(काष्ठा ul_pty *pty)
{
	निश्चित(pty);
	वापस pty->master;
}

pid_t ul_pty_get_child(काष्ठा ul_pty *pty)
{
	निश्चित(pty);
	वापस pty->child;
}

/* it's active when signals are redirected to sigfd */
पूर्णांक ul_pty_is_running(काष्ठा ul_pty *pty)
{
	निश्चित(pty);
	वापस pty->sigfd >= 0;
}

व्योम ul_pty_set_mainloop_time(काष्ठा ul_pty *pty, काष्ठा timeval *tv)
{
	निश्चित(pty);
	अगर (!tv) {
		DBG_OBJ(IO, pty, ul_debug("mainloop time: clear"));
		timerclear(&pty->अग्र_callback_time);
	} अन्यथा {
		pty->अग्र_callback_time.tv_sec = tv->tv_sec;
		pty->अग्र_callback_time.tv_usec = tv->tv_usec;
		DBG_OBJ(IO, pty, ul_debug("mainloop time: %"PRId64".%06"PRId64,
				(int64_t) tv->tv_sec, (int64_t) tv->tv_usec));
	}
}

अटल व्योम pty_signals_cleanup(काष्ठा ul_pty *pty)
{
	अगर (pty->sigfd != -1)
		बंद(pty->sigfd);
	pty->sigfd = -1;

	/* restore original setting */
	sigprocmask(SIG_SETMASK, &pty->orgsig, शून्य);
}

/* call me before fork() */
पूर्णांक ul_pty_setup(काष्ठा ul_pty *pty)
{
	काष्ठा termios attrs;
	पूर्णांक rc = 0;

	निश्चित(pty->sigfd == -1);

	/* save the current signals setting (to make ul_pty_cleanup() usable,
	 * otherwise see ul_pty_signals_setup() */
	sigprocmask(0, शून्य, &pty->orgsig);

	अगर (pty->isterm) {
		DBG_OBJ(SETUP, pty, ul_debug("create for terminal"));

		/* original setting of the current terminal */
		अगर (tcgetattr(STDIN_FILENO, &pty->stdin_attrs) != 0) {
			rc = -त्रुटि_सं;
			जाओ done;
		}

		attrs = pty->stdin_attrs;
		अगर (pty->slave_echo)
			attrs.c_lflag |= ECHO;
		अन्यथा
			attrs.c_lflag &= ~ECHO;

		ioctl(STDIN_FILENO, TIOCGWINSZ, (अक्षर *)&pty->win);
		/* create master+slave */
		rc = openpty(&pty->master, &pty->slave, शून्य, &attrs, &pty->win);
		अगर (rc)
			जाओ done;

	} अन्यथा {
		DBG_OBJ(SETUP, pty, ul_debug("create for non-terminal"));

		rc = openpty(&pty->master, &pty->slave, शून्य, शून्य, शून्य);
		अगर (rc)
			जाओ done;

		tcgetattr(pty->slave, &attrs);

		अगर (pty->slave_echo)
			attrs.c_lflag |= ECHO;
		अन्यथा
			attrs.c_lflag &= ~ECHO;

		tcsetattr(pty->slave, TCSANOW, &attrs);
	}

	fcntl(pty->master, F_SETFL, O_NONBLOCK);

done:
	अगर (rc)
		ul_pty_cleanup(pty);

	DBG_OBJ(SETUP, pty, ul_debug("pty setup done [master=%d, slave=%d, rc=%d]",
				pty->master, pty->slave, rc));
	वापस rc;
}

/* call me after ul_pty_signals_setup() and before fork() */
पूर्णांक ul_pty_terminal_setup(काष्ठा ul_pty *pty)
{
	काष्ठा termios attrs;

	अगर (!pty->isterm)
		वापस 0;

	attrs = pty->stdin_attrs;
	cfmakeraw(&attrs);
	अगर (tcsetattr(STDIN_FILENO, TCSANOW, &attrs))
		वापस -त्रुटि_सं;

	DBG_OBJ(SETUP, pty, ul_debug("terminal raw mode setup done"));
	वापस 0;
}

/* call me before fork() */
पूर्णांक ul_pty_signals_setup(काष्ठा ul_pty *pty)
{
	sigset_t ourset;
	पूर्णांक rc = 0;

	निश्चित(pty->sigfd == -1);

	/* save the current signals setting */
	sigprocmask(0, शून्य, &pty->orgsig);

	sigfillset(&ourset);
	अगर (sigprocmask(SIG_BLOCK, &ourset, शून्य)) {
		rc = -त्रुटि_सं;
		जाओ done;
	}

	sigemptyset(&ourset);
	sigaddset(&ourset, SIGCHLD);
	sigaddset(&ourset, SIGWINCH);
	sigaddset(&ourset, SIGALRM);
	sigaddset(&ourset, SIGHUP);
	sigaddset(&ourset, संक_इति);
	sigaddset(&ourset, संक_विघ्न);
	sigaddset(&ourset, SIGQUIT);

	अगर (pty->callbacks.flush_logs)
		sigaddset(&ourset, SIGUSR1);

	अगर ((pty->sigfd = signalfd(-1, &ourset, SFD_CLOEXEC)) < 0)
		rc = -त्रुटि_सं;
done:
	अगर (rc)
		ul_pty_cleanup(pty);

	DBG_OBJ(SETUP, pty, ul_debug("pty signals setup done [rc=%d]", rc));
	वापस rc;
}

/* cleanup in parent process */
व्योम ul_pty_cleanup(काष्ठा ul_pty *pty)
{
	काष्ठा termios rtt;

	pty_signals_cleanup(pty);

	अगर (pty->master == -1 || !pty->isterm)
		वापस;

	DBG_OBJ(DONE, pty, ul_debug("cleanup"));
	rtt = pty->stdin_attrs;
	tcsetattr(STDIN_FILENO, TCSADRAIN, &rtt);
}

पूर्णांक ul_pty_chownmod_slave(काष्ठा ul_pty *pty, uid_t uid, gid_t gid, mode_t mode)
{
	अगर (fchown(pty->slave, uid, gid))
		वापस -त्रुटि_सं;
	अगर (fchmod(pty->slave, mode))
		वापस -त्रुटि_सं;
	वापस 0;
}

/* call me in child process */
व्योम ul_pty_init_slave(काष्ठा ul_pty *pty)
{
	DBG_OBJ(SETUP, pty, ul_debug("initialize slave"));

	setsid();

	ioctl(pty->slave, TIOCSCTTY, 1);
	बंद(pty->master);

	dup2(pty->slave, STDIN_FILENO);
	dup2(pty->slave, STDOUT_FILENO);
	dup2(pty->slave, STDERR_FILENO);

	बंद(pty->slave);

	अगर (pty->sigfd >= 0)
		बंद(pty->sigfd);

	pty->slave = -1;
	pty->master = -1;
	pty->sigfd = -1;

	sigprocmask(SIG_SETMASK, &pty->orgsig, शून्य);

	DBG_OBJ(SETUP, pty, ul_debug("... initialize slave done"));
}

अटल पूर्णांक write_output(अक्षर *obuf, ssize_t bytes)
{
	DBG(IO, ul_debug(" writing output"));

	अगर (ul_write_all(STDOUT_FILENO, obuf, bytes)) {
		DBG(IO, ul_debug("  writing output *failed*"));
		वापस -त्रुटि_सं;
	}

	वापस 0;
}

अटल पूर्णांक schedule_child_write(काष्ठा ul_pty *pty, अक्षर *buf, माप_प्रकार bufsz, पूर्णांक final)
{
	काष्ठा ul_pty_child_बफर *stash;

	अगर (pty->free_buffers) {
		stash = pty->free_buffers;
		pty->free_buffers = stash->अग्र;
		स_रखो(stash, 0, माप(*stash));
	} अन्यथा
		stash = सुस्मृति(1, माप(*stash));
	अगर (!stash)
		वापस -1;

	निश्चित(bufsz <= माप(stash->buf));

	स_नकल(stash->buf, buf, bufsz);
	stash->आकार = bufsz;
	stash->final_निवेश = final ? 1 : 0;

	अगर (pty->child_बफर_शीर्ष)
		pty->child_बफर_पुच्छ = pty->child_बफर_पुच्छ->अग्र = stash;
	अन्यथा
		pty->child_बफर_शीर्ष = pty->child_बफर_पुच्छ = stash;
	वापस 0;
}

/*
 * The pty is usually faster than shell, so it's a good idea to wait until
 * the previous message has been already read by shell from slave before we
 * write to master. This is necessary especially for EOF situation when we can
 * send EOF to master before shell is fully initialized, to workaround this
 * problem we wait until slave is empty. For example:
 *
 *   echo "date" | su --pty
 *
 * Unfortunately, the child (usually shell) can ignore stdin at all, so we
 * don't wait forever to avoid dead locks...
 *
 * Note that su --pty is primarily designed for interactive sessions as it
 * maintains master+slave tty stuff within the session. Use pipe to write to
 * pty and assume non-interactive (tee-like) behavior is NOT well supported.
 */
अटल व्योम drain_child_buffers(काष्ठा ul_pty *pty)
{
	अचिन्हित पूर्णांक tries = 0;
	काष्ठा pollfd fd = { .fd = pty->slave, .events = POLLIN };

	DBG_OBJ(IO, pty, ul_debug(" waiting for empty slave"));
	जबतक (poll(&fd, 1, 10) == 1 && tries < 8) {
		DBG_OBJ(IO, pty, ul_debug("   slave is not empty"));
		xusleep(250000);
		tries++;
	}
	अगर (tries < 8)
		DBG_OBJ(IO, pty, ul_debug("   slave is empty now"));

	DBG_OBJ(IO, pty, ul_debug(" sending EOF to master"));
}

अटल पूर्णांक flush_child_buffers(काष्ठा ul_pty *pty, पूर्णांक *anything)
{
	पूर्णांक rc = 0, any = 0;

	जबतक (pty->child_बफर_शीर्ष) {
		काष्ठा ul_pty_child_बफर *hd = pty->child_बफर_शीर्ष;
		ssize_t ret;

		अगर (hd->final_निवेश)
			drain_child_buffers(pty);

		DBG_OBJ(IO, hd, ul_debug(" stdin --> master trying %zu bytes", hd->आकार - hd->cursor));

		ret = ग_लिखो(pty->master, hd->buf + hd->cursor, hd->आकार - hd->cursor);
		अगर (ret == -1) {
			DBG_OBJ(IO, hd, ul_debug("   EAGAIN"));
			अगर (!(त्रुटि_सं == EINTR || त्रुटि_सं == EAGAIN))
				rc = -त्रुटि_सं;
			जाओ out;
		}
		DBG_OBJ(IO, hd, ul_debug("   wrote %zd", ret));
		any = 1;
		hd->cursor += ret;

		अगर (hd->cursor == hd->आकार) {
			pty->child_बफर_शीर्ष = hd->अग्र;
			अगर (!hd->अग्र)
				pty->child_बफर_पुच्छ = शून्य;

			hd->अग्र = pty->free_buffers;
			pty->free_buffers = hd;
		}
	}
out:
	/* without sync write_output() will write both input &
	 * shell output that looks like double echoing */
	अगर (any)
		fdatasync(pty->master);

	अगर (anything)
		*anything = any;
	वापस rc;
}

व्योम ul_pty_write_eof_to_child(काष्ठा ul_pty *pty)
{
	अक्षर c = DEF_EOF;
	schedule_child_write(pty, &c, माप(अक्षर), 1);
}

अटल पूर्णांक mainloop_callback(काष्ठा ul_pty *pty)
{
	पूर्णांक rc;

	अगर (!pty->callbacks.mainloop)
		वापस 0;

	DBG_OBJ(IO, pty, ul_debug("calling mainloop callback"));
	rc = pty->callbacks.mainloop(pty->callback_तथ्य);

	DBG_OBJ(IO, pty, ul_debug(" callback done [rc=%d]", rc));
	वापस rc;
}

अटल पूर्णांक handle_io(काष्ठा ul_pty *pty, पूर्णांक fd, पूर्णांक *eof)
{
	अक्षर buf[बफ_मान];
	ssize_t bytes;
	पूर्णांक rc = 0;
	sigset_t set;

	DBG_OBJ(IO, pty, ul_debug(" handle I/O on fd=%d", fd));
	*eof = 0;

	sigemptyset(&set);
	sigaddset(&set, SIGTTIN);
	sigprocmask(SIG_UNBLOCK, &set, शून्य);
	/* read from active FD */
	bytes = पढ़ो(fd, buf, माप(buf));
	sigprocmask(SIG_BLOCK, &set, शून्य);
	अगर (bytes == -1) {
		अगर (त्रुटि_सं == EAGAIN || त्रुटि_सं == EINTR)
			वापस 0;
		वापस -त्रुटि_सं;
	}

	अगर (bytes == 0) {
		*eof = 1;
		वापस 0;
	}

	/* from stdin (user) to command */
	अगर (fd == STDIN_FILENO) {
		DBG_OBJ(IO, pty, ul_debug(" stdin --> master %zd bytes queued", bytes));

		अगर (schedule_child_write(pty, buf, bytes, 0))
			वापस -त्रुटि_सं;

	/* from command (master) to stdout */
	} अन्यथा अगर (fd == pty->master) {
		DBG_OBJ(IO, pty, ul_debug(" master --> stdout %zd bytes", bytes));
		write_output(buf, bytes);
	}

	अगर (pty->callbacks.log_stream_activity)
		rc = pty->callbacks.log_stream_activity(
					pty->callback_तथ्य, fd, buf, bytes);

	वापस rc;
}

व्योम ul_pty_wait_for_child(काष्ठा ul_pty *pty)
{
	पूर्णांक status;
	pid_t pid;
	पूर्णांक options = 0;

	अगर (pty->child == (pid_t) -1)
		वापस;

	DBG(SIG, ul_debug("waiting for child [child=%d]", (पूर्णांक) pty->child));

	अगर (ul_pty_is_running(pty)) {
		/* wait for specific child */
		options = WNOHANG;
		क्रम (;;) {
			pid = waitpid(pty->child, &status, options);
			DBG(SIG, ul_debug(" waitpid done [rc=%d]", (पूर्णांक) pid));
			अगर (pid != (pid_t) - 1) {
				अगर (pty->callbacks.child_die)
					pty->callbacks.child_die(
							pty->callback_तथ्य,
							pty->child, status);
				ul_pty_set_child(pty, (pid_t) -1);
			} अन्यथा
				अवरोध;
		}
	} अन्यथा {
		/* final wait */
		जबतक ((pid = waitpid(-1, &status, options)) > 0) {
			DBG(SIG, ul_debug(" waitpid done [rc=%d]", (पूर्णांक) pid));
			अगर (pid == pty->child) {
				अगर (pty->callbacks.child_die)
					pty->callbacks.child_die(
							pty->callback_तथ्य,
							pty->child, status);
				ul_pty_set_child(pty, (pid_t) -1);
			}
		}
	}
}

अटल पूर्णांक handle_signal(काष्ठा ul_pty *pty, पूर्णांक fd)
{
	काष्ठा signalfd_siginfo info;
	ssize_t bytes;
	पूर्णांक rc = 0;

	DBG_OBJ(SIG, pty, ul_debug(" handle signal on fd=%d", fd));

	bytes = पढ़ो(fd, &info, माप(info));
	अगर (bytes != माप(info)) {
		अगर (bytes < 0 && (त्रुटि_सं == EAGAIN || त्रुटि_सं == EINTR))
			वापस 0;
		वापस -त्रुटि_सं;
	}

	चयन (info.ssi_signo) {
	हाल SIGCHLD:
		DBG_OBJ(SIG, pty, ul_debug(" get signal SIGCHLD"));

		अगर (info.ssi_code == CLD_EXITED
		    || info.ssi_code == CLD_KILLED
		    || info.ssi_code == CLD_DUMPED) {

			अगर (pty->callbacks.child_wait)
				pty->callbacks.child_wait(pty->callback_तथ्य,
							  pty->child);
			अन्यथा
				ul_pty_wait_for_child(pty);

		} अन्यथा अगर (info.ssi_status == SIGSTOP && pty->child > 0) {
			pty->callbacks.child_sigstop(pty->callback_तथ्य,
						     pty->child);
		}

		अगर (pty->child <= 0) {
			DBG_OBJ(SIG, pty, ul_debug(" no child, setting leaving timeout"));
			pty->poll_timeout = 10;
			timerclear(&pty->अग्र_callback_time);
		}
		वापस 0;
	हाल SIGWINCH:
		DBG_OBJ(SIG, pty, ul_debug(" get signal SIGWINCH"));
		अगर (pty->isterm) {
			ioctl(STDIN_FILENO, TIOCGWINSZ, (अक्षर *)&pty->win);
			ioctl(pty->slave, TIOCSWINSZ, (अक्षर *)&pty->win);

			अगर (pty->callbacks.log_signal)
				rc = pty->callbacks.log_signal(pty->callback_तथ्य,
							&info, (व्योम *) &pty->win);
		}
		अवरोध;
	हाल SIGALRM:
		FALLTHROUGH;
	हाल SIGHUP:
		FALLTHROUGH;
	हाल संक_इति:
		FALLTHROUGH;
	हाल संक_विघ्न:
		FALLTHROUGH;
	हाल SIGQUIT:
		DBG_OBJ(SIG, pty, ul_debug(" get signal SIG{ALRM,HUP,TERM,INT,QUIT}"));
		pty->delivered_signal = info.ssi_signo;
		/* Child termination is going to generate SIGCHLD (see above) */
		अगर (pty->child > 0)
			समाप्त(pty->child, संक_इति);

		अगर (pty->callbacks.log_signal)
			rc = pty->callbacks.log_signal(pty->callback_तथ्य,
					&info, (व्योम *) &pty->win);
		अवरोध;
	हाल SIGUSR1:
		DBG_OBJ(SIG, pty, ul_debug(" get signal SIGUSR1"));
		अगर (pty->callbacks.flush_logs)
			rc = pty->callbacks.flush_logs(pty->callback_तथ्य);
		अवरोध;
	शेष:
		पात();
	}

	वापस rc;
}

/* loop in parent */
पूर्णांक ul_pty_proxy_master(काष्ठा ul_pty *pty)
{
	पूर्णांक rc = 0, ret, eof = 0;
	क्रमागत {
		POLLFD_SIGNAL = 0,
		POLLFD_MASTER,
		POLLFD_STDIN

	};
	काष्ठा pollfd pfd[] = {
		[POLLFD_SIGNAL] = { .fd = -1,		.events = POLLIN | POLLERR | POLLHUP },
		[POLLFD_MASTER] = { .fd = pty->master,  .events = POLLIN | POLLERR | POLLHUP },
		[POLLFD_STDIN]	= { .fd = STDIN_FILENO, .events = POLLIN | POLLERR | POLLHUP }
	};

	/* We use signalfd, and standard signals by handlers are completely blocked */
	निश्चित(pty->sigfd >= 0);

	pfd[POLLFD_SIGNAL].fd = pty->sigfd;
	pty->poll_timeout = -1;

	जबतक (!pty->delivered_signal) {
		माप_प्रकार i;
		पूर्णांक errsv, timeout;

		DBG_OBJ(IO, pty, ul_debug("--poll() loop--"));

		/* note, callback usually updates @next_callback_time */
		अगर (timerisset(&pty->अग्र_callback_time)) {
			काष्ठा timeval now = { 0 };

			DBG_OBJ(IO, pty, ul_debug(" callback requested"));
			gettime_monotonic(&now);
			अगर (timercmp(&now, &pty->अग्र_callback_time, >)) {
				rc = mainloop_callback(pty);
				अगर (rc)
					अवरोध;
			}
		}

		/* set timeout */
		अगर (timerisset(&pty->अग्र_callback_time)) {
			काष्ठा timeval now = { 0 }, rest = { 0 };

			gettime_monotonic(&now);
			timersub(&pty->अग्र_callback_time, &now, &rest);
			timeout = (rest.tv_sec * MSEC_PER_SEC) +  (rest.tv_usec / USEC_PER_MSEC);
		} अन्यथा
			timeout = pty->poll_timeout;

		/* use POLLOUT (aka "writing is now possible") if data queued */
		अगर (pty->child_बफर_शीर्ष)
			pfd[POLLFD_MASTER].events |= POLLOUT;
		अन्यथा
			pfd[POLLFD_MASTER].events &= ~POLLOUT;

		/* wait for input, signal or timeout */
		DBG_OBJ(IO, pty, ul_debug("calling poll() [timeout=%dms]", timeout));
		ret = poll(pfd, ARRAY_आकार(pfd), timeout);

		errsv = त्रुटि_सं;
		DBG_OBJ(IO, pty, ul_debug("poll() rc=%d", ret));

		/* error */
		अगर (ret < 0) {
			अगर (errsv == EAGAIN)
				जारी;
			rc = -त्रुटि_सं;
			अवरोध;
		}

		/* timeout */
		अगर (ret == 0) {
			अगर (timerisset(&pty->अग्र_callback_time)) {
				rc = mainloop_callback(pty);
				अगर (rc == 0)
					जारी;
			} अन्यथा {
				rc = 0;
			}

			DBG_OBJ(IO, pty, ul_debug("leaving poll() loop [timeout=%d, rc=%d]", timeout, rc));
			अवरोध;
		}
		/* event */
		क्रम (i = 0; i < ARRAY_आकार(pfd); i++) {
			अगर (pfd[i].revents == 0)
				जारी;

			DBG_OBJ(IO, pty, ul_debug(" active pfd[%s].fd=%d %s %s %s %s %s",
						i == POLLFD_STDIN  ? "stdin" :
						i == POLLFD_MASTER ? "master" :
						i == POLLFD_SIGNAL ? "signal" : "???",
						pfd[i].fd,
						pfd[i].revents & POLLIN  ? "POLLIN" : "",
						pfd[i].revents & POLLOUT ? "POLLOUT" : "",
						pfd[i].revents & POLLHUP ? "POLLHUP" : "",
						pfd[i].revents & POLLERR ? "POLLERR" : "",
						pfd[i].revents & POLLNVAL ? "POLLNVAL" : ""));

			अगर (i == POLLFD_SIGNAL)
				rc = handle_signal(pty, pfd[i].fd);
			अन्यथा {
				अगर (pfd[i].revents & POLLIN)
					rc = handle_io(pty, pfd[i].fd, &eof); /*तथ्य*/
				अगर (pfd[i].revents & POLLOUT) /* i == POLLFD_MASTER */
					rc = flush_child_buffers(pty, शून्य);
			}

			अगर (rc) {
				पूर्णांक anything = 1;

				ul_pty_write_eof_to_child(pty);
				क्रम (anything = 1; anything;)
					flush_child_buffers(pty, &anything);
				अवरोध;
			}

			अगर (i == POLLFD_SIGNAL)
				जारी;

			/* EOF maybe detected in two ways; they are as follows:
			 *	A) poll() return POLLHUP event after close()
			 *	B) read() returns 0 (no data)
			 *
			 * POLLNVAL means that fd is closed.
			 */
			अगर ((pfd[i].revents & POLLHUP) || (pfd[i].revents & POLLNVAL) || eof) {
				DBG_OBJ(IO, pty, ul_debug(" ignore FD"));
				अगर (i == POLLFD_STDIN) {
					pfd[i].fd = -1;
					ul_pty_write_eof_to_child(pty);
				} अन्यथा /* i == POLLFD_MASTER */
					pfd[i].revents &= ~POLLIN;
			}
		}
		अगर (rc)
			अवरोध;
	}

	अगर (rc && pty->child && pty->child != (pid_t) -1 && !pty->delivered_signal) {
		समाप्त(pty->child, संक_इति);
		sleep(2);
		समाप्त(pty->child, SIGKILL);
	}

	pty_signals_cleanup(pty);

	DBG(IO, ul_debug("poll() done [signal=%d, rc=%d]", pty->delivered_signal, rc));
	वापस rc;
}

#अगर_घोषित TEST_PROGRAM_PTY
/*
 * $ make test_pty
 * $ ./test_pty
 *
 * ... and see for example tty(1) or "ps afu"
 */
अटल व्योम child_sigstop(व्योम *तथ्य __attribute__((__unused__)), pid_t child)
{
	समाप्त(getpid(), SIGSTOP);
	समाप्त(child, SIGCONT);
}

पूर्णांक मुख्य(पूर्णांक argc, अक्षर *argv[])
{
	काष्ठा ul_pty_callbacks *cb;
	स्थिर अक्षर *shell, *command = शून्य, *shname = शून्य;
	पूर्णांक caught_signal = 0;
	pid_t child;
	काष्ठा ul_pty *pty;

	shell = दो_पर्या("SHELL");
	अगर (shell == शून्य)
		shell = _PATH_BSHELL;
	अगर (argc == 2)
		command = argv[1];

	ul_pty_init_debug(0);

	pty = ul_new_pty(isatty(STDIN_FILENO));
	अगर (!pty)
		err(निकास_त्रुटि, "failed to allocate PTY handler");

	cb = ul_pty_get_callbacks(pty);
	cb->child_sigstop = child_sigstop;

	अगर (ul_pty_setup(pty))
		err(निकास_त्रुटि, "failed to create pseudo-terminal");
	अगर (ul_pty_signals_setup(pty))
		err(निकास_त्रुटि, "failed to initialize signals handler");
	अगर (ul_pty_terminal_setup(pty))
		err(निकास_त्रुटि, "failed to setup terminal");

	ख_साफ(मानक_निकास);			/* ??? */

	चयन ((पूर्णांक) (child = विभाजन())) {
	हाल -1: /* error */
		ul_pty_cleanup(pty);
		err(निकास_त्रुटि, "cannot create child process");
		अवरोध;

	हाल 0: /* child */
		ul_pty_init_slave(pty);

		संकेत(संक_इति, संक_पूर्व); /* because /etc/csh.login */

		shname = म_खोजप(shell, '/');
		shname = shname ? shname + 1 : shell;

		अगर (command)
			execl(shell, shname, "-c", command, (अक्षर *)शून्य);
		अन्यथा
			execl(shell, shname, "-i", (अक्षर *)शून्य);
		err(निकास_त्रुटि, "failed to execute %s", shell);
		अवरोध;

	शेष:
		अवरोध;
	}

	/* parent */
	ul_pty_set_child(pty, child);

	/* this is the main loop */
	ul_pty_proxy_master(pty);

	/* all done; cleanup and kill */
	caught_signal = ul_pty_get_delivered_signal(pty);

	अगर (!caught_signal && ul_pty_get_child(pty) != (pid_t)-1)
		ul_pty_wait_for_child(pty);	/* final wait */

	अगर (caught_signal && ul_pty_get_child(pty) != (pid_t)-1) {
		ख_लिखो(मानक_त्रुटि, "\nSession terminated, killing shell...");
		समाप्त(child, संक_इति);
		sleep(2);
		समाप्त(child, SIGKILL);
		ख_लिखो(मानक_त्रुटि, " ...killed.\n");
	}

	ul_pty_cleanup(pty);
	ul_free_pty(pty);
	वापस निकास_सफल;
}

#पूर्ण_अगर /* TEST_PROGRAM */

