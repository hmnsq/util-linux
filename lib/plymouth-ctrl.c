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

#ଅନ୍ତର୍ଭାଵ <errno.h>
#ଅନ୍ତର୍ଭାଵ <limits.h>
#ଅନ୍ତର୍ଭାଵ <poll.h>
#ଅନ୍ତର୍ଭାଵ <signal.h>
#ଅନ୍ତର୍ଭାଵ <stdarg.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <sys/mman.h>
#ଅନ୍ତର୍ଭାଵ <sys/socket.h>
#ଅନ୍ତର୍ଭାଵ <sys/types.h>
#ଅନ୍ତର୍ଭାଵ <sys/un.h>
#ଅନ୍ତର୍ଭାଵ <unistd.h>

#ଅନ୍ତର୍ଭାଵ "all-io.h"
#ଅନ୍ତର୍ଭାଵ "c.h"
#ଅନ୍ତର୍ଭାଵ "nls.h"
#ଅନ୍ତର୍ଭାଵ "plymouth-ctrl.h"

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା can_read(ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd, ଅଚଳ long timeout)
{
	ସଂରଚନା pollfd fds = {
		.fd = fd,
		.events = POLLIN|POLLPRI,
		.revents = 0,
	};
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

	କର {
		ret = poll(&fds, 1, timeout);
	} ଯେତେବେଳେ ((ret < 0) && (errno == EINTR));

	ଫେରନ୍ତୁ (ret == 1) && (fds.revents & (POLLIN|POLLPRI));
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା open_un_socket_and_connect(ଶୂନ୍ୟ)
{
	/* The abstract UNIX socket of plymouth */
	ସଂରଚନା sockaddr_un su = {
		.sun_family = AF_UNIX,
		.sun_path = PLYMOUTH_SOCKET_PATH,
	};
	ଅଚଳ ପୂର୍ଣ୍ଣସଂଖ୍ୟା one = 1;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd, ret;

	fd = socket(PF_UNIX, SOCK_STREAM|SOCK_CLOEXEC|SOCK_NONBLOCK, 0);
	ଯଦି (fd < 0) {
		warnx(_("cannot open UNIX socket"));
		ଯାଅ err;
	}

	ret = setsockopt(fd, SOL_SOCKET, SO_PASSCRED, &one, ଆକାର(one));
	ଯଦି (ret < 0) {
		warnx(_("cannot set option for UNIX socket"));
		close(fd);
		fd = -1;
		ଯାଅ err;
	}

	/* Note, the abstract PLYMOUTH_SOCKET_PATH has a leading NULL byte */
	ret = connect(fd, (ସଂରଚନା sockaddr *) &su,
		offsetof(ସଂରଚନା sockaddr_un, sun_path) + 1 + strlen(su.sun_path+1));
	ଯଦି (ret < 0) {
		ଯଦି (errno != ECONNREFUSED)
			warnx(_("cannot connect on UNIX socket"));
		close(fd);
		fd = -1;
		ଯାଅ err;
	}
err:
	ଫେରନ୍ତୁ fd;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା plymouth_command(ପୂର୍ଣ୍ଣସଂଖ୍ୟା cmd, ...)
{
	uint8_t answer[2], command[2];
	ସଂରଚନା sigaction sp, op;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା fdsock = -1, ret = 0;

	sigemptyset (&sp.sa_mask);
	sp.sa_handler = SIG_IGN;
	sp.sa_flags = SA_RESTART;
	sigaction(SIGPIPE, &sp, &op);

	/* The plymouthd does read at least two bytes. */
	command[1] = '\0';
	ବାଛ (cmd) {
	ସ୍ଥିତି MAGIC_PING:
		fdsock = open_un_socket_and_connect();
		ଯଦି (fdsock >= 0) {
			command[0] = cmd;
			ul_write_all(fdsock, command, ଆକାର(command));
		}
		ଭାଙ୍ଗ;
	ସ୍ଥିତି MAGIC_QUIT:
		fdsock = open_un_socket_and_connect();
		ଯଦି (fdsock >= 0) {
			command[0] = cmd;
			ul_write_all(fdsock, command, ଆକାର(command));
		}
		ଭାଙ୍ଗ;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		warnx(_("the plymouth request %c is not implemented"), cmd);
	ସ୍ଥିତି '?':
		ଯାଅ err;
	}

	answer[0] = '\0';
	ଯଦି (fdsock >= 0) {
		ଯଦି (can_read(fdsock, 1000))
			ul_read_all(fdsock, (ଅକ୍ଷର *) &answer[0], ଆକାର(answer));
		close(fdsock);
	}
	sigaction(SIGPIPE, &op, ଖାଲି);
	ret = (answer[0] == ANSWER_ACK) ? 1 : 0;
err:
	ଫେରନ୍ତୁ ret;
}

