#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define INPUT_BUFFER_SIZE 100
#define MAX_ARGS 10

void callUnixFunc(int argc, char** argv) {

    pid_t child_pid, wpid;
    int status = 0;
    int exitStatus = -1;

    if ((child_pid = fork()) == 0) {
        exitStatus = execvp(argv[0], argv);

        if (exitStatus == -1) {
            fprintf(stderr, "COMMAND %s FAILED WITH ARGS: ", argv[0]);
            for (int i = 0; i < argc; i++) {
                fprintf(stderr, "%s ", argv[i]);
            }
            fprintf(stderr, "\n");
            exit(1);
        }
    }

    //need to wait here for the child process to finish
    while ((wpid = wait(&status)) > 0);
    //could do some nice handling of newlines - if the last line in stdout is not empty / a newline, print one?
}

int main() {
    
    //display a command prompt interface
    //allow a user to enter text to stdin
    //parse that text and do what they ask for

    char input_buffer[INPUT_BUFFER_SIZE];
    char* argv[MAX_ARGS];
    
    //TODO input_buffer (?) and argv(!!) are not ever memset or reset between commands
        //this does not seem to matter? running ls -lah then ls with no or fewer args gives correct results
    
    while (1) {
        printf("Seashell ~ ");
        //fgets reads from stdin and writes to input_buffer
        //TODO: handle buffer overflow
        if (fgets(input_buffer, sizeof input_buffer, stdin) != NULL) {

            //fgets includes the newline we use to finish the input
            //trim it by searching for \n in the input buffer and replacing it with a null terminator
            input_buffer[strcspn(input_buffer, "\n")] = '\0';
            //now that \n is \0, strlen works properly
            size_t len = strlen(input_buffer);

            //parse the input: split by spaces
            //use strtok to tokenise it

            char* tok = strtok(input_buffer, " ");
            //now each call to strtok(NULL, " ") returns the pointer to the next token (word) or NULL if there are none left

            int argc = 0;

            //fill argv with tokens - i.e. words that we have entered into the shell
            while (tok != NULL) {
                argv[argc] = tok;
                argc++;
                tok = strtok(NULL, " ");
            }

            //for execvp argv must be null terminated
            argv[argc] = NULL;

            //handle builtins
            if (strcmp(argv[0], "cd") == 0) {
                if (argc < 2) {
                    fprintf(stderr, "cd: missing operand\n");
                } else if (chdir(argv[1]) != 0) {
                    perror("cd");
                }
                continue;
            } 

            else {
                callUnixFunc(argc, argv);
            }
        }
    }

    //todo:
    //cannot run mkdir then cd into that folder - does cd work at all?
    //cannot touch test.txt then cat main.c > test.txt
    //the internal file system is not being updated as we run these commands, even though the
    //files we create this way DO exist on the filesystem

    //cd is a 'builtin' so cannot run in a child process
    // >, <, | will not work because i have not implemented that - the shell does the piping not the base command run with execvp

    //up arrow key support
    //  -> keep buffer of some amount of argc and argv's, cycle through them if you press up arrow

    //amazingly within this program you can delete the binary for it, recompile with gcc, and run it with ./main
    //can you edit the source code of this program from inside it ???? then recompile ?
    //YES YOU CAN !

    return 0;
}
