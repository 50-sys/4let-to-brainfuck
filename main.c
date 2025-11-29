#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef enum{
	BRAINFUCK,
	C,
	MACHINE,
} TILL_CODE;

typedef struct{
	TILL_CODE till_code;
	int convert_lines;
	char* output; 
	char* input;
	char* gcc_options;
	char* word;		
} Options;

void exit_err(char *message){
	fprintf(stderr, message, strlen(message));
	exit(1);
}

Options parse_args(int argc, char** argv){
	int errn;
	char* errs;
	int arg_count = 0;

#define ERR_INVALID_OPTION 0
#define ERR_NOT_4_LETTERS 1
#define ERR_INVALID_FLAG 2
#define ERR_TOO_MANY_ARGUMENTS 3
#define ERR_NO_OPTION_GIVEN 4
#define ERR_NO_FILE_SPECIFIED 5

	Options options = {MACHINE, 0, NULL, NULL, NULL, NULL};

	int i = 1;
	for (; i < argc; i++){
		if (argv[i][0] == '-'){
			if (argv[i][1] == '-'){
				if (i == argc - 1){
					errn = ERR_NO_OPTION_GIVEN;
					errs = argv[i];
					goto err;
				}
				char *argvp = &argv[i][2];
				if (!strcmp(argvp, "output")){
					options.output = malloc(strlen(&argv[i+1][0])*sizeof(char));
					strcpy(options.output, &argv[i+1][0]);
				}
				else if (!strcmp(argvp, "gcc-options")){
					options.gcc_options = malloc(strlen(&argv[i+1][0])*sizeof(char));
					strcpy(options.gcc_options, &argv[i+1][0]);
				}
				else if (!strcmp(argvp, "word")){
					if (strlen(argv[i+1]) != 4){
						errn = ERR_NOT_4_LETTERS;
						goto err;
					}
					options.word = malloc(4*sizeof(char));
					strcpy(options.word, argv[i+1]);
				}
				else{
					errn = ERR_INVALID_OPTION;
					errs = &argv[i][2];
					goto err;				
				}
				i++;
			}else{
				char *argvp = &argv[i][1];
				if (!strcmp(argvp, "n"))
					options.convert_lines = 1;
				else if (!strcmp(argvp, "c"))
					options.till_code = BRAINFUCK;
				else if (!strcmp(argvp, "C"))
					options.till_code = C;
				else if (!strcmp(argvp, "o")){
					strcpy(options.output, &argv[i+1][0]);
					i++;
				}
				else if (!strcmp(argvp, "g")){
					strcpy(options.gcc_options, &argv[i+1][0]);
					i++;
				}
				else{
					errn = ERR_INVALID_FLAG;
					errs = &argv[i][1];
					goto err;
				}
			}

		}else{
			if (arg_count > 0){
				errn = ERR_TOO_MANY_ARGUMENTS;
				errs = &argv[i][0];
				goto err;
			}
			options.input = malloc(strlen(argv[i])*sizeof(char));
			strcpy(options.input, argv[i]);
			arg_count++;
		}
	}
	if (options.input == NULL){
		errn = ERR_NO_FILE_SPECIFIED;
		goto err;
	}

	return options;
	
err: 
	switch (errn){
			case ERR_INVALID_OPTION:
				fprintf(stderr, "Invalid option: '%s'.\n", errs);
				exit(1);
				break;
			case ERR_NOT_4_LETTERS:
				fprintf(stderr, "Word has to be 4 letters.\n");
				exit(1);
				break;
			case ERR_INVALID_FLAG:
				fprintf(stderr, "Invalid flag: '%s'.\n", errs);
				exit(1);
				break;
			case ERR_TOO_MANY_ARGUMENTS:
				fprintf(stderr, "Too many arguments: '%s'.\n", errs);
				exit(1);
				break;
			case ERR_NO_OPTION_GIVEN:
				fprintf(stderr, "No option provided for: '%s'.\n", errs);
				exit(1);
				break;
			case ERR_NO_FILE_SPECIFIED:
				fprintf(stderr, "No filename specified.\n");
				exit(1);
				break;
	}
}

int 
main(int argc, char** argv){

	int errn;
	char *errs;

#define ERR_COULD_NOT_OPEN_FILE 0
#define ERR_COULD_NOT_ALL_MEMORY 1

	Options options = parse_args(argc, argv);
	
	FILE *file = fopen(options.input, "r");
	if (file == NULL){
		errn = ERR_COULD_NOT_OPEN_FILE;
		errs = options.input;
		goto err;
	}
	
	fseek(file, 0L, SEEK_END);
	int fsize = ftell(file);
	rewind(file);
	
	char *buffer = malloc(fsize*sizeof(char));
	if (buffer == NULL){
		errn = ERR_COULD_NOT_ALL_MEMORY;
		goto err;
	}
	
	char c;
	int i = 0;
	while ((c = fgetc(file)) != EOF)
		buffer[i++] = c;

	fclose(file);

	free(buffer);
	return 0;
	
err:
	switch (errn){
			case ERR_COULD_NOT_OPEN_FILE:
				fprintf(stderr, "Could not open file for reading: '%s'.\n", errs);
				exit(1);
				break;
			case ERR_COULD_NOT_ALL_MEMORY:
				fprintf(stderr, "Could not allocate memory for file reading.\n");
				exit(1);
				break;
	}

}
