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

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ can_read(ಪೂರ್ಣಾಂಕ fd, ಸ್ಥಿರಾಂಕ ದೀರ್ಘ timeout)
{
	ರಚನೆ pollfd fds = {
		.fd = fd,
		.events = POLLIN|POLLPRI,
		.revents = 0,
	};
	ಪೂರ್ಣಾಂಕ ret;

	ಮಾಡು {
		ret = poll(&fds, 1, timeout);
	} ಆಗ ((ret < 0) && (errno == EINTR));

	ಹಿಂತಿರುಗಿ (ret == 1) && (fds.revents & (POLLIN|POLLPRI));
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ open_un_socket_and_connect(ಶೂನ್ಯ)
{
	/* The abstract UNIX socket of plymouth */
	ರಚನೆ sockaddr_un su = {
		.sun_family = AF_UNIX,
		.sun_path = PLYMOUTH_SOCKET_ಮಾರ್ಗ,
	};
	ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ one = 1;
	ಪೂರ್ಣಾಂಕ fd, ret;

	fd = socket(PF_UNIX, SOCK_STREAM|SOCK_CLOEXEC|SOCK_NONBLOCK, 0);
	ಆದರೆ (fd < 0) {
		warnx(_("cannot open UNIX socket"));
		ಹೋಗು err;
	}

	ret = setsockopt(fd, SOL_SOCKET, SO_PASSCRED, &one, ಗಾತ್ರ(one));
	ಆದರೆ (ret < 0) {
		warnx(_("cannot set option for UNIX socket"));
		close(fd);
		fd = -1;
		ಹೋಗು err;
	}

	/* Note, the abstract PLYMOUTH_SOCKET_PATH has a leading NULL byte */
	ret = connect(fd, (ರಚನೆ sockaddr *) &su,
		offsetof(ರಚನೆ sockaddr_un, sun_path) + 1 + ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(su.sun_path+1));
	ಆದರೆ (ret < 0) {
		ಆದರೆ (errno != ECONNREFUSED)
			warnx(_("cannot connect on UNIX socket"));
		close(fd);
		fd = -1;
		ಹೋಗು err;
	}
err:
	ಹಿಂತಿರುಗಿ fd;
}

ಪೂರ್ಣಾಂಕ plymouth_command(ಪೂರ್ಣಾಂಕ cmd, ...)
{
	uint8_t answer[2], command[2];
	ರಚನೆ sigaction sp, op;
	ಪೂರ್ಣಾಂಕ fdsock = -1, ret = 0;

	sigemptyset (&sp.sa_mask);
	sp.sa_handler = SIG_IGN;
	sp.sa_flags = SA_RESTART;
	sigaction(SIGPIPE, &sp, &op);

	/* The plymouthd does read at least two bytes. */
	command[1] = '\0';
	ಆಯ್ಕೆ (cmd) {
	ಸ್ಥಿತಿ MAGIC_PING:
		fdsock = open_un_socket_and_connect();
		ಆದರೆ (fdsock >= 0) {
			command[0] = cmd;
			ul_write_all(fdsock, command, ಗಾತ್ರ(command));
		}
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ MAGIC_QUIT:
		fdsock = open_un_socket_and_connect();
		ಆದರೆ (fdsock >= 0) {
			command[0] = cmd;
			ul_write_all(fdsock, command, ಗಾತ್ರ(command));
		}
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		warnx(_("the plymouth request %c is not implemented"), cmd);
	ಸ್ಥಿತಿ '?':
		ಹೋಗು err;
	}

	answer[0] = '\0';
	ಆದರೆ (fdsock >= 0) {
		ಆದರೆ (can_read(fdsock, 1000))
			ul_read_all(fdsock, (ಅಕ್ಷರ *) &answer[0], ಗಾತ್ರ(answer));
		close(fdsock);
	}
	sigaction(SIGPIPE, &op, ಖಾಲಿ);
	ret = (answer[0] == ANSWER_ACK) ? 1 : 0;
err:
	ಹಿಂತಿರುಗಿ ret;
}

