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

#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <limits.h>
#ಅನ್ತರ್ಭಾವ <poll.h>
#ಅನ್ತರ್ಭಾವ <signal.h>
#ಅನ್ತರ್ಭಾವ <stdarg.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <sys/mman.h>
#ಅನ್ತರ್ಭಾವ <sys/socket.h>
#ಅನ್ತರ್ಭಾವ <sys/types.h>
#ಅನ್ತರ್ಭಾವ <sys/un.h>
#ಅನ್ತರ್ಭಾವ <unistd.h>

#ಅನ್ತರ್ಭಾವ "all-io.h"
#ಅನ್ತರ್ಭಾವ "c.h"
#ಅನ್ತರ್ಭಾವ "nls.h"
#ಅನ್ತರ್ಭಾವ "plymouth-ctrl.h"

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ can_read(ಪೂರ್ಣಾಙ್ಕ fd, ಧ್ರುವ long timeout)
{
	ಸಂರಚನಾ pollfd fds = {
		.fd = fd,
		.events = POLLIN|POLLPRI,
		.revents = 0,
	};
	ಪೂರ್ಣಾಙ್ಕ ret;

	ಕುರು {
		ret = poll(&fds, 1, timeout);
	} ಯಾವತ್ ((ret < 0) && (errno == EINTR));

	ಪ್ರತ್ಯಾಗಮ (ret == 1) && (fds.revents & (POLLIN|POLLPRI));
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ open_un_socket_and_connect(ಶೂನ್ಯ)
{
	/* The abstract UNIX socket of plymouth */
	ಸಂರಚನಾ sockaddr_un su = {
		.sun_family = AF_UNIX,
		.sun_path = PLYMOUTH_SOCKET_PATH,
	};
	ಧ್ರುವ ಪೂರ್ಣಾಙ್ಕ one = 1;
	ಪೂರ್ಣಾಙ್ಕ fd, ret;

	fd = socket(PF_UNIX, SOCK_STREAM|SOCK_CLOEXEC|SOCK_NONBLOCK, 0);
	ಯದಿ (fd < 0) {
		warnx(_("cannot open UNIX socket"));
		ಗಚ್ಛ err;
	}

	ret = setsockopt(fd, SOL_SOCKET, SO_PASSCRED, &one, ಪರಿಮಾಣ(one));
	ಯದಿ (ret < 0) {
		warnx(_("cannot set option for UNIX socket"));
		close(fd);
		fd = -1;
		ಗಚ್ಛ err;
	}

	/* Note, the abstract PLYMOUTH_SOCKET_PATH has a leading NULL byte */
	ret = connect(fd, (ಸಂರಚನಾ sockaddr *) &su,
		offsetof(ಸಂರಚನಾ sockaddr_un, sun_path) + 1 + strlen(su.sun_path+1));
	ಯದಿ (ret < 0) {
		ಯದಿ (errno != ECONNREFUSED)
			warnx(_("cannot connect on UNIX socket"));
		close(fd);
		fd = -1;
		ಗಚ್ಛ err;
	}
err:
	ಪ್ರತ್ಯಾಗಮ fd;
}

ಪೂರ್ಣಾಙ್ಕ plymouth_command(ಪೂರ್ಣಾಙ್ಕ cmd, ...)
{
	uint8_t answer[2], command[2];
	ಸಂರಚನಾ sigaction sp, op;
	ಪೂರ್ಣಾಙ್ಕ fdsock = -1, ret = 0;

	sigemptyset (&sp.sa_mask);
	sp.sa_handler = SIG_IGN;
	sp.sa_flags = SA_RESTART;
	sigaction(SIGPIPE, &sp, &op);

	/* The plymouthd does read at least two bytes. */
	command[1] = '\0';
	ವಿಕಲ್ಪ (cmd) {
	ಪಕ್ಷ MAGIC_PING:
		fdsock = open_un_socket_and_connect();
		ಯದಿ (fdsock >= 0) {
			command[0] = cmd;
			ul_write_all(fdsock, command, ಪರಿಮಾಣ(command));
		}
		ಭಙ್ಗ;
	ಪಕ್ಷ MAGIC_QUIT:
		fdsock = open_un_socket_and_connect();
		ಯದಿ (fdsock >= 0) {
			command[0] = cmd;
			ul_write_all(fdsock, command, ಪರಿಮಾಣ(command));
		}
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		warnx(_("the plymouth request %c is not implemented"), cmd);
	ಪಕ್ಷ '?':
		ಗಚ್ಛ err;
	}

	answer[0] = '\0';
	ಯದಿ (fdsock >= 0) {
		ಯದಿ (can_read(fdsock, 1000))
			ul_read_all(fdsock, (ವರ್ಣ *) &answer[0], ಪರಿಮಾಣ(answer));
		close(fdsock);
	}
	sigaction(SIGPIPE, &op, ರಿಕ್ತ);
	ret = (answer[0] == ANSWER_ACK) ? 1 : 0;
err:
	ಪ್ರತ್ಯಾಗಮ ret;
}

