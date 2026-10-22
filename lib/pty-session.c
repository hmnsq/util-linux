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

शून्य ul_pty_init_debug(पूर्णाङ्क mask)
{
	यदि (ulpty_debug_mask)
		फर्काऊ;
	__UL_INIT_DEBUG_FROM_ENV(ulpty, ULPTY_DEBUG_, mask, ULPTY_DEBUG);
}

संरचना ul_pty *ul_new_pty(पूर्णाङ्क is_stdin_tty)
{
	संरचना ul_pty *pty = सुस्मृति(1, परिमाण(*pty));

	यदि (!pty)
		फर्काऊ रिक्त;

	DBG_OBJ(SETUP, pty, ul_debug("alloc handler"));
	pty->isterm = is_stdin_tty;
	pty->master = -1;
	pty->slave = -1;
	pty->sigfd = -1;
	pty->child = (pid_t) -1;

	फर्काऊ pty;
}

शून्य ul_free_pty(संरचना ul_pty *pty)
{
	संरचना ul_pty_child_buffer *hd;

	जबसम्म ((hd = pty->child_buffer_head)) {
		pty->child_buffer_head = hd->next;
		मुक्त(hd);
	}

	जबसम्म ((hd = pty->free_buffers)) {
		pty->free_buffers = hd->next;
		मुक्त(hd);
	}
	मुक्त(pty);
}

शून्य ul_pty_slave_echo(संरचना ul_pty *pty, पूर्णाङ्क enable)
{
	प्रतिज्ञा(pty);
	pty->slave_echo = enable ? 1 : 0;
}

पूर्णाङ्क ul_pty_get_delivered_signal(संरचना ul_pty *pty)
{
	प्रतिज्ञा(pty);
	फर्काऊ pty->delivered_signal;
}

संरचना ul_pty_callbacks *ul_pty_get_callbacks(संरचना ul_pty *pty)
{
	प्रतिज्ञा(pty);
	फर्काऊ &pty->callbacks;
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

पूर्णाङ्क ul_pty_get_childfd(संरचना ul_pty *pty)
{
	प्रतिज्ञा(pty);
	फर्काऊ pty->master;
}

pid_t ul_pty_get_child(संरचना ul_pty *pty)
{
	प्रतिज्ञा(pty);
	फर्काऊ pty->child;
}

/* it's active when signals are redirected to sigfd */
पूर्णाङ्क ul_pty_is_running(संरचना ul_pty *pty)
{
	प्रतिज्ञा(pty);
	फर्काऊ pty->sigfd >= 0;
}

शून्य ul_pty_set_mainloop_time(संरचना ul_pty *pty, संरचना timeval *tv)
{
	प्रतिज्ञा(pty);
	यदि (!tv) {
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
	यदि (pty->sigfd != -1)
		बंद(pty->sigfd);
	pty->sigfd = -1;

	/* restore original setting */
	sigprocmask(SIG_SETMASK, &pty->orgsig, रिक्त);
}

/* call me before fork() */
पूर्णाङ्क ul_pty_setup(संरचना ul_pty *pty)
{
	संरचना termios attrs;
	पूर्णाङ्क rc = 0;

	प्रतिज्ञा(pty->sigfd == -1);

	/* save the current signals setting (to make ul_pty_cleanup() usable,
	 * otherwise see ul_pty_signals_setup() */
	sigprocmask(0, रिक्त, &pty->orgsig);

	यदि (pty->isterm) {
		DBG_OBJ(SETUP, pty, ul_debug("create for terminal"));

		/* original setting of the current terminal */
		यदि (tcgetattr(STDIN_FILENO, &pty->stdin_attrs) != 0) {
			rc = -त्रुटि_सं;
			गच्छ done;
		}

		attrs = pty->stdin_attrs;
		यदि (pty->slave_echo)
			attrs.c_lflag |= ECHO;
		अन्यथा
			attrs.c_lflag &= ~ECHO;

		ioctl(STDIN_FILENO, TIOCGWINSZ, (वर्ण *)&pty->win);
		/* create master+slave */
		rc = openpty(&pty->master, &pty->slave, रिक्त, &attrs, &pty->win);
		यदि (rc)
			गच्छ done;

	} अन्यथा {
		DBG_OBJ(SETUP, pty, ul_debug("create for non-terminal"));

		rc = openpty(&pty->master, &pty->slave, रिक्त, रिक्त, रिक्त);
		यदि (rc)
			गच्छ done;

		tcgetattr(pty->slave, &attrs);

		यदि (pty->slave_echo)
			attrs.c_lflag |= ECHO;
		अन्यथा
			attrs.c_lflag &= ~ECHO;

		tcsetattr(pty->slave, TCSANOW, &attrs);
	}

	fcntl(pty->master, F_SETFL, O_NONBLOCK);

done:
	यदि (rc)
		ul_pty_cleanup(pty);

	DBG_OBJ(SETUP, pty, ul_debug("pty setup done [master=%d, slave=%d, rc=%d]",
				pty->master, pty->slave, rc));
	फर्काऊ rc;
}

/* call me after ul_pty_signals_setup() and before fork() */
पूर्णाङ्क ul_pty_terminal_setup(संरचना ul_pty *pty)
{
	संरचना termios attrs;

	यदि (!pty->isterm)
		फर्काऊ 0;

	attrs = pty->stdin_attrs;
	cfmakeraw(&attrs);
	यदि (tcsetattr(STDIN_FILENO, TCSANOW, &attrs))
		फर्काऊ -त्रुटि_सं;

	DBG_OBJ(SETUP, pty, ul_debug("terminal raw mode setup done"));
	फर्काऊ 0;
}

/* call me before fork() */
पूर्णाङ्क ul_pty_signals_setup(संरचना ul_pty *pty)
{
	sigset_t ourset;
	पूर्णाङ्क rc = 0;

	प्रतिज्ञा(pty->sigfd == -1);

	/* save the current signals setting */
	sigprocmask(0, रिक्त, &pty->orgsig);

	sigfillset(&ourset);
	यदि (sigprocmask(SIG_BLOCK, &ourset, रिक्त)) {
		rc = -त्रुटि_सं;
		गच्छ done;
	}

	sigemptyset(&ourset);
	sigaddset(&ourset, SIGCHLD);
	sigaddset(&ourset, SIGWINCH);
	sigaddset(&ourset, SIGALRM);
	sigaddset(&ourset, SIGHUP);
	sigaddset(&ourset, संक_इति);
	sigaddset(&ourset, संक_विघ्न);
	sigaddset(&ourset, SIGQUIT);

	यदि (pty->callbacks.flush_logs)
		sigaddset(&ourset, SIGUSR1);

	यदि ((pty->sigfd = signalfd(-1, &ourset, SFD_CLOEXEC)) < 0)
		rc = -त्रुटि_सं;
done:
	यदि (rc)
		ul_pty_cleanup(pty);

	DBG_OBJ(SETUP, pty, ul_debug("pty signals setup done [rc=%d]", rc));
	फर्काऊ rc;
}

/* cleanup in parent process */
शून्य ul_pty_cleanup(संरचना ul_pty *pty)
{
	संरचना termios rtt;

	pty_signals_cleanup(pty);

	यदि (pty->master == -1 || !pty->isterm)
		फर्काऊ;

	DBG_OBJ(DONE, pty, ul_debug("cleanup"));
	rtt = pty->stdin_attrs;
	tcsetattr(STDIN_FILENO, TCSADRAIN, &rtt);
}

पूर्णाङ्क ul_pty_chownmod_slave(संरचना ul_pty *pty, uid_t uid, gid_t gid, mode_t mode)
{
	यदि (fchown(pty->slave, uid, gid))
		फर्काऊ -त्रुटि_सं;
	यदि (fchmod(pty->slave, mode))
		फर्काऊ -त्रुटि_सं;
	फर्काऊ 0;
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

	यदि (pty->sigfd >= 0)
		बंद(pty->sigfd);

	pty->slave = -1;
	pty->master = -1;
	pty->sigfd = -1;

	sigprocmask(SIG_SETMASK, &pty->orgsig, रिक्त);

	DBG_OBJ(SETUP, pty, ul_debug("... initialize slave done"));
}

स्थिर पूर्णाङ्क write_output(वर्ण *obuf, ssize_t bytes)
{
	DBG(IO, ul_debug(" writing output"));

	यदि (ul_write_all(STDOUT_FILENO, obuf, bytes)) {
		DBG(IO, ul_debug("  writing output *failed*"));
		फर्काऊ -त्रुटि_सं;
	}

	फर्काऊ 0;
}

स्थिर पूर्णाङ्क schedule_child_write(संरचना ul_pty *pty, वर्ण *buf, माप_प्रकार bufsz, पूर्णाङ्क final)
{
	संरचना ul_pty_child_buffer *stash;

	यदि (pty->free_buffers) {
		stash = pty->free_buffers;
		pty->free_buffers = stash->next;
		स_रखो(stash, 0, परिमाण(*stash));
	} अन्यथा
		stash = सुस्मृति(1, परिमाण(*stash));
	यदि (!stash)
		फर्काऊ -1;

	प्रतिज्ञा(bufsz <= परिमाण(stash->buf));

	स_नकल(stash->buf, buf, bufsz);
	stash->size = bufsz;
	stash->final_आगत = final ? 1 : 0;

	यदि (pty->child_buffer_head)
		pty->child_buffer_tail = pty->child_buffer_tail->next = stash;
	अन्यथा
		pty->child_buffer_head = pty->child_buffer_tail = stash;
	फर्काऊ 0;
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
	अचिन्हित पूर्णाङ्क tries = 0;
	संरचना pollfd fd = { .fd = pty->slave, .events = POLLIN };

	DBG_OBJ(IO, pty, ul_debug(" waiting for empty slave"));
	जबसम्म (poll(&fd, 1, 10) == 1 && tries < 8) {
		DBG_OBJ(IO, pty, ul_debug("   slave is not empty"));
		xusleep(250000);
		tries++;
	}
	यदि (tries < 8)
		DBG_OBJ(IO, pty, ul_debug("   slave is empty now"));

	DBG_OBJ(IO, pty, ul_debug(" sending EOF to master"));
}

स्थिर पूर्णाङ्क flush_child_buffers(संरचना ul_pty *pty, पूर्णाङ्क *anything)
{
	पूर्णाङ्क rc = 0, any = 0;

	जबसम्म (pty->child_buffer_head) {
		संरचना ul_pty_child_buffer *hd = pty->child_buffer_head;
		ssize_t ret;

		यदि (hd->final_आगत)
			drain_child_buffers(pty);

		DBG_OBJ(IO, hd, ul_debug(" stdin --> master trying %zu bytes", hd->size - hd->cursor));

		ret = लिख(pty->master, hd->buf + hd->cursor, hd->size - hd->cursor);
		यदि (ret == -1) {
			DBG_OBJ(IO, hd, ul_debug("   EAGAIN"));
			यदि (!(त्रुटि_सं == EINTR || त्रुटि_सं == EAGAIN))
				rc = -त्रुटि_सं;
			गच्छ out;
		}
		DBG_OBJ(IO, hd, ul_debug("   wrote %zd", ret));
		any = 1;
		hd->cursor += ret;

		यदि (hd->cursor == hd->size) {
			pty->child_buffer_head = hd->next;
			यदि (!hd->next)
				pty->child_buffer_tail = रिक्त;

			hd->next = pty->free_buffers;
			pty->free_buffers = hd;
		}
	}
out:
	/* without sync write_output() will write both input &
	 * shell output that looks like double echoing */
	यदि (any)
		fdatasync(pty->master);

	यदि (anything)
		*anything = any;
	फर्काऊ rc;
}

शून्य ul_pty_write_eof_to_child(संरचना ul_pty *pty)
{
	वर्ण c = DEF_EOF;
	schedule_child_write(pty, &c, परिमाण(वर्ण), 1);
}

स्थिर पूर्णाङ्क mainloop_callback(संरचना ul_pty *pty)
{
	पूर्णाङ्क rc;

	यदि (!pty->callbacks.mainloop)
		फर्काऊ 0;

	DBG_OBJ(IO, pty, ul_debug("calling mainloop callback"));
	rc = pty->callbacks.mainloop(pty->callback_data);

	DBG_OBJ(IO, pty, ul_debug(" callback done [rc=%d]", rc));
	फर्काऊ rc;
}

स्थिर पूर्णाङ्क handle_io(संरचना ul_pty *pty, पूर्णाङ्क fd, पूर्णाङ्क *eof)
{
	वर्ण buf[बफ_मान];
	ssize_t bytes;
	पूर्णाङ्क rc = 0;
	sigset_t set;

	DBG_OBJ(IO, pty, ul_debug(" handle I/O on fd=%d", fd));
	*eof = 0;

	sigemptyset(&set);
	sigaddset(&set, SIGTTIN);
	sigprocmask(SIG_UNBLOCK, &set, रिक्त);
	/* read from active FD */
	bytes = पठ(fd, buf, परिमाण(buf));
	sigprocmask(SIG_BLOCK, &set, रिक्त);
	यदि (bytes == -1) {
		यदि (त्रुटि_सं == EAGAIN || त्रुटि_सं == EINTR)
			फर्काऊ 0;
		फर्काऊ -त्रुटि_सं;
	}

	यदि (bytes == 0) {
		*eof = 1;
		फर्काऊ 0;
	}

	/* from stdin (user) to command */
	यदि (fd == STDIN_FILENO) {
		DBG_OBJ(IO, pty, ul_debug(" stdin --> master %zd bytes queued", bytes));

		यदि (schedule_child_write(pty, buf, bytes, 0))
			फर्काऊ -त्रुटि_सं;

	/* from command (master) to stdout */
	} अन्यथा यदि (fd == pty->master) {
		DBG_OBJ(IO, pty, ul_debug(" master --> stdout %zd bytes", bytes));
		write_output(buf, bytes);
	}

	यदि (pty->callbacks.log_stream_activity)
		rc = pty->callbacks.log_stream_activity(
					pty->callback_data, fd, buf, bytes);

	फर्काऊ rc;
}

शून्य ul_pty_wait_for_child(संरचना ul_pty *pty)
{
	पूर्णाङ्क status;
	pid_t pid;
	पूर्णाङ्क options = 0;

	यदि (pty->child == (pid_t) -1)
		फर्काऊ;

	DBG(SIG, ul_debug("waiting for child [child=%d]", (पूर्णाङ्क) pty->child));

	यदि (ul_pty_is_running(pty)) {
		/* wait for specific child */
		options = WNOHANG;
		लागि (;;) {
			pid = waitpid(pty->child, &status, options);
			DBG(SIG, ul_debug(" waitpid done [rc=%d]", (पूर्णाङ्क) pid));
			यदि (pid != (pid_t) - 1) {
				यदि (pty->callbacks.child_die)
					pty->callbacks.child_die(
							pty->callback_data,
							pty->child, status);
				ul_pty_set_child(pty, (pid_t) -1);
			} अन्यथा
				रोक;
		}
	} अन्यथा {
		/* final wait */
		जबसम्म ((pid = waitpid(-1, &status, options)) > 0) {
			DBG(SIG, ul_debug(" waitpid done [rc=%d]", (पूर्णाङ्क) pid));
			यदि (pid == pty->child) {
				यदि (pty->callbacks.child_die)
					pty->callbacks.child_die(
							pty->callback_data,
							pty->child, status);
				ul_pty_set_child(pty, (pid_t) -1);
			}
		}
	}
}

स्थिर पूर्णाङ्क handle_signal(संरचना ul_pty *pty, पूर्णाङ्क fd)
{
	संरचना signalfd_siginfo info;
	ssize_t bytes;
	पूर्णाङ्क rc = 0;

	DBG_OBJ(SIG, pty, ul_debug(" handle signal on fd=%d", fd));

	bytes = पठ(fd, &info, परिमाण(info));
	यदि (bytes != परिमाण(info)) {
		यदि (bytes < 0 && (त्रुटि_सं == EAGAIN || त्रुटि_सं == EINTR))
			फर्काऊ 0;
		फर्काऊ -त्रुटि_सं;
	}

	विकल्प (info.ssi_signo) {
	पक्ष SIGCHLD:
		DBG_OBJ(SIG, pty, ul_debug(" get signal SIGCHLD"));

		यदि (info.ssi_code == CLD_EXITED
		    || info.ssi_code == CLD_KILLED
		    || info.ssi_code == CLD_DUMPED) {

			यदि (pty->callbacks.child_wait)
				pty->callbacks.child_wait(pty->callback_data,
							  pty->child);
			अन्यथा
				ul_pty_wait_for_child(pty);

		} अन्यथा यदि (info.ssi_status == SIGSTOP && pty->child > 0) {
			pty->callbacks.child_sigstop(pty->callback_data,
						     pty->child);
		}

		यदि (pty->child <= 0) {
			DBG_OBJ(SIG, pty, ul_debug(" no child, setting leaving timeout"));
			pty->poll_timeout = 10;
			timerclear(&pty->next_callback_time);
		}
		फर्काऊ 0;
	पक्ष SIGWINCH:
		DBG_OBJ(SIG, pty, ul_debug(" get signal SIGWINCH"));
		यदि (pty->isterm) {
			ioctl(STDIN_FILENO, TIOCGWINSZ, (वर्ण *)&pty->win);
			ioctl(pty->slave, TIOCSWINSZ, (वर्ण *)&pty->win);

			यदि (pty->callbacks.log_signal)
				rc = pty->callbacks.log_signal(pty->callback_data,
							&info, (शून्य *) &pty->win);
		}
		रोक;
	पक्ष SIGALRM:
		FALLTHROUGH;
	पक्ष SIGHUP:
		FALLTHROUGH;
	पक्ष संक_इति:
		FALLTHROUGH;
	पक्ष संक_विघ्न:
		FALLTHROUGH;
	पक्ष SIGQUIT:
		DBG_OBJ(SIG, pty, ul_debug(" get signal SIG{ALRM,HUP,TERM,INT,QUIT}"));
		pty->delivered_signal = info.ssi_signo;
		/* Child termination is going to generate SIGCHLD (see above) */
		यदि (pty->child > 0)
			समाप्त(pty->child, संक_इति);

		यदि (pty->callbacks.log_signal)
			rc = pty->callbacks.log_signal(pty->callback_data,
					&info, (शून्य *) &pty->win);
		रोक;
	पक्ष SIGUSR1:
		DBG_OBJ(SIG, pty, ul_debug(" get signal SIGUSR1"));
		यदि (pty->callbacks.flush_logs)
			rc = pty->callbacks.flush_logs(pty->callback_data);
		रोक;
	स्वभाव:
		पात();
	}

	फर्काऊ rc;
}

/* loop in parent */
पूर्णाङ्क ul_pty_proxy_master(संरचना ul_pty *pty)
{
	पूर्णाङ्क rc = 0, ret, eof = 0;
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

	जबसम्म (!pty->delivered_signal) {
		माप_प्रकार i;
		पूर्णाङ्क errsv, timeout;

		DBG_OBJ(IO, pty, ul_debug("--poll() loop--"));

		/* note, callback usually updates @next_callback_time */
		यदि (timerisset(&pty->next_callback_time)) {
			संरचना timeval now = { 0 };

			DBG_OBJ(IO, pty, ul_debug(" callback requested"));
			gettime_monotonic(&now);
			यदि (timercmp(&now, &pty->next_callback_time, >)) {
				rc = mainloop_callback(pty);
				यदि (rc)
					रोक;
			}
		}

		/* set timeout */
		यदि (timerisset(&pty->next_callback_time)) {
			संरचना timeval now = { 0 }, rest = { 0 };

			gettime_monotonic(&now);
			timersub(&pty->next_callback_time, &now, &rest);
			timeout = (rest.tv_sec * MSEC_PER_SEC) +  (rest.tv_usec / USEC_PER_MSEC);
		} अन्यथा
			timeout = pty->poll_timeout;

		/* use POLLOUT (aka "writing is now possible") if data queued */
		यदि (pty->child_buffer_head)
			pfd[POLLFD_MASTER].events |= POLLOUT;
		अन्यथा
			pfd[POLLFD_MASTER].events &= ~POLLOUT;

		/* wait for input, signal or timeout */
		DBG_OBJ(IO, pty, ul_debug("calling poll() [timeout=%dms]", timeout));
		ret = poll(pfd, ARRAY_SIZE(pfd), timeout);

		errsv = त्रुटि_सं;
		DBG_OBJ(IO, pty, ul_debug("poll() rc=%d", ret));

		/* error */
		यदि (ret < 0) {
			यदि (errsv == EAGAIN)
				जारी;
			rc = -त्रुटि_सं;
			रोक;
		}

		/* timeout */
		यदि (ret == 0) {
			यदि (timerisset(&pty->next_callback_time)) {
				rc = mainloop_callback(pty);
				यदि (rc == 0)
					जारी;
			} अन्यथा {
				rc = 0;
			}

			DBG_OBJ(IO, pty, ul_debug("leaving poll() loop [timeout=%d, rc=%d]", timeout, rc));
			रोक;
		}
		/* event */
		लागि (i = 0; i < ARRAY_SIZE(pfd); i++) {
			यदि (pfd[i].revents == 0)
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

			यदि (i == POLLFD_SIGNAL)
				rc = handle_signal(pty, pfd[i].fd);
			अन्यथा {
				यदि (pfd[i].revents & POLLIN)
					rc = handle_io(pty, pfd[i].fd, &eof); /* data */
				यदि (pfd[i].revents & POLLOUT) /* i == POLLFD_MASTER */
					rc = flush_child_buffers(pty, रिक्त);
			}

			यदि (rc) {
				पूर्णाङ्क anything = 1;

				ul_pty_write_eof_to_child(pty);
				लागि (anything = 1; anything;)
					flush_child_buffers(pty, &anything);
				रोक;
			}

			यदि (i == POLLFD_SIGNAL)
				जारी;

			/* EOF maybe detected in two ways; they are as follows:
			 *	A) poll() return POLLHUP event after close()
			 *	B) read() returns 0 (no data)
			 *
			 * POLLNVAL means that fd is closed.
			 */
			यदि ((pfd[i].revents & POLLHUP) || (pfd[i].revents & POLLNVAL) || eof) {
				DBG_OBJ(IO, pty, ul_debug(" ignore FD"));
				यदि (i == POLLFD_STDIN) {
					pfd[i].fd = -1;
					ul_pty_write_eof_to_child(pty);
				} अन्यथा /* i == POLLFD_MASTER */
					pfd[i].revents &= ~POLLIN;
			}
		}
		यदि (rc)
			रोक;
	}

	यदि (rc && pty->child && pty->child != (pid_t) -1 && !pty->delivered_signal) {
		समाप्त(pty->child, संक_इति);
		sleep(2);
		समाप्त(pty->child, SIGKILL);
	}

	pty_signals_cleanup(pty);

	DBG(IO, ul_debug("poll() done [signal=%d, rc=%d]", pty->delivered_signal, rc));
	फर्काऊ rc;
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

पूर्णाङ्क main(पूर्णाङ्क argc, वर्ण *argv[])
{
	संरचना ul_pty_callbacks *cb;
	अचर वर्ण *shell, *command = रिक्त, *shname = रिक्त;
	पूर्णाङ्क caught_signal = 0;
	pid_t child;
	संरचना ul_pty *pty;

	shell = दो_पर्या("SHELL");
	यदि (shell == रिक्त)
		shell = _PATH_BSHELL;
	यदि (argc == 2)
		command = argv[1];

	ul_pty_init_debug(0);

	pty = ul_new_pty(isatty(STDIN_FILENO));
	यदि (!pty)
		err(निकास_त्रुटि, "failed to allocate PTY handler");

	cb = ul_pty_get_callbacks(pty);
	cb->child_sigstop = child_sigstop;

	यदि (ul_pty_setup(pty))
		err(निकास_त्रुटि, "failed to create pseudo-terminal");
	यदि (ul_pty_signals_setup(pty))
		err(निकास_त्रुटि, "failed to initialize signals handler");
	यदि (ul_pty_terminal_setup(pty))
		err(निकास_त्रुटि, "failed to setup terminal");

	ख_साफ(मानक_निकास);			/* ??? */

	विकल्प ((पूर्णाङ्क) (child = विभाजन())) {
	पक्ष -1: /* error */
		ul_pty_cleanup(pty);
		err(निकास_त्रुटि, "cannot create child process");
		रोक;

	पक्ष 0: /* child */
		ul_pty_init_slave(pty);

		सङ्केत(संक_इति, संक_पूर्व); /* because /etc/csh.login */

		shname = म_खोजप(shell, '/');
		shname = shname ? shname + 1 : shell;

		यदि (command)
			execl(shell, shname, "-c", command, (वर्ण *)रिक्त);
		अन्यथा
			execl(shell, shname, "-i", (वर्ण *)रिक्त);
		err(निकास_त्रुटि, "failed to execute %s", shell);
		रोक;

	स्वभाव:
		रोक;
	}

	/* parent */
	ul_pty_set_child(pty, child);

	/* this is the main loop */
	ul_pty_proxy_master(pty);

	/* all done; cleanup and kill */
	caught_signal = ul_pty_get_delivered_signal(pty);

	यदि (!caught_signal && ul_pty_get_child(pty) != (pid_t)-1)
		ul_pty_wait_for_child(pty);	/* final wait */

	यदि (caught_signal && ul_pty_get_child(pty) != (pid_t)-1) {
		ख_लिखो(मानक_त्रुटि, "\nSession terminated, killing shell...");
		समाप्त(child, संक_इति);
		sleep(2);
		समाप्त(child, SIGKILL);
		ख_लिखो(मानक_त्रुटि, " ...killed.\n");
	}

	ul_pty_cleanup(pty);
	ul_free_pty(pty);
	फर्काऊ निकास_सफल;
}

#endif /* TEST_PROGRAM */

