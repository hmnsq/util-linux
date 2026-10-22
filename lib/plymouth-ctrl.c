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

#অন্তর্ভাৱ <errno.h>
#অন্তর্ভাৱ <limits.h>
#অন্তর্ভাৱ <poll.h>
#অন্তর্ভাৱ <signal.h>
#অন্তর্ভাৱ <stdarg.h>
#অন্তর্ভাৱ <stdlib.h>
#অন্তর্ভাৱ <sys/mman.h>
#অন্তর্ভাৱ <sys/socket.h>
#অন্তর্ভাৱ <sys/types.h>
#অন্তর্ভাৱ <sys/un.h>
#অন্তর্ভাৱ <unistd.h>

#অন্তর্ভাৱ "all-io.h"
#অন্তর্ভাৱ "c.h"
#অন্তর্ভাৱ "nls.h"
#অন্তর্ভাৱ "plymouth-ctrl.h"

স্থির পূর্ণাঙ্ক can_read(পূর্ণাঙ্ক fd, ধ্রুৱ long timeout)
{
	সংরচনা pollfd fds = {
		.fd = fd,
		.events = POLLIN|POLLPRI,
		.revents = 0,
	};
	পূর্ণাঙ্ক ret;

	কৰা {
		ret = poll(&fds, 1, timeout);
	} যেতিয়ালৈ ((ret < 0) && (errno == EINTR));

	ঘূৰাই (ret == 1) && (fds.revents & (POLLIN|POLLPRI));
}

স্থির পূর্ণাঙ্ক open_un_socket_and_connect(শূন্য)
{
	/* The abstract UNIX socket of plymouth */
	সংরচনা sockaddr_un su = {
		.sun_family = AF_UNIX,
		.sun_path = PLYMOUTH_SOCKET_PATH,
	};
	ধ্রুৱ পূর্ণাঙ্ক one = 1;
	পূর্ণাঙ্ক fd, ret;

	fd = socket(PF_UNIX, SOCK_STREAM|SOCK_CLOEXEC|SOCK_NONBLOCK, 0);
	যদি (fd < 0) {
		warnx(_("cannot open UNIX socket"));
		গচ্ছ err;
	}

	ret = setsockopt(fd, SOL_SOCKET, SO_PASSCRED, &one, পরিমাণ(one));
	যদি (ret < 0) {
		warnx(_("cannot set option for UNIX socket"));
		close(fd);
		fd = -1;
		গচ্ছ err;
	}

	/* Note, the abstract PLYMOUTH_SOCKET_PATH has a leading NULL byte */
	ret = connect(fd, (সংরচনা sockaddr *) &su,
		offsetof(সংরচনা sockaddr_un, sun_path) + 1 + strlen(su.sun_path+1));
	যদি (ret < 0) {
		যদি (errno != ECONNREFUSED)
			warnx(_("cannot connect on UNIX socket"));
		close(fd);
		fd = -1;
		গচ্ছ err;
	}
err:
	ঘূৰাই fd;
}

পূর্ণাঙ্ক plymouth_command(পূর্ণাঙ্ক cmd, ...)
{
	uint8_t answer[2], command[2];
	সংরচনা sigaction sp, op;
	পূর্ণাঙ্ক fdsock = -1, ret = 0;

	sigemptyset (&sp.sa_mask);
	sp.sa_handler = SIG_IGN;
	sp.sa_flags = SA_RESTART;
	sigaction(SIGPIPE, &sp, &op);

	/* The plymouthd does read at least two bytes. */
	command[1] = '\0';
	ৱিকল্প (cmd) {
	পক্ষ MAGIC_PING:
		fdsock = open_un_socket_and_connect();
		যদি (fdsock >= 0) {
			command[0] = cmd;
			ul_write_all(fdsock, command, পরিমাণ(command));
		}
		ভঙ্গ;
	পক্ষ MAGIC_QUIT:
		fdsock = open_un_socket_and_connect();
		যদি (fdsock >= 0) {
			command[0] = cmd;
			ul_write_all(fdsock, command, পরিমাণ(command));
		}
		ভঙ্গ;
	স্ৱভাৱ:
		warnx(_("the plymouth request %c is not implemented"), cmd);
	পক্ষ '?':
		গচ্ছ err;
	}

	answer[0] = '\0';
	যদি (fdsock >= 0) {
		যদি (can_read(fdsock, 1000))
			ul_read_all(fdsock, (ৱর্ণ *) &answer[0], পরিমাণ(answer));
		close(fdsock);
	}
	sigaction(SIGPIPE, &op, রিক্ত);
	ret = (answer[0] == ANSWER_ACK) ? 1 : 0;
err:
	ঘূৰাই ret;
}

