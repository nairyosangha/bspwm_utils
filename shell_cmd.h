#ifndef SHELL_CMD_H
#define SHELL_CMD_H
#include <sys/types.h>

#define READ 	(0)
#define WRITE 	(1)
typedef struct process {
	pid_t pid;
	int fd_write; // FD that process with pid writes to
	int fd_read; // FD that process with pid reads from
} process_t;

process_t process(char *cmd[]);

#endif
