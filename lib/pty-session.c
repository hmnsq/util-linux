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
#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <pty.h>
#അന്തര്ഭാവ <poll.h>
#അന്തര്ഭാവ <sys/signalfd.h>
#അന്തര്ഭാവ <paths.h>
#അന്തര്ഭാവ <sys/types.h>
#അന്തര്ഭാവ <sys/wait.h>
#അന്തര്ഭാവ <inttypes.h>

#അന്തര്ഭാവ "c.h"
#അന്തര്ഭാവ "all-io.h"
#അന്തര്ഭാവ "ttyutils.h"
#അന്തര്ഭാവ "pty-session.h"
#അന്തര്ഭാവ "monotonic.h"
#അന്തര്ഭാവ "debug.h"

സ്ഥിതം UL_DEBUG_DEFINE_MASK(ulpty);
UL_DEBUG_DEFINE_MASKNAMES(ulpty) = UL_DEBUG_EMPTY_MASKNAMES;

#define ULPTY_DEBUG_INIT	(1 << 1)
#define ULPTY_DEBUG_SETUP	(1 << 2)
#define ULPTY_DEBUG_SIG		(1 << 3)
#define ULPTY_DEBUG_IO		(1 << 4)
#define ULPTY_DEBUG_DONE	(1 << 5)

#define DBG(m, x)		__UL_DBG(ulpty, ULPTY_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulpty, ULPTY_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(ulpty, ULPTY_DEBUG_, m, x)

ശൂന്യം ul_pty_init_debug(പൂർണ്ണസംഖ്യ mask)
{
	എങ്കിൽ (ulpty_debug_mask)
		തിരികെ;
	__UL_INIT_DEBUG_FROM_ENV(ulpty, ULPTY_DEBUG_, mask, ULPTY_DEBUG);
}

ഘടന ul_pty *ul_new_pty(പൂർണ്ണസംഖ്യ is_stdin_tty)
{
	ഘടന ul_pty *pty = calloc(1, വലുപ്പം(*pty));

	എങ്കിൽ (!pty)
		തിരികെ ശൂന്യ;

	DBG_OBJ(SETUP, pty, ul_debug("alloc handler"));
	pty->isterm = is_stdin_tty;
	pty->master = -1;
	pty->slave = -1;
	pty->sigfd = -1;
	pty->child = (pid_t) -1;

	തിരികെ pty;
}

ശൂന്യം ul_free_pty(ഘടന ul_pty *pty)
{
	ഘടന ul_pty_child_buffer *hd;

	ആകുമ്പോൾ ((hd = pty->child_buffer_head)) {
		pty->child_buffer_head = hd->next;
		സ്വതന്ത്രം(hd);
	}

	ആകുമ്പോൾ ((hd = pty->free_buffers)) {
		pty->free_buffers = hd->next;
		സ്വതന്ത്രം(hd);
	}
	സ്വതന്ത്രം(pty);
}

ശൂന്യം ul_pty_slave_echo(ഘടന ul_pty *pty, പൂർണ്ണസംഖ്യ enable)
{
	പ്രതിജ്ഞാ(pty);
	pty->slave_echo = enable ? 1 : 0;
}

പൂർണ്ണസംഖ്യ ul_pty_get_delivered_signal(ഘടന ul_pty *pty)
{
	പ്രതിജ്ഞാ(pty);
	തിരികെ pty->delivered_signal;
}

ഘടന ul_pty_callbacks *ul_pty_get_callbacks(ഘടന ul_pty *pty)
{
	പ്രതിജ്ഞാ(pty);
	തിരികെ &pty->callbacks;
}

ശൂന്യം ul_pty_set_callback_data(ഘടന ul_pty *pty, ശൂന്യം *data)
{
	പ്രതിജ്ഞാ(pty);
	pty->callback_data = data;
}

ശൂന്യം ul_pty_set_child(ഘടന ul_pty *pty, pid_t child)
{
	പ്രതിജ്ഞാ(pty);
	pty->child = child;
}

പൂർണ്ണസംഖ്യ ul_pty_get_childfd(ഘടന ul_pty *pty)
{
	പ്രതിജ്ഞാ(pty);
	തിരികെ pty->master;
}

pid_t ul_pty_get_child(ഘടന ul_pty *pty)
{
	പ്രതിജ്ഞാ(pty);
	തിരികെ pty->child;
}

/* it's active when signals are redirected to sigfd */
പൂർണ്ണസംഖ്യ ul_pty_is_running(ഘടന ul_pty *pty)
{
	പ്രതിജ്ഞാ(pty);
	തിരികെ pty->sigfd >= 0;
}

ശൂന്യം ul_pty_set_mainloop_time(ഘടന ul_pty *pty, ഘടന timeval *tv)
{
	പ്രതിജ്ഞാ(pty);
	എങ്കിൽ (!tv) {
		DBG_OBJ(IO, pty, ul_debug("mainloop time: clear"));
		timerclear(&pty->next_callback_time);
	} അല്ലെങ്കിൽ {
		pty->next_callback_time.tv_sec = tv->tv_sec;
		pty->next_callback_time.tv_usec = tv->tv_usec;
		DBG_OBJ(IO, pty, ul_debug("mainloop time: %"PRId64".%06"PRId64,
				(int64_t) tv->tv_sec, (int64_t) tv->tv_usec));
	}
}

സ്ഥിതം ശൂന്യം pty_signals_cleanup(ഘടന ul_pty *pty)
{
	എങ്കിൽ (pty->sigfd != -1)
		close(pty->sigfd);
	pty->sigfd = -1;

	/* restore original setting */
	sigprocmask(SIG_SETMASK, &pty->orgsig, ശൂന്യ);
}

/* call me before fork() */
പൂർണ്ണസംഖ്യ ul_pty_setup(ഘടന ul_pty *pty)
{
	ഘടന termios attrs;
	പൂർണ്ണസംഖ്യ rc = 0;

	പ്രതിജ്ഞാ(pty->sigfd == -1);

	/* save the current signals setting (to make ul_pty_cleanup() usable,
	 * otherwise see ul_pty_signals_setup() */
	sigprocmask(0, ശൂന്യ, &pty->orgsig);

	എങ്കിൽ (pty->isterm) {
		DBG_OBJ(SETUP, pty, ul_debug("create for terminal"));

		/* original setting of the current terminal */
		എങ്കിൽ (tcgetattr(STDIN_FILENO, &pty->stdin_attrs) != 0) {
			rc = -errno;
			പോകുക done;
		}

		attrs = pty->stdin_attrs;
		എങ്കിൽ (pty->slave_echo)
			attrs.c_lflag |= ECHO;
		അല്ലെങ്കിൽ
			attrs.c_lflag &= ~ECHO;

		ioctl(STDIN_FILENO, TIOCGWINSZ, (അക്ഷരം *)&pty->win);
		/* create master+slave */
		rc = openpty(&pty->master, &pty->slave, ശൂന്യ, &attrs, &pty->win);
		എങ്കിൽ (rc)
			പോകുക done;

	} അല്ലെങ്കിൽ {
		DBG_OBJ(SETUP, pty, ul_debug("create for non-terminal"));

		rc = openpty(&pty->master, &pty->slave, ശൂന്യ, ശൂന്യ, ശൂന്യ);
		എങ്കിൽ (rc)
			പോകുക done;

		tcgetattr(pty->slave, &attrs);

		എങ്കിൽ (pty->slave_echo)
			attrs.c_lflag |= ECHO;
		അല്ലെങ്കിൽ
			attrs.c_lflag &= ~ECHO;

		tcsetattr(pty->slave, TCSANOW, &attrs);
	}

	fcntl(pty->master, F_SETFL, O_NONBLOCK);

done:
	എങ്കിൽ (rc)
		ul_pty_cleanup(pty);

	DBG_OBJ(SETUP, pty, ul_debug("pty setup done [master=%d, slave=%d, rc=%d]",
				pty->master, pty->slave, rc));
	തിരികെ rc;
}

/* call me after ul_pty_signals_setup() and before fork() */
പൂർണ്ണസംഖ്യ ul_pty_terminal_setup(ഘടന ul_pty *pty)
{
	ഘടന termios attrs;

	എങ്കിൽ (!pty->isterm)
		തിരികെ 0;

	attrs = pty->stdin_attrs;
	cfmakeraw(&attrs);
	എങ്കിൽ (tcsetattr(STDIN_FILENO, TCSANOW, &attrs))
		തിരികെ -errno;

	DBG_OBJ(SETUP, pty, ul_debug("terminal raw mode setup done"));
	തിരികെ 0;
}

/* call me before fork() */
പൂർണ്ണസംഖ്യ ul_pty_signals_setup(ഘടന ul_pty *pty)
{
	sigset_t ourset;
	പൂർണ്ണസംഖ്യ rc = 0;

	പ്രതിജ്ഞാ(pty->sigfd == -1);

	/* save the current signals setting */
	sigprocmask(0, ശൂന്യ, &pty->orgsig);

	sigfillset(&ourset);
	എങ്കിൽ (sigprocmask(SIG_BLOCK, &ourset, ശൂന്യ)) {
		rc = -errno;
		പോകുക done;
	}

	sigemptyset(&ourset);
	sigaddset(&ourset, SIGCHLD);
	sigaddset(&ourset, SIGWINCH);
	sigaddset(&ourset, SIGALRM);
	sigaddset(&ourset, SIGHUP);
	sigaddset(&ourset, SIGTERM);
	sigaddset(&ourset, SIGINT);
	sigaddset(&ourset, SIGQUIT);

	എങ്കിൽ (pty->callbacks.flush_logs)
		sigaddset(&ourset, SIGUSR1);

	എങ്കിൽ ((pty->sigfd = signalfd(-1, &ourset, SFD_CLOEXEC)) < 0)
		rc = -errno;
done:
	എങ്കിൽ (rc)
		ul_pty_cleanup(pty);

	DBG_OBJ(SETUP, pty, ul_debug("pty signals setup done [rc=%d]", rc));
	തിരികെ rc;
}

/* cleanup in parent process */
ശൂന്യം ul_pty_cleanup(ഘടന ul_pty *pty)
{
	ഘടന termios rtt;

	pty_signals_cleanup(pty);

	എങ്കിൽ (pty->master == -1 || !pty->isterm)
		തിരികെ;

	DBG_OBJ(DONE, pty, ul_debug("cleanup"));
	rtt = pty->stdin_attrs;
	tcsetattr(STDIN_FILENO, TCSADRAIN, &rtt);
}

പൂർണ്ണസംഖ്യ ul_pty_chownmod_slave(ഘടന ul_pty *pty, uid_t uid, gid_t gid, mode_t mode)
{
	എങ്കിൽ (fchown(pty->slave, uid, gid))
		തിരികെ -errno;
	എങ്കിൽ (fchmod(pty->slave, mode))
		തിരികെ -errno;
	തിരികെ 0;
}

/* call me in child process */
ശൂന്യം ul_pty_init_slave(ഘടന ul_pty *pty)
{
	DBG_OBJ(SETUP, pty, ul_debug("initialize slave"));

	setsid();

	ioctl(pty->slave, TIOCSCTTY, 1);
	close(pty->master);

	dup2(pty->slave, STDIN_FILENO);
	dup2(pty->slave, STDOUT_FILENO);
	dup2(pty->slave, STDERR_FILENO);

	close(pty->slave);

	എങ്കിൽ (pty->sigfd >= 0)
		close(pty->sigfd);

	pty->slave = -1;
	pty->master = -1;
	pty->sigfd = -1;

	sigprocmask(SIG_SETMASK, &pty->orgsig, ശൂന്യ);

	DBG_OBJ(SETUP, pty, ul_debug("... initialize slave done"));
}

സ്ഥിതം പൂർണ്ണസംഖ്യ write_output(അക്ഷരം *obuf, ssize_t bytes)
{
	DBG(IO, ul_debug(" writing output"));

	എങ്കിൽ (ul_write_all(STDOUT_FILENO, obuf, bytes)) {
		DBG(IO, ul_debug("  writing output *failed*"));
		തിരികെ -errno;
	}

	തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ schedule_child_write(ഘടന ul_pty *pty, അക്ഷരം *buf, size_t bufsz, പൂർണ്ണസംഖ്യ final)
{
	ഘടന ul_pty_child_buffer *stash;

	എങ്കിൽ (pty->free_buffers) {
		stash = pty->free_buffers;
		pty->free_buffers = stash->next;
		memset(stash, 0, വലുപ്പം(*stash));
	} അല്ലെങ്കിൽ
		stash = calloc(1, വലുപ്പം(*stash));
	എങ്കിൽ (!stash)
		തിരികെ -1;

	പ്രതിജ്ഞാ(bufsz <= വലുപ്പം(stash->buf));

	memcpy(stash->buf, buf, bufsz);
	stash->size = bufsz;
	stash->final_നിവേശം = final ? 1 : 0;

	എങ്കിൽ (pty->child_buffer_head)
		pty->child_buffer_tail = pty->child_buffer_tail->next = stash;
	അല്ലെങ്കിൽ
		pty->child_buffer_head = pty->child_buffer_tail = stash;
	തിരികെ 0;
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
സ്ഥിതം ശൂന്യം drain_child_buffers(ഘടന ul_pty *pty)
{
	unsigned പൂർണ്ണസംഖ്യ tries = 0;
	ഘടന pollfd fd = { .fd = pty->slave, .events = POLLIN };

	DBG_OBJ(IO, pty, ul_debug(" waiting for empty slave"));
	ആകുമ്പോൾ (poll(&fd, 1, 10) == 1 && tries < 8) {
		DBG_OBJ(IO, pty, ul_debug("   slave is not empty"));
		xusleep(250000);
		tries++;
	}
	എങ്കിൽ (tries < 8)
		DBG_OBJ(IO, pty, ul_debug("   slave is empty now"));

	DBG_OBJ(IO, pty, ul_debug(" sending EOF to master"));
}

സ്ഥിതം പൂർണ്ണസംഖ്യ flush_child_buffers(ഘടന ul_pty *pty, പൂർണ്ണസംഖ്യ *anything)
{
	പൂർണ്ണസംഖ്യ rc = 0, any = 0;

	ആകുമ്പോൾ (pty->child_buffer_head) {
		ഘടന ul_pty_child_buffer *hd = pty->child_buffer_head;
		ssize_t ret;

		എങ്കിൽ (hd->final_നിവേശം)
			drain_child_buffers(pty);

		DBG_OBJ(IO, hd, ul_debug(" stdin --> master trying %zu bytes", hd->size - hd->cursor));

		ret = ലിഖ(pty->master, hd->buf + hd->cursor, hd->size - hd->cursor);
		എങ്കിൽ (ret == -1) {
			DBG_OBJ(IO, hd, ul_debug("   EAGAIN"));
			എങ്കിൽ (!(errno == EINTR || errno == EAGAIN))
				rc = -errno;
			പോകുക out;
		}
		DBG_OBJ(IO, hd, ul_debug("   wrote %zd", ret));
		any = 1;
		hd->cursor += ret;

		എങ്കിൽ (hd->cursor == hd->size) {
			pty->child_buffer_head = hd->next;
			എങ്കിൽ (!hd->next)
				pty->child_buffer_tail = ശൂന്യ;

			hd->next = pty->free_buffers;
			pty->free_buffers = hd;
		}
	}
out:
	/* without sync write_output() will write both input &
	 * shell output that looks like double echoing */
	എങ്കിൽ (any)
		fdatasync(pty->master);

	എങ്കിൽ (anything)
		*anything = any;
	തിരികെ rc;
}

ശൂന്യം ul_pty_write_eof_to_child(ഘടന ul_pty *pty)
{
	അക്ഷരം c = DEF_EOF;
	schedule_child_write(pty, &c, വലുപ്പം(അക്ഷരം), 1);
}

സ്ഥിതം പൂർണ്ണസംഖ്യ mainloop_callback(ഘടന ul_pty *pty)
{
	പൂർണ്ണസംഖ്യ rc;

	എങ്കിൽ (!pty->callbacks.mainloop)
		തിരികെ 0;

	DBG_OBJ(IO, pty, ul_debug("calling mainloop callback"));
	rc = pty->callbacks.mainloop(pty->callback_data);

	DBG_OBJ(IO, pty, ul_debug(" callback done [rc=%d]", rc));
	തിരികെ rc;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ handle_io(ഘടന ul_pty *pty, പൂർണ്ണസംഖ്യ fd, പൂർണ്ണസംഖ്യ *eof)
{
	അക്ഷരം buf[BUFSIZ];
	ssize_t bytes;
	പൂർണ്ണസംഖ്യ rc = 0;
	sigset_t set;

	DBG_OBJ(IO, pty, ul_debug(" handle I/O on fd=%d", fd));
	*eof = 0;

	sigemptyset(&set);
	sigaddset(&set, SIGTTIN);
	sigprocmask(SIG_UNBLOCK, &set, ശൂന്യ);
	/* read from active FD */
	bytes = പഠ(fd, buf, വലുപ്പം(buf));
	sigprocmask(SIG_BLOCK, &set, ശൂന്യ);
	എങ്കിൽ (bytes == -1) {
		എങ്കിൽ (errno == EAGAIN || errno == EINTR)
			തിരികെ 0;
		തിരികെ -errno;
	}

	എങ്കിൽ (bytes == 0) {
		*eof = 1;
		തിരികെ 0;
	}

	/* from stdin (user) to command */
	എങ്കിൽ (fd == STDIN_FILENO) {
		DBG_OBJ(IO, pty, ul_debug(" stdin --> master %zd bytes queued", bytes));

		എങ്കിൽ (schedule_child_write(pty, buf, bytes, 0))
			തിരികെ -errno;

	/* from command (master) to stdout */
	} അല്ലെങ്കിൽ എങ്കിൽ (fd == pty->master) {
		DBG_OBJ(IO, pty, ul_debug(" master --> stdout %zd bytes", bytes));
		write_output(buf, bytes);
	}

	എങ്കിൽ (pty->callbacks.log_stream_activity)
		rc = pty->callbacks.log_stream_activity(
					pty->callback_data, fd, buf, bytes);

	തിരികെ rc;
}

ശൂന്യം ul_pty_wait_for_child(ഘടന ul_pty *pty)
{
	പൂർണ്ണസംഖ്യ status;
	pid_t pid;
	പൂർണ്ണസംഖ്യ options = 0;

	എങ്കിൽ (pty->child == (pid_t) -1)
		തിരികെ;

	DBG(SIG, ul_debug("waiting for child [child=%d]", (പൂർണ്ണസംഖ്യ) pty->child));

	എങ്കിൽ (ul_pty_is_running(pty)) {
		/* wait for specific child */
		options = WNOHANG;
		വേണ്ടി (;;) {
			pid = waitpid(pty->child, &status, options);
			DBG(SIG, ul_debug(" waitpid done [rc=%d]", (പൂർണ്ണസംഖ്യ) pid));
			എങ്കിൽ (pid != (pid_t) - 1) {
				എങ്കിൽ (pty->callbacks.child_die)
					pty->callbacks.child_die(
							pty->callback_data,
							pty->child, status);
				ul_pty_set_child(pty, (pid_t) -1);
			} അല്ലെങ്കിൽ
				നിർത്തുക;
		}
	} അല്ലെങ്കിൽ {
		/* final wait */
		ആകുമ്പോൾ ((pid = waitpid(-1, &status, options)) > 0) {
			DBG(SIG, ul_debug(" waitpid done [rc=%d]", (പൂർണ്ണസംഖ്യ) pid));
			എങ്കിൽ (pid == pty->child) {
				എങ്കിൽ (pty->callbacks.child_die)
					pty->callbacks.child_die(
							pty->callback_data,
							pty->child, status);
				ul_pty_set_child(pty, (pid_t) -1);
			}
		}
	}
}

സ്ഥിതം പൂർണ്ണസംഖ്യ handle_signal(ഘടന ul_pty *pty, പൂർണ്ണസംഖ്യ fd)
{
	ഘടന signalfd_siginfo info;
	ssize_t bytes;
	പൂർണ്ണസംഖ്യ rc = 0;

	DBG_OBJ(SIG, pty, ul_debug(" handle signal on fd=%d", fd));

	bytes = പഠ(fd, &info, വലുപ്പം(info));
	എങ്കിൽ (bytes != വലുപ്പം(info)) {
		എങ്കിൽ (bytes < 0 && (errno == EAGAIN || errno == EINTR))
			തിരികെ 0;
		തിരികെ -errno;
	}

	തിരഞ്ഞെടുപ്പ് (info.ssi_signo) {
	അവസ്ഥ SIGCHLD:
		DBG_OBJ(SIG, pty, ul_debug(" get signal SIGCHLD"));

		എങ്കിൽ (info.ssi_code == CLD_EXITED
		    || info.ssi_code == CLD_KILLED
		    || info.ssi_code == CLD_DUMPED) {

			എങ്കിൽ (pty->callbacks.child_wait)
				pty->callbacks.child_wait(pty->callback_data,
							  pty->child);
			അല്ലെങ്കിൽ
				ul_pty_wait_for_child(pty);

		} അല്ലെങ്കിൽ എങ്കിൽ (info.ssi_status == SIGSTOP && pty->child > 0) {
			pty->callbacks.child_sigstop(pty->callback_data,
						     pty->child);
		}

		എങ്കിൽ (pty->child <= 0) {
			DBG_OBJ(SIG, pty, ul_debug(" no child, setting leaving timeout"));
			pty->poll_timeout = 10;
			timerclear(&pty->next_callback_time);
		}
		തിരികെ 0;
	അവസ്ഥ SIGWINCH:
		DBG_OBJ(SIG, pty, ul_debug(" get signal SIGWINCH"));
		എങ്കിൽ (pty->isterm) {
			ioctl(STDIN_FILENO, TIOCGWINSZ, (അക്ഷരം *)&pty->win);
			ioctl(pty->slave, TIOCSWINSZ, (അക്ഷരം *)&pty->win);

			എങ്കിൽ (pty->callbacks.log_signal)
				rc = pty->callbacks.log_signal(pty->callback_data,
							&info, (ശൂന്യം *) &pty->win);
		}
		നിർത്തുക;
	അവസ്ഥ SIGALRM:
		FALLTHROUGH;
	അവസ്ഥ SIGHUP:
		FALLTHROUGH;
	അവസ്ഥ SIGTERM:
		FALLTHROUGH;
	അവസ്ഥ SIGINT:
		FALLTHROUGH;
	അവസ്ഥ SIGQUIT:
		DBG_OBJ(SIG, pty, ul_debug(" get signal SIG{ALRM,HUP,TERM,INT,QUIT}"));
		pty->delivered_signal = info.ssi_signo;
		/* Child termination is going to generate SIGCHLD (see above) */
		എങ്കിൽ (pty->child > 0)
			kill(pty->child, SIGTERM);

		എങ്കിൽ (pty->callbacks.log_signal)
			rc = pty->callbacks.log_signal(pty->callback_data,
					&info, (ശൂന്യം *) &pty->win);
		നിർത്തുക;
	അവസ്ഥ SIGUSR1:
		DBG_OBJ(SIG, pty, ul_debug(" get signal SIGUSR1"));
		എങ്കിൽ (pty->callbacks.flush_logs)
			rc = pty->callbacks.flush_logs(pty->callback_data);
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		abort();
	}

	തിരികെ rc;
}

/* loop in parent */
പൂർണ്ണസംഖ്യ ul_pty_proxy_master(ഘടന ul_pty *pty)
{
	പൂർണ്ണസംഖ്യ rc = 0, ret, eof = 0;
	എണ്ണൽ {
		POLLFD_SIGNAL = 0,
		POLLFD_MASTER,
		POLLFD_STDIN

	};
	ഘടന pollfd pfd[] = {
		[POLLFD_SIGNAL] = { .fd = -1,		.events = POLLIN | POLLERR | POLLHUP },
		[POLLFD_MASTER] = { .fd = pty->master,  .events = POLLIN | POLLERR | POLLHUP },
		[POLLFD_STDIN]	= { .fd = STDIN_FILENO, .events = POLLIN | POLLERR | POLLHUP }
	};

	/* We use signalfd, and standard signals by handlers are completely blocked */
	പ്രതിജ്ഞാ(pty->sigfd >= 0);

	pfd[POLLFD_SIGNAL].fd = pty->sigfd;
	pty->poll_timeout = -1;

	ആകുമ്പോൾ (!pty->delivered_signal) {
		size_t i;
		പൂർണ്ണസംഖ്യ errsv, timeout;

		DBG_OBJ(IO, pty, ul_debug("--poll() loop--"));

		/* note, callback usually updates @next_callback_time */
		എങ്കിൽ (timerisset(&pty->next_callback_time)) {
			ഘടന timeval now = { 0 };

			DBG_OBJ(IO, pty, ul_debug(" callback requested"));
			gettime_monotonic(&now);
			എങ്കിൽ (timercmp(&now, &pty->next_callback_time, >)) {
				rc = mainloop_callback(pty);
				എങ്കിൽ (rc)
					നിർത്തുക;
			}
		}

		/* set timeout */
		എങ്കിൽ (timerisset(&pty->next_callback_time)) {
			ഘടന timeval now = { 0 }, rest = { 0 };

			gettime_monotonic(&now);
			timersub(&pty->next_callback_time, &now, &rest);
			timeout = (rest.tv_sec * MSEC_PER_SEC) +  (rest.tv_usec / USEC_PER_MSEC);
		} അല്ലെങ്കിൽ
			timeout = pty->poll_timeout;

		/* use POLLOUT (aka "writing is now possible") if data queued */
		എങ്കിൽ (pty->child_buffer_head)
			pfd[POLLFD_MASTER].events |= POLLOUT;
		അല്ലെങ്കിൽ
			pfd[POLLFD_MASTER].events &= ~POLLOUT;

		/* wait for input, signal or timeout */
		DBG_OBJ(IO, pty, ul_debug("calling poll() [timeout=%dms]", timeout));
		ret = poll(pfd, ARRAY_SIZE(pfd), timeout);

		errsv = errno;
		DBG_OBJ(IO, pty, ul_debug("poll() rc=%d", ret));

		/* error */
		എങ്കിൽ (ret < 0) {
			എങ്കിൽ (errsv == EAGAIN)
				തുടരുക;
			rc = -errno;
			നിർത്തുക;
		}

		/* timeout */
		എങ്കിൽ (ret == 0) {
			എങ്കിൽ (timerisset(&pty->next_callback_time)) {
				rc = mainloop_callback(pty);
				എങ്കിൽ (rc == 0)
					തുടരുക;
			} അല്ലെങ്കിൽ {
				rc = 0;
			}

			DBG_OBJ(IO, pty, ul_debug("leaving poll() loop [timeout=%d, rc=%d]", timeout, rc));
			നിർത്തുക;
		}
		/* event */
		വേണ്ടി (i = 0; i < ARRAY_SIZE(pfd); i++) {
			എങ്കിൽ (pfd[i].revents == 0)
				തുടരുക;

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

			എങ്കിൽ (i == POLLFD_SIGNAL)
				rc = handle_signal(pty, pfd[i].fd);
			അല്ലെങ്കിൽ {
				എങ്കിൽ (pfd[i].revents & POLLIN)
					rc = handle_io(pty, pfd[i].fd, &eof); /* data */
				എങ്കിൽ (pfd[i].revents & POLLOUT) /* i == POLLFD_MASTER */
					rc = flush_child_buffers(pty, ശൂന്യ);
			}

			എങ്കിൽ (rc) {
				പൂർണ്ണസംഖ്യ anything = 1;

				ul_pty_write_eof_to_child(pty);
				വേണ്ടി (anything = 1; anything;)
					flush_child_buffers(pty, &anything);
				നിർത്തുക;
			}

			എങ്കിൽ (i == POLLFD_SIGNAL)
				തുടരുക;

			/* EOF maybe detected in two ways; they are as follows:
			 *	A) poll() return POLLHUP event after close()
			 *	B) read() returns 0 (no data)
			 *
			 * POLLNVAL means that fd is closed.
			 */
			എങ്കിൽ ((pfd[i].revents & POLLHUP) || (pfd[i].revents & POLLNVAL) || eof) {
				DBG_OBJ(IO, pty, ul_debug(" ignore FD"));
				എങ്കിൽ (i == POLLFD_STDIN) {
					pfd[i].fd = -1;
					ul_pty_write_eof_to_child(pty);
				} അല്ലെങ്കിൽ /* i == POLLFD_MASTER */
					pfd[i].revents &= ~POLLIN;
			}
		}
		എങ്കിൽ (rc)
			നിർത്തുക;
	}

	എങ്കിൽ (rc && pty->child && pty->child != (pid_t) -1 && !pty->delivered_signal) {
		kill(pty->child, SIGTERM);
		sleep(2);
		kill(pty->child, SIGKILL);
	}

	pty_signals_cleanup(pty);

	DBG(IO, ul_debug("poll() done [signal=%d, rc=%d]", pty->delivered_signal, rc));
	തിരികെ rc;
}

#ifdef TEST_PROGRAM_PTY
/*
 * $ make test_pty
 * $ ./test_pty
 *
 * ... and see for example tty(1) or "ps afu"
 */
സ്ഥിതം ശൂന്യം child_sigstop(ശൂന്യം *data __attribute__((__unused__)), pid_t child)
{
	kill(getpid(), SIGSTOP);
	kill(child, SIGCONT);
}

പൂർണ്ണസംഖ്യ പ്രധാന(പൂർണ്ണസംഖ്യ argc, അക്ഷരം *argv[])
{
	ഘടന ul_pty_callbacks *cb;
	സ്ഥിരം അക്ഷരം *shell, *command = ശൂന്യ, *shname = ശൂന്യ;
	പൂർണ്ണസംഖ്യ caught_signal = 0;
	pid_t child;
	ഘടന ul_pty *pty;

	shell = getenv("SHELL");
	എങ്കിൽ (shell == ശൂന്യ)
		shell = _PATH_BSHELL;
	എങ്കിൽ (argc == 2)
		command = argv[1];

	ul_pty_init_debug(0);

	pty = ul_new_pty(isatty(STDIN_FILENO));
	എങ്കിൽ (!pty)
		err(EXIT_FAILURE, "failed to allocate PTY handler");

	cb = ul_pty_get_callbacks(pty);
	cb->child_sigstop = child_sigstop;

	എങ്കിൽ (ul_pty_setup(pty))
		err(EXIT_FAILURE, "failed to create pseudo-terminal");
	എങ്കിൽ (ul_pty_signals_setup(pty))
		err(EXIT_FAILURE, "failed to initialize signals handler");
	എങ്കിൽ (ul_pty_terminal_setup(pty))
		err(EXIT_FAILURE, "failed to setup terminal");

	fflush(stdout);			/* ??? */

	തിരഞ്ഞെടുപ്പ് ((പൂർണ്ണസംഖ്യ) (child = fork())) {
	അവസ്ഥ -1: /* error */
		ul_pty_cleanup(pty);
		err(EXIT_FAILURE, "cannot create child process");
		നിർത്തുക;

	അവസ്ഥ 0: /* child */
		ul_pty_init_slave(pty);

		സൂചന(SIGTERM, SIG_DFL); /* because /etc/csh.login */

		shname = strrchr(shell, '/');
		shname = shname ? shname + 1 : shell;

		എങ്കിൽ (command)
			execl(shell, shname, "-c", command, (അക്ഷരം *)ശൂന്യ);
		അല്ലെങ്കിൽ
			execl(shell, shname, "-i", (അക്ഷരം *)ശൂന്യ);
		err(EXIT_FAILURE, "failed to execute %s", shell);
		നിർത്തുക;

	സ്ഥിരസ്ഥിതി:
		നിർത്തുക;
	}

	/* parent */
	ul_pty_set_child(pty, child);

	/* this is the main loop */
	ul_pty_proxy_master(pty);

	/* all done; cleanup and kill */
	caught_signal = ul_pty_get_delivered_signal(pty);

	എങ്കിൽ (!caught_signal && ul_pty_get_child(pty) != (pid_t)-1)
		ul_pty_wait_for_child(pty);	/* final wait */

	എങ്കിൽ (caught_signal && ul_pty_get_child(pty) != (pid_t)-1) {
		fprintf(stderr, "\nSession terminated, killing shell...");
		kill(child, SIGTERM);
		sleep(2);
		kill(child, SIGKILL);
		fprintf(stderr, " ...killed.\n");
	}

	ul_pty_cleanup(pty);
	ul_free_pty(pty);
	തിരികെ EXIT_SUCCESS;
}

#endif /* TEST_PROGRAM */

