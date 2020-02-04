#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <errno.h>

/*
 * This function parses the next subcommand seperated by |, <, > etc
 * Place the content in args where args[0] = command name
 * args[1] [2] etc will be the arguments
 *
 * Return the leading character after this command,
 * which could be |, <, >, or a regular character
 * For example,  for "ls > tmp",  return the position of ">"
 * For example,  for "ls | wc ",  return the position of "|"
 */
char* parse(char * lineptr, char **args)
{
    //while lineIn isn't done.
    while (*lineptr != '\0')
    {
        // If it's whitespace, tab or newline,
        // turn it into \0 and keep moving till we find the next token.
        // This makes sure each arg has a \0 immidiately after it
        // without needing to copy parts of lineIn to new strings.
        while (!(isdigit(*lineptr) || isalpha(*lineptr) || *lineptr == '-'
                 || *lineptr == '.' || *lineptr == '\"'|| *lineptr == '/'
                 || *lineptr == '_'))
        {
            //break out if we reach an "end"
            if(*lineptr == '\0' || *lineptr == '<' || *lineptr == '>'
               || *lineptr == '|' || *lineptr == '&')
                break;

            *lineptr = '\0';
            lineptr++;
        }

        //break out of the 2nd while loop if we reach an "end".
        if(*lineptr == '\0' || *lineptr == '<' || *lineptr == '>'
           || *lineptr == '|' || *lineptr == '&' )
            break;

        //mark we've found a new arg
        *args = lineptr;
        args++;

        // keep moving till the argument ends. Once we reach a termination symbol,
        // the arg has ended, so go back to the start of the loop.
        while (isdigit(*lineptr) || isalpha(*lineptr) || *lineptr == '-'
               || *lineptr == '.' || *lineptr == '\"'|| *lineptr == '/'
               || *lineptr == '_')
            lineptr++;
    }

    *args = NULL;

    return lineptr;
}


/*
 * This function forks a child to execute  a command stored
 * in a string array *args[].
 * inPipe -- the input stream file descriptor.  For example,  wc < temp
 * outPipe --the output stream file descriptor.  For example, ls > temp
 */
void fchild(char **args,int inPipe, int outPipe)
{
    pid_t pid;

    pid = fork();
    if (pid == 0)/*Child  process*/
    {

        /*Call dup2 to setup redirection, and then call excevep*/
        dup2(inPipe, 0); //inPipe -> stdin
        dup2(outPipe, 1); //outPipe -> stdout
        int execReturn = execvp(args[0], args);
        /*Your solution*/

        if (execReturn < 0)
        {
            printf("ERROR: exec failed\n");
            exit(1);
        }

        _exit(0);

    }

    if (pid < 0)
    {
        printf("ERROR: Failed to fork child process.\n");
        exit(1);
    }

    if(inPipe != 0)
        close(inPipe); /*clean up, release file control resource*/

    if(outPipe != 1)
        close(outPipe); /*clean up, release file control  resource*/
}


/*
 * This function parses and executes a command started from the string position
 * pointed by linePtr. This function is called recursively to execute a
 * subcommand one by one.
 *
 * linePtr --  points to the starting position of the next command in the input command string
 * length -- the length of the remaining command string
 * inPipe is the input file descriptor, initially it is 0, gradually it may be changed as we parse subcommmands.
 * out is the output file descriptor, initially it is 1, gradually it may be changed as we parse subcommmands.
 * The inPipe default value  is 0 and  outPipe default value is 1.
 */
void runcmd(char * linePtr, int length, int inPipe, int outPipe)
{
    char * args[length];
    char * nextChar = parse(linePtr, args);

    if (args[0] != NULL)
    {
        /*Exit if seeing "exit" command*/
        /*Your solution*/
        if(strcmp(args[0], "exit") == 0) exit(0);

        if (*nextChar == '<' && inPipe == 0)
        {
            //It is input redirection, setup the file name to read from
            char * in[length];

            //nextChar+1 moves the character position after <,
            //thus points to a file name
            nextChar = parse(nextChar+1,in);

            /* Change inPipe so it follows the redirection */

            /*Your solutuon*/
            //set inPipe as the file descriptor value from file, this value is returned from open()
            //only need to READ ONLY since we wont be writing to the file
            inPipe =  open(in[0], O_RDONLY);

        }

        if (*nextChar == '>')
        {   /*It is output redirection, setup the file name to write*/
            /*Your solutuon*/

            char * out[length];
            nextChar = parse(nextChar+1,out);
            //set outpipe to open file's file descriptor value 
            outPipe =  open(out[0], O_CREAT|O_TRUNC|O_WRONLY, 0644);


        }

        if (*nextChar == '|')
        { /*It is a pipe, setup the input and output descriptors */
            /*execute the subcommand that has been parsed, but setup the output using this pipe*/
            /*Your solution*/
            int fd[2]; //fd[0] = read end of pipe, fd[1] = write end of pipe
            pipe(fd);
            outPipe = fd[1];
            fchild(args,inPipe,outPipe);

            /*execute the remaining subcommands, but setup the input using this pipe*/

            /*Your solution*/
            //if we have a|b|c a redirects to b rdirects to c, need to call runcmd 
            runcmd(nextChar+1, strlen(nextChar)-1,fd[0], 1);
            close(fd[0]);

            return;
        }

        if (*nextChar == '\0')
        { /*There is noting special after this subcommand, so we just execute in a regular way*/
            fchild(args,inPipe,outPipe);
            return;
        }

        //else: some problem, so throw a fit.
        printf("ERROR: Invalid input: %c\n",*nextChar);
    }
}

//handler for SIGINT
void ctrl_c(int sig){
  signal(sig,SIG_IGN);
  exit(0);
}

int main(int argc, char *argv[])
{
    /*Your solution*/
    signal(SIGINT,ctrl_c);
    char lineIn[1024];

    while(1)
    {
        if (fgets(lineIn,1024,stdin) == NULL)
            break;

        int len = 0;
        while (lineIn[len] != '\0')
            len++;

        /* remove the \n that fgets adds to the end */
        if (len != 0 && lineIn[len-1] == '\n')
        {
            lineIn[len-1] = '\0';
            len--;
        }

        //Run this string of subcommands with 0 as default input stream
        //and 1 as default output stream
        runcmd(lineIn, len,0,1);

        /*Wait for the child completes */
        wait(NULL);


    }

    return 0;
}
