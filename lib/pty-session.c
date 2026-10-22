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
#include <stdio.h>
#include <stdlib.h>
#include <pty.h>
#include <poll.h>
#include <sys/signalfd.h>
#include <paths.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <inttypes.h>

#include "c.h"
#include "all-io.h"
#include "ttyutils.h"
#include "pty-session.h"
#include "monotonic.h"
#include "debug.h"

static UL_DEBUG_DEFINE_MASK(ulpty);
UL_DEBUG_DEFINE_MASKNAMES(ulpty) = UL_DEBUG_EMPTY_MASKNAMES;

#define ULPTY_DEBUG_INIT	(1 << 1)
#define ULPTY_DEBUG_SETUP	(1 << 2)
#define ULPTY_DEBUG_SIG		(1 << 3)
#define ULPTY_DEBUG_IO		(1 << 4)
#define ULPTY_DEBUG_DONE	(1 << 5)

#define DBG(m, x)		__UL_DBG(ulpty, ULPTY_DEBUG_, m, x)
#define DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulpty, ULPTY_DEBUG_, m, h, x)
#define ON_DBG(m, x)		__UL_DBG_CALL(ulpty, ULPTY_DEBUG_, m, x)

void ul_pty_init_debug(ຈຳນວນເຕັມ mask)
{
	ຖ້າ (ulpty_debug_mask)
		ສົ່ງຄືນ;
	__UL_INIT_DEBUG_FROM_ENV(ulpty, ULPTY_DEBUG_, mask, ULPTY_DEBUG);
}

sa_nracanaa ul_pty *ul_new_pty(ຈຳນວນເຕັມ is_stdin_tty)
{
	sa_nracanaa ul_pty *pty = calloc(1, parimaa_na(*pty));

	ຖ້າ (!pty)
		ສົ່ງຄືນ NULL;

	DBG_OBJ(SETUP, pty, ul_debug("alloc handler"));
	pty->isterm = is_stdin_tty;
	pty->master = -1;
	pty->slave = -1;
	pty->sigfd = -1;
	pty->child = (pid_t) -1;

	ສົ່ງຄືນ pty;
}

void ul_free_pty(sa_nracanaa ul_pty *pty)
{
	sa_nracanaa ul_pty_child_buffer *hd;

	ໃນຂະນະທີ່ ((hd = pty->child_buffer_head)) {
		pty->child_buffer_head = hd->next;
		free(hd);
	}

	ໃນຂະນະທີ່ ((hd = pty->free_buffers)) {
		pty->free_buffers = hd->next;
		free(hd);
	}
	free(pty);
}

void ul_pty_slave_echo(sa_nracanaa ul_pty *pty, ຈຳນວນເຕັມ enable)
{
	assert(pty);
	pty->slave_echo = enable ? 1 : 0;
}

ຈຳນວນເຕັມ ul_pty_get_delivered_signal(sa_nracanaa ul_pty *pty)
{
	assert(pty);
	ສົ່ງຄືນ pty->delivered_signal;
}

sa_nracanaa ul_pty_callbacks *ul_pty_get_callbacks(sa_nracanaa ul_pty *pty)
{
	assert(pty);
	ສົ່ງຄືນ &pty->callbacks;
}

void ul_pty_set_callback_data(sa_nracanaa ul_pty *pty, void *data)
{
	assert(pty);
	pty->callback_data = data;
}

void ul_pty_set_child(sa_nracanaa ul_pty *pty, pid_t child)
{
	assert(pty);
	pty->child = child;
}

ຈຳນວນເຕັມ ul_pty_get_childfd(sa_nracanaa ul_pty *pty)
{
	assert(pty);
	ສົ່ງຄືນ pty->master;
}

pid_t ul_pty_get_child(sa_nracanaa ul_pty *pty)
{
	assert(pty);
	ສົ່ງຄືນ pty->child;
}

/* it's active when signals are redirected to sigfd */
ຈຳນວນເຕັມ ul_pty_is_running(sa_nracanaa ul_pty *pty)
{
	assert(pty);
	ສົ່ງຄືນ pty->sigfd >= 0;
}

void ul_pty_set_mainloop_time(sa_nracanaa ul_pty *pty, sa_nracanaa timeval *tv)
{
	assert(pty);
	ຖ້າ (!tv) {
		DBG_OBJ(IO, pty, ul_debug("mainloop time: clear"));
		timerclear(&pty->next_callback_time);
	} ບໍ່ດັ່ງນັ້ນ {
		pty->next_callback_time.tv_sec = tv->tv_sec;
		pty->next_callback_time.tv_usec = tv->tv_usec;
		DBG_OBJ(IO, pty, ul_debug("mainloop time: %"PRId64".%06"PRId64,
				(int64_t) tv->tv_sec, (int64_t) tv->tv_usec));
	}
}

static void pty_signals_cleanup(sa_nracanaa ul_pty *pty)
{
	ຖ້າ (pty->sigfd != -1)
		close(pty->sigfd);
	pty->sigfd = -1;

	/* restore original setting */
	sigprocmask(SIG_SETMASK, &pty->orgsig, NULL);
}

/* call me before fork() */
ຈຳນວນເຕັມ ul_pty_setup(sa_nracanaa ul_pty *pty)
{
	sa_nracanaa termios attrs;
	ຈຳນວນເຕັມ rc = 0;

	assert(pty->sigfd == -1);

	/* save the current signals setting (to make ul_pty_cleanup() usable,
	 * otherwise see ul_pty_signals_setup() */
	sigprocmask(0, NULL, &pty->orgsig);

	ຖ້າ (pty->isterm) {
		DBG_OBJ(SETUP, pty, ul_debug("create for terminal"));

		/* original setting of the current terminal */
		ຖ້າ (tcgetattr(STDIN_FILENO, &pty->stdin_attrs) != 0) {
			rc = -errno;
			goto done;
		}

		attrs = pty->stdin_attrs;
		ຖ້າ (pty->slave_echo)
			attrs.c_lflag |= ECHO;
		ບໍ່ດັ່ງນັ້ນ
			attrs.c_lflag &= ~ECHO;

		ioctl(STDIN_FILENO, TIOCGWINSZ, (var_na *)&pty->win);
		/* create master+slave */
		rc = openpty(&pty->master, &pty->slave, NULL, &attrs, &pty->win);
		ຖ້າ (rc)
			goto done;

	} ບໍ່ດັ່ງນັ້ນ {
		DBG_OBJ(SETUP, pty, ul_debug("create for non-terminal"));

		rc = openpty(&pty->master, &pty->slave, NULL, NULL, NULL);
		ຖ້າ (rc)
			goto done;

		tcgetattr(pty->slave, &attrs);

		ຖ້າ (pty->slave_echo)
			attrs.c_lflag |= ECHO;
		ບໍ່ດັ່ງນັ້ນ
			attrs.c_lflag &= ~ECHO;

		tcsetattr(pty->slave, TCSANOW, &attrs);
	}

	fcntl(pty->master, F_SETFL, O_NONBLOCK);

done:
	ຖ້າ (rc)
		ul_pty_cleanup(pty);

	DBG_OBJ(SETUP, pty, ul_debug("pty setup done [master=%d, slave=%d, rc=%d]",
				pty->master, pty->slave, rc));
	ສົ່ງຄືນ rc;
}

/* call me after ul_pty_signals_setup() and before fork() */
ຈຳນວນເຕັມ ul_pty_terminal_setup(sa_nracanaa ul_pty *pty)
{
	sa_nracanaa termios attrs;

	ຖ້າ (!pty->isterm)
		ສົ່ງຄືນ 0;

	attrs = pty->stdin_attrs;
	cfmakeraw(&attrs);
	ຖ້າ (tcsetattr(STDIN_FILENO, TCSANOW, &attrs))
		ສົ່ງຄືນ -errno;

	DBG_OBJ(SETUP, pty, ul_debug("terminal raw mode setup done"));
	ສົ່ງຄືນ 0;
}

/* call me before fork() */
ຈຳນວນເຕັມ ul_pty_signals_setup(sa_nracanaa ul_pty *pty)
{
	sigset_t ourset;
	ຈຳນວນເຕັມ rc = 0;

	assert(pty->sigfd == -1);

	/* save the current signals setting */
	sigprocmask(0, NULL, &pty->orgsig);

	sigfillset(&ourset);
	ຖ້າ (sigprocmask(SIG_BLOCK, &ourset, NULL)) {
		rc = -errno;
		goto done;
	}

	sigemptyset(&ourset);
	sigaddset(&ourset, SIGCHLD);
	sigaddset(&ourset, SIGWINCH);
	sigaddset(&ourset, SIGALRM);
	sigaddset(&ourset, SIGHUP);
	sigaddset(&ourset, SIGTERM);
	sigaddset(&ourset, SIGINT);
	sigaddset(&ourset, SIGQUIT);

	ຖ້າ (pty->callbacks.flush_logs)
		sigaddset(&ourset, SIGUSR1);

	ຖ້າ ((pty->sigfd = signalfd(-1, &ourset, SFD_CLOEXEC)) < 0)
		rc = -errno;
done:
	ຖ້າ (rc)
		ul_pty_cleanup(pty);

	DBG_OBJ(SETUP, pty, ul_debug("pty signals setup done [rc=%d]", rc));
	ສົ່ງຄືນ rc;
}

/* cleanup in parent process */
void ul_pty_cleanup(sa_nracanaa ul_pty *pty)
{
	sa_nracanaa termios rtt;

	pty_signals_cleanup(pty);

	ຖ້າ (pty->master == -1 || !pty->isterm)
		ສົ່ງຄືນ;

	DBG_OBJ(DONE, pty, ul_debug("cleanup"));
	rtt = pty->stdin_attrs;
	tcsetattr(STDIN_FILENO, TCSADRAIN, &rtt);
}

ຈຳນວນເຕັມ ul_pty_chownmod_slave(sa_nracanaa ul_pty *pty, uid_t uid, gid_t gid, mode_t mode)
{
	ຖ້າ (fchown(pty->slave, uid, gid))
		ສົ່ງຄືນ -errno;
	ຖ້າ (fchmod(pty->slave, mode))
		ສົ່ງຄືນ -errno;
	ສົ່ງຄືນ 0;
}

/* call me in child process */
void ul_pty_init_slave(sa_nracanaa ul_pty *pty)
{
	DBG_OBJ(SETUP, pty, ul_debug("initialize slave"));

	setsid();

	ioctl(pty->slave, TIOCSCTTY, 1);
	close(pty->master);

	dup2(pty->slave, STDIN_FILENO);
	dup2(pty->slave, STDOUT_FILENO);
	dup2(pty->slave, STDERR_FILENO);

	close(pty->slave);

	ຖ້າ (pty->sigfd >= 0)
		close(pty->sigfd);

	pty->slave = -1;
	pty->master = -1;
	pty->sigfd = -1;

	sigprocmask(SIG_SETMASK, &pty->orgsig, NULL);

	DBG_OBJ(SETUP, pty, ul_debug("... initialize slave done"));
}

static ຈຳນວນເຕັມ write_output(var_na *obuf, ssize_t bytes)
{
	DBG(IO, ul_debug(" writing output"));

	ຖ້າ (ul_write_all(STDOUT_FILENO, obuf, bytes)) {
		DBG(IO, ul_debug("  writing output *failed*"));
		ສົ່ງຄືນ -errno;
	}

	ສົ່ງຄືນ 0;
}

static ຈຳນວນເຕັມ schedule_child_write(sa_nracanaa ul_pty *pty, var_na *buf, size_t bufsz, ຈຳນວນເຕັມ final)
{
	sa_nracanaa ul_pty_child_buffer *stash;

	ຖ້າ (pty->free_buffers) {
		stash = pty->free_buffers;
		pty->free_buffers = stash->next;
		memset(stash, 0, parimaa_na(*stash));
	} ບໍ່ດັ່ງນັ້ນ
		stash = calloc(1, parimaa_na(*stash));
	ຖ້າ (!stash)
		ສົ່ງຄືນ -1;

	assert(bufsz <= parimaa_na(stash->buf));

	memcpy(stash->buf, buf, bufsz);
	stash->size = bufsz;
	stash->final_input = final ? 1 : 0;

	ຖ້າ (pty->child_buffer_head)
		pty->child_buffer_tail = pty->child_buffer_tail->next = stash;
	ບໍ່ດັ່ງນັ້ນ
		pty->child_buffer_head = pty->child_buffer_tail = stash;
	ສົ່ງຄືນ 0;
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
static void drain_child_buffers(sa_nracanaa ul_pty *pty)
{
	unsigned ຈຳນວນເຕັມ tries = 0;
	sa_nracanaa pollfd fd = { .fd = pty->slave, .events = POLLIN };

	DBG_OBJ(IO, pty, ul_debug(" waiting for empty slave"));
	ໃນຂະນະທີ່ (poll(&fd, 1, 10) == 1 && tries < 8) {
		DBG_OBJ(IO, pty, ul_debug("   slave is not empty"));
		xusleep(250000);
		tries++;
	}
	ຖ້າ (tries < 8)
		DBG_OBJ(IO, pty, ul_debug("   slave is empty now"));

	DBG_OBJ(IO, pty, ul_debug(" sending EOF to master"));
}

static ຈຳນວນເຕັມ flush_child_buffers(sa_nracanaa ul_pty *pty, ຈຳນວນເຕັມ *anything)
{
	ຈຳນວນເຕັມ rc = 0, any = 0;

	ໃນຂະນະທີ່ (pty->child_buffer_head) {
		sa_nracanaa ul_pty_child_buffer *hd = pty->child_buffer_head;
		ssize_t ret;

		ຖ້າ (hd->final_input)
			drain_child_buffers(pty);

		DBG_OBJ(IO, hd, ul_debug(" stdin --> master trying %zu bytes", hd->size - hd->cursor));

		ret = write(pty->master, hd->buf + hd->cursor, hd->size - hd->cursor);
		ຖ້າ (ret == -1) {
			DBG_OBJ(IO, hd, ul_debug("   EAGAIN"));
			ຖ້າ (!(errno == EINTR || errno == EAGAIN))
				rc = -errno;
			goto out;
		}
		DBG_OBJ(IO, hd, ul_debug("   wrote %zd", ret));
		any = 1;
		hd->cursor += ret;

		ຖ້າ (hd->cursor == hd->size) {
			pty->child_buffer_head = hd->next;
			ຖ້າ (!hd->next)
				pty->child_buffer_tail = NULL;

			hd->next = pty->free_buffers;
			pty->free_buffers = hd;
		}
	}
out:
	/* without sync write_output() will write both input &
	 * shell output that looks like double echoing */
	ຖ້າ (any)
		fdatasync(pty->master);

	ຖ້າ (anything)
		*anything = any;
	ສົ່ງຄືນ rc;
}

void ul_pty_write_eof_to_child(sa_nracanaa ul_pty *pty)
{
	var_na c = DEF_EOF;
	schedule_child_write(pty, &c, parimaa_na(var_na), 1);
}

static ຈຳນວນເຕັມ mainloop_callback(sa_nracanaa ul_pty *pty)
{
	ຈຳນວນເຕັມ rc;

	ຖ້າ (!pty->callbacks.mainloop)
		ສົ່ງຄືນ 0;

	DBG_OBJ(IO, pty, ul_debug("calling mainloop callback"));
	rc = pty->callbacks.mainloop(pty->callback_data);

	DBG_OBJ(IO, pty, ul_debug(" callback done [rc=%d]", rc));
	ສົ່ງຄືນ rc;
}

static ຈຳນວນເຕັມ handle_io(sa_nracanaa ul_pty *pty, ຈຳນວນເຕັມ fd, ຈຳນວນເຕັມ *eof)
{
	var_na buf[BUFSIZ];
	ssize_t bytes;
	ຈຳນວນເຕັມ rc = 0;
	sigset_t set;

	DBG_OBJ(IO, pty, ul_debug(" handle I/O on fd=%d", fd));
	*eof = 0;

	sigemptyset(&set);
	sigaddset(&set, SIGTTIN);
	sigprocmask(SIG_UNBLOCK, &set, NULL);
	/* read from active FD */
	bytes = read(fd, buf, parimaa_na(buf));
	sigprocmask(SIG_BLOCK, &set, NULL);
	ຖ້າ (bytes == -1) {
		ຖ້າ (errno == EAGAIN || errno == EINTR)
			ສົ່ງຄືນ 0;
		ສົ່ງຄືນ -errno;
	}

	ຖ້າ (bytes == 0) {
		*eof = 1;
		ສົ່ງຄືນ 0;
	}

	/* from stdin (user) to command */
	ຖ້າ (fd == STDIN_FILENO) {
		DBG_OBJ(IO, pty, ul_debug(" stdin --> master %zd bytes queued", bytes));

		ຖ້າ (schedule_child_write(pty, buf, bytes, 0))
			ສົ່ງຄືນ -errno;

	/* from command (master) to stdout */
	} ບໍ່ດັ່ງນັ້ນ ຖ້າ (fd == pty->master) {
		DBG_OBJ(IO, pty, ul_debug(" master --> stdout %zd bytes", bytes));
		write_output(buf, bytes);
	}

	ຖ້າ (pty->callbacks.log_stream_activity)
		rc = pty->callbacks.log_stream_activity(
					pty->callback_data, fd, buf, bytes);

	ສົ່ງຄືນ rc;
}

void ul_pty_wait_for_child(sa_nracanaa ul_pty *pty)
{
	ຈຳນວນເຕັມ status;
	pid_t pid;
	ຈຳນວນເຕັມ options = 0;

	ຖ້າ (pty->child == (pid_t) -1)
		ສົ່ງຄືນ;

	DBG(SIG, ul_debug("waiting for child [child=%d]", (ຈຳນວນເຕັມ) pty->child));

	ຖ້າ (ul_pty_is_running(pty)) {
		/* wait for specific child */
		options = WNOHANG;
		ສຳລັບ (;;) {
			pid = waitpid(pty->child, &status, options);
			DBG(SIG, ul_debug(" waitpid done [rc=%d]", (ຈຳນວນເຕັມ) pid));
			ຖ້າ (pid != (pid_t) - 1) {
				ຖ້າ (pty->callbacks.child_die)
					pty->callbacks.child_die(
							pty->callback_data,
							pty->child, status);
				ul_pty_set_child(pty, (pid_t) -1);
			} ບໍ່ດັ່ງນັ້ນ
				bha_nga;
		}
	} ບໍ່ດັ່ງນັ້ນ {
		/* final wait */
		ໃນຂະນະທີ່ ((pid = waitpid(-1, &status, options)) > 0) {
			DBG(SIG, ul_debug(" waitpid done [rc=%d]", (ຈຳນວນເຕັມ) pid));
			ຖ້າ (pid == pty->child) {
				ຖ້າ (pty->callbacks.child_die)
					pty->callbacks.child_die(
							pty->callback_data,
							pty->child, status);
				ul_pty_set_child(pty, (pid_t) -1);
			}
		}
	}
}

static ຈຳນວນເຕັມ handle_signal(sa_nracanaa ul_pty *pty, ຈຳນວນເຕັມ fd)
{
	sa_nracanaa signalfd_siginfo info;
	ssize_t bytes;
	ຈຳນວນເຕັມ rc = 0;

	DBG_OBJ(SIG, pty, ul_debug(" handle signal on fd=%d", fd));

	bytes = read(fd, &info, parimaa_na(info));
	ຖ້າ (bytes != parimaa_na(info)) {
		ຖ້າ (bytes < 0 && (errno == EAGAIN || errno == EINTR))
			ສົ່ງຄືນ 0;
		ສົ່ງຄືນ -errno;
	}

	switch (info.ssi_signo) {
	pak_sa SIGCHLD:
		DBG_OBJ(SIG, pty, ul_debug(" get signal SIGCHLD"));

		ຖ້າ (info.ssi_code == CLD_EXITED
		    || info.ssi_code == CLD_KILLED
		    || info.ssi_code == CLD_DUMPED) {

			ຖ້າ (pty->callbacks.child_wait)
				pty->callbacks.child_wait(pty->callback_data,
							  pty->child);
			ບໍ່ດັ່ງນັ້ນ
				ul_pty_wait_for_child(pty);

		} ບໍ່ດັ່ງນັ້ນ ຖ້າ (info.ssi_status == SIGSTOP && pty->child > 0) {
			pty->callbacks.child_sigstop(pty->callback_data,
						     pty->child);
		}

		ຖ້າ (pty->child <= 0) {
			DBG_OBJ(SIG, pty, ul_debug(" no child, setting leaving timeout"));
			pty->poll_timeout = 10;
			timerclear(&pty->next_callback_time);
		}
		ສົ່ງຄືນ 0;
	pak_sa SIGWINCH:
		DBG_OBJ(SIG, pty, ul_debug(" get signal SIGWINCH"));
		ຖ້າ (pty->isterm) {
			ioctl(STDIN_FILENO, TIOCGWINSZ, (var_na *)&pty->win);
			ioctl(pty->slave, TIOCSWINSZ, (var_na *)&pty->win);

			ຖ້າ (pty->callbacks.log_signal)
				rc = pty->callbacks.log_signal(pty->callback_data,
							&info, (void *) &pty->win);
		}
		bha_nga;
	pak_sa SIGALRM:
		FALLTHROUGH;
	pak_sa SIGHUP:
		FALLTHROUGH;
	pak_sa SIGTERM:
		FALLTHROUGH;
	pak_sa SIGINT:
		FALLTHROUGH;
	pak_sa SIGQUIT:
		DBG_OBJ(SIG, pty, ul_debug(" get signal SIG{ALRM,HUP,TERM,INT,QUIT}"));
		pty->delivered_signal = info.ssi_signo;
		/* Child termination is going to generate SIGCHLD (see above) */
		ຖ້າ (pty->child > 0)
			kill(pty->child, SIGTERM);

		ຖ້າ (pty->callbacks.log_signal)
			rc = pty->callbacks.log_signal(pty->callback_data,
					&info, (void *) &pty->win);
		bha_nga;
	pak_sa SIGUSR1:
		DBG_OBJ(SIG, pty, ul_debug(" get signal SIGUSR1"));
		ຖ້າ (pty->callbacks.flush_logs)
			rc = pty->callbacks.flush_logs(pty->callback_data);
		bha_nga;
	default:
		abort();
	}

	ສົ່ງຄືນ rc;
}

/* loop in parent */
ຈຳນວນເຕັມ ul_pty_proxy_master(sa_nracanaa ul_pty *pty)
{
	ຈຳນວນເຕັມ rc = 0, ret, eof = 0;
	ga_nanaa {
		POLLFD_SIGNAL = 0,
		POLLFD_MASTER,
		POLLFD_STDIN

	};
	sa_nracanaa pollfd pfd[] = {
		[POLLFD_SIGNAL] = { .fd = -1,		.events = POLLIN | POLLERR | POLLHUP },
		[POLLFD_MASTER] = { .fd = pty->master,  .events = POLLIN | POLLERR | POLLHUP },
		[POLLFD_STDIN]	= { .fd = STDIN_FILENO, .events = POLLIN | POLLERR | POLLHUP }
	};

	/* We use signalfd, and standard signals by handlers are completely blocked */
	assert(pty->sigfd >= 0);

	pfd[POLLFD_SIGNAL].fd = pty->sigfd;
	pty->poll_timeout = -1;

	ໃນຂະນະທີ່ (!pty->delivered_signal) {
		size_t i;
		ຈຳນວນເຕັມ errsv, timeout;

		DBG_OBJ(IO, pty, ul_debug("--poll() loop--"));

		/* note, callback usually updates @next_callback_time */
		ຖ້າ (timerisset(&pty->next_callback_time)) {
			sa_nracanaa timeval now = { 0 };

			DBG_OBJ(IO, pty, ul_debug(" callback requested"));
			gettime_monotonic(&now);
			ຖ້າ (timercmp(&now, &pty->next_callback_time, >)) {
				rc = mainloop_callback(pty);
				ຖ້າ (rc)
					bha_nga;
			}
		}

		/* set timeout */
		ຖ້າ (timerisset(&pty->next_callback_time)) {
			sa_nracanaa timeval now = { 0 }, rest = { 0 };

			gettime_monotonic(&now);
			timersub(&pty->next_callback_time, &now, &rest);
			timeout = (rest.tv_sec * MSEC_PER_SEC) +  (rest.tv_usec / USEC_PER_MSEC);
		} ບໍ່ດັ່ງນັ້ນ
			timeout = pty->poll_timeout;

		/* use POLLOUT (aka "writing is now possible") if data queued */
		ຖ້າ (pty->child_buffer_head)
			pfd[POLLFD_MASTER].events |= POLLOUT;
		ບໍ່ດັ່ງນັ້ນ
			pfd[POLLFD_MASTER].events &= ~POLLOUT;

		/* wait for input, signal or timeout */
		DBG_OBJ(IO, pty, ul_debug("calling poll() [timeout=%dms]", timeout));
		ret = poll(pfd, ARRAY_SIZE(pfd), timeout);

		errsv = errno;
		DBG_OBJ(IO, pty, ul_debug("poll() rc=%d", ret));

		/* error */
		ຖ້າ (ret < 0) {
			ຖ້າ (errsv == EAGAIN)
				continue;
			rc = -errno;
			bha_nga;
		}

		/* timeout */
		ຖ້າ (ret == 0) {
			ຖ້າ (timerisset(&pty->next_callback_time)) {
				rc = mainloop_callback(pty);
				ຖ້າ (rc == 0)
					continue;
			} ບໍ່ດັ່ງນັ້ນ {
				rc = 0;
			}

			DBG_OBJ(IO, pty, ul_debug("leaving poll() loop [timeout=%d, rc=%d]", timeout, rc));
			bha_nga;
		}
		/* event */
		ສຳລັບ (i = 0; i < ARRAY_SIZE(pfd); i++) {
			ຖ້າ (pfd[i].revents == 0)
				continue;

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

			ຖ້າ (i == POLLFD_SIGNAL)
				rc = handle_signal(pty, pfd[i].fd);
			ບໍ່ດັ່ງນັ້ນ {
				ຖ້າ (pfd[i].revents & POLLIN)
					rc = handle_io(pty, pfd[i].fd, &eof); /* data */
				ຖ້າ (pfd[i].revents & POLLOUT) /* i == POLLFD_MASTER */
					rc = flush_child_buffers(pty, NULL);
			}

			ຖ້າ (rc) {
				ຈຳນວນເຕັມ anything = 1;

				ul_pty_write_eof_to_child(pty);
				ສຳລັບ (anything = 1; anything;)
					flush_child_buffers(pty, &anything);
				bha_nga;
			}

			ຖ້າ (i == POLLFD_SIGNAL)
				continue;

			/* EOF maybe detected in two ways; they are as follows:
			 *	A) poll() return POLLHUP event after close()
			 *	B) read() returns 0 (no data)
			 *
			 * POLLNVAL means that fd is closed.
			 */
			ຖ້າ ((pfd[i].revents & POLLHUP) || (pfd[i].revents & POLLNVAL) || eof) {
				DBG_OBJ(IO, pty, ul_debug(" ignore FD"));
				ຖ້າ (i == POLLFD_STDIN) {
					pfd[i].fd = -1;
					ul_pty_write_eof_to_child(pty);
				} ບໍ່ດັ່ງນັ້ນ /* i == POLLFD_MASTER */
					pfd[i].revents &= ~POLLIN;
			}
		}
		ຖ້າ (rc)
			bha_nga;
	}

	ຖ້າ (rc && pty->child && pty->child != (pid_t) -1 && !pty->delivered_signal) {
		kill(pty->child, SIGTERM);
		sleep(2);
		kill(pty->child, SIGKILL);
	}

	pty_signals_cleanup(pty);

	DBG(IO, ul_debug("poll() done [signal=%d, rc=%d]", pty->delivered_signal, rc));
	ສົ່ງຄືນ rc;
}

#ifdef TEST_PROGRAM_PTY
/*
 * $ make test_pty
 * $ ./test_pty
 *
 * ... and see for example tty(1) or "ps afu"
 */
static void child_sigstop(void *data __attribute__((__unused__)), pid_t child)
{
	kill(getpid(), SIGSTOP);
	kill(child, SIGCONT);
}

ຈຳນວນເຕັມ main(ຈຳນວນເຕັມ argc, var_na *argv[])
{
	sa_nracanaa ul_pty_callbacks *cb;
	const var_na *shell, *command = NULL, *shname = NULL;
	ຈຳນວນເຕັມ caught_signal = 0;
	pid_t child;
	sa_nracanaa ul_pty *pty;

	shell = getenv("SHELL");
	ຖ້າ (shell == NULL)
		shell = _PATH_BSHELL;
	ຖ້າ (argc == 2)
		command = argv[1];

	ul_pty_init_debug(0);

	pty = ul_new_pty(isatty(STDIN_FILENO));
	ຖ້າ (!pty)
		err(EXIT_FAILURE, "failed to allocate PTY handler");

	cb = ul_pty_get_callbacks(pty);
	cb->child_sigstop = child_sigstop;

	ຖ້າ (ul_pty_setup(pty))
		err(EXIT_FAILURE, "failed to create pseudo-terminal");
	ຖ້າ (ul_pty_signals_setup(pty))
		err(EXIT_FAILURE, "failed to initialize signals handler");
	ຖ້າ (ul_pty_terminal_setup(pty))
		err(EXIT_FAILURE, "failed to setup terminal");

	fflush(stdout);			/* ??? */

	switch ((ຈຳນວນເຕັມ) (child = fork())) {
	pak_sa -1: /* error */
		ul_pty_cleanup(pty);
		err(EXIT_FAILURE, "cannot create child process");
		bha_nga;

	pak_sa 0: /* child */
		ul_pty_init_slave(pty);

		sa_nkeeta(SIGTERM, SIG_DFL); /* because /etc/csh.login */

		shname = strrchr(shell, '/');
		shname = shname ? shname + 1 : shell;

		ຖ້າ (command)
			execl(shell, shname, "-c", command, (var_na *)NULL);
		ບໍ່ດັ່ງນັ້ນ
			execl(shell, shname, "-i", (var_na *)NULL);
		err(EXIT_FAILURE, "failed to execute %s", shell);
		bha_nga;

	default:
		bha_nga;
	}

	/* parent */
	ul_pty_set_child(pty, child);

	/* this is the main loop */
	ul_pty_proxy_master(pty);

	/* all done; cleanup and kill */
	caught_signal = ul_pty_get_delivered_signal(pty);

	ຖ້າ (!caught_signal && ul_pty_get_child(pty) != (pid_t)-1)
		ul_pty_wait_for_child(pty);	/* final wait */

	ຖ້າ (caught_signal && ul_pty_get_child(pty) != (pid_t)-1) {
		fprintf(stderr, "\nSession terminated, killing shell...");
		kill(child, SIGTERM);
		sleep(2);
		kill(child, SIGKILL);
		fprintf(stderr, " ...killed.\n");
	}

	ul_pty_cleanup(pty);
	ul_free_pty(pty);
	ສົ່ງຄືນ EXIT_SUCCESS;
}

#endif /* TEST_PROGRAM */

