/**
 * shell.cc
 *
 * Contains simple implementation for shell commands, provided for CS170 S18 pa0
 * Edited by Nathan Vandervoort and Jordan Ang
 *
 * -- Sample session --
 *  $> echo hello
 *  hello
 *  $> shell: ls
 *  Makefile  simple  simple_shell.c
 *  $> exit
**/


#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <cstdlib>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <vector>
#include <string>
#include <sstream>

#define MAX_TOKEN_LENGTH 50
#define MAX_TOKEN_COUNT 100
#define MAX_LINE_LENGTH 512
#define MAX_COMMANDS_CONNECTED_BY_PIPE 3
#define TERMINAL_PROMPT "[pa0]$ "
#define EXIT_CODE EXIT_SUCCESS
//#define VERBOSE 1  // don't know how to do this automatically in CMake

using namespace std;

/** Represents one command between pipes with redirects */
class Command {
public:

    Command() : command(NULL) {}

    Command(string cmd_str) : command(NULL) {
        int arg_count = 0;
        int io_redirect_file_indicator = 0; // 0 = None, 1 = Input, 2 = Output

        // Seperate full_command by spaces
        stringstream cmd_ss(cmd_str);
        string token;

        while (getline(cmd_ss, token, ' ')) {
            if(token.empty()) continue;  // ignore leading spaces
            if (command == NULL) {
                char * cmd = new char[MAX_TOKEN_LENGTH];
                strcpy(cmd, token.c_str());
                command = args[arg_count] = cmd;
                arg_count++;
#ifdef VERBOSE
                printf("--Command: '%s'\n", command);
#endif
            } else if (io_redirect_file_indicator == 1) {
                input_redirect_filename = token;
                io_redirect_file_indicator = 0;
            } else if (io_redirect_file_indicator == 2) {
                output_redirect_filename = token;
                io_redirect_file_indicator = 0;
            } else if (token[0] == '<') {
                io_redirect_file_indicator = 1;
            } else if (token[0] == '>') {
                io_redirect_file_indicator = 2;
            } else {
                char * arg = new char[MAX_TOKEN_LENGTH];
                strcpy(arg, token.c_str());
                args[arg_count] = arg;
                arg_count++;
            }
        }

        args[arg_count] = NULL;
    }

    bool is_exit() {
        return strcmp(command, "exit") == 0;
    }

    bool is_echo_last_return() {
        return strcmp(command, "echo") == 0 && strcmp(args[1], "$?") == 0;
    }

    /** Setup redirects and execute */
    void execute() {
        setup_redirects();
        execvp(command, args);
        perror(command);
    }

    /** Set up pipe to next command and executes */
    void pipe_to_next() {
        int fd[2];
        pipe(fd);

        pid_t pid2 = fork();
        if (pid2 == -1) {           /* error        */
            perror("");
        } else if (pid2 > 0) {      /* old child    */
            // redirect next command's stdin
            dup2(fd[0], STDIN_FILENO);
            close(fd[1]);
            if (waitpid(pid2, NULL, 0) == -1) perror("Error waiting for process");
        } else {                    /* child        */
            // redirect this command's stdout
            dup2(fd[1], STDOUT_FILENO);
            close(fd[0]);
            execute();
        }
    }

    /** Create vector of Commands from string */
    static vector<Command> split_commands(string line) {
        if (line.empty()) return vector<Command>();
        line.pop_back(); // get rid of the new line

        vector<Command> list_of_commands;
        stringstream line_ss(line);
        string single_command;

        while(getline(line_ss, single_command, '|')) {
            if(list_of_commands.size() <= MAX_COMMANDS_CONNECTED_BY_PIPE) {
                list_of_commands.push_back(Command(single_command));
            } else printf("Error: Max commands connected by pipe is %d\n",
                          MAX_COMMANDS_CONNECTED_BY_PIPE);
        }

        return list_of_commands;
    }

private:
    const char* command;
    string input_redirect_filename, output_redirect_filename;
    char * args[MAX_TOKEN_COUNT];

    /** Set up redirects ('<', '>') within command */
    void setup_redirects() {
        redirect(STDIN_FILENO, input_redirect_filename);
        redirect(STDOUT_FILENO, output_redirect_filename);
    }

    static void redirect(int direction, string filename) {
        if (filename.empty()) return;
        int newfd;

        /// Easiest way to redirect input and ouput according to 'Hints and Suggestions'         ///

        // a) open the input file
        if ((newfd = open(filename.c_str(), direction == STDIN_FILENO? O_RDONLY : O_CREAT|O_TRUNC|O_RDWR, 0644)) < 0)
            perror(filename.c_str()); /* open failed */

        close(direction);           // b) close the corresponding standard file descriptor
        dup2(newfd, direction);     // c) use dup2 to make file descriptor 0 or 1 correspond to newly open file
        close(newfd);               // d) close newly opened file (without closing standard file descriptor);
    }

};

/** Run and pipe all provided commands */
void run_commands(vector<Command> commands) {
    static int last_exit_code = 0;

    if (commands.empty()) return;
    if (commands[0].is_exit()) exit(EXIT_CODE);

    // Retrieve last return value
    if (commands[0].is_echo_last_return()) {
        printf("%d\n", last_exit_code);
        return;
    }

    pid_t pid = fork();
    if (pid == -1) perror("");
    else if (pid > 0) {         /* parent   */
        int status;
        if (waitpid(pid, &status, 0) == -1) perror("Error waiting for process");
        last_exit_code = WIFEXITED(status)? WEXITSTATUS(status) : status;
    } else {                    /* child    */
        // set up each command, pipe it to the next one, and execute
        for (int i=0; i<commands.size()-1; i++) commands[i].pipe_to_next();

        // execute last command (stdin already redirected in last pipe_to_next call)
        commands.back().execute();
    }
}

void sig_hndlr(int code) {
    static int count = 0;
    if (count < 1) count++;
    else {
        printf("\n");
        exit(EXIT_CODE);
    }
}

int main(){
    char line[MAX_LINE_LENGTH];
    signal(SIGTSTP, sig_hndlr);
    printf(TERMINAL_PROMPT);
    while (fgets(line, MAX_LINE_LENGTH, stdin)) {
        run_commands(Command::split_commands(line));
        printf(TERMINAL_PROMPT);
    }
    return 0;
}
