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
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <pty.h>
#अन्तर्भाव <poll.h>
#अन्तर्भाव <sys/signalfd.h>
#अन्तर्भाव <paths.h>
#अन्तर्भाव <sys/types.h>
#अन्तर्भाव <sys/wait.h>
#अन्तर्भाव <inttypes.h>

#अन्तर्भाव "c.h"
#अन्तर्भाव "all-io.h"
#अन्तर्भाव "ttyutils.h"
#अन्तर्भाव "pty-session.h"
#अन्तर्भाव "monotonic.h"
#अन्तर्भाव "debug.h"

स्थिर UL_DEBUG_DEFINE_MASK(ulpty);
UL_DEBUG_DEFINE_MASKNAMES(ulpty) = UL_DEBUG_EMPTY_MASKNAMES;

#define ULPTY_DEBUG_INIT	(1 << 1)
#define ULPTY_DEBUG_SETUP	(1 << 2)
#define ULPTY_DEBUG_SIG		(1 << 3)
#define ULPTY_DEBUG_IO		(1 << 4)
#define ULPTY_DEBUG_DONE	(1 << 5)

#define DBG(m, x)		__UL_DBG(ulpty, ULPTY_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulpty, ULPTY_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(ulpty, ULPTY_DEBUG_, m, x)

शून्य ul_pty_init_debug(पूर्णांक mask)
{
	जर (ulpty_debug_mask)
		परत;
	__UL_INIT_DEBUG_FROM_ENV(ulpty, ULPTY_DEBUG_, mask, ULPTY_DEBUG);
}

संरचना ul_pty *ul_new_pty(पूर्णांक is_stdin_tty)
{
	संरचना ul_pty *pty = सुस्मृति(1, आकार(*pty));

	जर (!pty)
		परत रिक्त;

	DBG_OBJ(SETUP, pty, ul_debug("alloc handler"));
	pty->isterm = is_stdin_tty;
	pty->master = -1;
	pty->slave = -1;
	pty->sigfd = -1;
	pty->child = (pid_t) -1;

	परत pty;
}

शून्य ul_free_pty(संरचना ul_pty *pty)
{
	संरचना ul_pty_child_buffer *hd;

	पर्यंत ((hd = pty->child_buffer_head)) {
		pty->child_buffer_head = hd->next;
		मुक्त(hd);
	}

	पर्यंत ((hd = pty->free_buffers)) {
		pty->free_buffers = hd->next;
		मुक्त(hd);
	}
	मुक्त(pty);
}

शून्य ul_pty_slave_echo(संरचना ul_pty *pty, पूर्णांक enable)
{
	प्रतिज्ञा(pty);
	pty->slave_echo = enable ? 1 : 0;
}

पूर्णांक ul_pty_get_delivered_signal(संरचना ul_pty *pty)
{
	प्रतिज्ञा(pty);
	परत pty->delivered_signal;
}

संरचना ul_pty_callbacks *ul_pty_get_callbacks(संरचना ul_pty *pty)
{
	प्रतिज्ञा(pty);
	परत &pty->callbacks;
}

शून्य ul_pty_set_callback_data(संरचना ul_pty *pty, शून्य *data)
{
	प्रतिज्ञा(pty);
	pty->callback_data = data;
}

शून्य ul_pty_set_child(संरचना ul_pty *pty, pid_t child)
{
	प्रतिज्ञा(pty);
	pty->child = child;
}

पूर्णांक ul_pty_get_childfd(संरचना ul_pty *pty)
{
	प्रतिज्ञा(pty);
	परत pty->master;
}

pid_t ul_pty_get_child(संरचना ul_pty *pty)
{
	प्रतिज्ञा(pty);
	परत pty->child;
}

/* it's active when signals are redirected to sigfd */
पूर्णांक ul_pty_is_running(संरचना ul_pty *pty)
{
	प्रतिज्ञा(pty);
	परत pty->sigfd >= 0;
}

शून्य ul_pty_set_mainloop_time(संरचना ul_pty *pty, संरचना timeval *tv)
{
	प्रतिज्ञा(pty);
	जर (!tv) {
		DBG_OBJ(IO, pty, ul_debug("mainloop time: clear"));
		timerclear(&pty->next_callback_time);
	} अन्यथा {
		pty->next_callback_time.tv_sec = tv->tv_sec;
		pty->next_callback_time.tv_usec = tv->tv_usec;
		DBG_OBJ(IO, pty, ul_debug("mainloop time: %"PRId64".%06"PRId64,
				(int64_t) tv->tv_sec, (int64_t) tv->tv_usec));
	}
}

स्थिर शून्य pty_signals_cleanup(संरचना ul_pty *pty)
{
	जर (pty->sigfd != -1)
		बंद(pty->sigfd);
	pty->sigfd = -1;

	/* restore original setting */
	sigprocmask(SIG_SETMASK, &pty->orgsig, रिक्त);
}

/* call me before fork() */
पूर्णांक ul_pty_setup(संरचना ul_pty *pty)
{
	संरचना termios attrs;
	पूर्णांक rc = 0;

	प्रतिज्ञा(pty->sigfd == -1);

	/* save the current signals setting (to make ul_pty_cleanup() usable,
	 * otherwise see ul_pty_signals_setup() */
	sigprocmask(0, रिक्त, &pty->orgsig);

	जर (pty->isterm) {
		DBG_OBJ(SETUP, pty, ul_debug("create for terminal"));

		/* original setting of the current terminal */
		जर (tcgetattr(STDIN_FILENO, &pty->stdin_attrs) != 0) {
			rc = -त्रुटि_सं;
			जा done;
		}

		attrs = pty->stdin_attrs;
		जर (pty->slave_echo)
			attrs.c_lflag |= ECHO;
		अन्यथा
			attrs.c_lflag &= ~ECHO;

		ioctl(STDIN_FILENO, TIOCGWINSZ, (अक्षर *)&pty->win);
		/* create master+slave */
		rc = openpty(&pty->master, &pty->slave, रिक्त, &attrs, &pty->win);
		जर (rc)
			जा done;

	} अन्यथा {
		DBG_OBJ(SETUP, pty, ul_debug("create for non-terminal"));

		rc = openpty(&pty->master, &pty->slave, रिक्त, रिक्त, रिक्त);
		जर (rc)
			जा done;

		tcgetattr(pty->slave, &attrs);

		जर (pty->slave_echo)
			attrs.c_lflag |= ECHO;
		अन्यथा
			attrs.c_lflag &= ~ECHO;

		tcsetattr(pty->slave, TCSANOW, &attrs);
	}

	fcntl(pty->master, F_SETFL, O_NONBLOCK);

done:
	जर (rc)
		ul_pty_cleanup(pty);

	DBG_OBJ(SETUP, pty, ul_debug("pty setup done [master=%d, slave=%d, rc=%d]",
				pty->master, pty->slave, rc));
	परत rc;
}

/* call me after ul_pty_signals_setup() and before fork() */
पूर्णांक ul_pty_terminal_setup(संरचना ul_pty *pty)
{
	संरचना termios attrs;

	जर (!pty->isterm)
		परत 0;

	attrs = pty->stdin_attrs;
	cfmakeraw(&attrs);
	जर (tcsetattr(STDIN_FILENO, TCSANOW, &attrs))
		परत -त्रुटि_सं;

	DBG_OBJ(SETUP, pty, ul_debug("terminal raw mode setup done"));
	परत 0;
}

/* call me before fork() */
पूर्णांक ul_pty_signals_setup(संरचना ul_pty *pty)
{
	sigset_t ourset;
	पूर्णांक rc = 0;

	प्रतिज्ञा(pty->sigfd == -1);

	/* save the current signals setting */
	sigprocmask(0, रिक्त, &pty->orgsig);

	sigfillset(&ourset);
	जर (sigprocmask(SIG_BLOCK, &ourset, रिक्त)) {
		rc = -त्रुटि_सं;
		जा done;
	}

	sigemptyset(&ourset);
	sigaddset(&ourset, SIGCHLD);
	sigaddset(&ourset, SIGWINCH);
	sigaddset(&ourset, SIGALRM);
	sigaddset(&ourset, SIGHUP);
	sigaddset(&ourset, संक_इति);
	sigaddset(&ourset, संक_विघ्न);
	sigaddset(&ourset, SIGQUIT);

	जर (pty->callbacks.flush_logs)
		sigaddset(&ourset, SIGUSR1);

	जर ((pty->sigfd = signalfd(-1, &ourset, SFD_CLOEXEC)) < 0)
		rc = -त्रुटि_सं;
done:
	जर (rc)
		ul_pty_cleanup(pty);

	DBG_OBJ(SETUP, pty, ul_debug("pty signals setup done [rc=%d]", rc));
	परत rc;
}

/* cleanup in parent process */
शून्य ul_pty_cleanup(संरचना ul_pty *pty)
{
	संरचना termios rtt;

	pty_signals_cleanup(pty);

	जर (pty->master == -1 || !pty->isterm)
		परत;

	DBG_OBJ(DONE, pty, ul_debug("cleanup"));
	rtt = pty->stdin_attrs;
	tcsetattr(STDIN_FILENO, TCSADRAIN, &rtt);
}

पूर्णांक ul_pty_chownmod_slave(संरचना ul_pty *pty, uid_t uid, gid_t gid, mode_t mode)
{
	जर (fchown(pty->slave, uid, gid))
		परत -त्रुटि_सं;
	जर (fchmod(pty->slave, mode))
		परत -त्रुटि_सं;
	परत 0;
}

/* call me in child process */
शून्य ul_pty_init_slave(संरचना ul_pty *pty)
{
	DBG_OBJ(SETUP, pty, ul_debug("initialize slave"));

	setsid();

	ioctl(pty->slave, TIOCSCTTY, 1);
	बंद(pty->master);

	dup2(pty->slave, STDIN_FILENO);
	dup2(pty->slave, STDOUT_FILENO);
	dup2(pty->slave, STDERR_FILENO);

	बंद(pty->slave);

	जर (pty->sigfd >= 0)
		बंद(pty->sigfd);

	pty->slave = -1;
	pty->master = -1;
	pty->sigfd = -1;

	sigprocmask(SIG_SETMASK, &pty->orgsig, रिक्त);

	DBG_OBJ(SETUP, pty, ul_debug("... initialize slave done"));
}

स्थिर पूर्णांक write_output(अक्षर *obuf, ssize_t bytes)
{
	DBG(IO, ul_debug(" writing output"));

	जर (ul_write_all(STDOUT_FILENO, obuf, bytes)) {
		DBG(IO, ul_debug("  writing output *failed*"));
		परत -त्रुटि_सं;
	}

	परत 0;
}

स्थिर पूर्णांक schedule_child_write(संरचना ul_pty *pty, अक्षर *buf, माप_प्रकार bufsz, पूर्णांक final)
{
	संरचना ul_pty_child_buffer *stash;

	जर (pty->free_buffers) {
		stash = pty->free_buffers;
		pty->free_buffers = stash->next;
		स_रखो(stash, 0, आकार(*stash));
	} अन्यथा
		stash = सुस्मृति(1, आकार(*stash));
	जर (!stash)
		परत -1;

	प्रतिज्ञा(bufsz <= आकार(stash->buf));

	स_नकल(stash->buf, buf, bufsz);
	stash->size = bufsz;
	stash->final_आदान = final ? 1 : 0;

	जर (pty->child_buffer_head)
		pty->child_buffer_tail = pty->child_buffer_tail->next = stash;
	अन्यथा
		pty->child_buffer_head = pty->child_buffer_tail = stash;
	परत 0;
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
स्थिर शून्य drain_child_buffers(संरचना ul_pty *pty)
{
	अचिन्हित पूर्णांक tries = 0;
	संरचना pollfd fd = { .fd = pty->slave, .events = POLLIN };

	DBG_OBJ(IO, pty, ul_debug(" waiting for empty slave"));
	पर्यंत (poll(&fd, 1, 10) == 1 && tries < 8) {
		DBG_OBJ(IO, pty, ul_debug("   slave is not empty"));
		xusleep(250000);
		tries++;
	}
	जर (tries < 8)
		DBG_OBJ(IO, pty, ul_debug("   slave is empty now"));

	DBG_OBJ(IO, pty, ul_debug(" sending EOF to master"));
}

स्थिर पूर्णांक flush_child_buffers(संरचना ul_pty *pty, पूर्णांक *anything)
{
	पूर्णांक rc = 0, any = 0;

	पर्यंत (pty->child_buffer_head) {
		संरचना ul_pty_child_buffer *hd = pty->child_buffer_head;
		ssize_t ret;

		जर (hd->final_आदान)
			drain_child_buffers(pty);

		DBG_OBJ(IO, hd, ul_debug(" stdin --> master trying %zu bytes", hd->size - hd->cursor));

		ret = लिख(pty->master, hd->buf + hd->cursor, hd->size - hd->cursor);
		जर (ret == -1) {
			DBG_OBJ(IO, hd, ul_debug("   EAGAIN"));
			जर (!(त्रुटि_सं == EINTR || त्रुटि_सं == EAGAIN))
				rc = -त्रुटि_सं;
			जा out;
		}
		DBG_OBJ(IO, hd, ul_debug("   wrote %zd", ret));
		any = 1;
		hd->cursor += ret;

		जर (hd->cursor == hd->size) {
			pty->child_buffer_head = hd->next;
			जर (!hd->next)
				pty->child_buffer_tail = रिक्त;

			hd->next = pty->free_buffers;
			pty->free_buffers = hd;
		}
	}
out:
	/* without sync write_output() will write both input &
	 * shell output that looks like double echoing */
	जर (any)
		fdatasync(pty->master);

	जर (anything)
		*anything = any;
	परत rc;
}

शून्य ul_pty_write_eof_to_child(संरचना ul_pty *pty)
{
	अक्षर c = DEF_EOF;
	schedule_child_write(pty, &c, आकार(अक्षर), 1);
}

स्थिर पूर्णांक mainloop_callback(संरचना ul_pty *pty)
{
	पूर्णांक rc;

	जर (!pty->callbacks.mainloop)
		परत 0;

	DBG_OBJ(IO, pty, ul_debug("calling mainloop callback"));
	rc = pty->callbacks.mainloop(pty->callback_data);

	DBG_OBJ(IO, pty, ul_debug(" callback done [rc=%d]", rc));
	परत rc;
}

स्थिर पूर्णांक handle_io(संरचना ul_pty *pty, पूर्णांक fd, पूर्णांक *eof)
{
	अक्षर buf[बफ_मान];
	ssize_t bytes;
	पूर्णांक rc = 0;
	sigset_t set;

	DBG_OBJ(IO, pty, ul_debug(" handle I/O on fd=%d", fd));
	*eof = 0;

	sigemptyset(&set);
	sigaddset(&set, SIGTTIN);
	sigprocmask(SIG_UNBLOCK, &set, रिक्त);
	/* read from active FD */
	bytes = पठ(fd, buf, आकार(buf));
	sigprocmask(SIG_BLOCK, &set, रिक्त);
	जर (bytes == -1) {
		जर (त्रुटि_सं == EAGAIN || त्रुटि_सं == EINTR)
			परत 0;
		परत -त्रुटि_सं;
	}

	जर (bytes == 0) {
		*eof = 1;
		परत 0;
	}

	/* from stdin (user) to command */
	जर (fd == STDIN_FILENO) {
		DBG_OBJ(IO, pty, ul_debug(" stdin --> master %zd bytes queued", bytes));

		जर (schedule_child_write(pty, buf, bytes, 0))
			परत -त्रुटि_सं;

	/* from command (master) to stdout */
	} अन्यथा जर (fd == pty->master) {
		DBG_OBJ(IO, pty, ul_debug(" master --> stdout %zd bytes", bytes));
		write_output(buf, bytes);
	}

	जर (pty->callbacks.log_stream_activity)
		rc = pty->callbacks.log_stream_activity(
					pty->callback_data, fd, buf, bytes);

	परत rc;
}

शून्य ul_pty_wait_for_child(संरचना ul_pty *pty)
{
	पूर्णांक status;
	pid_t pid;
	पूर्णांक options = 0;

	जर (pty->child == (pid_t) -1)
		परत;

	DBG(SIG, ul_debug("waiting for child [child=%d]", (पूर्णांक) pty->child));

	जर (ul_pty_is_running(pty)) {
		/* wait for specific child */
		options = WNOHANG;
		साठी (;;) {
			pid = waitpid(pty->child, &status, options);
			DBG(SIG, ul_debug(" waitpid done [rc=%d]", (पूर्णांक) pid));
			जर (pid != (pid_t) - 1) {
				जर (pty->callbacks.child_die)
					pty->callbacks.child_die(
							pty->callback_data,
							pty->child, status);
				ul_pty_set_child(pty, (pid_t) -1);
			} अन्यथा
				थांबा;
		}
	} अन्यथा {
		/* final wait */
		पर्यंत ((pid = waitpid(-1, &status, options)) > 0) {
			DBG(SIG, ul_debug(" waitpid done [rc=%d]", (पूर्णांक) pid));
			जर (pid == pty->child) {
				जर (pty->callbacks.child_die)
					pty->callbacks.child_die(
							pty->callback_data,
							pty->child, status);
				ul_pty_set_child(pty, (pid_t) -1);
			}
		}
	}
}

स्थिर पूर्णांक handle_signal(संरचना ul_pty *pty, पूर्णांक fd)
{
	संरचना signalfd_siginfo info;
	ssize_t bytes;
	पूर्णांक rc = 0;

	DBG_OBJ(SIG, pty, ul_debug(" handle signal on fd=%d", fd));

	bytes = पठ(fd, &info, आकार(info));
	जर (bytes != आकार(info)) {
		जर (bytes < 0 && (त्रुटि_सं == EAGAIN || त्रुटि_सं == EINTR))
			परत 0;
		परत -त्रुटि_सं;
	}

	निवड (info.ssi_signo) {
	स्थिती SIGCHLD:
		DBG_OBJ(SIG, pty, ul_debug(" get signal SIGCHLD"));

		जर (info.ssi_code == CLD_EXITED
		    || info.ssi_code == CLD_KILLED
		    || info.ssi_code == CLD_DUMPED) {

			जर (pty->callbacks.child_wait)
				pty->callbacks.child_wait(pty->callback_data,
							  pty->child);
			अन्यथा
				ul_pty_wait_for_child(pty);

		} अन्यथा जर (info.ssi_status == SIGSTOP && pty->child > 0) {
			pty->callbacks.child_sigstop(pty->callback_data,
						     pty->child);
		}

		जर (pty->child <= 0) {
			DBG_OBJ(SIG, pty, ul_debug(" no child, setting leaving timeout"));
			pty->poll_timeout = 10;
			timerclear(&pty->next_callback_time);
		}
		परत 0;
	स्थिती SIGWINCH:
		DBG_OBJ(SIG, pty, ul_debug(" get signal SIGWINCH"));
		जर (pty->isterm) {
			ioctl(STDIN_FILENO, TIOCGWINSZ, (अक्षर *)&pty->win);
			ioctl(pty->slave, TIOCSWINSZ, (अक्षर *)&pty->win);

			जर (pty->callbacks.log_signal)
				rc = pty->callbacks.log_signal(pty->callback_data,
							&info, (शून्य *) &pty->win);
		}
		थांबा;
	स्थिती SIGALRM:
		FALLTHROUGH;
	स्थिती SIGHUP:
		FALLTHROUGH;
	स्थिती संक_इति:
		FALLTHROUGH;
	स्थिती संक_विघ्न:
		FALLTHROUGH;
	स्थिती SIGQUIT:
		DBG_OBJ(SIG, pty, ul_debug(" get signal SIG{ALRM,HUP,TERM,INT,QUIT}"));
		pty->delivered_signal = info.ssi_signo;
		/* Child termination is going to generate SIGCHLD (see above) */
		जर (pty->child > 0)
			समाप्त(pty->child, संक_इति);

		जर (pty->callbacks.log_signal)
			rc = pty->callbacks.log_signal(pty->callback_data,
					&info, (शून्य *) &pty->win);
		थांबा;
	स्थिती SIGUSR1:
		DBG_OBJ(SIG, pty, ul_debug(" get signal SIGUSR1"));
		जर (pty->callbacks.flush_logs)
			rc = pty->callbacks.flush_logs(pty->callback_data);
		थांबा;
	पूर्वनिर्धारित:
		पात();
	}

	परत rc;
}

/* loop in parent */
पूर्णांक ul_pty_proxy_master(संरचना ul_pty *pty)
{
	पूर्णांक rc = 0, ret, eof = 0;
	गणना {
		POLLFD_SIGNAL = 0,
		POLLFD_MASTER,
		POLLFD_STDIN

	};
	संरचना pollfd pfd[] = {
		[POLLFD_SIGNAL] = { .fd = -1,		.events = POLLIN | POLLERR | POLLHUP },
		[POLLFD_MASTER] = { .fd = pty->master,  .events = POLLIN | POLLERR | POLLHUP },
		[POLLFD_STDIN]	= { .fd = STDIN_FILENO, .events = POLLIN | POLLERR | POLLHUP }
	};

	/* We use signalfd, and standard signals by handlers are completely blocked */
	प्रतिज्ञा(pty->sigfd >= 0);

	pfd[POLLFD_SIGNAL].fd = pty->sigfd;
	pty->poll_timeout = -1;

	पर्यंत (!pty->delivered_signal) {
		माप_प्रकार i;
		पूर्णांक errsv, timeout;

		DBG_OBJ(IO, pty, ul_debug("--poll() loop--"));

		/* note, callback usually updates @next_callback_time */
		जर (timerisset(&pty->next_callback_time)) {
			संरचना timeval now = { 0 };

			DBG_OBJ(IO, pty, ul_debug(" callback requested"));
			gettime_monotonic(&now);
			जर (timercmp(&now, &pty->next_callback_time, >)) {
				rc = mainloop_callback(pty);
				जर (rc)
					थांबा;
			}
		}

		/* set timeout */
		जर (timerisset(&pty->next_callback_time)) {
			संरचना timeval now = { 0 }, rest = { 0 };

			gettime_monotonic(&now);
			timersub(&pty->next_callback_time, &now, &rest);
			timeout = (rest.tv_sec * MSEC_PER_SEC) +  (rest.tv_usec / USEC_PER_MSEC);
		} अन्यथा
			timeout = pty->poll_timeout;

		/* use POLLOUT (aka "writing is now possible") if data queued */
		जर (pty->child_buffer_head)
			pfd[POLLFD_MASTER].events |= POLLOUT;
		अन्यथा
			pfd[POLLFD_MASTER].events &= ~POLLOUT;

		/* wait for input, signal or timeout */
		DBG_OBJ(IO, pty, ul_debug("calling poll() [timeout=%dms]", timeout));
		ret = poll(pfd, ARRAY_SIZE(pfd), timeout);

		errsv = त्रुटि_सं;
		DBG_OBJ(IO, pty, ul_debug("poll() rc=%d", ret));

		/* error */
		जर (ret < 0) {
			जर (errsv == EAGAIN)
				सुरूठेवा;
			rc = -त्रुटि_सं;
			थांबा;
		}

		/* timeout */
		जर (ret == 0) {
			जर (timerisset(&pty->next_callback_time)) {
				rc = mainloop_callback(pty);
				जर (rc == 0)
					सुरूठेवा;
			} अन्यथा {
				rc = 0;
			}

			DBG_OBJ(IO, pty, ul_debug("leaving poll() loop [timeout=%d, rc=%d]", timeout, rc));
			थांबा;
		}
		/* event */
		साठी (i = 0; i < ARRAY_SIZE(pfd); i++) {
			जर (pfd[i].revents == 0)
				सुरूठेवा;

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

			जर (i == POLLFD_SIGNAL)
				rc = handle_signal(pty, pfd[i].fd);
			अन्यथा {
				जर (pfd[i].revents & POLLIN)
					rc = handle_io(pty, pfd[i].fd, &eof); /* data */
				जर (pfd[i].revents & POLLOUT) /* i == POLLFD_MASTER */
					rc = flush_child_buffers(pty, रिक्त);
			}

			जर (rc) {
				पूर्णांक anything = 1;

				ul_pty_write_eof_to_child(pty);
				साठी (anything = 1; anything;)
					flush_child_buffers(pty, &anything);
				थांबा;
			}

			जर (i == POLLFD_SIGNAL)
				सुरूठेवा;

			/* EOF maybe detected in two ways; they are as follows:
			 *	A) poll() return POLLHUP event after close()
			 *	B) read() returns 0 (no data)
			 *
			 * POLLNVAL means that fd is closed.
			 */
			जर ((pfd[i].revents & POLLHUP) || (pfd[i].revents & POLLNVAL) || eof) {
				DBG_OBJ(IO, pty, ul_debug(" ignore FD"));
				जर (i == POLLFD_STDIN) {
					pfd[i].fd = -1;
					ul_pty_write_eof_to_child(pty);
				} अन्यथा /* i == POLLFD_MASTER */
					pfd[i].revents &= ~POLLIN;
			}
		}
		जर (rc)
			थांबा;
	}

	जर (rc && pty->child && pty->child != (pid_t) -1 && !pty->delivered_signal) {
		समाप्त(pty->child, संक_इति);
		sleep(2);
		समाप्त(pty->child, SIGKILL);
	}

	pty_signals_cleanup(pty);

	DBG(IO, ul_debug("poll() done [signal=%d, rc=%d]", pty->delivered_signal, rc));
	परत rc;
}

#ifdef TEST_PROGRAM_PTY
/*
 * $ make test_pty
 * $ ./test_pty
 *
 * ... and see for example tty(1) or "ps afu"
 */
स्थिर शून्य child_sigstop(शून्य *data __attribute__((__unused__)), pid_t child)
{
	समाप्त(getpid(), SIGSTOP);
	समाप्त(child, SIGCONT);
}

पूर्णांक main(पूर्णांक argc, अक्षर *argv[])
{
	संरचना ul_pty_callbacks *cb;
	स्थिरांक अक्षर *shell, *command = रिक्त, *shname = रिक्त;
	पूर्णांक caught_signal = 0;
	pid_t child;
	संरचना ul_pty *pty;

	shell = दो_पर्या("SHELL");
	जर (shell == रिक्त)
		shell = _PATH_BSHELL;
	जर (argc == 2)
		command = argv[1];

	ul_pty_init_debug(0);

	pty = ul_new_pty(isatty(STDIN_FILENO));
	जर (!pty)
		err(निकास_त्रुटि, "failed to allocate PTY handler");

	cb = ul_pty_get_callbacks(pty);
	cb->child_sigstop = child_sigstop;

	जर (ul_pty_setup(pty))
		err(निकास_त्रुटि, "failed to create pseudo-terminal");
	जर (ul_pty_signals_setup(pty))
		err(निकास_त्रुटि, "failed to initialize signals handler");
	जर (ul_pty_terminal_setup(pty))
		err(निकास_त्रुटि, "failed to setup terminal");

	ख_साफ(मानक_निकास);			/* ??? */

	निवड ((पूर्णांक) (child = विभाजन())) {
	स्थिती -1: /* error */
		ul_pty_cleanup(pty);
		err(निकास_त्रुटि, "cannot create child process");
		थांबा;

	स्थिती 0: /* child */
		ul_pty_init_slave(pty);

		संकेत(संक_इति, संक_पूर्व); /* because /etc/csh.login */

		shname = म_खोजप(shell, '/');
		shname = shname ? shname + 1 : shell;

		जर (command)
			execl(shell, shname, "-c", command, (अक्षर *)रिक्त);
		अन्यथा
			execl(shell, shname, "-i", (अक्षर *)रिक्त);
		err(निकास_त्रुटि, "failed to execute %s", shell);
		थांबा;

	पूर्वनिर्धारित:
		थांबा;
	}

	/* parent */
	ul_pty_set_child(pty, child);

	/* this is the main loop */
	ul_pty_proxy_master(pty);

	/* all done; cleanup and kill */
	caught_signal = ul_pty_get_delivered_signal(pty);

	जर (!caught_signal && ul_pty_get_child(pty) != (pid_t)-1)
		ul_pty_wait_for_child(pty);	/* final wait */

	जर (caught_signal && ul_pty_get_child(pty) != (pid_t)-1) {
		ख_लिखो(मानक_त्रुटि, "\nSession terminated, killing shell...");
		समाप्त(child, संक_इति);
		sleep(2);
		समाप्त(child, SIGKILL);
		ख_लिखो(मानक_त्रुटि, " ...killed.\n");
	}

	ul_pty_cleanup(pty);
	ul_free_pty(pty);
	परत निकास_सफल;
}

#endif /* TEST_PROGRAM */

