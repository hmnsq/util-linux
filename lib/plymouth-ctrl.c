/*
 * plymouth-ctrl.c	Simply communications with plymouthd
 *			to avoid forked sub processes and/or
 *			missed plymouth send commands tool
 *			due a plymouthd replacement.
 *
 * Copyright (c) 2016 SUSE Linux GmbH, All rights reserved.
 * Copyright (c) 2016 Werner Fink <werner@suse.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://gnu.org/licenses/>.
 *
 * Author: Werner Fink <werner@suse.de>
 */

#అన్తర్భావ <errno.h>
#అన్తర్భావ <limits.h>
#అన్తర్భావ <poll.h>
#అన్తర్భావ <signal.h>
#అన్తర్భావ <stdarg.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <sys/mman.h>
#అన్తర్భావ <sys/socket.h>
#అన్తర్భావ <sys/types.h>
#అన్తర్భావ <sys/un.h>
#అన్తర్భావ <unistd.h>

#అన్తర్భావ "all-io.h"
#అన్తర్భావ "c.h"
#అన్తర్భావ "nls.h"
#అన్తర్భావ "plymouth-ctrl.h"

స్థిర పూర్ణాంకం can_read(పూర్ణాంకం fd, స్థిరరాశి దీర్ఘ timeout)
{
	నిర్మాణం pollfd fds = {
		.fd = fd,
		.events = POLLIN|POLLPRI,
		.revents = 0,
	};
	పూర్ణాంకం ret;

	చేయి {
		ret = poll(&fds, 1, timeout);
	} అయితే ((ret < 0) && (errno == EINTR));

	తిరిగి (ret == 1) && (fds.revents & (POLLIN|POLLPRI));
}

స్థిర పూర్ణాంకం open_un_socket_and_connect(శూన్య)
{
	/* The abstract UNIX socket of plymouth */
	నిర్మాణం sockaddr_un su = {
		.sun_family = AF_UNIX,
		.sun_path = PLYMOUTH_SOCKET_మార్గం,
	};
	స్థిరరాశి పూర్ణాంకం one = 1;
	పూర్ణాంకం fd, ret;

	fd = socket(PF_UNIX, SOCK_STREAM|SOCK_CLOEXEC|SOCK_NONBLOCK, 0);
	ఒకవేళ (fd < 0) {
		warnx(_("cannot open UNIX socket"));
		వెళ్ళు err;
	}

	ret = setsockopt(fd, SOL_SOCKET, SO_PASSCRED, &one, పరిమాణం(one));
	ఒకవేళ (ret < 0) {
		warnx(_("cannot set option for UNIX socket"));
		close(fd);
		fd = -1;
		వెళ్ళు err;
	}

	/* Note, the abstract PLYMOUTH_SOCKET_PATH has a leading NULL byte */
	ret = connect(fd, (నిర్మాణం sockaddr *) &su,
		offsetof(నిర్మాణం sockaddr_un, sun_path) + 1 + అక్షరశ్రేణి_పొడవు(su.sun_path+1));
	ఒకవేళ (ret < 0) {
		ఒకవేళ (errno != ECONNREFUSED)
			warnx(_("cannot connect on UNIX socket"));
		close(fd);
		fd = -1;
		వెళ్ళు err;
	}
err:
	తిరిగి fd;
}

పూర్ణాంకం plymouth_command(పూర్ణాంకం cmd, ...)
{
	uint8_t answer[2], command[2];
	నిర్మాణం sigaction sp, op;
	పూర్ణాంకం fdsock = -1, ret = 0;

	sigemptyset (&sp.sa_mask);
	sp.sa_handler = SIG_IGN;
	sp.sa_flags = SA_RESTART;
	sigaction(SIGPIPE, &sp, &op);

	/* The plymouthd does read at least two bytes. */
	command[1] = '\0';
	ఎంపిక (cmd) {
	స్థితి MAGIC_PING:
		fdsock = open_un_socket_and_connect();
		ఒకవేళ (fdsock >= 0) {
			command[0] = cmd;
			ul_write_all(fdsock, command, పరిమాణం(command));
		}
		ఆపు;
	స్థితి MAGIC_QUIT:
		fdsock = open_un_socket_and_connect();
		ఒకవేళ (fdsock >= 0) {
			command[0] = cmd;
			ul_write_all(fdsock, command, పరిమాణం(command));
		}
		ఆపు;
	అప్రమేయ:
		warnx(_("the plymouth request %c is not implemented"), cmd);
	స్థితి '?':
		వెళ్ళు err;
	}

	answer[0] = '\0';
	ఒకవేళ (fdsock >= 0) {
		ఒకవేళ (can_read(fdsock, 1000))
			ul_read_all(fdsock, (అక్షరం *) &answer[0], పరిమాణం(answer));
		close(fdsock);
	}
	sigaction(SIGPIPE, &op, ఖాళీ);
	ret = (answer[0] == ANSWER_ACK) ? 1 : 0;
err:
	తిరిగి ret;
}

