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
#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <pty.h>
#ଅନ୍ତର୍ଭାଵ <poll.h>
#ଅନ୍ତର୍ଭାଵ <sys/signalfd.h>
#ଅନ୍ତର୍ଭାଵ <paths.h>
#ଅନ୍ତର୍ଭାଵ <sys/types.h>
#ଅନ୍ତର୍ଭାଵ <sys/wait.h>
#ଅନ୍ତର୍ଭାଵ <inttypes.h>

#ଅନ୍ତର୍ଭାଵ "c.h"
#ଅନ୍ତର୍ଭାଵ "all-io.h"
#ଅନ୍ତର୍ଭାଵ "ttyutils.h"
#ଅନ୍ତର୍ଭାଵ "pty-session.h"
#ଅନ୍ତର୍ଭାଵ "monotonic.h"
#ଅନ୍ତର୍ଭାଵ "debug.h"

ସ୍ଥିର UL_DEBUG_DEFINE_MASK(ulpty);
UL_DEBUG_DEFINE_MASKNAMES(ulpty) = UL_DEBUG_EMPTY_MASKNAMES;

#define ULPTY_DEBUG_INIT	(1 << 1)
#define ULPTY_DEBUG_SETUP	(1 << 2)
#define ULPTY_DEBUG_SIG		(1 << 3)
#define ULPTY_DEBUG_IO		(1 << 4)
#define ULPTY_DEBUG_DONE	(1 << 5)

#define DBG(m, x)		__UL_DBG(ulpty, ULPTY_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulpty, ULPTY_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(ulpty, ULPTY_DEBUG_, m, x)

ଶୂନ୍ୟ ul_pty_init_debug(ପୂର୍ଣ୍ଣସଂଖ୍ୟା mask)
{
	ଯଦି (ulpty_debug_mask)
		ଫେରନ୍ତୁ;
	__UL_INIT_DEBUG_FROM_ENV(ulpty, ULPTY_DEBUG_, mask, ULPTY_DEBUG);
}

ସଂରଚନା ul_pty *ul_new_pty(ପୂର୍ଣ୍ଣସଂଖ୍ୟା is_stdin_tty)
{
	ସଂରଚନା ul_pty *pty = calloc(1, ଆକାର(*pty));

	ଯଦି (!pty)
		ଫେରନ୍ତୁ ଖାଲି;

	DBG_OBJ(SETUP, pty, ul_debug("alloc handler"));
	pty->isterm = is_stdin_tty;
	pty->master = -1;
	pty->slave = -1;
	pty->sigfd = -1;
	pty->child = (pid_t) -1;

	ଫେରନ୍ତୁ pty;
}

ଶୂନ୍ୟ ul_free_pty(ସଂରଚନା ul_pty *pty)
{
	ସଂରଚନା ul_pty_child_buffer *hd;

	ଯେତେବେଳେ ((hd = pty->child_buffer_head)) {
		pty->child_buffer_head = hd->next;
		ମୁକ୍ତ(hd);
	}

	ଯେତେବେଳେ ((hd = pty->free_buffers)) {
		pty->free_buffers = hd->next;
		ମୁକ୍ତ(hd);
	}
	ମୁକ୍ତ(pty);
}

ଶୂନ୍ୟ ul_pty_slave_echo(ସଂରଚନା ul_pty *pty, ପୂର୍ଣ୍ଣସଂଖ୍ୟା enable)
{
	ପ୍ରତିଜ୍ଞା(pty);
	pty->slave_echo = enable ? 1 : 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_pty_get_delivered_signal(ସଂରଚନା ul_pty *pty)
{
	ପ୍ରତିଜ୍ଞା(pty);
	ଫେରନ୍ତୁ pty->delivered_signal;
}

ସଂରଚନା ul_pty_callbacks *ul_pty_get_callbacks(ସଂରଚନା ul_pty *pty)
{
	ପ୍ରତିଜ୍ଞା(pty);
	ଫେରନ୍ତୁ &pty->callbacks;
}

ଶୂନ୍ୟ ul_pty_set_callback_data(ସଂରଚନା ul_pty *pty, ଶୂନ୍ୟ *data)
{
	ପ୍ରତିଜ୍ଞା(pty);
	pty->callback_data = data;
}

ଶୂନ୍ୟ ul_pty_set_child(ସଂରଚନା ul_pty *pty, pid_t child)
{
	ପ୍ରତିଜ୍ଞା(pty);
	pty->child = child;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_pty_get_childfd(ସଂରଚନା ul_pty *pty)
{
	ପ୍ରତିଜ୍ଞା(pty);
	ଫେରନ୍ତୁ pty->master;
}

pid_t ul_pty_get_child(ସଂରଚନା ul_pty *pty)
{
	ପ୍ରତିଜ୍ଞା(pty);
	ଫେରନ୍ତୁ pty->child;
}

/* it's active when signals are redirected to sigfd */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_pty_is_running(ସଂରଚନା ul_pty *pty)
{
	ପ୍ରତିଜ୍ଞା(pty);
	ଫେରନ୍ତୁ pty->sigfd >= 0;
}

ଶୂନ୍ୟ ul_pty_set_mainloop_time(ସଂରଚନା ul_pty *pty, ସଂରଚନା timeval *tv)
{
	ପ୍ରତିଜ୍ଞା(pty);
	ଯଦି (!tv) {
		DBG_OBJ(IO, pty, ul_debug("mainloop time: clear"));
		timerclear(&pty->next_callback_time);
	} ଅନ୍ୟଥା {
		pty->next_callback_time.tv_sec = tv->tv_sec;
		pty->next_callback_time.tv_usec = tv->tv_usec;
		DBG_OBJ(IO, pty, ul_debug("mainloop time: %"PRId64".%06"PRId64,
				(int64_t) tv->tv_sec, (int64_t) tv->tv_usec));
	}
}

ସ୍ଥିର ଶୂନ୍ୟ pty_signals_cleanup(ସଂରଚନା ul_pty *pty)
{
	ଯଦି (pty->sigfd != -1)
		close(pty->sigfd);
	pty->sigfd = -1;

	/* restore original setting */
	sigprocmask(SIG_SETMASK, &pty->orgsig, ଖାଲି);
}

/* call me before fork() */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_pty_setup(ସଂରଚନା ul_pty *pty)
{
	ସଂରଚନା termios attrs;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = 0;

	ପ୍ରତିଜ୍ଞା(pty->sigfd == -1);

	/* save the current signals setting (to make ul_pty_cleanup() usable,
	 * otherwise see ul_pty_signals_setup() */
	sigprocmask(0, ଖାଲି, &pty->orgsig);

	ଯଦି (pty->isterm) {
		DBG_OBJ(SETUP, pty, ul_debug("create for terminal"));

		/* original setting of the current terminal */
		ଯଦି (tcgetattr(STDIN_FILENO, &pty->stdin_attrs) != 0) {
			rc = -errno;
			ଯାଅ done;
		}

		attrs = pty->stdin_attrs;
		ଯଦି (pty->slave_echo)
			attrs.c_lflag |= ECHO;
		ଅନ୍ୟଥା
			attrs.c_lflag &= ~ECHO;

		ioctl(STDIN_FILENO, TIOCGWINSZ, (ଅକ୍ଷର *)&pty->win);
		/* create master+slave */
		rc = openpty(&pty->master, &pty->slave, ଖାଲି, &attrs, &pty->win);
		ଯଦି (rc)
			ଯାଅ done;

	} ଅନ୍ୟଥା {
		DBG_OBJ(SETUP, pty, ul_debug("create for non-terminal"));

		rc = openpty(&pty->master, &pty->slave, ଖାଲି, ଖାଲି, ଖାଲି);
		ଯଦି (rc)
			ଯାଅ done;

		tcgetattr(pty->slave, &attrs);

		ଯଦି (pty->slave_echo)
			attrs.c_lflag |= ECHO;
		ଅନ୍ୟଥା
			attrs.c_lflag &= ~ECHO;

		tcsetattr(pty->slave, TCSANOW, &attrs);
	}

	fcntl(pty->master, F_SETFL, O_NONBLOCK);

done:
	ଯଦି (rc)
		ul_pty_cleanup(pty);

	DBG_OBJ(SETUP, pty, ul_debug("pty setup done [master=%d, slave=%d, rc=%d]",
				pty->master, pty->slave, rc));
	ଫେରନ୍ତୁ rc;
}

/* call me after ul_pty_signals_setup() and before fork() */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_pty_terminal_setup(ସଂରଚନା ul_pty *pty)
{
	ସଂରଚନା termios attrs;

	ଯଦି (!pty->isterm)
		ଫେରନ୍ତୁ 0;

	attrs = pty->stdin_attrs;
	cfmakeraw(&attrs);
	ଯଦି (tcsetattr(STDIN_FILENO, TCSANOW, &attrs))
		ଫେରନ୍ତୁ -errno;

	DBG_OBJ(SETUP, pty, ul_debug("terminal raw mode setup done"));
	ଫେରନ୍ତୁ 0;
}

/* call me before fork() */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_pty_signals_setup(ସଂରଚନା ul_pty *pty)
{
	sigset_t ourset;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = 0;

	ପ୍ରତିଜ୍ଞା(pty->sigfd == -1);

	/* save the current signals setting */
	sigprocmask(0, ଖାଲି, &pty->orgsig);

	sigfillset(&ourset);
	ଯଦି (sigprocmask(SIG_BLOCK, &ourset, ଖାଲି)) {
		rc = -errno;
		ଯାଅ done;
	}

	sigemptyset(&ourset);
	sigaddset(&ourset, SIGCHLD);
	sigaddset(&ourset, SIGWINCH);
	sigaddset(&ourset, SIGALRM);
	sigaddset(&ourset, SIGHUP);
	sigaddset(&ourset, SIGTERM);
	sigaddset(&ourset, SIGINT);
	sigaddset(&ourset, SIGQUIT);

	ଯଦି (pty->callbacks.flush_logs)
		sigaddset(&ourset, SIGUSR1);

	ଯଦି ((pty->sigfd = signalfd(-1, &ourset, SFD_CLOEXEC)) < 0)
		rc = -errno;
done:
	ଯଦି (rc)
		ul_pty_cleanup(pty);

	DBG_OBJ(SETUP, pty, ul_debug("pty signals setup done [rc=%d]", rc));
	ଫେରନ୍ତୁ rc;
}

/* cleanup in parent process */
ଶୂନ୍ୟ ul_pty_cleanup(ସଂରଚନା ul_pty *pty)
{
	ସଂରଚନା termios rtt;

	pty_signals_cleanup(pty);

	ଯଦି (pty->master == -1 || !pty->isterm)
		ଫେରନ୍ତୁ;

	DBG_OBJ(DONE, pty, ul_debug("cleanup"));
	rtt = pty->stdin_attrs;
	tcsetattr(STDIN_FILENO, TCSADRAIN, &rtt);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_pty_chownmod_slave(ସଂରଚନା ul_pty *pty, uid_t uid, gid_t gid, mode_t mode)
{
	ଯଦି (fchown(pty->slave, uid, gid))
		ଫେରନ୍ତୁ -errno;
	ଯଦି (fchmod(pty->slave, mode))
		ଫେରନ୍ତୁ -errno;
	ଫେରନ୍ତୁ 0;
}

/* call me in child process */
ଶୂନ୍ୟ ul_pty_init_slave(ସଂରଚନା ul_pty *pty)
{
	DBG_OBJ(SETUP, pty, ul_debug("initialize slave"));

	setsid();

	ioctl(pty->slave, TIOCSCTTY, 1);
	close(pty->master);

	dup2(pty->slave, STDIN_FILENO);
	dup2(pty->slave, STDOUT_FILENO);
	dup2(pty->slave, STDERR_FILENO);

	close(pty->slave);

	ଯଦି (pty->sigfd >= 0)
		close(pty->sigfd);

	pty->slave = -1;
	pty->master = -1;
	pty->sigfd = -1;

	sigprocmask(SIG_SETMASK, &pty->orgsig, ଖାଲି);

	DBG_OBJ(SETUP, pty, ul_debug("... initialize slave done"));
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା write_output(ଅକ୍ଷର *obuf, ssize_t bytes)
{
	DBG(IO, ul_debug(" writing output"));

	ଯଦି (ul_write_all(STDOUT_FILENO, obuf, bytes)) {
		DBG(IO, ul_debug("  writing output *failed*"));
		ଫେରନ୍ତୁ -errno;
	}

	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା schedule_child_write(ସଂରଚନା ul_pty *pty, ଅକ୍ଷର *buf, size_t bufsz, ପୂର୍ଣ୍ଣସଂଖ୍ୟା final)
{
	ସଂରଚନା ul_pty_child_buffer *stash;

	ଯଦି (pty->free_buffers) {
		stash = pty->free_buffers;
		pty->free_buffers = stash->next;
		memset(stash, 0, ଆକାର(*stash));
	} ଅନ୍ୟଥା
		stash = calloc(1, ଆକାର(*stash));
	ଯଦି (!stash)
		ଫେରନ୍ତୁ -1;

	ପ୍ରତିଜ୍ଞା(bufsz <= ଆକାର(stash->buf));

	memcpy(stash->buf, buf, bufsz);
	stash->size = bufsz;
	stash->final_ନିବେଶ = final ? 1 : 0;

	ଯଦି (pty->child_buffer_head)
		pty->child_buffer_tail = pty->child_buffer_tail->next = stash;
	ଅନ୍ୟଥା
		pty->child_buffer_head = pty->child_buffer_tail = stash;
	ଫେରନ୍ତୁ 0;
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
ସ୍ଥିର ଶୂନ୍ୟ drain_child_buffers(ସଂରଚନା ul_pty *pty)
{
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା tries = 0;
	ସଂରଚନା pollfd fd = { .fd = pty->slave, .events = POLLIN };

	DBG_OBJ(IO, pty, ul_debug(" waiting for empty slave"));
	ଯେତେବେଳେ (poll(&fd, 1, 10) == 1 && tries < 8) {
		DBG_OBJ(IO, pty, ul_debug("   slave is not empty"));
		xusleep(250000);
		tries++;
	}
	ଯଦି (tries < 8)
		DBG_OBJ(IO, pty, ul_debug("   slave is empty now"));

	DBG_OBJ(IO, pty, ul_debug(" sending EOF to master"));
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା flush_child_buffers(ସଂରଚନା ul_pty *pty, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *anything)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = 0, any = 0;

	ଯେତେବେଳେ (pty->child_buffer_head) {
		ସଂରଚନା ul_pty_child_buffer *hd = pty->child_buffer_head;
		ssize_t ret;

		ଯଦି (hd->final_ନିବେଶ)
			drain_child_buffers(pty);

		DBG_OBJ(IO, hd, ul_debug(" stdin --> master trying %zu bytes", hd->size - hd->cursor));

		ret = ଲିଖ(pty->master, hd->buf + hd->cursor, hd->size - hd->cursor);
		ଯଦି (ret == -1) {
			DBG_OBJ(IO, hd, ul_debug("   EAGAIN"));
			ଯଦି (!(errno == EINTR || errno == EAGAIN))
				rc = -errno;
			ଯାଅ out;
		}
		DBG_OBJ(IO, hd, ul_debug("   wrote %zd", ret));
		any = 1;
		hd->cursor += ret;

		ଯଦି (hd->cursor == hd->size) {
			pty->child_buffer_head = hd->next;
			ଯଦି (!hd->next)
				pty->child_buffer_tail = ଖାଲି;

			hd->next = pty->free_buffers;
			pty->free_buffers = hd;
		}
	}
out:
	/* without sync write_output() will write both input &
	 * shell output that looks like double echoing */
	ଯଦି (any)
		fdatasync(pty->master);

	ଯଦି (anything)
		*anything = any;
	ଫେରନ୍ତୁ rc;
}

ଶୂନ୍ୟ ul_pty_write_eof_to_child(ସଂରଚନା ul_pty *pty)
{
	ଅକ୍ଷର c = DEF_EOF;
	schedule_child_write(pty, &c, ଆକାର(ଅକ୍ଷର), 1);
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା mainloop_callback(ସଂରଚନା ul_pty *pty)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

	ଯଦି (!pty->callbacks.mainloop)
		ଫେରନ୍ତୁ 0;

	DBG_OBJ(IO, pty, ul_debug("calling mainloop callback"));
	rc = pty->callbacks.mainloop(pty->callback_data);

	DBG_OBJ(IO, pty, ul_debug(" callback done [rc=%d]", rc));
	ଫେରନ୍ତୁ rc;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା handle_io(ସଂରଚନା ul_pty *pty, ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *eof)
{
	ଅକ୍ଷର buf[BUFSIZ];
	ssize_t bytes;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = 0;
	sigset_t set;

	DBG_OBJ(IO, pty, ul_debug(" handle I/O on fd=%d", fd));
	*eof = 0;

	sigemptyset(&set);
	sigaddset(&set, SIGTTIN);
	sigprocmask(SIG_UNBLOCK, &set, ଖାଲି);
	/* read from active FD */
	bytes = ପଠ(fd, buf, ଆକାର(buf));
	sigprocmask(SIG_BLOCK, &set, ଖାଲି);
	ଯଦି (bytes == -1) {
		ଯଦି (errno == EAGAIN || errno == EINTR)
			ଫେରନ୍ତୁ 0;
		ଫେରନ୍ତୁ -errno;
	}

	ଯଦି (bytes == 0) {
		*eof = 1;
		ଫେରନ୍ତୁ 0;
	}

	/* from stdin (user) to command */
	ଯଦି (fd == STDIN_FILENO) {
		DBG_OBJ(IO, pty, ul_debug(" stdin --> master %zd bytes queued", bytes));

		ଯଦି (schedule_child_write(pty, buf, bytes, 0))
			ଫେରନ୍ତୁ -errno;

	/* from command (master) to stdout */
	} ଅନ୍ୟଥା ଯଦି (fd == pty->master) {
		DBG_OBJ(IO, pty, ul_debug(" master --> stdout %zd bytes", bytes));
		write_output(buf, bytes);
	}

	ଯଦି (pty->callbacks.log_stream_activity)
		rc = pty->callbacks.log_stream_activity(
					pty->callback_data, fd, buf, bytes);

	ଫେରନ୍ତୁ rc;
}

ଶୂନ୍ୟ ul_pty_wait_for_child(ସଂରଚନା ul_pty *pty)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା status;
	pid_t pid;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା options = 0;

	ଯଦି (pty->child == (pid_t) -1)
		ଫେରନ୍ତୁ;

	DBG(SIG, ul_debug("waiting for child [child=%d]", (ପୂର୍ଣ୍ଣସଂଖ୍ୟା) pty->child));

	ଯଦି (ul_pty_is_running(pty)) {
		/* wait for specific child */
		options = WNOHANG;
		ପାଇଁ (;;) {
			pid = waitpid(pty->child, &status, options);
			DBG(SIG, ul_debug(" waitpid done [rc=%d]", (ପୂର୍ଣ୍ଣସଂଖ୍ୟା) pid));
			ଯଦି (pid != (pid_t) - 1) {
				ଯଦି (pty->callbacks.child_die)
					pty->callbacks.child_die(
							pty->callback_data,
							pty->child, status);
				ul_pty_set_child(pty, (pid_t) -1);
			} ଅନ୍ୟଥା
				ଭାଙ୍ଗ;
		}
	} ଅନ୍ୟଥା {
		/* final wait */
		ଯେତେବେଳେ ((pid = waitpid(-1, &status, options)) > 0) {
			DBG(SIG, ul_debug(" waitpid done [rc=%d]", (ପୂର୍ଣ୍ଣସଂଖ୍ୟା) pid));
			ଯଦି (pid == pty->child) {
				ଯଦି (pty->callbacks.child_die)
					pty->callbacks.child_die(
							pty->callback_data,
							pty->child, status);
				ul_pty_set_child(pty, (pid_t) -1);
			}
		}
	}
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା handle_signal(ସଂରଚନା ul_pty *pty, ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd)
{
	ସଂରଚନା signalfd_siginfo info;
	ssize_t bytes;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = 0;

	DBG_OBJ(SIG, pty, ul_debug(" handle signal on fd=%d", fd));

	bytes = ପଠ(fd, &info, ଆକାର(info));
	ଯଦି (bytes != ଆକାର(info)) {
		ଯଦି (bytes < 0 && (errno == EAGAIN || errno == EINTR))
			ଫେରନ୍ତୁ 0;
		ଫେରନ୍ତୁ -errno;
	}

	ବାଛ (info.ssi_signo) {
	ସ୍ଥିତି SIGCHLD:
		DBG_OBJ(SIG, pty, ul_debug(" get signal SIGCHLD"));

		ଯଦି (info.ssi_code == CLD_EXITED
		    || info.ssi_code == CLD_KILLED
		    || info.ssi_code == CLD_DUMPED) {

			ଯଦି (pty->callbacks.child_wait)
				pty->callbacks.child_wait(pty->callback_data,
							  pty->child);
			ଅନ୍ୟଥା
				ul_pty_wait_for_child(pty);

		} ଅନ୍ୟଥା ଯଦି (info.ssi_status == SIGSTOP && pty->child > 0) {
			pty->callbacks.child_sigstop(pty->callback_data,
						     pty->child);
		}

		ଯଦି (pty->child <= 0) {
			DBG_OBJ(SIG, pty, ul_debug(" no child, setting leaving timeout"));
			pty->poll_timeout = 10;
			timerclear(&pty->next_callback_time);
		}
		ଫେରନ୍ତୁ 0;
	ସ୍ଥିତି SIGWINCH:
		DBG_OBJ(SIG, pty, ul_debug(" get signal SIGWINCH"));
		ଯଦି (pty->isterm) {
			ioctl(STDIN_FILENO, TIOCGWINSZ, (ଅକ୍ଷର *)&pty->win);
			ioctl(pty->slave, TIOCSWINSZ, (ଅକ୍ଷର *)&pty->win);

			ଯଦି (pty->callbacks.log_signal)
				rc = pty->callbacks.log_signal(pty->callback_data,
							&info, (ଶୂନ୍ୟ *) &pty->win);
		}
		ଭାଙ୍ଗ;
	ସ୍ଥିତି SIGALRM:
		FALLTHROUGH;
	ସ୍ଥିତି SIGHUP:
		FALLTHROUGH;
	ସ୍ଥିତି SIGTERM:
		FALLTHROUGH;
	ସ୍ଥିତି SIGINT:
		FALLTHROUGH;
	ସ୍ଥିତି SIGQUIT:
		DBG_OBJ(SIG, pty, ul_debug(" get signal SIG{ALRM,HUP,TERM,INT,QUIT}"));
		pty->delivered_signal = info.ssi_signo;
		/* Child termination is going to generate SIGCHLD (see above) */
		ଯଦି (pty->child > 0)
			kill(pty->child, SIGTERM);

		ଯଦି (pty->callbacks.log_signal)
			rc = pty->callbacks.log_signal(pty->callback_data,
					&info, (ଶୂନ୍ୟ *) &pty->win);
		ଭାଙ୍ଗ;
	ସ୍ଥିତି SIGUSR1:
		DBG_OBJ(SIG, pty, ul_debug(" get signal SIGUSR1"));
		ଯଦି (pty->callbacks.flush_logs)
			rc = pty->callbacks.flush_logs(pty->callback_data);
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		abort();
	}

	ଫେରନ୍ତୁ rc;
}

/* loop in parent */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ul_pty_proxy_master(ସଂରଚନା ul_pty *pty)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = 0, ret, eof = 0;
	ଗଣନା {
		POLLFD_SIGNAL = 0,
		POLLFD_MASTER,
		POLLFD_STDIN

	};
	ସଂରଚନା pollfd pfd[] = {
		[POLLFD_SIGNAL] = { .fd = -1,		.events = POLLIN | POLLERR | POLLHUP },
		[POLLFD_MASTER] = { .fd = pty->master,  .events = POLLIN | POLLERR | POLLHUP },
		[POLLFD_STDIN]	= { .fd = STDIN_FILENO, .events = POLLIN | POLLERR | POLLHUP }
	};

	/* We use signalfd, and standard signals by handlers are completely blocked */
	ପ୍ରତିଜ୍ଞା(pty->sigfd >= 0);

	pfd[POLLFD_SIGNAL].fd = pty->sigfd;
	pty->poll_timeout = -1;

	ଯେତେବେଳେ (!pty->delivered_signal) {
		size_t i;
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା errsv, timeout;

		DBG_OBJ(IO, pty, ul_debug("--poll() loop--"));

		/* note, callback usually updates @next_callback_time */
		ଯଦି (timerisset(&pty->next_callback_time)) {
			ସଂରଚନା timeval now = { 0 };

			DBG_OBJ(IO, pty, ul_debug(" callback requested"));
			gettime_monotonic(&now);
			ଯଦି (timercmp(&now, &pty->next_callback_time, >)) {
				rc = mainloop_callback(pty);
				ଯଦି (rc)
					ଭାଙ୍ଗ;
			}
		}

		/* set timeout */
		ଯଦି (timerisset(&pty->next_callback_time)) {
			ସଂରଚନା timeval now = { 0 }, rest = { 0 };

			gettime_monotonic(&now);
			timersub(&pty->next_callback_time, &now, &rest);
			timeout = (rest.tv_sec * MSEC_PER_SEC) +  (rest.tv_usec / USEC_PER_MSEC);
		} ଅନ୍ୟଥା
			timeout = pty->poll_timeout;

		/* use POLLOUT (aka "writing is now possible") if data queued */
		ଯଦି (pty->child_buffer_head)
			pfd[POLLFD_MASTER].events |= POLLOUT;
		ଅନ୍ୟଥା
			pfd[POLLFD_MASTER].events &= ~POLLOUT;

		/* wait for input, signal or timeout */
		DBG_OBJ(IO, pty, ul_debug("calling poll() [timeout=%dms]", timeout));
		ret = poll(pfd, ARRAY_SIZE(pfd), timeout);

		errsv = errno;
		DBG_OBJ(IO, pty, ul_debug("poll() rc=%d", ret));

		/* error */
		ଯଦି (ret < 0) {
			ଯଦି (errsv == EAGAIN)
				ଜାରି;
			rc = -errno;
			ଭାଙ୍ଗ;
		}

		/* timeout */
		ଯଦି (ret == 0) {
			ଯଦି (timerisset(&pty->next_callback_time)) {
				rc = mainloop_callback(pty);
				ଯଦି (rc == 0)
					ଜାରି;
			} ଅନ୍ୟଥା {
				rc = 0;
			}

			DBG_OBJ(IO, pty, ul_debug("leaving poll() loop [timeout=%d, rc=%d]", timeout, rc));
			ଭାଙ୍ଗ;
		}
		/* event */
		ପାଇଁ (i = 0; i < ARRAY_SIZE(pfd); i++) {
			ଯଦି (pfd[i].revents == 0)
				ଜାରି;

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

			ଯଦି (i == POLLFD_SIGNAL)
				rc = handle_signal(pty, pfd[i].fd);
			ଅନ୍ୟଥା {
				ଯଦି (pfd[i].revents & POLLIN)
					rc = handle_io(pty, pfd[i].fd, &eof); /* data */
				ଯଦି (pfd[i].revents & POLLOUT) /* i == POLLFD_MASTER */
					rc = flush_child_buffers(pty, ଖାଲି);
			}

			ଯଦି (rc) {
				ପୂର୍ଣ୍ଣସଂଖ୍ୟା anything = 1;

				ul_pty_write_eof_to_child(pty);
				ପାଇଁ (anything = 1; anything;)
					flush_child_buffers(pty, &anything);
				ଭାଙ୍ଗ;
			}

			ଯଦି (i == POLLFD_SIGNAL)
				ଜାରି;

			/* EOF maybe detected in two ways; they are as follows:
			 *	A) poll() return POLLHUP event after close()
			 *	B) read() returns 0 (no data)
			 *
			 * POLLNVAL means that fd is closed.
			 */
			ଯଦି ((pfd[i].revents & POLLHUP) || (pfd[i].revents & POLLNVAL) || eof) {
				DBG_OBJ(IO, pty, ul_debug(" ignore FD"));
				ଯଦି (i == POLLFD_STDIN) {
					pfd[i].fd = -1;
					ul_pty_write_eof_to_child(pty);
				} ଅନ୍ୟଥା /* i == POLLFD_MASTER */
					pfd[i].revents &= ~POLLIN;
			}
		}
		ଯଦି (rc)
			ଭାଙ୍ଗ;
	}

	ଯଦି (rc && pty->child && pty->child != (pid_t) -1 && !pty->delivered_signal) {
		kill(pty->child, SIGTERM);
		sleep(2);
		kill(pty->child, SIGKILL);
	}

	pty_signals_cleanup(pty);

	DBG(IO, ul_debug("poll() done [signal=%d, rc=%d]", pty->delivered_signal, rc));
	ଫେରନ୍ତୁ rc;
}

#ifdef TEST_PROGRAM_PTY
/*
 * $ make test_pty
 * $ ./test_pty
 *
 * ... and see for example tty(1) or "ps afu"
 */
ସ୍ଥିର ଶୂନ୍ୟ child_sigstop(ଶୂନ୍ୟ *data __attribute__((__unused__)), pid_t child)
{
	kill(getpid(), SIGSTOP);
	kill(child, SIGCONT);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା ମୁଖ୍ୟ(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର *argv[])
{
	ସଂରଚନା ul_pty_callbacks *cb;
	ଅଚଳ ଅକ୍ଷର *shell, *command = ଖାଲି, *shname = ଖାଲି;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା caught_signal = 0;
	pid_t child;
	ସଂରଚନା ul_pty *pty;

	shell = getenv("SHELL");
	ଯଦି (shell == ଖାଲି)
		shell = _PATH_BSHELL;
	ଯଦି (argc == 2)
		command = argv[1];

	ul_pty_init_debug(0);

	pty = ul_new_pty(isatty(STDIN_FILENO));
	ଯଦି (!pty)
		err(EXIT_FAILURE, "failed to allocate PTY handler");

	cb = ul_pty_get_callbacks(pty);
	cb->child_sigstop = child_sigstop;

	ଯଦି (ul_pty_setup(pty))
		err(EXIT_FAILURE, "failed to create pseudo-terminal");
	ଯଦି (ul_pty_signals_setup(pty))
		err(EXIT_FAILURE, "failed to initialize signals handler");
	ଯଦି (ul_pty_terminal_setup(pty))
		err(EXIT_FAILURE, "failed to setup terminal");

	fflush(stdout);			/* ??? */

	ବାଛ ((ପୂର୍ଣ୍ଣସଂଖ୍ୟା) (child = fork())) {
	ସ୍ଥିତି -1: /* error */
		ul_pty_cleanup(pty);
		err(EXIT_FAILURE, "cannot create child process");
		ଭାଙ୍ଗ;

	ସ୍ଥିତି 0: /* child */
		ul_pty_init_slave(pty);

		ସଙ୍କେତ(SIGTERM, SIG_DFL); /* because /etc/csh.login */

		shname = strrchr(shell, '/');
		shname = shname ? shname + 1 : shell;

		ଯଦି (command)
			execl(shell, shname, "-c", command, (ଅକ୍ଷର *)ଖାଲି);
		ଅନ୍ୟଥା
			execl(shell, shname, "-i", (ଅକ୍ଷର *)ଖାଲି);
		err(EXIT_FAILURE, "failed to execute %s", shell);
		ଭାଙ୍ଗ;

	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଭାଙ୍ଗ;
	}

	/* parent */
	ul_pty_set_child(pty, child);

	/* this is the main loop */
	ul_pty_proxy_master(pty);

	/* all done; cleanup and kill */
	caught_signal = ul_pty_get_delivered_signal(pty);

	ଯଦି (!caught_signal && ul_pty_get_child(pty) != (pid_t)-1)
		ul_pty_wait_for_child(pty);	/* final wait */

	ଯଦି (caught_signal && ul_pty_get_child(pty) != (pid_t)-1) {
		fprintf(stderr, "\nSession terminated, killing shell...");
		kill(child, SIGTERM);
		sleep(2);
		kill(child, SIGKILL);
		fprintf(stderr, " ...killed.\n");
	}

	ul_pty_cleanup(pty);
	ul_free_pty(pty);
	ଫେରନ୍ତୁ EXIT_SUCCESS;
}

#endif /* TEST_PROGRAM */

