#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <signal.h>
#define MAX_TOKEN_LENGTH 50
#define MAX_TOKEN_COUNT 100
#define MAX_LINE_LENGTH 512
#define STDIN 0
#define STDOUT 1
#define READ_END 0
#define WRITE_END 1

void runcommand(char* command, char** args, int read_fd, int write_fd) {
	pid_t pid = fork();
	
	if(pid != 0) { 
		// parent
		
		//if write_fd is the read end of a pipe
		//then close it
		if(write_fd != STDOUT){
			close(write_fd);
		}
		waitpid(pid, NULL, 0);
	} else {
		// child
		
		// redirection for < and >
		int i=0;
		char* outfile = NULL;
		char* infile = NULL;
		int infd = -1;
		int outfd = -1;
	
		while(args[i] != NULL) {
			if(strcmp(args[i], "<") == 0) {
				if(args[i+1] != NULL)
					infile = args[i+1];
					// replace < with NULL
					args[i] = NULL;
			}

			else if(strcmp(args[i], ">") == 0) {
				if(args[i+1] != NULL) {
					outfile = args[i+1];
					// replace > with NULL
					args[i] = NULL;
				}
			} 
			i++;
		}
		// file read/write
		if(infile && (infile[0]!='\0')) {
			//stdin changed to <
			read_fd = open(infile, O_RDONLY, 0666);
			infd = read_fd;
			// case of nonexistent file
			if(infd != -1) {
				// dupe stdin to be the file we are reading from
				dup2(read_fd, STDIN_FILENO);
			} else {
				perror(command);
				exit(1);
			}
			close(infd);
		}
		if(outfile && (outfile[0]!='\0')) {
			// stdout changed to >
			write_fd = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
			outfd = write_fd;
			dup2(outfd, STDOUT_FILENO);
			close(outfd);
		}
		
		// piping cases for files
		if(read_fd != infd){
			dup2(read_fd, STDIN_FILENO);
		}
		if(write_fd != outfd){
			dup2(write_fd, STDOUT_FILENO);
		}
		
		//error check for faulty exec
		if (execvp(command, args) == -1) {
			perror(command);
			exit(1);
		}
	}
}

void buildArgs(char** arguments, int argument_count){
	char* arg[MAX_TOKEN_COUNT];
	int arg_index = 0;
	int read_fd = STDIN;
	int write_fd = STDOUT;
	int i;
	int fd[2];
	for(i=0; i<=argument_count; i++){
		//last pipe command
		if(arguments[i] == NULL){
			arg[arg_index] = NULL;
			
			//change write pipe to stdout
			if(write_fd != STDOUT){
				read_fd = fd[READ_END];
				write_fd = STDOUT;
			}
			runcommand(arg[0], arg, read_fd, write_fd);
			read_fd = STDIN;
			write_fd = STDOUT;
		}
		
		//pipe case
		else if(strncmp(arguments[i], "|", 2) == 0){
			if(read_fd != STDIN | write_fd != STDOUT){
				read_fd = fd[READ_END];
			}
			pipe(fd);
			write_fd = fd[WRITE_END];
			arg[arg_index] = NULL;
			runcommand(arg[0], arg, read_fd, write_fd);
			arg_index = 0;
		}
		//build arg array
		else{
			arg[arg_index] = arguments[i];
			arg_index++;
		}
	}
}

//ctrl+z twice
void stop2(int sig) {
	exit(0);
}

//ctrl+z once
void stop1(int sig) {
	signal(SIGTSTP, stop2);
}

int main(){
	signal(SIGTSTP, stop1);
	char line[MAX_LINE_LENGTH];
	while(fgets(line, MAX_LINE_LENGTH, stdin)) {
		//build char array of command line arguments

		line[strlen(line)-1] = '\0'; // get rid of the new line
		char* command = NULL;
		char* arguments[MAX_TOKEN_COUNT];
		int argument_count = 0;

		char* token = strtok(line, " ");
		while(token) {
			arguments[argument_count] = token;
			argument_count++;
			token = strtok(NULL, " ");
		}
		//null terminate end of line
		arguments[argument_count] = NULL;

		// exit case
		command = arguments[0];
		if(argument_count>0 && strcmp(command, "exit") == 0)
			exit(0);
		
		// done reading in everything, time to parse
		buildArgs(arguments, argument_count);
	}
	return 0;
}