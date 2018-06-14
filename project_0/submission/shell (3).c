#include <stdio.h>
#include <string.h> // strtok
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <fcntl.h> // file control

#define MAX_TOKEN_LENGTH 32
#define MAX_TOKEN_COUNT 100
#define MAX_LINE_LENGTH 512
#define MAX_COMMAND_COUNT 3

typedef enum { false, true } bool; // custom bool type

struct cmd {
	char **arguments;
	char input_file[MAX_LINE_LENGTH];
	char output_file[MAX_LINE_LENGTH];
	bool has_input_file;
	bool has_output_file;
};

// parses a line to get the arguments, the input file, and the output file
struct cmd parse_cmd(char* cmd_arr) {
	char** arguments = malloc(MAX_TOKEN_COUNT * sizeof(char*));
	int argument_count = 0;
	char* token = strtok(&cmd_arr[0], " ");
	bool expecting_input_file = false;
	bool expecting_output_file = false;
	struct cmd command = {0, "", "", false, false};
	while(token) {
		if (expecting_input_file) {
			strcpy(command.input_file, token);
			command.has_input_file = true;
			expecting_input_file = false;
		}
		else if (expecting_output_file) {
			strcpy(command.output_file, token);
			command.has_output_file = true;
			expecting_output_file = false;
		}
		else if (strcmp(token, "<") == 0) {
			expecting_input_file = true;
		}
		else if (strcmp(token, ">") == 0) {
			expecting_output_file = true;
		}
		else {
			arguments[argument_count] = token;
			argument_count++;
		}
		token = strtok(NULL, " ");
	}
	if (expecting_input_file || expecting_output_file) {
		perror("bash: syntax error near unexpected token `newline'\n");
	}
	arguments[argument_count] = NULL;
	if(argument_count > 0 && strcmp(arguments[0], "exit") == 0)
		exit(0);
	command.arguments = arguments;
	return command;
}

void runcommands(char** cmd_arr, int length) {
	int new_fds[2], old_fds[2];
	bool prev_cmd = false, next_cmd = false;
	for(int i = 0; i < length; i++){
		struct cmd command = parse_cmd(cmd_arr[i]);
		// check if pipe is needed
		if(i < length - 1){
			pipe(new_fds);
			next_cmd = true;
		}
		else{
			next_cmd = false;
		}
		pid_t pid = fork();
		if(pid == 0){ // child
			// piping
			if(prev_cmd){
				// redirect output to input of next and close input fd
				dup2(old_fds[0], STDIN_FILENO);
				close(old_fds[0]);
				close(old_fds[1]);
			}
			if(next_cmd){
				
				close(new_fds[0]);
				dup2(new_fds[1], STDOUT_FILENO);
				close(new_fds[1]);
			}
			// i/o redirection
			if (command.has_input_file) {
				// if < file (read)
				int input_fd;
				if ((input_fd = open(command.input_file, O_RDONLY, 0)) < 0) {
					perror(command.input_file);
					exit(-1);
				}
				dup2(input_fd, STDIN_FILENO);
				close(input_fd);
			}
			if (command.has_output_file) {
				// if > file (create or truncate)
				int output_fd;
				if ((output_fd = open(command.output_file, O_CREAT|O_TRUNC|O_WRONLY, 0644)) < 0) {
					perror(command.output_file);
					exit(-1);
				}
				dup2(output_fd, STDOUT_FILENO);
				close(output_fd);
			}
			if(execvp(command.arguments[0], command.arguments) == -1)
				perror(command.arguments[0]);
		}
		else{ // parent
			if(prev_cmd){
				close(old_fds[0]);
				close(old_fds[1]);
			}
			if(next_cmd){
				old_fds[0] = new_fds[0];
				old_fds[1] = new_fds[1];
			}
	    	waitpid(pid, NULL, 0);
		}
		free(command.arguments);
		prev_cmd = true;
	}
	if(length > 1){
		close(old_fds[0]);
		close(old_fds[1]);
	}
}

void handler(int sig) {
	static int count = 0;
	count++;
	if(count == 1)
		signal(SIGTSTP, SIG_DFL);
}

int main(){
	signal(SIGTSTP, handler);
	char line[MAX_LINE_LENGTH];
	int i;
	while(fgets(line, MAX_LINE_LENGTH, stdin)) {
		// Build the command and arguments, using execv conventions.
		line[strlen(line)-1] = '\0'; // get rid of the new line
		char* cmd_arr[MAX_COMMAND_COUNT];
		int cmd_count = 0;
		char* token = strtok(line, "|");

		while(token) {
			cmd_arr[cmd_count] = token;
			cmd_count++;
			token = strtok(NULL, "|");
		}
		runcommands(cmd_arr, cmd_count);
	}
	return 0;
}
