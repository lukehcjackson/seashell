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
            fprintf(stderr, "\x1b[31mCOMMAND %s FAILED WITH ARGS: ", argv[0]);
            for (int i = 0; i < argc; i++) {
                fprintf(stderr, "%s ", argv[i]);
            }
            fprintf(stderr, "\x1b[0m\n");
            exit(1);
        }
    }

    //need to wait here for the child process to finish
    while ((wpid = wait(&status)) > 0);
    //todo: could do some nice handling of newlines - if the last line in stdout is not empty / a newline, print one?
}

void getCwdFromShell(char** parts) {
    char pwd_buffer[1024];

    if (getcwd(pwd_buffer, sizeof(pwd_buffer)) == NULL) { //getcwd returns non-null on success
        fprintf(stderr, "\x1b[31mINTERNAL PWD FAILED");
        perror("pwd");
    } else {
        //have whole working directory in pwd_buffer
        //want to trim this path to /seashell/.../.../.../
        //and change the output at the start of a new command entry to reflect this

        char* tok = strtok(pwd_buffer, "/");
        while (tok != NULL && (strcmp(tok, "seashell") != 0)) {
            tok = strtok(NULL, "/");
        }

        if (tok == NULL) {
            //did not find 'seashell' in the path
            //this is fragile to the user renaming the git directory name
            //todo: better to find the base path on startup
            return;
        }

        //tok points to seashell so advance once more
        tok = strtok(NULL, "/");

        //char* parts[1024];
        int i = 0;
        while (tok != NULL) {
            parts[i] = tok;
            i++;
            tok = strtok(NULL, "/");
        }

        parts[i] = NULL;

        //for (int j = 0; j < i; j++) {
        //    puts(parts[j]);
        //}
    }
}

int main() {
    
    //display a command prompt interface
    //allow a user to enter text to stdin
    //parse that text and do what they ask for

    char input_buffer[INPUT_BUFFER_SIZE];
    char* argv[MAX_ARGS];

    char* pathParts[1024];
    
    //TODO input_buffer (?) and argv(!!) are not ever memset or reset between commands
        //this does not seem to matter? running ls -lah then ls with no or fewer args gives correct results
    
    while (1) {

        printf("\x1b[36mSeashell");
        for (int i = 0; pathParts[i] != NULL; i++) {
            printf("/%s", pathParts[i]);
        }
        printf(" ~ \x1b[0m");

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
                    fprintf(stderr, "\x1b[31mcd: missing operand\x1b[0m\n");
                } else if (chdir(argv[1]) != 0) { //chdir returns 0 on success
                    perror("cd"); //use perror here because chdir is a syscall, and so modifies errno on failure -> perror uses this to give a descriptive error message
                }
                //todo: change 'Seashell:' to show the current directory we are in
                getCwdFromShell(pathParts);

                //int a = 0;
                //while (pathParts[a] != NULL) {
                //    puts(pathParts[a]);
                //    a++;
                //}
            } 

            else if (strcmp(argv[0], "pwd") == 0) {
                char pwd_buffer[1024];

                if (getcwd(pwd_buffer, sizeof(pwd_buffer)) == NULL) { //getcwd returns non-null on success
                    perror("pwd");
                } else {
                    puts(pwd_buffer);
                }
            }

            else {
                callUnixFunc(argc, argv);
            }
        }
    }

    //todo:
    //cannot touch test.txt then cat main.c > test.txt

    //cd is a 'builtin' so cannot run in a child process
    // >, <, | will not work because i have not implemented that - the shell does the piping not the base command run with execvp


    //support all builtins that i want to - 'if a command needs to change the shell's state it must be a builtin'
    //implement piping
    //up arrow key support
    //  -> keep buffer of some amount of argc and argv's, cycle through them if you press up arrow
    //does execvp support any installed commands? if i install cowsay, lolcat (probably not), etc does it work?
    //colours in the output ???

    //amazingly within this program you can delete the binary for it, recompile with gcc, and run it with ./main
    //can you edit the source code of this program from inside it ???? then recompile ?
    //YES YOU CAN !

    return 0;
}
