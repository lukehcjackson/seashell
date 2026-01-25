#include "seashell.h"

int main() {

    char input_buffer[INPUT_BUFFER_SIZE];
    char* argv[MAX_ARGS];

    char basePath[MAX_PATH_LENGTH];
    char* pathParts[MAX_PATH_LENGTH];
    pathParts[0] = NULL;

    getBasePath(basePath, sizeof(basePath));

    showMeAShell(2);
    
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
        if (fgets(input_buffer, sizeof input_buffer, stdin) == NULL) {
            //handle null => EOF
            continue;
        }

        //fgets includes the newline we use to finish the input
        //trim it by searching for \n in the input buffer and replacing it with a null terminator
        input_buffer[strcspn(input_buffer, "\n")] = '\0';

        //detect buffer overflow
        size_t input_length = strlen(input_buffer);
        //if we read the max amount of data and the last character we read is not a delimiter => user entered too much text and it overflowed
        if ((input_length == sizeof(input_buffer) - 1) && (input_buffer[input_length - 1] != '\0')) {
            //we know there has been buffer overflow - flush stdin so the start of the next command is not polluted by the end of this one
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            fprintf(stderr, COLOUR_RED "INPUT TOO LONG\n" COLOUR_RESET);
            continue;
        }

        //skip leading whitespace
        char* p = input_buffer;
        while (*p && isspace((unsigned char)*p)) {
            p++;
        }
        //is this an empty line / whitespace only line?
        if (*p == '\0') {
            continue;
        }

        //parse the input: split by spaces
        //use strtok to tokenise it
        //better to actually split by whitespace in general - robust to tabs
        char* tok = strtok(input_buffer, " \t");
        //now each call to strtok(NULL, " ") returns the pointer to the next token (word) or NULL if there are none left

        int argc = 0;
        //fill argv with tokens - i.e. words that we have entered into the shell
        while (tok != NULL) {
            argv[argc] = tok;
            argc++;
            tok = strtok(NULL, " \t");
        }

        //---- HANDLE INPUT AND OUTPUT REDIRECTION -----
        //in doing this, compress argv from ["cat", "<", "input.txt", ">", "output.txt", NULL]
        //to ["cat", NULL]

        char* input_file = NULL;
        char* output_file = NULL;
        int appendingOutput = 0;

        int write_idx = 0;

        for (int read_idx = 0; read_idx < argc; read_idx++) {

            if (argv[read_idx] == NULL) continue;

            // INPUT REDIRECTION
            if (strcmp(argv[read_idx], "<") == 0) {
                if (read_idx + 1 < argc) { //we have seen <, but did the user give us the file to get input from?
                    input_file = argv[read_idx+1];
                    read_idx++; //skip the filename that comes after <
                } else {
                    fprintf(stderr, COLOUR_RED "Syntax error: expected file after '<'\n" COLOUR_RESET);
                }

                continue;
            }

            // OUTPUT REDIRECTION
            if ((strcmp(argv[read_idx], ">") == 0) || (strcmp(argv[read_idx], ">>") == 0)) {
                //if we read >>, we are appending instead of overwriting output
                appendingOutput = (strcmp(argv[read_idx], ">>") == 0) ? 1 : 0;

                if (read_idx + 1 < argc) { //we have seen > / >>, but did the user really give us a file to redirect to?
                    output_file = argv[read_idx+1];
                    read_idx++; //skip the filename
                } else {
                    fprintf(stderr, COLOUR_RED "Syntax error: expected file after '>'\n" COLOUR_RESET);
                }

                continue;
            }

            //normal argument, not < or >
            //this compresses argv to remove <, >, and the input/output files
            argv[write_idx] = argv[read_idx];
            write_idx++;

        }

        //ensure argv is null-terminated
        argv[write_idx] = NULL;
        argc = write_idx;

        // --------  CUSTOM FUNCTIONS ---------
        if (strcmp(argv[0], "shell") == 0) {
            //todo: while this is enigmatic, this command should obviously probably work from other directories
            //either move working directory -> root, run command, move back
            //or maintain a path to the root directory from the current directory and use that in the cat command to print the shell
            if (pathParts[0] != NULL) {
                printf(COLOUR_BLUE "You can't move a shell from it's home. Try again from the root directory.\n" COLOUR_RESET);
            } else {
                showMeAShell(2);
            }
        }

        else if (strcmp(argv[0], "bigshell") == 0) {
            if (pathParts[0] != NULL) {
                printf(COLOUR_BLUE "You can't move a shell from it's home. Try again from the root directory.\n" COLOUR_RESET);
            } else {
                showMeAShell(1);
            }
        }

        //--------- BUILTINS -----------
        //cd
        else if (strcmp(argv[0], "cd") == 0) {
            if (argc < 2) {
                fprintf(stderr, COLOUR_RED "cd: missing operand\n" COLOUR_RESET);
            } else if (chdir(argv[1]) != 0) { //chdir returns 0 on success
                fprintf(stderr, COLOUR_RED);
                perror("cd"); //use perror here because chdir is a syscall, and so modifies errno on failure -> perror uses this to give a descriptive error message
                fprintf(stderr, COLOUR_RESET);
            }
            getCwdFromShell(pathParts, basePath);
        } 

        //pwd
        else if (strcmp(argv[0], "pwd") == 0) {
            char pwd_buffer[MAX_PATH_LENGTH];

            if (getcwd(pwd_buffer, sizeof(pwd_buffer)) == NULL) { //getcwd returns non-null on success
                fprintf(stderr, COLOUR_RED);
                perror("pwd");
                fprintf(stderr, COLOUR_RESET);
            } else {
                puts(pwd_buffer);
            }
        }

        // ----------- OTHER FUNCTIONS -----------
        else {

            callUnixFunc(argc, argv, input_file, output_file, appendingOutput);
            
        }
    }
    
    return 0;
}

//todo:

// other redirects | >> << ... (?)
//support all builtins that i want to - 'if a command needs to change the shell's state it must be a builtin'
//up arrow key support
//  -> keep buffer of some amount of argc and argv's, cycle through them if you press up arrow
//how do you do tab autocomplete?? 'complete' in bash
//  -> up arrow support and tab autocomplete rely on using character-by-character input instead of line-by-line
//  when i do this, consider the things this current approach is not at all robust to - "", '', escape characters
//  for instance there is no support right now for echo "hello world" or cd "my folder" or cd this>file.txt
//echo hello world does work by happenstance but mkdir "my folder" makes a folder called '"my' and a folder called 'folder"'

//colours in ls output ??

//next todo:
//  do i care about << and <<< ? <> ?
//  handle redirecting standard error with &

// | implementation