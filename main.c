#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#include <errno.h>


#if defined(__WIN32) || defined(__WIN64)
#include <fileapi.h>
#else
#include <sys/types.h>
#ifdef __APPLE__
#include <sys/syslimits.h>
#include <fcntl.h>
#endif
#endif

#define BF_LESS_THAN '<'
#define BF_MORE_THAN '>'
#define BF_CLOSING_BRAC ']'
#define BF_OPEN_BRAC '['
#define BF_PLUS '+'
#define BF_MINUS '-'
#define BF_DOT '.'
#define BF_COMMA ','


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

void print_help(){
	puts("\n4letbf [filename] [-n] [--word string]\n\
\t-n: convert line numbers in the first converter messages to 4-letter base-4 number system\n\
\t-c: stop after converting to brainfuck\n\
\t-C: stop after converting to C\n\
\t-h/--help: print this message\n\
\t-o/--output: output binary file name (default: [filename].out)\n\
\t-g/--gcc-options: options to be (literally) passed to gcc besides input and output file names\n\
\t--word: string of 4 characters that make up the word for the custom language\n\
\n\
\n\
Arguments:\n\
\tfilename: file to be compiled\n\
	 ");
}

Options parse_args(int argc, char** argv){

	int arg_count = 0;

#define ERR_NO_ERR -1
#define ERR_INVALID_OPTION 0
#define ERR_NOT_4_LETTERS 1
#define ERR_INVALID_FLAG 2
#define ERR_TOO_MANY_ARGUMENTS 3
#define ERR_NO_OPTION_GIVEN 4
#define ERR_NO_FILE_SPECIFIED 5

	int errn = ERR_NO_ERR;
	char* errs;

	Options options = {MACHINE, 0, NULL, NULL, NULL, NULL};

	int i = 1;
	for (; i < argc; i++){
		if (argv[i][0] == '-'){
			if (argv[i][1] == '-'){
				char *argvp = &argv[i][2];
				if (!strcmp(argvp, "help")){
					print_help();
					exit(0);
				}
				if (i == argc - 1){
					errn = ERR_NO_OPTION_GIVEN;
					errs = argv[i];
					goto err;
				}
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
					options.output = malloc(strlen(&argv[i+1][0])*sizeof(char));
					strcpy(options.output, &argv[i+1][0]);
					i++;
				}
				else if (!strcmp(argvp, "g")){
					options.gcc_options = malloc(strlen(&argv[i+1][0])*sizeof(char));
					strcpy(options.gcc_options, &argv[i+1][0]);
					i++;
				}
				else if (!strcmp(argvp, "h")){
					print_help();
					exit(0);
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
	return (Options) {};
}

void copy_whole_line(char *line_start, char **target){
	char c;
	int i;
	int linesize;

	for (i = 0; ((c = line_start[i]) != '\n') && c != '\0' && c != EOF; i++); 
	linesize = i;

	*target = malloc((linesize + 1)*sizeof(char));

	if (*target == NULL || linesize == 0){
		*target = malloc(17*sizeof(char));
		*target = "CANNOT READ FILE"; 
	}
	else if (linesize == 0){
		free(*target);
		*target = malloc(17*sizeof(char));
		*target = "CANNOT READ FILE"; 
	}
	else strncpy(*target, line_start, linesize); // have to check return value
		(*target)[linesize] = '\0';

}

int strncpy_null_space(char *dest, char *src, int n){
	char c;
	int i;

	for (i = 0; i < n && ((c = src[i]) != '\0') && c != EOF && !isspace(c); i++)
		dest[i] = c;
	dest[i] = '\0';
	if (i < n)
		return 1;
	return 0;
}

int determine_token_type(char* token, char* word){
	// returns -1 if token is invalid, if not the char value
	if (token[0] != tolower(word[0]))
		return -1;
	
	if (token[1] == tolower(word[1])){
		if (token[2] == tolower(word[2])){
			if (token[3] == tolower(word[3])){
				return BF_CLOSING_BRAC;
			}
			else if (token[3] == toupper(word[3])){
				return BF_OPEN_BRAC;
			}
		}
		else if (token[2] == toupper(word[2])){
			if (token[3] == tolower(word[3])){
				return BF_COMMA;
			}
			else if (token[3] == toupper(word[3])){
				return BF_DOT;
			}
		}
	}
	else if (token[1] == toupper(word[1])){
		if (token[2] == tolower(word[2])){
			if (token[3] == tolower(word[3])){
				return BF_PLUS;
			}
			else if (token[3] == toupper(word[3])){
				return BF_MINUS;
			}
		}
		else if (token[2] == toupper(word[2])){
			if (token[3] == tolower(word[3])){
				return BF_LESS_THAN;
			}
			else if (token[3] == toupper(word[3])){
				return BF_MORE_THAN;
			}
		}

	}
	return -1;
}

void parse_file(char *buffer, int size, char *word, FILE *file, int close_file){
	int i = 0;

	char c;
	int a; 

	int linen = 0;
	int col = 0;
	char *line;
	int line_start;

	char *token = malloc(sizeof(char)*5); 
	char bf_token;


#define ERR_NO_ERR -1
#define ERR_UNEXPECTED_EOF 0
#define ERR_EXPECTED_ASSIGN_AFTER_NAME 1
#define ERR_EXPECTED_QUOTES_BEFORE_RVALUE 2
#define ERR_UNTERMINATED_RVALUE 3
#define ERR_RVALUE_TOO_LONG 4
#define ERR_NO_WORD_SPECIFIED 5
#define ERR_INVALID_TOKEN 6
#define ERR_INCOMPLETE_TOKEN 7
#define ERR_INVALID_WORD 8


	int errn = ERR_NO_ERR;
	char *errs;

	while (i < size){
		linen++;
		for (col = 1, line_start = i; ((c = buffer[i]) != '\n') && i < size; i++, col++){
			if (isspace(c)) continue;
			else if (c == EOF){
				errn = ERR_UNEXPECTED_EOF;
				copy_whole_line(&buffer[line_start], &line);
				goto err;
			}
			else if (i + 3 > size){
				errn = ERR_INCOMPLETE_TOKEN;
				copy_whole_line(&buffer[line_start], &line);
				errs = token;
				goto err;
			}
			else if (!strncmp(&buffer[i], "WORD", 4)){
				i += 4;
				col += 4;
				for (i++, col++; isspace(buffer[i]); i++, col++){
					if (i >= size){
						col--;
						i--;
						break;
					}
				}
				if (buffer[i] != '=' || i >= size){
					errn = ERR_EXPECTED_ASSIGN_AFTER_NAME;
					copy_whole_line(&buffer[line_start], &line);
					goto err;
				}
				else{
					for (i++, col++;isspace(buffer[i]); i++, col++){
						if (i >= size){
							col--;
							i--;
							break;
						}
					}
				
					if (buffer[i] != '"' || i >= size){
						errn = ERR_EXPECTED_QUOTES_BEFORE_RVALUE;
						copy_whole_line(&buffer[line_start], &line);
						goto err;
					}
					else{
						i++;
						col++;
						a = i;
						for (i++, col++;(buffer[i] != '"') && (buffer[i] != '\0'); i++, col++){
							if (i >= size){
								col--;
								i--;
								break;
							}
						}
						if (buffer[i] == '\0' || i >= size){
							errn = ERR_UNTERMINATED_RVALUE; 
							copy_whole_line(&buffer[line_start], &line);
							goto err;
						}
						if (i - a != 4){
							errn = ERR_RVALUE_TOO_LONG;
							copy_whole_line(&buffer[line_start], &line);
							goto err;
						}
						word = malloc(5*sizeof(char));
						strncpy(word, &buffer[a], 4);
						word[4] = '\0';
						continue;
					}
				}
			}
			else{
				if (word == NULL){
					errn = ERR_NO_WORD_SPECIFIED;
					goto err;
				}
				strncpy_null_space(token, &buffer[i], 4);
				
				if (strlen(token) != 4){
					errn = ERR_INCOMPLETE_TOKEN;
					copy_whole_line(&buffer[line_start], &line);
					errs = token;
					goto err;
				}
				else{
					bf_token = determine_token_type(token, word);
					if (bf_token == -1){
						errn = ERR_INVALID_WORD;
						copy_whole_line(&buffer[line_start], &line);
						errs = token;
						goto err;
					}
					fputc((char) bf_token, file);
					i += 3;
					col += 3;
					continue;
				}
				
	
				errn = ERR_INVALID_TOKEN;
				copy_whole_line(&buffer[line_start], &line);
				if (strncpy_null_space(errs, &buffer[i], 5))
					errs = "UNKNOWN TOKEN";
				goto err;
			}
		}
		if (c == '\n') i++;
	}

	free(word);
	free(token);
	if (close_file)
		fclose(file);
	return;

err: 
	switch (errn){
		case ERR_UNEXPECTED_EOF:
			fprintf(stderr, "In line: '%s', line: %d, col: %d.\nUnexpected EOF before buffer ended.", line, linen, col);
			exit(1);
			break;
		case ERR_EXPECTED_ASSIGN_AFTER_NAME: 
			fprintf(stderr, "In line: '%s', line: %d, col: %d.\nExpected assignment (=) after name token.", line, linen, col);
			exit(1);
			break;
		case ERR_EXPECTED_QUOTES_BEFORE_RVALUE:
			fprintf(stderr, "In line: '%s', line: %d, col: %d.\nExpected rvalue in quotes.", line, linen, col);
			exit(1);
			break;
		case ERR_UNTERMINATED_RVALUE:
			fprintf(stderr, "In line: '%s', line: %d, col: %d.\nUnterminated rvalue.", line, linen, col);
			exit(1);
			break;
		case ERR_RVALUE_TOO_LONG:
			fprintf(stderr, "In line: '%s', line: %d, col: %d.\nRvalue too long.", line, linen, col);
			exit(1);
			break;
		case ERR_NO_WORD_SPECIFIED:
			fprintf(stderr, "No word specified neither in the options nor the file.");
			exit(1);
			break;
		case ERR_INVALID_TOKEN:
			fprintf(stderr, "In line: '%s', line: %d, col: %d.\nInvalid token beginning with '%s...'.", line, linen, col, errs);
			exit(1);
			break;
		case ERR_INCOMPLETE_TOKEN:
			fprintf(stderr, "In line: '%s', line: %d, col: %d.\nIncomplete token beginning with '%s...'.", line, linen, col, errs);
			exit(1);
			break;
		case ERR_INVALID_WORD:
			fprintf(stderr, "In line: '%s', line: %d, col: %d.\nInvalid word: '%s...'.", line, linen, col, errs);
			exit(1);
			break;

	}
}

int create_random_file(char *path, char *extension, int *fd){
	
	int i = 0;
#if defined(__WIN32) || defined(__WIN64)
	char filename[MAX_PATH] = GetTempFileNameA(path, "4bfXXXXXXXX", 0);
	*fd = _fileno(fopen(filename, "w"));
#elif defined(__APPLE__)
	char filename[strlen(path)+12];
	strcpy(filename, path);
	strcat(filename, "/XXXXXXXXXXX");
	char *file = malloc(sizeof(char)*strlen(filename));
	do{
		file = mkstemp((char *) filename);
		if ((*fd = fileno(fopen(file, "w"))) != -1)
			break;
		i++;
	}while (i < 100);
	free(file);
#else
	char filename[strlen(path)+12];
	strcpy(filename, path);
	strcat(filename, "/XXXXXXXXXXX");
	do{
		*fd = mkstemp((char *) filename);
		if (*fd != -1)
			break;
		i++;
	}while (i < 100);
#endif

	if (*fd == -1)
		return 1;

	return 0;
}

int remove_path(char *filename){
	int i;
	int last;
	char *temp = malloc(strlen(filename)*sizeof(char));

#if defined(__WIN32) || defined(__WIN64)
	char path_sep = '\\';
#else
	char path_sep = '/';
#endif

	if (filename == NULL)
		return 1;

	for (last = 0, i = 0; filename[i] != '\0'; i++)
		if (filename[i] == path_sep)
			last = i + 1;

	strcpy(temp, &filename[last]);
	strcpy(filename, temp);

	free(temp);
	return 0;

}

int get_filename_fd(int fd, char **filename, int rm_path){

	if (*filename == NULL)
		*filename = malloc(1024*sizeof(char));

#if defined(__WIN32) || defined(__WIN64)
#include <io.h>
	HANDLE fileh = _get_osfhandle(fd);
	HANDLE hFile;
	DWORD dwRet;

	dwRet = GetFinalPathNameByHandle(fileh, *filename, MAX_PATH, VOLUME_NAME_NT);

#elif defined(__APPLE__)
	if (fcntl(fd, F_GETPATH, *filename) == -1)
	{
		fprintf(stderr, "Could not get the name of file of fd: '%d'.\n", fd);
		exit(1);
	}
#else
	int len;
	char proc_fd_path[1024] = "\0";
	sprintf(proc_fd_path, "/proc/self/fd/%d", fd);

	if ((len = readlink(proc_fd_path, *filename, 1024)) == -1){
		fprintf(stderr, "Could not get the name of file of fd: '%d'.\n", fd);
		exit(1);
	}
	(*filename)[len] = '\0';
#endif

	if (rm_path)
		remove_path(*filename);
	return 0;
}

int get_path_fd(int fd, char **filename){
	return get_filename_fd(fd, filename, 0);
}

int copy_file(char *src, char *dest, int close){

	char c;

	FILE *in = fopen(src, "r");
	FILE *out = fopen(dest, "w");

#define ERR_NO_ERR -1
#define ERR_COULD_NOT_OPEN_FILE 0

	int errn = ERR_NO_ERR;
	char *errs;

	if (in == NULL){
		errn = ERR_COULD_NOT_OPEN_FILE;
		errs = src;
		goto err;
	}
	if (out == NULL){
		errn = ERR_COULD_NOT_OPEN_FILE;
		errs = dest;
		goto err;
	}

	while ((c = getc(in)) != EOF)
		putc(c, out);

	if (close & 0B01)
		fclose(in);
	if (close & 0B10)
		fclose(out);

	return 0;
		

err:
	switch (errn){
		case ERR_COULD_NOT_OPEN_FILE:
			fprintf(stderr, "Could not open the file: '%s.\n", errs);
			exit(1);
			break;
		case ERR_NO_ERR:
			break;

	}
	

}

int 
main(int argc, char** argv){


#define ERR_NO_ERR -1
#define ERR_COULD_NOT_OPEN_FILE 0
#define ERR_COULD_NOT_ALL_MEMORY 1
#define ERR_COULD_NOT_GENERATE_FNAME_BF 2
#define ERR_COULD_NOT_CREATE_CHILD_PROC 3
#define ERR_COULD_NOT_CALL_COMMAND 4

	int errn = ERR_NO_ERR;
	char *errs;

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

	if (fsize == 0){
		return 0;
	}

	fsize--;
	
	char *buffer = malloc((fsize + 1)*sizeof(char));
	if (buffer == NULL){
		errn = ERR_COULD_NOT_ALL_MEMORY;
		goto err;
	}

	char c;
	int i = 0;
	while ((c = fgetc(file)) != EOF)
		buffer[i++] = c;
	buffer[i] = '\0';

	fclose(file);

	char *filename = NULL;
	int fd;

	if (options.till_code == BRAINFUCK){
		if (options.output == NULL){
			filename = malloc(sizeof(char)*5);
			filename = "a.bf";
			file = fopen((const char*) filename, "w");
		}
		else{
			filename = options.output;
		}
	}

#if defined(__WIN32) || defined(__WIN64)
	else if (create_random_file("%USERPROFILE%\\AppData\\Local\\Temp", ".b", &fd)){
		errs = malloc(sizeof(char)*strlen("%USERPROFILE%\\AppData\\Local\\Temp")+)+1);
		errs = "%USERPROFILE%\\AppData\\Local\\Temp"; 
#else
	else if (create_random_file("/tmp", ".b", &fd)){
		errs = malloc(sizeof(char)*5);
		errs = "/tmp";
#endif
		errn = ERR_COULD_NOT_GENERATE_FNAME_BF;
		goto err;
	}
	else{
		file = fdopen(fd, "w");
	}

	if (file == NULL){
		errn = ERR_COULD_NOT_OPEN_FILE;
		errs = options.input;
		goto err;
	}
	parse_file(buffer, fsize, options.word, file, 0);

	if (options.till_code == BRAINFUCK) return 0;

	get_path_fd(fd, &filename);
	fclose(file);

    pid_t  pid;
    int status;

	char *const args[] = {"bftoc.py", filename, (char *) NULL};

    if ((pid = fork()) < 0){
		errn = ERR_COULD_NOT_CREATE_CHILD_PROC;
		errs = malloc(sizeof(char)*9);
		errs = "bftoc.py";
		goto err;
    }
    else if (pid == 0) {          
    	if (execvp("bftoc.py", args) < 0){
			errn = ERR_COULD_NOT_CALL_COMMAND;
			errs = malloc(sizeof(char)*1024);
			errs = "bftoc.py";
			strcat(errs, " ");
			strcat(errs, filename);
			printf("err: %d\n", errno);
			goto err;
        }
    }
    else {                                  
         while (wait(&status) != pid);
    }

	strcat(filename, ".c");
	if (options.till_code == C){
		char *output;
		if (options.output == NULL){
			output = malloc(sizeof(char)*4);
			output = "a.c";
		}
		else{
			output = options.output;
		}
		copy_file(filename, output, 0B10);
		return 0;
	}

	char *output;
	if (options.output != NULL){
		output = malloc(sizeof(char)*(10 + strlen(options.output)));
		strcpy(output, "--output=");
		strcat(output, options.output);
		output[9 + strlen(options.output)] = '\0';
	}
	else{
		if (options.gcc_options != NULL)
			output = options.gcc_options;
	}
	char *const argsc[] = {"gcc", filename, output, options.gcc_options, (char *) NULL};

    if ((pid = fork()) < 0){
		errn = ERR_COULD_NOT_CREATE_CHILD_PROC;
		errs = malloc(sizeof(char)*9);
		errs = "gcc";
		goto err;
    }
    else if (pid == 0) {          
    	if (execvp("gcc", argsc) < 0){
			errn = ERR_COULD_NOT_CALL_COMMAND;
			errs = malloc(sizeof(char)*1024);
			errs = "gcc";
			strcat(errs, " ");
			strcat(errs, filename);
			printf("err: %d\n", errno);
			goto err;
        }
    }
    else {                                  
         while (wait(&status) != pid);
    }

	free(buffer);
	free(filename);
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
			case ERR_COULD_NOT_GENERATE_FNAME_BF:
				fprintf(stderr, "Could not generate file name in directory /tmp for brainfuck file.\n");
				exit(1);
				break;
			case ERR_COULD_NOT_CREATE_CHILD_PROC:
				fprintf(stderr, "Could not create the child process for: '%s'.\n", errs);
				exit(1);
				break;
			case ERR_COULD_NOT_CALL_COMMAND:
				fprintf(stderr, "Could not execute the command: '%s'.\n", errs);
				exit(1);
				break;
	}

}
