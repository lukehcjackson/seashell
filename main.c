#include <stdio.h>
#include <string.h>
#include <process.h>

#define INPUT_BUFFER_SIZE 100
#define MAX_ARGS 10

int main() {
    
    //display a command prompt interface
    //allow a user to enter text to stdin
    //parse that text and do what they ask for

    char input_buffer[INPUT_BUFFER_SIZE];
    char* argv[MAX_ARGS];

    //TODO free these
    
    //TODO input_buffer (?) and argv(!!) are not ever memset or reset between commands
    
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
            /*
            for (int i = 0; i < argc; i++) {
                puts(argv[i]);
            }
            */

            //now do different things depending on what argv[0] is
            //this would be much easier if we only supported unix but i am writing this on a windows machine
            //so have it do different things based on the OS but with the same seashell command

            // ls
            if (strcmp(argv[0], "ls") == 0) {

                //argument validation
                //ls [flags]
                //for a unix system we can just pass that as is, on windows we need to translate between unix flags and windows flags

                #ifdef _WIN32
                    printf("windows ls in this bitch");

                    //TODO
                    //to run anything from the windows command line you need to use MSVC
                    // => spend a day configuring a new C compiler

                #else
                    //todo test if this works
                    //do i need to fork this process before calling exec?
                    execvp(argv[0], argv);
                #endif
            }

        }
    }
    

    return 0;
}