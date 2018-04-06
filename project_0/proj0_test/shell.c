#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>


char* parse(char * lineptr, char **args)
{
    //while lineIn isn't done. 
    while (*lineptr != '\0') 
    {

        //If it's whitespace, tab or newline, turn it into \0 and keep moving till we find the next token.
        //This makes sure each arg has a \0 immidiately after it without needing to copy parts of lineIn to new strings. 
        while (!(isdigit(*lineptr) || isalpha(*lineptr) || *lineptr == '-' || *lineptr == '.' || *lineptr == '\"'|| *lineptr == '/' || *lineptr == '_'))
        {	
            //break out if we reach an "end"
            if(*lineptr == '\0' || *lineptr == '<' || *lineptr == '>' || *lineptr == '|' || *lineptr == '&')
                    break;

            *lineptr = '\0';
            lineptr++;
        }

        //break out of the 2nd while loop if we reach an "end".
        if(*lineptr == '\0' || *lineptr == '<' || *lineptr == '>' || *lineptr == '|' || *lineptr == '&' )
                break;


        //mark we've found a new arg
        *args = lineptr;	
        args++;

        //keep moving till the argument ends. Once we reach a termination symbol, the arg has ended, so go back to the start of the loop.
        while (isdigit(*lineptr) || isalpha(*lineptr) || *lineptr == '-'|| *lineptr == '.' || *lineptr == '\"'|| *lineptr == '/' || *lineptr == '_')
            lineptr++;
    }
    *args = '\0';
    return lineptr;
}



void execute(char **args,int inPipe, int outPipe, int bgflag)
{
    pid_t pid;
    int status;
    
    if (inPipe == 5)
        dup2(inPipe,0);
    
    

    pid = fork();


    if (pid > 0) 
    {
        if(!bgflag)
                (void)waitpid(pid, &status, 0);
    }


    else if (pid == 0)
    {
        signal(SIGINT, SIG_DFL);
        signal(SIGTERM, SIG_DFL);

        if(inPipe != 0)
            dup2(inPipe,0);

        if(outPipe != 1)
            dup2(outPipe,1);


        int execReturn = execvp(*args, args);

        if (execReturn < 0) 
        { 
            printf("ERROR: exec failed\n");
            exit(1);
        }
        
        _exit(0);

    }


    else
    {
        printf("ERROR: Failed to fork child process.\n");
        exit(1); 
    }
    
    if(inPipe != 0)
        close(inPipe);
    
    if(outPipe != 1)
        close(outPipe);


}



//inPipe default is 0, outPipe default is 1. If you're not sending a pipe to use, use those.
void run(char * linePtr, int length, int inPipe, int outPipe)
{
    int bgflag = 0;
    
    
    int i = 0;
    while(linePtr[i] != '\0')
        i++;
    if(linePtr[i-1] == '&')
        bgflag = 1;
    
   
    char * args[length];
    char * nextChar = parse(linePtr, args);

    //if not empty
    if(args[0] != '\0')
    {

        if (strcmp(args[0], "exit") == 0) 
            exit(0);

        if(*nextChar == '<' && inPipe == 0)
        {
            char * in[length];
            nextChar = parse(nextChar+1,in);
            inPipe = open(in[0], O_RDONLY);
        }

        if(*nextChar == '>')
        {
            char * out[length];
            nextChar = parse(nextChar+1, out);
            outPipe = open(out[0], O_CREAT|O_WRONLY, 0777);
        }


        if(*nextChar == '|')
        {
            int pipes[2];
            pipe(pipes);
            execute(args,inPipe,pipes[1], bgflag);
            run(nextChar+1, length - (nextChar+1 - linePtr), pipes[0], 1);
            return;
        }

        if(*nextChar == '\0' || (*nextChar == '&' && nextChar-linePtr == length-1))
        {
            execute(args,inPipe,outPipe, bgflag);
            return;
        }


        //else: some problem, so throw a fit.
        printf("ERROR: Invalid input: %c\n",*nextChar);

    }
	
	
}


void cnt(int sig) { 
       static int count=0; 
        printf("Total of %d INTERRUPTS received\n", ++count); 
        if(count==2) exit(0);
} 


int main(int argc, char *argv[])
{
    signal(SIGINT, SIG_IGN);
    signal(SIGTERM, SIG_IGN);
    signal(SIGQUIT, SIG_DFL);
    signal(SIGTSTP, cnt);
    char lineIn[1024];



    while(1) 
    {
        if(isatty(fileno(stdin)))
                printf("sish:>"); 
        
        
        if(fgets(lineIn,1024,stdin) == NULL)
                break;
                
        
        int len = 0;
        while(lineIn[len] != '\0')
            len++;
        
        //remove the \n that fgets adds to the end
        if(len != 0 && lineIn[len-1] == '\n')
        {
            lineIn[len-1] = '\0';
            len--;
        }

        
        
        run(lineIn, len,0,1);

        

    }
    
    return 0;
}
