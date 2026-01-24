#include "seashell.h"

void callUnixFunc(int argc, char** argv) {

    pid_t child_pid, wpid;
    int status = 0;
    int exitStatus = -1;

    if ((child_pid = fork()) == 0) {
        exitStatus = execvp(argv[0], argv);

        if (exitStatus == -1) {
            fprintf(stderr, COLOUR_RED "COMMAND %s FAILED WITH ARGS: ", argv[0]);
            for (int i = 0; i < argc; i++) {
                fprintf(stderr, "%s ", argv[i]);
            }
            fprintf(stderr, COLOUR_RESET "\n");
            exit(1);
        }
    }

    //need to wait here for the child process to finish
    while ((wpid = wait(&status)) > 0);
    //todo: could do some nice handling of newlines - if the last line in stdout is not empty / a newline, print one?
}

void getBasePath(char* base, size_t size) {
   char pwd_buffer[MAX_PATH_LENGTH];

    if (getcwd(pwd_buffer, sizeof(pwd_buffer)) == NULL) { //getcwd returns non-null on success
        fprintf(stderr, COLOUR_RED "INTERNAL PWD FAILED");
        perror("pwd");
        fprintf(stderr, COLOUR_RESET "\n");
        exit(1);
    } else {
        //have whole working directory in pwd_buffer
        //want to trim this path to the last directory
        //and save this as our 'base path'

        char* tok = strtok(pwd_buffer, "/");
        char* last = NULL;
        while (tok != NULL) {
            last = tok;
            tok = strtok(NULL, "/");
        }

        if (last != NULL) {
            strncpy(base, last, size-1);
            base[size-1] = '\0';
        }

    } 
}

void getCwdFromShell(char** parts, char* base) {
    char pwd_buffer[MAX_PATH_LENGTH];

    if (getcwd(pwd_buffer, sizeof(pwd_buffer)) == NULL) { //getcwd returns non-null on success

        fprintf(stderr, COLOUR_RED "INTERNAL PWD FAILED");
        perror("pwd");
        fprintf(stderr, COLOUR_RESET "\n");
        exit(1);

    } else {
        //have whole working directory in pwd_buffer
        //want to trim this path to /seashell/.../.../.../
        //and change the output at the start of a new command entry to reflect this

        char* tok = strtok(pwd_buffer, "/");
        while (tok != NULL && (strcmp(tok, base) != 0)) {
            tok = strtok(NULL, "/");
        }

        if (tok == NULL) {
            //did not find basePath in the pathname
            fprintf(stderr, COLOUR_RED "FAILED TO FIND BASENAME IN WORKING DIRECTORY\n" COLOUR_RESET);
            exit(1);
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
    }
}

void showMeAShell(int shell) {

    printf(COLOUR_BLUE);
    fflush(stdout);

    char* shellFile;
    //can make this a big if statement if we want more than two versions of this shell command
    shellFile = shell == 1 ? "shell.txt" : "shell2.txt";

    char* tmpArgs[] = {
        "cat",
        shellFile,
        NULL
    };

    //output the contents of the shellFile with cat
    callUnixFunc(2, tmpArgs);

    printf(COLOUR_RESET);
}