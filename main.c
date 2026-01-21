#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
    #include <process.h>
#endif
#include <unistd.h>
#include <sys/wait.h>

#define INPUT_BUFFER_SIZE 100
#define MAX_ARGS 10

int callUnixFunc(int argc, char** argv) {

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

}

int main() {
    
    //display a command prompt interface
    //allow a user to enter text to stdin
    //parse that text and do what they ask for

    #ifdef _WIN32
        #define PLATFORM_WINDOWS  1
        #define PLATFORM_UNIX     0
    #else
        #define PLATFORM_WINDOWS  0
        #define PLATFORM_UNIX     1
    #endif

    char input_buffer[INPUT_BUFFER_SIZE];
    char* argv[MAX_ARGS];
    
    //TODO input_buffer (?) and argv(!!) are not ever memset or reset between commands
        //this does not seem to matter? running ls -lah then ls with no or fewer args gives correct results
    
    printf("seashell version 2 running");
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

            /*
            for (int i = 0; i < argc; i++) {
                puts(argv[i]);
            }
            */
            

            //now do different things depending on what argv[0] is
            //this would be much easier if we only supported unix but i am writing this on a windows machine
            //so have it do different things based on the OS but with the same seashell command

            #if PLATFORM_UNIX
                pid_t child_pid, wpid;
                int status = 0;
                int exitStatus = -1;
            #endif

            // ls
            if (strcmp(argv[0], "ls") == 0) {

                //argument validation
                //ls [flags]
                //for a unix system we can just pass that as is, on windows we need to translate between unix flags and windows flags

                #if PLATFORM_UNIX

                    //todo correctly handle ls argument order: ls [args] [file(s)]
                    //do i even need to do this? when you type the command into the shell we assume you know what the args are
                    //and if you give them wrong then we just return the normal exit code / msg ?

                    callUnixFunc(argc, argv);

                #endif

                #if PLATFORM_WINDOWS

                    printf("windows ls");

                    //TODO
                    //to run anything from the windows command line you need to use MSVC
                    // => spend a day configuring a new C compiler

                #endif
            }

            else if (strcmp(argv[0], "cat") == 0) {

                #if PLATFORM_UNIX

                    callUnixFunc(argc, argv);
                    printf("\n");

                #endif

                #if PLATFORM_WINDOWS
                    printf("windows placeholder");
                #endif

            }

            else {
                //just for a laugh run any unix command because this is completely function agnostic at the moment
                #if PLATFORM_UNIX
                    callUnixFunc(argc, argv);
                #endif
            }

        }
    }

    //todo:
    //cannot run mkdir then cd into that folder
    // up arrow key support

    //amazingly within this program you can delete the binary for it, recompile with gcc, and run it with ./main
    //can you edit the source code of this program from inside it ???? then recompile ?
    //YES YOU CAN !

    return 0;
}
