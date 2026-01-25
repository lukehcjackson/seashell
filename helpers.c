#include "seashell.h"

void callUnixFunc(int argc, char** argv, char* input_file, char* output_file, int appendingOutput) {

    pid_t child_pid, wpid;
    int status = 0;
    int exitStatus = -1;

    if ((child_pid = fork()) == 0) {

        //---- handle stdin input redirection -------
        if (input_file != NULL) {
            //instead of reading from stdin, read in from a file
            //read-only (do i need file permissions ??)
            int fd_in = open(input_file, O_RDONLY);
            if (fd_in < 0) {
                //failed to open the file
                fprintf(stderr, COLOUR_RED "FAILED TO OPEN FILE TO REDIRECT INPUT");
                perror("open");
                fprintf(stderr, COLOUR_RESET "\n");
                exit(1);
            }

            //redirect stdin to this file
            dup2(fd_in, STDIN_FILENO);
            close(fd_in);
        }

        //---- handle stdout output redirection -------
        if (output_file != NULL) {
            //instead of writing to stdout, write to a file
            int fd_out;
            if (appendingOutput) {
                //write-only, create the file if it doesnt exist, append if it does exist - 0644 gives permissions to this file
                fd_out = open(output_file, O_WRONLY | O_CREAT | O_APPEND, 0644);
            } else {
                //write-only, create the file if it doesnt exist, overwrite it if it does exist - 0644 gives permissions to this file
                fd_out = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            }
            if (fd_out < 0) {
                //failed to open the file
                fprintf(stderr, COLOUR_RED "FAILED TO OPEN FILE TO REDIRECT OUTPUT");
                perror("open");
                fprintf(stderr, COLOUR_RESET "\n");
                exit(1);
            }

            //redirect stdout to this file
            dup2(fd_out, STDOUT_FILENO);
            close(fd_out);
        }

        //exec
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
    callUnixFunc(2, tmpArgs, NULL, NULL, 0);

    printf(COLOUR_RESET);
}