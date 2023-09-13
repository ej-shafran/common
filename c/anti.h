#ifndef __ANTI_H
#define __ANTI_H

#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/wait.h>
#include <unistd.h>

#define da(type)                 \
	struct {                 \
		type *items;     \
		size_t count;    \
		size_t capacity; \
	}

#define DA_INIT_CAP 255

#define da_append(da, item)                                                    \
	do {                                                                   \
		if ((da)->count >= (da)->capacity) {                           \
			(da)->capacity = (da)->capacity == 0 ?                 \
						 DA_INIT_CAP :                 \
						 (da)->capacity * 2;           \
			(da)->items = (typeof((da)->items)) realloc((da)->items,                     \
					      (da)->capacity *                 \
						      sizeof(*(da)->items));   \
			assert((da)->items != NULL && "Could not reallocate"); \
		}                                                              \
                                                                               \
		(da)->items[(da)->count++] = (item);                           \
	} while (0)

#define da_append_many(da, new_items, new_items_count)                         \
	do {                                                                   \
		if ((da)->count + new_items_count > (da)->capacity) {          \
			if ((da)->capacity == 0) {                             \
				(da)->capacity = DA_INIT_CAP;                  \
			}                                                      \
			while ((da)->count + new_items_count >                 \
			       (da)->capacity) {                               \
				(da)->capacity *= 2;                           \
			}                                                      \
			(da)->items = realloc((da)->items,                     \
					      (da)->capacity *                 \
						      sizeof(*(da)->items));   \
			assert((da)->items != NULL && "Could not reallocate"); \
		}                                                              \
		memcpy((da)->items + (da)->count, new_items,                   \
		       new_items_count * sizeof(*(da)->items));                \
		(da)->count += new_items_count;                                \
	} while (0);

#define da_foreach(type, item, da) \
	for (type *item = (da).items; item < (da).items + (da).count; item++)

#define sb_append_cstr(sb, cstr)          \
	do {                              \
		const char *s = (cstr);   \
		int n = strlen(s);        \
		da_append_many(sb, s, n); \
	} while (0);
#define sb_append_nul(sb) da_append(sb, '\0')

#define cmd_append(cmd, ...) cmd_append_null(cmd, __VA_ARGS__, NULL)

typedef da(char) String_Builder;
typedef da(const char *) Cmd;

void cmd_append_null(Cmd *cmd, ...);

void cmd_render(Cmd cmd, String_Builder *output);

#ifdef __WIN32
typedef HANDLE Pid;
#define INVALID_PID NULL
#else
typedef int Pid;
#define INVALID_PID -1
#endif // __WIN32

Pid cmd_run_async(Cmd cmd);

bool proc_wait(Pid pid);

bool cmd_run_sync(Cmd cmd);

#define ANTI_IMPLEMENTATION
#ifdef ANTI_IMPLEMENTATION

void cmd_append_null(Cmd *cmd, ...)
{
	va_list args;

	va_start(args, cmd);

	const char *arg = va_arg(args, const char *);
	while (arg != NULL) {
		da_append(cmd, arg);

		arg = va_arg(args, const char *);
	}

	va_end(args);
}

void cmd_render(Cmd cmd, String_Builder *output)
{
	da_foreach(const char *, item, cmd)
	{
		sb_append_cstr(output, *item);
		da_append(output, ' ');
	}
	sb_append_nul(output);
}

Pid cmd_run_async(Cmd cmd)
{
	String_Builder sb = { 0 };
	cmd_render(cmd, &sb);
	fprintf(stderr, "[CMD] %s\n", sb.items);
	free(sb.items);

	pid_t cpid = fork();
	if (cpid < 0) {
		fprintf(stderr, "[ERROR] Could not fork child process: %s\n",
			strerror(errno));
		return INVALID_PID;
	}

	if (cpid == 0) {
		if (execvp(cmd.items[0], (char *const *)cmd.items) < 0) {
			fprintf(stderr,
				"[ERROR] Could not exec child process: %s\n",
				strerror(errno));
			exit(1);
		}
	}

	return cpid;
}

bool proc_wait(Pid pid)
{
	for (;;) {
		int wstatus = 0;
		if (waitpid(pid, &wstatus, 0) < 0) {
			fprintf(stderr,
				"[ERROR] Could not wait on comand (pid %d): %s\n",
				pid, strerror(errno));
			return false;
		}

		if (WIFEXITED(wstatus)) {
			int exit_status = WEXITSTATUS(wstatus);
			if (exit_status != 0) {
				fprintf(stderr,
					"[ERROR] Comand exited with exit code %d\n",
					exit_status);
				return false;
			}

			return true;
		}
	}
}

bool cmd_run_sync(Cmd cmd)
{
	Pid pid = cmd_run_async(cmd);
	if (pid == INVALID_PID)
		return false;
	return proc_wait(pid);
}

#endif // ANTI_IMPLEMENTATION

#endif // __ANTI_H
