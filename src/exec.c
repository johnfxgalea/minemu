
/* This file is part of minemu
 *
 * Copyright 2010-2011 Erik Bosman <erik@minemu.org>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <string.h>
#include <errno.h>

#include "exec.h"
#include "error.h"
#include "syscalls.h"
#include "load_elf.h"
#include "load_script.h"
#include "options.h"
#include "threads.h"

int can_load_binary(elf_prog_t *prog)
{
	int err = can_load_elf(prog);

	if (err)
		err = can_load_script(prog);

    debug("exec.c: Checking if can load binary %d", err);
	return err;
}

int load_binary(elf_prog_t *prog)
{
    debug("exec.c: calling load_elf");
	if ( load_elf(prog) == 0 )
		return 0;

    debug("exec.c: Loading elf failed, trying script");
	return load_script(prog);
}

static char *exec_argv[65536+64];
static long argv_lock = 0;

long user_execve(char *filename, char *argv[], char *envp[])
{
    debug("exec.c: user_execve syscall encountered for %s", filename);
	unsigned long count = strings_count(argv);
	if ( count + option_args_count() + 2 > sizeof(exec_argv)/sizeof(char*) )
		return -E2BIG;

	elf_prog_t prog =
	{
		.filename = filename,
		.argv     = argv,
		.envp     = envp,
	};
	long ret = can_load_binary(&prog);
	if (ret)
		return ret;

	/* from this point on we assume that the execve will succeed
	 * any failures will result in a SIGKILL, just as the kernel
	 * would generate after the point of no return.
	 */

	mutex_lock(&argv_lock); /* get exclusive access to the argv buffer */
	exec_argv[0] = argv[0];
	char maskbuf[17];
	char **user_argv = option_args_setup(&exec_argv[1], filename, maskbuf);
	memcpy(user_argv, argv, sizeof(char *)*(count+1));
	sys_execve_or_die("/proc/self/exe", exec_argv, envp);
	return 0xdeadbeef;
}

