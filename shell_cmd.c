#include <unistd.h>
#include "shell_cmd.h"

process_t process(char *cmd[]) {
	int pipe_write[2], pipe_read[2];
	pipe(pipe_write);
	pipe(pipe_read);
	// process_t gets passed to parent, so we want access to 
	// the READ end of the pipe that child writes to, and
	// the WRITE end of the pipe that child reads from
	process_t process = { fork(), pipe_write[READ], pipe_read[WRITE] };
	if (process.pid == 0) { // child process
		dup2(pipe_write[WRITE], STDOUT_FILENO);
		close(pipe_write[READ]); close(pipe_write[WRITE]);
		dup2(pipe_read[READ], STDIN_FILENO);
		close(pipe_read[READ]); close(pipe_read[WRITE]);
		execvp(cmd[0], cmd);
	}
	// these ends aren't necessary in parent, close them
	close(pipe_write[WRITE]); 
	close(pipe_read[READ]); 
	return process;
}
