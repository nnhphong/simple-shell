#include "byos.h"
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
// Find out what other #include's you need! (E.g., see man pages.)

#define REDIR_MODE (O_WRONLY | O_CREAT | O_TRUNC)
#define FD_NOT_EXIST -50705

int handle_forx(const struct cmd *c, int outer_redir_fd);
int handle_echo(const struct cmd *c, int outer_redir_fd);
int handl_list(const struct cmd *c, int outer_redir_fd);

int file_redir(const struct cmd *c) {
	if (c->redir_stdout != NULL) {
		int fd = open(c->redir_stdout, REDIR_MODE, 0666);
		if (fd < 0) {
			printf("Failed to open file %s\n", c->redir_stdout);
		}
		return fd;
	}
	return STDOUT_FILENO;
}

int handle_list(const struct cmd *c, int outer_redir_fd) {
	int fd = FD_NOT_EXIST;
	if (c->type == LIST && c->redir_stdout != NULL) {
		fd = file_redir(c);
	}
	else if (outer_redir_fd != FD_NOT_EXIST) {
		fd = outer_redir_fd;
	}

	if (c->type == ECHO) {
		return handle_echo(c, outer_redir_fd);
	}
	else if (c->type == FORX) {
		return handle_forx(c, outer_redir_fd);
	}
	if (c->data.list.n == 0) return 0;
	
	int ret;
	for (int i = 0; i < c->data.list.n; i++) {
		ret = handle_list(&c->data.list.cmds[i], fd);
	}
	
	if (fd != FD_NOT_EXIST && fd != outer_redir_fd) {
		close(fd);
	}	
	return ret;
}

void set_cloexec_flag(int fd) {
	int oldflag = fcntl(fd, F_GETFD);
	if (oldflag < 0) return;
	fcntl(fd, F_SETFD, oldflag | FD_CLOEXEC);
}

int handle_forx(const struct cmd *c, int outer_redir_fd) {
	int fd, saved_stdout = dup(STDOUT_FILENO);
	if ((fd = file_redir(c)) == -1) {
		return 1;
	}

	if (fd != STDOUT_FILENO) {
		// redirect stdout to file in a single command
		dup2(fd, STDOUT_FILENO);
	}
	else if (outer_redir_fd != FD_NOT_EXIST) { 
		// this command is inside a group that need redirection
		fd = outer_redir_fd;
		dup2(fd, STDOUT_FILENO);
	}	

	int p = fork();
	if (!p) {
		set_cloexec_flag(saved_stdout);
		set_cloexec_flag(fd);
		if (execvp(c->data.forx.pathname, c->data.forx.argv) == -1) {
			exit(127);
		}
	}
		
	if (fd != STDOUT_FILENO) {
		dup2(saved_stdout, STDOUT_FILENO); // restore stdout
		close(saved_stdout); 
		if (fd != outer_redir_fd) {
			// not close now as we need it for other commands as well
			close(fd); 
		}
	}
	
	int status;
	wait(&status);
	if (WIFEXITED(status)) return WEXITSTATUS(status);
	if (WIFSIGNALED(status)) return 128 + WTERMSIG(status);
}

int handle_echo(const struct cmd *c, int outer_redir_fd) {
	int fd;
	if ((fd = file_redir(c)) == -1) {
		return 1;
	}

	if (fd == STDOUT_FILENO && outer_redir_fd != FD_NOT_EXIST) {
		fd = outer_redir_fd;
	}

	write(fd, c->data.echo.arg, strlen(c->data.echo.arg));
	if (fd != STDOUT_FILENO && fd != outer_redir_fd) {
		close(fd);
	}
	return 0;
}

int interp(const struct cmd *c) {
	if (c->type == LIST) {
		handle_list(c, FD_NOT_EXIST);
	}
	else if (c->type == FORX) {
		handle_forx(c, FD_NOT_EXIST);
	}
	else {
		handle_echo(c, FD_NOT_EXIST);
	}
}			
