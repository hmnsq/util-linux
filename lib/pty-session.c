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
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <pty.h>
#అన్తర్భావ <poll.h>
#అన్తర్భావ <sys/signalfd.h>
#అన్తర్భావ <paths.h>
#అన్తర్భావ <sys/types.h>
#అన్తర్భావ <sys/wait.h>
#అన్తర్భావ <inttypes.h>

#అన్తర్భావ "c.h"
#అన్తర్భావ "all-io.h"
#అన్తర్భావ "ttyutils.h"
#అన్తర్భావ "pty-session.h"
#అన్తర్భావ "monotonic.h"
#అన్తర్భావ "debug.h"

స్థిర UL_DEBUG_DEFINE_ముసుగు(ulpty);
UL_DEBUG_DEFINE_MASKNAMES(ulpty) = UL_DEBUG_ఖాళీలు_MASKNAMES;

#నిర్వచించు ULPTY_DEBUG_INIT	(1 << 1)
#నిర్వచించు ULPTY_DEBUG_అమరిక	(1 << 2)
#నిర్వచించు ULPTY_DEBUG_SIG		(1 << 3)
#నిర్వచించు ULPTY_DEBUG_IO		(1 << 4)
#నిర్వచించు ULPTY_DEBUG_పూర్తి	(1 << 5)

#నిర్వచించు DBG(m, x)		__UL_DBG(ulpty, ULPTY_DEBUG_, m, x)
#నిర్వచించు DBG_OBJ(m, h, x)	__UL_DBG_OBJ(ulpty, ULPTY_DEBUG_, m, h, x)
#నిర్వచించు ON_DBG(m, x)		__UL_DBG_CALL(ulpty, ULPTY_DEBUG_, m, x)

శూన్య ul_pty_init_debug(పూర్ణాంకం ముసుగు)
{
	ఒకవేళ (ulpty_debug_ముసుగు)
		తిరిగి;
	__UL_INIT_DEBUG_FROM_ENV(ulpty, ULPTY_DEBUG_, ముసుగు, ULPTY_DEBUG);
}

నిర్మాణం ul_pty *ul_new_pty(పూర్ణాంకం is_stdin_tty)
{
	నిర్మాణం ul_pty *pty = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం(*pty));

	ఒకవేళ (!pty)
		తిరిగి ఖాళీ;

	DBG_OBJ(అమరిక, pty, ul_debug("alloc handler"));
	pty->isterm = is_stdin_tty;
	pty->master = -1;
	pty->slave = -1;
	pty->sigfd = -1;
	pty->బిడ్డ = (pid_t) -1;

	తిరిగి pty;
}

శూన్య ul_free_pty(నిర్మాణం ul_pty *pty)
{
	నిర్మాణం ul_pty_బిడ్డ_బఫరు *hd;

	అయితే ((hd = pty->బిడ్డ_బఫరు_తల)) {
		pty->బిడ్డ_బఫరు_తల = hd->తదుపరి;
		విడుదల(hd);
	}

	అయితే ((hd = pty->free_buffers)) {
		pty->free_buffers = hd->తదుపరి;
		విడుదల(hd);
	}
	విడుదల(pty);
}

శూన్య ul_pty_slave_echo(నిర్మాణం ul_pty *pty, పూర్ణాంకం enable)
{
	ప్రతిజ్ఞా(pty);
	pty->slave_echo = enable ? 1 : 0;
}

పూర్ణాంకం ul_pty_get_delivered_signal(నిర్మాణం ul_pty *pty)
{
	ప్రతిజ్ఞా(pty);
	తిరిగి pty->delivered_signal;
}

నిర్మాణం ul_pty_callbacks *ul_pty_get_callbacks(నిర్మాణం ul_pty *pty)
{
	ప్రతిజ్ఞా(pty);
	తిరిగి &pty->callbacks;
}

శూన్య ul_pty_సమితి_callback_దత్తాంశం(నిర్మాణం ul_pty *pty, శూన్య *దత్తాంశం)
{
	ప్రతిజ్ఞా(pty);
	pty->callback_దత్తాంశం = దత్తాంశం;
}

శూన్య ul_pty_సమితి_బిడ్డ(నిర్మాణం ul_pty *pty, pid_t బిడ్డ)
{
	ప్రతిజ్ఞా(pty);
	pty->బిడ్డ = బిడ్డ;
}

పూర్ణాంకం ul_pty_get_childfd(నిర్మాణం ul_pty *pty)
{
	ప్రతిజ్ఞా(pty);
	తిరిగి pty->master;
}

pid_t ul_pty_get_బిడ్డ(నిర్మాణం ul_pty *pty)
{
	ప్రతిజ్ఞా(pty);
	తిరిగి pty->బిడ్డ;
}

/* it's active when signals are redirected to sigfd */
పూర్ణాంకం ul_pty_is_running(నిర్మాణం ul_pty *pty)
{
	ప్రతిజ్ఞా(pty);
	తిరిగి pty->sigfd >= 0;
}

శూన్య ul_pty_సమితి_mainloop_సమయం(నిర్మాణం ul_pty *pty, నిర్మాణం timeval *tv)
{
	ప్రతిజ్ఞా(pty);
	ఒకవేళ (!tv) {
		DBG_OBJ(IO, pty, ul_debug("mainloop time: clear"));
		timerclear(&pty->తదుపరి_callback_సమయం);
	} లేకపోతే {
		pty->తదుపరి_callback_సమయం.tv_sec = tv->tv_sec;
		pty->తదుపరి_callback_సమయం.tv_usec = tv->tv_usec;
		DBG_OBJ(IO, pty, ul_debug("mainloop time: %"PRId64".%06"PRId64,
				(int64_t) tv->tv_sec, (int64_t) tv->tv_usec));
	}
}

స్థిర శూన్య pty_సంకేతాలు_cleanup(నిర్మాణం ul_pty *pty)
{
	ఒకవేళ (pty->sigfd != -1)
		close(pty->sigfd);
	pty->sigfd = -1;

	/* restore original setting */
	sigprocmask(SIG_SETMASK, &pty->orgsig, ఖాళీ);
}

/* call me before fork() */
పూర్ణాంకం ul_pty_అమరిక(నిర్మాణం ul_pty *pty)
{
	నిర్మాణం termios attrs;
	పూర్ణాంకం rc = 0;

	ప్రతిజ్ఞా(pty->sigfd == -1);

	/* save the current signals setting (to make ul_pty_cleanup() usable,
	 * otherwise see ul_pty_signals_setup() */
	sigprocmask(0, ఖాళీ, &pty->orgsig);

	ఒకవేళ (pty->isterm) {
		DBG_OBJ(అమరిక, pty, ul_debug("create for terminal"));

		/* original setting of the current terminal */
		ఒకవేళ (tcgetattr(STDIN_FILENO, &pty->stdin_attrs) != 0) {
			rc = -errno;
			వెళ్ళు పూర్తి;
		}

		attrs = pty->stdin_attrs;
		ఒకవేళ (pty->slave_echo)
			attrs.c_lflag |= ECHO;
		లేకపోతే
			attrs.c_lflag &= ~ECHO;

		ioctl(STDIN_FILENO, TIOCGWINSZ, (అక్షరం *)&pty->win);
		/* create master+slave */
		rc = openpty(&pty->master, &pty->slave, ఖాళీ, &attrs, &pty->win);
		ఒకవేళ (rc)
			వెళ్ళు పూర్తి;

	} లేకపోతే {
		DBG_OBJ(అమరిక, pty, ul_debug("create for non-terminal"));

		rc = openpty(&pty->master, &pty->slave, ఖాళీ, ఖాళీ, ఖాళీ);
		ఒకవేళ (rc)
			వెళ్ళు పూర్తి;

		tcgetattr(pty->slave, &attrs);

		ఒకవేళ (pty->slave_echo)
			attrs.c_lflag |= ECHO;
		లేకపోతే
			attrs.c_lflag &= ~ECHO;

		tcsetattr(pty->slave, TCSANOW, &attrs);
	}

	fcntl(pty->master, F_SETFL, O_NONBLOCK);

పూర్తి:
	ఒకవేళ (rc)
		ul_pty_cleanup(pty);

	DBG_OBJ(అమరిక, pty, ul_debug("pty setup done [master=%d, slave=%d, rc=%d]",
				pty->master, pty->slave, rc));
	తిరిగి rc;
}

/* call me after ul_pty_signals_setup() and before fork() */
పూర్ణాంకం ul_pty_terminal_అమరిక(నిర్మాణం ul_pty *pty)
{
	నిర్మాణం termios attrs;

	ఒకవేళ (!pty->isterm)
		తిరిగి 0;

	attrs = pty->stdin_attrs;
	cfmakeraw(&attrs);
	ఒకవేళ (tcsetattr(STDIN_FILENO, TCSANOW, &attrs))
		తిరిగి -errno;

	DBG_OBJ(అమరిక, pty, ul_debug("terminal raw mode setup done"));
	తిరిగి 0;
}

/* call me before fork() */
పూర్ణాంకం ul_pty_సంకేతాలు_అమరిక(నిర్మాణం ul_pty *pty)
{
	sigset_t ourset;
	పూర్ణాంకం rc = 0;

	ప్రతిజ్ఞా(pty->sigfd == -1);

	/* save the current signals setting */
	sigprocmask(0, ఖాళీ, &pty->orgsig);

	sigfillset(&ourset);
	ఒకవేళ (sigprocmask(SIG_BLOCK, &ourset, ఖాళీ)) {
		rc = -errno;
		వెళ్ళు పూర్తి;
	}

	sigemptyset(&ourset);
	sigaddset(&ourset, SIGCHLD);
	sigaddset(&ourset, SIGWINCH);
	sigaddset(&ourset, SIGALRM);
	sigaddset(&ourset, SIGHUP);
	sigaddset(&ourset, SIGTERM);
	sigaddset(&ourset, SIGINT);
	sigaddset(&ourset, SIGQUIT);

	ఒకవేళ (pty->callbacks.flush_logs)
		sigaddset(&ourset, SIGUSR1);

	ఒకవేళ ((pty->sigfd = signalfd(-1, &ourset, SFD_CLOEXEC)) < 0)
		rc = -errno;
పూర్తి:
	ఒకవేళ (rc)
		ul_pty_cleanup(pty);

	DBG_OBJ(అమరిక, pty, ul_debug("pty signals setup done [rc=%d]", rc));
	తిరిగి rc;
}

/* cleanup in parent process */
శూన్య ul_pty_cleanup(నిర్మాణం ul_pty *pty)
{
	నిర్మాణం termios rtt;

	pty_సంకేతాలు_cleanup(pty);

	ఒకవేళ (pty->master == -1 || !pty->isterm)
		తిరిగి;

	DBG_OBJ(పూర్తి_2, pty, ul_debug("cleanup"));
	rtt = pty->stdin_attrs;
	tcsetattr(STDIN_FILENO, TCSADRAIN, &rtt);
}

పూర్ణాంకం ul_pty_chownmod_slave(నిర్మాణం ul_pty *pty, uid_t uid, gid_t gid, mode_t బహుళకం)
{
	ఒకవేళ (fchown(pty->slave, uid, gid))
		తిరిగి -errno;
	ఒకవేళ (fchmod(pty->slave, బహుళకం))
		తిరిగి -errno;
	తిరిగి 0;
}

/* call me in child process */
శూన్య ul_pty_init_slave(నిర్మాణం ul_pty *pty)
{
	DBG_OBJ(అమరిక, pty, ul_debug("initialize slave"));

	setsid();

	ioctl(pty->slave, TIOCSCTTY, 1);
	close(pty->master);

	dup2(pty->slave, STDIN_FILENO);
	dup2(pty->slave, STDOUT_FILENO);
	dup2(pty->slave, STDERR_FILENO);

	close(pty->slave);

	ఒకవేళ (pty->sigfd >= 0)
		close(pty->sigfd);

	pty->slave = -1;
	pty->master = -1;
	pty->sigfd = -1;

	sigprocmask(SIG_SETMASK, &pty->orgsig, ఖాళీ);

	DBG_OBJ(అమరిక, pty, ul_debug("... initialize slave done"));
}

స్థిర పూర్ణాంకం write_output(అక్షరం *obuf, ssize_t బైట్లు)
{
	DBG(IO, ul_debug(" writing output"));

	ఒకవేళ (ul_write_all(STDOUT_FILENO, obuf, బైట్లు)) {
		DBG(IO, ul_debug("  writing output *failed*"));
		తిరిగి -errno;
	}

	తిరిగి 0;
}

స్థిర పూర్ణాంకం కాలపట్టిక_బిడ్డ_write(నిర్మాణం ul_pty *pty, అక్షరం *buf, size_t bufsz, పూర్ణాంకం final)
{
	నిర్మాణం ul_pty_బిడ్డ_బఫరు *stash;

	ఒకవేళ (pty->free_buffers) {
		stash = pty->free_buffers;
		pty->free_buffers = stash->తదుపరి;
		memset(stash, 0, పరిమాణం(*stash));
	} లేకపోతే
		stash = సున్నా_జ్ఞాపకం_ఇవ్వు(1, పరిమాణం(*stash));
	ఒకవేళ (!stash)
		తిరిగి -1;

	ప్రతిజ్ఞా(bufsz <= పరిమాణం(stash->buf));

	జ్ఞాపకం_నకలు(stash->buf, buf, bufsz);
	stash->కొలత = bufsz;
	stash->final_ఆదానం = final ? 1 : 0;

	ఒకవేళ (pty->బిడ్డ_బఫరు_తల)
		pty->బిడ్డ_బఫరు_తోక = pty->బిడ్డ_బఫరు_తోక->తదుపరి = stash;
	లేకపోతే
		pty->బిడ్డ_బఫరు_తల = pty->బిడ్డ_బఫరు_తోక = stash;
	తిరిగి 0;
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
స్థిర శూన్య drain_బిడ్డ_buffers(నిర్మాణం ul_pty *pty)
{
	గుర్తులేని పూర్ణాంకం tries = 0;
	నిర్మాణం pollfd fd = { .fd = pty->slave, .events = POLLIN };

	DBG_OBJ(IO, pty, ul_debug(" waiting for empty slave"));
	అయితే (poll(&fd, 1, 10) == 1 && tries < 8) {
		DBG_OBJ(IO, pty, ul_debug("   slave is not empty"));
		xusleep(250000);
		tries++;
	}
	ఒకవేళ (tries < 8)
		DBG_OBJ(IO, pty, ul_debug("   slave is empty now"));

	DBG_OBJ(IO, pty, ul_debug(" sending EOF to master"));
}

స్థిర పూర్ణాంకం flush_బిడ్డ_buffers(నిర్మాణం ul_pty *pty, పూర్ణాంకం *anything)
{
	పూర్ణాంకం rc = 0, any = 0;

	అయితే (pty->బిడ్డ_బఫరు_తల) {
		నిర్మాణం ul_pty_బిడ్డ_బఫరు *hd = pty->బిడ్డ_బఫరు_తల;
		ssize_t ret;

		ఒకవేళ (hd->final_ఆదానం)
			drain_బిడ్డ_buffers(pty);

		DBG_OBJ(IO, hd, ul_debug(" stdin --> master trying %zu bytes", hd->కొలత - hd->cursor));

		ret = లిఖ(pty->master, hd->buf + hd->cursor, hd->కొలత - hd->cursor);
		ఒకవేళ (ret == -1) {
			DBG_OBJ(IO, hd, ul_debug("   EAGAIN"));
			ఒకవేళ (!(errno == EINTR || errno == EAGAIN))
				rc = -errno;
			వెళ్ళు బహిర్గతం;
		}
		DBG_OBJ(IO, hd, ul_debug("   wrote %zd", ret));
		any = 1;
		hd->cursor += ret;

		ఒకవేళ (hd->cursor == hd->కొలత) {
			pty->బిడ్డ_బఫరు_తల = hd->తదుపరి;
			ఒకవేళ (!hd->తదుపరి)
				pty->బిడ్డ_బఫరు_తోక = ఖాళీ;

			hd->తదుపరి = pty->free_buffers;
			pty->free_buffers = hd;
		}
	}
బహిర్గతం:
	/* without sync write_output() will write both input &
	 * shell output that looks like double echoing */
	ఒకవేళ (any)
		fdatasync(pty->master);

	ఒకవేళ (anything)
		*anything = any;
	తిరిగి rc;
}

శూన్య ul_pty_write_eof_గమ్యం_బిడ్డ(నిర్మాణం ul_pty *pty)
{
	అక్షరం c = DEF_EOF;
	కాలపట్టిక_బిడ్డ_write(pty, &c, పరిమాణం(అక్షరం), 1);
}

స్థిర పూర్ణాంకం mainloop_callback(నిర్మాణం ul_pty *pty)
{
	పూర్ణాంకం rc;

	ఒకవేళ (!pty->callbacks.mainloop)
		తిరిగి 0;

	DBG_OBJ(IO, pty, ul_debug("calling mainloop callback"));
	rc = pty->callbacks.mainloop(pty->callback_దత్తాంశం);

	DBG_OBJ(IO, pty, ul_debug(" callback done [rc=%d]", rc));
	తిరిగి rc;
}

స్థిర పూర్ణాంకం handle_io(నిర్మాణం ul_pty *pty, పూర్ణాంకం fd, పూర్ణాంకం *eof)
{
	అక్షరం buf[BUFSIZ];
	ssize_t బైట్లు;
	పూర్ణాంకం rc = 0;
	sigset_t సమితి;

	DBG_OBJ(IO, pty, ul_debug(" handle I/O on fd=%d", fd));
	*eof = 0;

	sigemptyset(&సమితి);
	sigaddset(&సమితి, SIGTTIN);
	sigprocmask(SIG_UNBLOCK, &సమితి, ఖాళీ);
	/* read from active FD */
	బైట్లు = పఠ(fd, buf, పరిమాణం(buf));
	sigprocmask(SIG_BLOCK, &సమితి, ఖాళీ);
	ఒకవేళ (బైట్లు == -1) {
		ఒకవేళ (errno == EAGAIN || errno == EINTR)
			తిరిగి 0;
		తిరిగి -errno;
	}

	ఒకవేళ (బైట్లు == 0) {
		*eof = 1;
		తిరిగి 0;
	}

	/* from stdin (user) to command */
	ఒకవేళ (fd == STDIN_FILENO) {
		DBG_OBJ(IO, pty, ul_debug(" stdin --> master %zd bytes queued", బైట్లు));

		ఒకవేళ (కాలపట్టిక_బిడ్డ_write(pty, buf, బైట్లు, 0))
			తిరిగి -errno;

	/* from command (master) to stdout */
	} లేకపోతే ఒకవేళ (fd == pty->master) {
		DBG_OBJ(IO, pty, ul_debug(" master --> stdout %zd bytes", బైట్లు));
		write_output(buf, బైట్లు);
	}

	ఒకవేళ (pty->callbacks.log_ప్రవాహం_activity)
		rc = pty->callbacks.log_ప్రవాహం_activity(
					pty->callback_దత్తాంశం, fd, buf, బైట్లు);

	తిరిగి rc;
}

శూన్య ul_pty_wait_for_బిడ్డ(నిర్మాణం ul_pty *pty)
{
	పూర్ణాంకం స్థితి_2;
	pid_t pid;
	పూర్ణాంకం options = 0;

	ఒకవేళ (pty->బిడ్డ == (pid_t) -1)
		తిరిగి;

	DBG(SIG, ul_debug("waiting for child [child=%d]", (పూర్ణాంకం) pty->బిడ్డ));

	ఒకవేళ (ul_pty_is_running(pty)) {
		/* wait for specific child */
		options = WNOHANG;
		కొరకు (;;) {
			pid = waitpid(pty->బిడ్డ, &స్థితి_2, options);
			DBG(SIG, ul_debug(" waitpid done [rc=%d]", (పూర్ణాంకం) pid));
			ఒకవేళ (pid != (pid_t) - 1) {
				ఒకవేళ (pty->callbacks.బిడ్డ_die)
					pty->callbacks.బిడ్డ_die(
							pty->callback_దత్తాంశం,
							pty->బిడ్డ, స్థితి_2);
				ul_pty_సమితి_బిడ్డ(pty, (pid_t) -1);
			} లేకపోతే
				ఆపు;
		}
	} లేకపోతే {
		/* final wait */
		అయితే ((pid = waitpid(-1, &స్థితి_2, options)) > 0) {
			DBG(SIG, ul_debug(" waitpid done [rc=%d]", (పూర్ణాంకం) pid));
			ఒకవేళ (pid == pty->బిడ్డ) {
				ఒకవేళ (pty->callbacks.బిడ్డ_die)
					pty->callbacks.బిడ్డ_die(
							pty->callback_దత్తాంశం,
							pty->బిడ్డ, స్థితి_2);
				ul_pty_సమితి_బిడ్డ(pty, (pid_t) -1);
			}
		}
	}
}

స్థిర పూర్ణాంకం handle_signal(నిర్మాణం ul_pty *pty, పూర్ణాంకం fd)
{
	నిర్మాణం signalfd_siginfo info;
	ssize_t బైట్లు;
	పూర్ణాంకం rc = 0;

	DBG_OBJ(SIG, pty, ul_debug(" handle signal on fd=%d", fd));

	బైట్లు = పఠ(fd, &info, పరిమాణం(info));
	ఒకవేళ (బైట్లు != పరిమాణం(info)) {
		ఒకవేళ (బైట్లు < 0 && (errno == EAGAIN || errno == EINTR))
			తిరిగి 0;
		తిరిగి -errno;
	}

	ఎంపిక (info.ssi_signo) {
	స్థితి SIGCHLD:
		DBG_OBJ(SIG, pty, ul_debug(" get signal SIGCHLD"));

		ఒకవేళ (info.ssi_code == CLD_EXITED
		    || info.ssi_code == CLD_KILLED
		    || info.ssi_code == CLD_DUMPED) {

			ఒకవేళ (pty->callbacks.బిడ్డ_wait)
				pty->callbacks.బిడ్డ_wait(pty->callback_దత్తాంశం,
							  pty->బిడ్డ);
			లేకపోతే
				ul_pty_wait_for_బిడ్డ(pty);

		} లేకపోతే ఒకవేళ (info.ssi_status == SIGSTOP && pty->బిడ్డ > 0) {
			pty->callbacks.బిడ్డ_sigstop(pty->callback_దత్తాంశం,
						     pty->బిడ్డ);
		}

		ఒకవేళ (pty->బిడ్డ <= 0) {
			DBG_OBJ(SIG, pty, ul_debug(" no child, setting leaving timeout"));
			pty->poll_timeout = 10;
			timerclear(&pty->తదుపరి_callback_సమయం);
		}
		తిరిగి 0;
	స్థితి SIGWINCH:
		DBG_OBJ(SIG, pty, ul_debug(" get signal SIGWINCH"));
		ఒకవేళ (pty->isterm) {
			ioctl(STDIN_FILENO, TIOCGWINSZ, (అక్షరం *)&pty->win);
			ioctl(pty->slave, TIOCSWINSZ, (అక్షరం *)&pty->win);

			ఒకవేళ (pty->callbacks.log_signal)
				rc = pty->callbacks.log_signal(pty->callback_దత్తాంశం,
							&info, (శూన్య *) &pty->win);
		}
		ఆపు;
	స్థితి SIGALRM:
		FALLTHROUGH;
	స్థితి SIGHUP:
		FALLTHROUGH;
	స్థితి SIGTERM:
		FALLTHROUGH;
	స్థితి SIGINT:
		FALLTHROUGH;
	స్థితి SIGQUIT:
		DBG_OBJ(SIG, pty, ul_debug(" get signal SIG{ALRM,HUP,TERM,INT,QUIT}"));
		pty->delivered_signal = info.ssi_signo;
		/* Child termination is going to generate SIGCHLD (see above) */
		ఒకవేళ (pty->బిడ్డ > 0)
			kill(pty->బిడ్డ, SIGTERM);

		ఒకవేళ (pty->callbacks.log_signal)
			rc = pty->callbacks.log_signal(pty->callback_దత్తాంశం,
					&info, (శూన్య *) &pty->win);
		ఆపు;
	స్థితి SIGUSR1:
		DBG_OBJ(SIG, pty, ul_debug(" get signal SIGUSR1"));
		ఒకవేళ (pty->callbacks.flush_logs)
			rc = pty->callbacks.flush_logs(pty->callback_దత్తాంశం);
		ఆపు;
	అప్రమేయ:
		abort();
	}

	తిరిగి rc;
}

/* loop in parent */
పూర్ణాంకం ul_pty_proxy_master(నిర్మాణం ul_pty *pty)
{
	పూర్ణాంకం rc = 0, ret, eof = 0;
	గణన {
		POLLFD_SIGNAL = 0,
		POLLFD_MASTER,
		POLLFD_STDIN

	};
	నిర్మాణం pollfd pfd[] = {
		[POLLFD_SIGNAL] = { .fd = -1,		.events = POLLIN | POLLERR | POLLHUP },
		[POLLFD_MASTER] = { .fd = pty->master,  .events = POLLIN | POLLERR | POLLHUP },
		[POLLFD_STDIN]	= { .fd = STDIN_FILENO, .events = POLLIN | POLLERR | POLLHUP }
	};

	/* We use signalfd, and standard signals by handlers are completely blocked */
	ప్రతిజ్ఞా(pty->sigfd >= 0);

	pfd[POLLFD_SIGNAL].fd = pty->sigfd;
	pty->poll_timeout = -1;

	అయితే (!pty->delivered_signal) {
		size_t i;
		పూర్ణాంకం errsv, timeout;

		DBG_OBJ(IO, pty, ul_debug("--poll() loop--"));

		/* note, callback usually updates @next_callback_time */
		ఒకవేళ (timerisset(&pty->తదుపరి_callback_సమయం)) {
			నిర్మాణం timeval now = { 0 };

			DBG_OBJ(IO, pty, ul_debug(" callback requested"));
			gettime_monotonic(&now);
			ఒకవేళ (timercmp(&now, &pty->తదుపరి_callback_సమయం, >)) {
				rc = mainloop_callback(pty);
				ఒకవేళ (rc)
					ఆపు;
			}
		}

		/* set timeout */
		ఒకవేళ (timerisset(&pty->తదుపరి_callback_సమయం)) {
			నిర్మాణం timeval now = { 0 }, rest = { 0 };

			gettime_monotonic(&now);
			timersub(&pty->తదుపరి_callback_సమయం, &now, &rest);
			timeout = (rest.tv_sec * MSEC_చొప్పున_SEC) +  (rest.tv_usec / USEC_చొప్పున_MSEC);
		} లేకపోతే
			timeout = pty->poll_timeout;

		/* use POLLOUT (aka "writing is now possible") if data queued */
		ఒకవేళ (pty->బిడ్డ_బఫరు_తల)
			pfd[POLLFD_MASTER].events |= POLLOUT;
		లేకపోతే
			pfd[POLLFD_MASTER].events &= ~POLLOUT;

		/* wait for input, signal or timeout */
		DBG_OBJ(IO, pty, ul_debug("calling poll() [timeout=%dms]", timeout));
		ret = poll(pfd, ARRAY_కొలత(pfd), timeout);

		errsv = errno;
		DBG_OBJ(IO, pty, ul_debug("poll() rc=%d", ret));

		/* error */
		ఒకవేళ (ret < 0) {
			ఒకవేళ (errsv == EAGAIN)
				కొనసాగు;
			rc = -errno;
			ఆపు;
		}

		/* timeout */
		ఒకవేళ (ret == 0) {
			ఒకవేళ (timerisset(&pty->తదుపరి_callback_సమయం)) {
				rc = mainloop_callback(pty);
				ఒకవేళ (rc == 0)
					కొనసాగు;
			} లేకపోతే {
				rc = 0;
			}

			DBG_OBJ(IO, pty, ul_debug("leaving poll() loop [timeout=%d, rc=%d]", timeout, rc));
			ఆపు;
		}
		/* event */
		కొరకు (i = 0; i < ARRAY_కొలత(pfd); i++) {
			ఒకవేళ (pfd[i].revents == 0)
				కొనసాగు;

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

			ఒకవేళ (i == POLLFD_SIGNAL)
				rc = handle_signal(pty, pfd[i].fd);
			లేకపోతే {
				ఒకవేళ (pfd[i].revents & POLLIN)
					rc = handle_io(pty, pfd[i].fd, &eof); /*దత్తాంశం*/
				ఒకవేళ (pfd[i].revents & POLLOUT) /* i == POLLFD_MASTER */
					rc = flush_బిడ్డ_buffers(pty, ఖాళీ);
			}

			ఒకవేళ (rc) {
				పూర్ణాంకం anything = 1;

				ul_pty_write_eof_గమ్యం_బిడ్డ(pty);
				కొరకు (anything = 1; anything;)
					flush_బిడ్డ_buffers(pty, &anything);
				ఆపు;
			}

			ఒకవేళ (i == POLLFD_SIGNAL)
				కొనసాగు;

			/* EOF maybe detected in two ways; they are as follows:
			 *	A) poll() return POLLHUP event after close()
			 *	B) read() returns 0 (no data)
			 *
			 * POLLNVAL means that fd is closed.
			 */
			ఒకవేళ ((pfd[i].revents & POLLHUP) || (pfd[i].revents & POLLNVAL) || eof) {
				DBG_OBJ(IO, pty, ul_debug(" ignore FD"));
				ఒకవేళ (i == POLLFD_STDIN) {
					pfd[i].fd = -1;
					ul_pty_write_eof_గమ్యం_బిడ్డ(pty);
				} లేకపోతే /* i == POLLFD_MASTER */
					pfd[i].revents &= ~POLLIN;
			}
		}
		ఒకవేళ (rc)
			ఆపు;
	}

	ఒకవేళ (rc && pty->బిడ్డ && pty->బిడ్డ != (pid_t) -1 && !pty->delivered_signal) {
		kill(pty->బిడ్డ, SIGTERM);
		sleep(2);
		kill(pty->బిడ్డ, SIGKILL);
	}

	pty_సంకేతాలు_cleanup(pty);

	DBG(IO, ul_debug("poll() done [signal=%d, rc=%d]", pty->delivered_signal, rc));
	తిరిగి rc;
}

#నిర్వచితమైతే TEST_కార్యక్రమం_PTY
/*
 * $ make test_pty
 * $ ./test_pty
 *
 * ... and see for example tty(1) or "ps afu"
 */
స్థిర శూన్య బిడ్డ_sigstop(శూన్య *దత్తాంశం __attribute__((__unused__)), pid_t బిడ్డ)
{
	kill(getpid(), SIGSTOP);
	kill(బిడ్డ, SIGCONT);
}

పూర్ణాంకం ప్రధాన(పూర్ణాంకం argc, అక్షరం *argv[])
{
	నిర్మాణం ul_pty_callbacks *cb;
	స్థిరరాశి అక్షరం *shell, *command = ఖాళీ, *shname = ఖాళీ;
	పూర్ణాంకం caught_signal = 0;
	pid_t బిడ్డ;
	నిర్మాణం ul_pty *pty;

	shell = getenv("SHELL");
	ఒకవేళ (shell == ఖాళీ)
		shell = _PATH_BSHELL;
	ఒకవేళ (argc == 2)
		command = argv[1];

	ul_pty_init_debug(0);

	pty = ul_new_pty(isatty(STDIN_FILENO));
	ఒకవేళ (!pty)
		err(EXIT_FAILURE, "failed to allocate PTY handler");

	cb = ul_pty_get_callbacks(pty);
	cb->బిడ్డ_sigstop = బిడ్డ_sigstop;

	ఒకవేళ (ul_pty_అమరిక(pty))
		err(EXIT_FAILURE, "failed to create pseudo-terminal");
	ఒకవేళ (ul_pty_సంకేతాలు_అమరిక(pty))
		err(EXIT_FAILURE, "failed to initialize signals handler");
	ఒకవేళ (ul_pty_terminal_అమరిక(pty))
		err(EXIT_FAILURE, "failed to setup terminal");

	fflush(stdout);			/* ??? */

	ఎంపిక ((పూర్ణాంకం) (బిడ్డ = fork())) {
	స్థితి -1: /* error */
		ul_pty_cleanup(pty);
		err(EXIT_FAILURE, "cannot create child process");
		ఆపు;

	స్థితి 0: /*బిడ్డ*/
		ul_pty_init_slave(pty);

		సంకేతం(SIGTERM, SIG_DFL); /* because /etc/csh.login */

		shname = strrchr(shell, '/');
		shname = shname ? shname + 1 : shell;

		ఒకవేళ (command)
			execl(shell, shname, "-c", command, (అక్షరం *)ఖాళీ);
		లేకపోతే
			execl(shell, shname, "-i", (అక్షరం *)ఖాళీ);
		err(EXIT_FAILURE, "failed to execute %s", shell);
		ఆపు;

	అప్రమేయ:
		ఆపు;
	}

	/*మాతృ*/
	ul_pty_సమితి_బిడ్డ(pty, బిడ్డ);

	/* this is the main loop */
	ul_pty_proxy_master(pty);

	/* all done; cleanup and kill */
	caught_signal = ul_pty_get_delivered_signal(pty);

	ఒకవేళ (!caught_signal && ul_pty_get_బిడ్డ(pty) != (pid_t)-1)
		ul_pty_wait_for_బిడ్డ(pty);	/* final wait */

	ఒకవేళ (caught_signal && ul_pty_get_బిడ్డ(pty) != (pid_t)-1) {
		fprintf(stderr, "\nSession terminated, killing shell...");
		kill(బిడ్డ, SIGTERM);
		sleep(2);
		kill(బిడ్డ, SIGKILL);
		fprintf(stderr, " ...killed.\n");
	}

	ul_pty_cleanup(pty);
	ul_free_pty(pty);
	తిరిగి EXIT_SUCCESS;
}

#అయితే_ముగింపు /* TEST_PROGRAM */

