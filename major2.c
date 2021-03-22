//CSCE 3600.004 Group 1
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>

#include <sys/wait.h>
#include <readline/readline.h>
#include <readline/history.h>

#define MAX_LETT 1000 //maximum number of letters
#define MAX_COMM 100  //maximum number of commands
#define MAX_HISTORY 20


struct history {

	char *commands[MAX_HISTORY];
	int n, k, flag;


};

typedef struct history History;


History *myhistory;// = {NULL, 0, 0, 0};

void execute_command_redirect(char* ); // redirect input and output.
int is_redirect(char* ); // to check if it is a command with redirection
void execution(char *, char **, char **);		//executes the commands from a given character array
void exec_argu(char **);						//function where single command is executed
void exec_argu_multi(char *, char **, char **); //function where multiple commands are executed
int analyzeString(char *, char **, char **);	//analyse the type of command
int semiColon(char *, char **);					//returns number of semiColon
bool semiColonCheck(char *);					//checks for semicolon(s)
void whiteSpace(char *, char **);				//seperates by whiteSpace
//exit
void exits(char *, char **);
bool is_exit(char *);
//pipe
void piped(char *, char **, char **);
bool is_piped(char *);
//cd
void cd(char **);
bool is_cd(char *);
//signal handling
typedef void (*sighandler_t)(int);
sighandler_t signal(int sigNum, sighandler_t handler); //The first parameter is the signal number based on the predefined value,
void sig(int);
bool is_sig(char *);
void add_command(char *);
void print_my_history();
void delete_history();
char* get_command(const int);
int check_history(char *);

int main(int argc, char *argv[])
{
	//read line from standard input (or file)
	char *parsed[MAX_COMM], *command[MAX_COMM], input[MAX_LETT];
	myhistory  = (History*) malloc(sizeof(History));

	for (int i = 0; i < MAX_HISTORY; i++)
		myhistory->commands[i] = (char*) malloc(sizeof(char) *  1024);

	myhistory->n = 0;
	myhistory->k = 0;
	myhistory->flag = 0;

	if (argc > 2)
		fprintf(stderr, "Too many arguments for code.\n");

	else if (argc == 1) //if there are no arguments, do interactive mode
	{
		while (1)
		{
			parsed[0] = '\0';
			command[0] = '\0';
			input[0] = '\0';

			//shell prompt
			printf(">> ");
			scanf(" %[^\n]", input);
			add_command(input);
			if (check_history(input)) {
				continue;
			}
			else if (is_redirect(input) == 1)
			 execute_command_redirect(input);

			else
				execution(input, command, parsed);
			//printf("\n");
		}
	}
	else //else, do batch mode
	{
		parsed[0] = '\0';
		command[0] = '\0';
		input[0] = '\0';

		FILE *file = fopen(argv[1], "r"); //read passed in file
		if (file == NULL)
		{
			fprintf(stderr, "Could not open file. Exiting...\n");
			exit(1);
		}
		//read each line read in from batch file
		while (fscanf(file, " %[^\n]", input) == 1)
		{
			if (feof(file))
				break;

			printf("\n>> %s\n\n", input);
			execution(input, command, parsed);

			parsed[0] = '\0';
			command[0] = '\0';
			input[0] = '\0';
		}
	}

	printf("Program ending...\n");
	return 0;
}

int check_history(char *input)  {

	char* sub;
	char str[1024];
	strcpy(str, input);

	int is_correct = 0;
	sub = strtok(str, " ");

	while (sub != NULL) {

		if (!strcmp(sub, "myhistory")) {
			sub = strtok(NULL, " ");
			if (sub == NULL) {
				print_my_history();
				is_correct = 1;
				break;
			}

			else if (!strcmp(sub, "-c")) {
				delete_history();
				is_correct = 1;
				break;
			}
			else if (!strcmp(sub, "-e")) {
				sub = strtok(NULL, " ");
				if (sub != NULL) {
					int kth = atoi(sub);
					char input2[MAX_LETT];
					strcpy(input2, get_command(kth));
					if (strlen(input2) > 0) {
						char *parsed[MAX_COMM], *command[MAX_COMM];
						parsed[0] = '\0';
						command[0] = '\0';
						execution(input2, command, parsed);
						is_correct = 1;
						break;
					}
					else
						printf("Error, invalid command number\n");
				}
			}
		}
		sub = strtok(NULL, " ");
	}
	return is_correct;
}

void print_my_history() {

	for (int i = 0; i < myhistory->n; i++) {
		printf("[%d] %s\n", i + 1, myhistory->commands[i]);
	}
}

char* get_command(const int k) {

	char* command;
	command = (char*) malloc(sizeof(char) * 1024);

	if (k >= 1 && myhistory->n > 1 && k  <= myhistory->n)
			strcpy(command, myhistory->commands[k - 1]);

	return command;
}

void add_command(char *command) {

	if (myhistory->k >= 20) {
		myhistory->k = 0;
		myhistory->flag = 1;
	}

	if (!myhistory->flag)
  	myhistory->n++;

	strcpy(myhistory->commands[myhistory->k++], command);

}

void delete_history() {

	myhistory->n = 0;
	myhistory->k = 0;
	myhistory->flag = 0;
}

int is_redirect(char *input)
{

	// loop through the input and check if you have the redirect command
	for (int i = 0; i < strlen(input); i++) {
		if (input[i] == '>' || input[i] == '<')
			return 1;
	}
	return 0;
}

// function that, given a redirection command, executes it
void execute_command_redirect(char *input)
{

  char * parsed[] = { "sh", "-c", input, NULL };
  if (fork() == 0)  //child
	{
      int out = execvp(parsed[0], parsed);
      exit(1);
      if (out == -1) //end child
          exit(0);
  }
  else // end parent
      wait(NULL);

}

//executes the commands from a given character array
void execution(char *input, char **command, char **parsed)
{
	int execType = 0;
	//parse line with command and arguments
	execType = analyzeString(input, command, parsed);
	/* 0 - no command exists
	   1 - single command
	   2 - multiple commands with ;
	   3 - an exit exists
	   4 - it is piped
	   5 - it is cd
	*/
	//execute command with arguments
	switch (execType)
	{
	case 1: //single command
		exec_argu(command);
		break;
	case 2: //multiple commands
		exec_argu_multi(input, command, parsed);
		break;
	case 3: //exit
		exits(input, command);
		break;
	case 4: //pipe
		piped(input, command, parsed);
		break;
	case 5: //cd
		cd(parsed);
		break;
	default: //0
		printf("There is nothing to execute.\n");
		break;
	}
}

//function where system command is executed
void exec_argu(char **c)
{
	pid_t pid = fork();
	if (pid == -1)
	{
		fprintf(stderr, "Unable to create child for execution\n");
		return;
	}

	if (pid == 0) //child
	{
		for (int i = 0; i < sizeof(c); i++)
			if (execvp(c[i], c) < 0)
				fprintf(stderr, "\nCould not execute command...\n");

		exit(0); //end child
	}
	else			//parent
		wait(NULL); //wait for child to finish
	printf("\n");
	return;
}
//function where multiple commands are executed
void exec_argu_multi(char *str, char **c1, char **c2)
{
	//count how many commands need execution
	int n = semiColon(str, c1);

	//execute each command
	for (int i = 0; i < n; i++)
	{
		if (strlen(c1[i]) != 0)
		{
			c2[0] = '\0';
			whiteSpace(c1[i], c2);
			exec_argu(c2);
		}
	}
}

//analyse the type of command
int analyzeString(char *str, char **comm, char **parse)
{
	char *temp[5];

	//if string has an exit
	if (is_exit(str))
		return 3;

	//if string has a pipe
	if (is_piped(str))
		return 4;

	//if string has a cd
	if (is_cd(str))
		return 5;

	bool colon = semiColonCheck(str); //get if multiple arguments
	if (colon)						  //if true, return 2
		return 2;
	else //if single command, seperate white spaces
	{
		whiteSpace(str, comm);
		return 1;
	}

	return 0;
}
//checks for semiColon
bool semiColonCheck(char *str)
{
	int num = 0;
	for (int i = 0; i < MAX_LETT; i++)
	{
		if (str[i] == ';')
			num++;

		//remove last ; if nothing else is there
		if ((str[i] == '\0' && str[i - 1] == ';') || (str[i] == '\0' && str[i - 1] == ' ' && str[i - 2] == ';'))
		{
			str[i - 1] = '\0';
			num--;
		}

		if (str[i] == '\0')
			break;
	}

	if (num > 0)
		return true;

	return false;
}
//returns number of semicolon(s)
int semiColon(char *str, char **temp)
{
	int count = 0;
	for (int i = 0; i < 5; i++)
	{
		temp[i] = strsep(&str, ";");
		if (temp[i] == NULL)
			break;

		if (strlen(temp[i]) == 0)
			i--;
		else
			count++;
	}

	if (temp[1] == NULL)
		return 0; //if no semicolon was found
	else
		return count; //semicolon found and the amount
}
//seperates by whiteSpace
void whiteSpace(char *str, char **comm)
{
	int n = 0;
	//find the first instance of null in comm array
	while (n < MAX_COMM)
	{
		if (comm[n] == NULL)
			break;
		n++;
	}

	for (int i = n; i < MAX_COMM; i++)
	{
		comm[i] = strsep(&str, " ");

		if (comm[i] == NULL)
			break;
		if (strlen(comm[i]) == 0)
			i--;
	}
}

//exit
void exits(char *in, char **comm)
{
	char *temp[5];

	//count how many commands need to be executed
	int count = semiColon(in, temp);

	//if there is only one argument, which would be exit
	if (count == 0)
	{
		printf("Exiting...\n");
		exit(0);
	}

	for (int i = 0; i < count; i++)
	{
		//transfer all non-exit commands
		if (strcmp(temp[i], "exit") != 0 && strcmp(temp[i], " exit") != 0 && strcmp(temp[i], "exit ") != 0)
		{
			comm[0] = '\0';
			//seperate by white whiteSpace
			whiteSpace(temp[i], comm);
			exec_argu(comm);
		}
	}

	printf("Exiting...\n");
	exit(0);
}

bool is_exit(char *input)
{
	bool exit = false;

	for (int i = 0; i < MAX_LETT; i++)
	{
		if (input[i] == 'e' && input[i + 1] == 'x' && input[i + 2] == 'i' && input[i + 3] == 't')
			exit = true;

		if (input[i] == '\0')
			break;
	}

	return exit;
}

//pipe
void piped(char *input, char **comm, char **parsed)
{
	//execute all commands in the string that do not have a pipe
	int n = semiColon(input, comm);

	for (int i = 0; i < n; i++)
	{
		if (strpbrk(comm[i], "|") == 0)
		{
			whiteSpace(comm[i], parsed);
			exec_argu(parsed);
			parsed[0] = '\0';
		}
		else //has a pipe
		{
			strcpy(input, comm[i]);
		}
	}

	//get number of pipes
	int count = -1;

	char *temp[3];
	for (int i = 0; i < 3; i++)
	{
		temp[i] = strsep(&input, "|");
		if (temp[i] == NULL)
			break;
		else
			count++;
	}

	if (count == 1) //there is one pipe
	{
		int fd[2];	  //0 - input (read), 1 - output (write)
		pid_t c1, c2; //child 1 and child 2
		int status;
		count = 0;
		if (pipe(fd) == -1)
		{
			fprintf(stderr, "Pipe failure.");
			return;
		}

		c1 = fork();
		if (c1 == -1)
		{
			fprintf(stderr, "Unable to create child for execution\n");
			return;
		}

		if (c1 == 0) //child 1
		{
			count = semiColon(temp[0], comm);
			if (count == 0)
				count = 1;

			close(fd[0]);
			dup2(fd[1], 1);

			for (int i = 0; i < count; i++)
			{
				whiteSpace(comm[i], parsed);
				if (execvp(parsed[i], parsed) < 0)
					fprintf(stderr, "\nCould not execute command...\n");
			}
			exit(0);
		}
		else //parent
		{
			c2 = fork();
			if (c2 == -1)
			{
				fprintf(stderr, "Unable to create child for execution\n");
				return;
			}

			if (c2 == 0) //child 2
			{
				count = semiColon(temp[1], comm);
				if (count == 0)
					count = 1;

				close(fd[1]);
				dup2(fd[0], 0);

				for (int i = 0; i < count; i++)
				{
					whiteSpace(comm[i], parsed);
					if (execvp(parsed[i], parsed) < 0)
						fprintf(stderr, "\nCould not execute command...\n");
				}
				exit(0);
			}

			//parent only one to get here
			close(fd[0]);
			close(fd[1]);

			for (int i = 0; i < 2; i++)
				wait(&status);
		}
	}
	else if (count == 2) //there are 2 pipes
	{
		int status, count = 0;
		pid_t c1, c2, c3;
		int fd1[2]; //0 - input (read), 1 - output (write)
		int fd2[2]; //0 - input (read), 1 - output (write)
		if (pipe(fd1) == -1)
		{
			fprintf(stderr, "Pipe failure.");
			return;
		}

		if (pipe(fd2) == -1)
		{
			fprintf(stderr, "Pipe failure.");
			return;
		}

		c1 = fork();
		if (c1 == -1)
		{
			fprintf(stderr, "Unable to create child for execution\n");
			return;
		}

		if (c1 == 0) //child 1
		{
			count = semiColon(temp[0], comm);
			if (count == 0)
				count = 1;

			close(fd1[0]);
			close(fd2[0]);
			close(fd2[1]);
			dup2(fd1[1], 1);

			for (int i = 0; i < count; i++)
			{
				whiteSpace(comm[i], parsed);
				if (execvp(parsed[i], parsed) < 0)
					fprintf(stderr, "\nCould not execute command...\n");
			}

			exit(0);
		}
		else //parent
		{
			c2 = fork();
			if (c2 == -1)
			{
				fprintf(stderr, "Unable to create child for execution\n");
				return;
			}

			if (c2 == 0) //child 2
			{
				count = semiColon(temp[1], comm);
				if (count == 0)
					count = 1;

				close(fd1[1]);
				close(fd2[0]);

				dup2(fd1[0], 0);
				dup2(fd2[1], 1);

				for (int i = 0; i < count; i++)
				{
					whiteSpace(comm[i], parsed);
					if (execvp(parsed[i], parsed) < 0)
						fprintf(stderr, "\nCould not execute command...\n");
				}

				exit(0);
			}
			else //parent
			{
				c3 = fork();
				if (c3 == -1)
				{
					fprintf(stderr, "Unable to create child for execution\n");
					return;
				}

				if (c3 == 0) //child 3
				{
					count = semiColon(temp[2], comm);
					if (count == 0)
						count = 1;

					close(fd1[0]);
					close(fd1[1]);
					close(fd2[1]);

					dup2(fd2[0], 0);

					for (int i = 0; i < count; i++)
					{
						whiteSpace(comm[i], parsed);
						if (execvp(parsed[i], parsed) < 0)
							fprintf(stderr, "\nCould not execute command...\n");
					}
					exit(0);
				}
			}

			close(fd1[0]);
			close(fd1[1]);
			close(fd2[0]);
			close(fd2[1]);

			for (int i = 0; i < 3; i++)
				wait(&status);
		}
	}
	else if (count > 2)
		fprintf(stderr, "Too many pipes in argument.\n");
}

bool is_piped(char *input)
{
	bool pipe = false;

	for (int i = 0; i < MAX_LETT; i++)
	{
		if (input[i] == '|')
			pipe = true;

		if (input[i] == '\0')
			break;
	}

	return pipe;
}

bool is_cd(char *input)
{
	bool exit = false;

	for (int i = 0; i < MAX_LETT; i++)
	{
		if (input[i] == 'c' && input[i + 1] == 'd')
			exit = true;
	}

	return exit;
}

void cd(char **parsed)
{
	if (parsed[1] == NULL)
	{
		fprintf(stderr, "Expected argument to \"cd\"\n");
	}
	else
	{
		if (chdir(parsed[1]) != 0)
		{
			fprintf(stderr, "Error to cd\n");
		}
	}
}

void sig(int sig)
{
}

