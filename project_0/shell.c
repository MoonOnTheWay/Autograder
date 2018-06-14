//CS170 Project 0 by Ryan Kirkpatrick and Victoria Sneddon
#include <signal.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

#define MAX_TOKEN_LENGTH 50
#define MAX_TOKEN_COUNT 100
#define MAX_LINE_LENGTH 512

// Simple implementation for Shell command
// Assume all arguments are seperated by space
// Erros are ignored when executing fgets(), fork(), and waitpid().

/**
 * Sample session
 *  shell: echo hello
 *   hello
 *   shell: ls
 *   Makefile  simple  simple_shell.c
 *   shell: exit
 **/

//Second handler for the signal to quit program on second Ctrl-Z
void signalQuitter(){
    exit(0);
}
//First signal handler to switch Ctrl-Z signal to run signalQuitter
void signalHandler(int sig){
    signal(SIGTSTP, signalQuitter);
}

//Basic run command function for replacing a process and executing
void runcommand(char** args){
    execvp(args[0], args);
    perror(args[0]);
    exit(1);
}

//connects a file as input to a cmd.
//cmdargs is the tokens of the full command
//inputPos is the position of a < in cmdargs
void redirectInput(char** cmdargs, int inputPos){
    if(inputPos != -2){
        char* filename;
        filename = cmdargs[inputPos + 1];

        int fd = open(filename, O_RDONLY);
        if(fd < 0){exit(1);}

        dup2(fd, 0);
        close(fd);
    }
}

//connects a file as output to a cmd.
//cmdargs is the tokens of the full command
//outputPos is the position of a > in cmdargs
void redirectOutput(char** cmdargs, int outputPos){
    if(outputPos != -2){
        char* filename;
        filename = cmdargs[outputPos + 1];

        int fd = open(filename, O_WRONLY | O_CREAT, 0666);
        if(fd < 0){exit(1);}

        dup2(fd, 1);
        close(fd);
    }
}

//Handles input and output redirection of a full command
//cmdargs are the tokens of the full command
void redirectHandler(char** cmdargs){
    char* args[MAX_TOKEN_COUNT];

    //Find the positions of the < and > in the cmdargs
    //And remove them and the filename from cmdargs and place into args
    int inputPos  = -2;
    int outputPos = -2;
    int i = 0;
    int z = 0;
    while(cmdargs[z] != NULL){
        if      (strcmp(cmdargs[z], "<") == 0){
            inputPos  = z;
            z = z + 2;
        }else if(strcmp(cmdargs[z], ">") == 0){
            outputPos = z;
            z = z + 2;
        }else{
            args[i] = cmdargs[z];
            i++;
            z++;
        }
    }
    args[i] = NULL;

    // i = 0;
    // while(args[i] != NULL){
    //     printf("args: %s\n", args[i]);
    //     i++;
    // }

    redirectInput(cmdargs, inputPos);
    redirectOutput(cmdargs, outputPos);
    runcommand(args);

}

//Separate commands by pipe and run each command
//args is the arguments of all commands
void pipeHandler(char** args){
    char* cmd1[MAX_TOKEN_COUNT];
    char* cmd2[MAX_TOKEN_COUNT];
    char* cmd3[MAX_TOKEN_COUNT];


    int cmdpos1 = 0;
    int cmdpos2 = 0;
    int cmdpos3 = 0;
    int numcmds = 1;
    int z = 0;

    //Add arguments to cmd lists
    while(args[z] != NULL){
        if(strcmp(args[z], "|") == 0){
            numcmds++;
        }else{
            switch(numcmds){
                case 1:{
                    cmd1[cmdpos1] = args[z];
                    cmdpos1++;
                    break;
                }case 2:{
                    cmd2[cmdpos2] = args[z];
                    cmdpos2++;
                    break;
                }case 3:{
                    cmd3[cmdpos3] = args[z];
                    cmdpos3++;
                    break;
                }
            }
        }
        z++;
    }
    cmd1[cmdpos1] = NULL;
    cmd2[cmdpos2] = NULL;
    cmd3[cmdpos3] = NULL;

    int fd1[2];
    int fd2[2];
    pipe(fd1);
    pipe(fd2);

    if(fork() == 0){//child1 cmd1
        if(numcmds > 0){
            if(numcmds > 1){ //pipe to second cmd
                dup2(fd1[1], 1);
            }

            close(fd1[0]);
            close(fd1[1]);
            close(fd2[0]);
            close(fd2[1]);
            redirectHandler(cmd1);
        } else{exit(0);}
    }
    else if(fork() == 0){//child2 cmd 2
        if(numcmds > 1){
            //Read from command 1
            dup2(fd1[0], 0);
            if(numcmds == 3){
                dup2(fd2[1], 1);
            }

            close(fd1[0]);
            close(fd1[1]);
            close(fd2[0]);
            close(fd2[1]);
            redirectHandler(cmd2);
        }else{exit(0);}

    }
    else{ //parent or cmd 3
        if(numcmds == 3){
            //Read from pipe 2
            dup2(fd2[0], 0);

            close(fd1[0]);
            close(fd1[1]);
            close(fd2[0]);
            close(fd2[1]);

            waitpid(-1, NULL, 0);
            redirectHandler(cmd3);
        }else{exit(0);}
    }
}

//creates child process to run the commands
void runCommands(char** args){
    pid_t pid = fork();
    if(pid) { // parent
        waitpid(pid, NULL, 0);
        usleep(5000);
    } else { // child
        // int z = 0;
        // while(args[z] != NULL){
        //     printf("args: %s \n",args[z]);
        //     z++;
        // }
        pipeHandler(args);
    }
}

int main(){
    char line[MAX_LINE_LENGTH];
    //printf("shell: ");


    signal(SIGTSTP, signalHandler);
    while(fgets(line, MAX_LINE_LENGTH, stdin)) {

        // Build the command and arguments, using execv conventions.
        //THIS IS THE FIX
        if(strlen(line) == 1) //reached eof or blank so exit
            exit(0);
        line[strlen(line)-1] = '\0'; // get rid of the new line
        char* arguments[MAX_TOKEN_COUNT];
        int argument_count = 0;

        char* token = strtok(line, " ");
        while(token) {
            arguments[argument_count] = token;
            //printf("%s\n", token);
            argument_count++;
            token = strtok(NULL, " ");
        }
        arguments[argument_count] = NULL;

        if(argument_count>0) {
            if (strcmp(arguments[0], "exit") == 0) exit(0);

            // int z = 0;
            // while(arguments[z] != NULL){
            //     printf("arguments: %s \n",arguments[z]);
            //     z++;
            // }
            runCommands(arguments);
        }
        //printf("shell: ");
    }
    return 0;
}