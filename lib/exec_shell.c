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

#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <unistd.h>
#অন্তর্ভাব <sys/types.h>
#অন্তর্ভাব <libgen.h>

#অন্তর্ভাব "nls.h"
#অন্তর্ভাব "c.h"
#অন্তর্ভাব "xalloc.h"

#অন্তর্ভাব "exec_shell.h"
#অন্তর্ভাব "default_shell.h"

শূন্য __attribute__((__noreturn__)) exec_shell(শূন্য)
{
	ধ্রুবক অক্ষর *shell = ul_default_shell(0, ফাঁকা);
	অক্ষর *shellc;
	ধ্রুবক অক্ষর *shell_basename;
	অক্ষর *arg0;

	shellc = xstrdup(shell);
	shell_basename = basename(shellc);
	xasprintf(&arg0, "-%s", shell_basename);

	execl(shell, arg0, (অক্ষর *)ফাঁকা);
	errexec(shell);
}
