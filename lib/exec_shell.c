/*
 * exec_shell() - launch a shell, else exit!
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://gnu.org/licenses/>.
 */

#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <unistd.h>
#ଅନ୍ତର୍ଭାଵ <sys/types.h>
#ଅନ୍ତର୍ଭାଵ <libgen.h>

#ଅନ୍ତର୍ଭାଵ "nls.h"
#ଅନ୍ତର୍ଭାଵ "c.h"
#ଅନ୍ତର୍ଭାଵ "xalloc.h"

#ଅନ୍ତର୍ଭାଵ "exec_shell.h"
#ଅନ୍ତର୍ଭାଵ "default_shell.h"

ଶୂନ୍ୟ __attribute__((__noreturn__)) exec_shell(ଶୂନ୍ୟ)
{
	ଅଚଳ ଅକ୍ଷର *shell = ul_default_shell(0, ଖାଲି);
	ଅକ୍ଷର *shellc;
	ଅଚଳ ଅକ୍ଷର *shell_basename;
	ଅକ୍ଷର *arg0;

	shellc = xstrdup(shell);
	shell_basename = basename(shellc);
	xasprintf(&arg0, "-%s", shell_basename);

	execl(shell, arg0, (ଅକ୍ଷର *)ଖାଲି);
	errexec(shell);
}
