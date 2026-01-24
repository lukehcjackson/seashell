#include "seashell.h"

int main() {

    char input_buffer[INPUT_BUFFER_SIZE];
    char* argv[MAX_ARGS];

    char basePath[MAX_PATH_LENGTH];
    char* pathParts[MAX_PATH_LENGTH];

    getBasePath(basePath, sizeof(basePath));
    
    //todo: input_buffer (?) and argv(!!) are not ever memset or reset between commands
        //this does not seem to matter? running ls -lah then ls with no or fewer args gives correct results

    while (1) {

        //todo: if the last character in stdout was not \n, print a newline
        //more complicated than it sounds - can't just read stdout, have to capture every output of every command and print it myself, then conditionally print \n

        //display Seashell/dir1/dir2 ~ 
        printf(COLOUR_CYAN "Seashell");
        for (int i = 0; pathParts[i] != NULL; i++) {
            printf("/%s", pathParts[i]);
        }
        printf(" ~ " COLOUR_RESET);

        //fgets reads from stdin and writes to input_buffer
        //TODO: handle buffer overflow
        if (fgets(input_buffer, sizeof input_buffer, stdin) == NULL) {
            //handle null
            continue;
        }

        //fgets includes the newline we use to finish the input
        //trim it by searching for \n in the input buffer and replacing it with a null terminator
        input_buffer[strcspn(input_buffer, "\n")] = '\0';

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

        //------ HANDLE OUTPUT REDIRECTION --------

        int output_redirect = 0;
        char* output_file = NULL;

        for (int i = 0; i < argc; i++) {
            if (strcmp(argv[i], ">") == 0) {
                output_redirect = 1;
                if (i + 1 < argc) { //we have seen <, but did the user really give us a file to redirect to?
                    output_file = argv[i+1];
                } else {
                    fprintf(stderr, COLOUR_RED "Syntax error: expected file after '>'\n" COLOUR_RESET);
                }

                argv[i] = NULL;
                break;

            }
        }

        // --------  CUSTOM FUNCTIONS ---------
        if (strcmp(argv[0], "shell") == 0) {
            showMeAShell(2);
        }

        else if (strcmp(argv[0], "bigshell") == 0) {
            showMeAShell(1);
        }

        //--------- BUILTINS -----------
        //cd
        else if (strcmp(argv[0], "cd") == 0) {
            if (argc < 2) {
                fprintf(stderr, COLOUR_RED "cd: missing operand\n" COLOUR_RESET);
            } else if (chdir(argv[1]) != 0) { //chdir returns 0 on success
                perror("cd"); //use perror here because chdir is a syscall, and so modifies errno on failure -> perror uses this to give a descriptive error message
            }
            getCwdFromShell(pathParts, basePath);
        } 

        //pwd
        else if (strcmp(argv[0], "pwd") == 0) {
            char pwd_buffer[MAX_PATH_LENGTH];

            if (getcwd(pwd_buffer, sizeof(pwd_buffer)) == NULL) { //getcwd returns non-null on success
                perror("pwd");
            } else {
                puts(pwd_buffer);
            }
        }

        // ----------- OTHER FUNCTIONS -----------
        else {

            if (output_redirect) {
                callUnixFunc(argc, argv, output_file);
            } else {
                callUnixFunc(argc, argv, NULL);
            }
            
        }
    }
    

    return 0;
}

//todo:
//cannot touch test.txt then cat main.c > test.txt

//cd is a 'builtin' so cannot run in a child process
// >, <, | will not work because i have not implemented that - the shell does the piping not the base command run with execvp
// other redirects? >> << ... (?)

//support all builtins that i want to - 'if a command needs to change the shell's state it must be a builtin'
//implement piping
//up arrow key support
//  -> keep buffer of some amount of argc and argv's, cycle through them if you press up arrow
//how do you do tab autocomplete?? 'complete' in bash
//  -> up arrow support and tab autocomplete rely on using character-by-character input instead of line-by-line
//custom commands - custom version of something like neofetch and draw a blue ascii shell ??

//shell only works in root directory - either warn the user or redirect -> execute -> redirect back
//crashes if you just put in whitespace, tabs, spaces, enter key - need to sanitise this input before checking / dereferencing it


//next todo: input redirection, handle both input and output redirection on the same line correctly, handle multiple > > and < < on the same line in the right order