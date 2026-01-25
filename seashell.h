#ifndef SEASHELL_H
#define SEASHELL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <ctype.h>

#define INPUT_BUFFER_SIZE 100
#define MAX_ARGS 10
#define MAX_PATH_LENGTH 1024

#define COLOUR_RESET  "\x1b[0m"
#define COLOUR_RED    "\x1b[31m"
#define COLOUR_CYAN   "\x1b[36m"
#define COLOUR_BLUE   "\x1b[94m"

void callUnixFunc(int argc, char** argv, char* input_file, char* output_file, int appendOutput);
void getBasePath(char* base, size_t size);
void getCwdFromShell(char** parts, char* base);
void showMeAShell(int shell);

#endif