/*
	Braden Lee
	Course: cs344_001_f2020
	Date: 10/31/2020

	Program description:
	-Provide a prompt for running commands
	-Handle blank lines and comments, which are lines beginning with the # character
	-Provide expansion for the variable $$
	-Execute 3 commands exit, cd, and status via code built into the shell
	-Execute other commands by creating new processes using a function from the exec family of functions
	-Support input and output redirection
	-Support running commands in foreground and background processes
	-Implement custom handlers for 2 signals, SIGINT and SIGTSTP
*/

#include <stdio.h>
#include <stdlib.h> //exit
#include <sys/types.h> //exploration proccess concept & states pid_t
#include <unistd.h> //exploration proccess concept & states fork() execvp, dup2
#include <string.h> //for string manipulation (strcmp, strtok, strcpy, strlen, strdup)
#include <sys/wait.h> //wait and waitpid
#include <signal.h> // for signal handlers
#include <fcntl.h> // for close file descriptor on execution (Exploration: process and I/O)

//global var for foreground testing
int ForegroundVar = 1;

void userExit() 
{
	//for when the user exits
	exit(0);

	return;
}

// Checks the path if it exist or not
void cd(char * file_path)
{
	//file path does not exist
	if (chdir(file_path))
	{
		
		fprintf(stderr, "ERROR with %s \n", file_path);
		perror("");
		fflush(stderr);

		return;
	}
	//exist, so change directories to the existing file path
	else
	{
		chdir(file_path);

		return;
	}
}

//Checks command array for file path, else if empty give path to user's "HOME" directory
void cd_temp(char ** cArray, int cArrayIndex)
{

	if (cArrayIndex == 1)
	{
		cd(getenv("HOME"));
	}
	// If a path is found with the cd command, pass the parameter to function CD.
	else
	{
		cd(cArray[1]);
	}

	return;
}

/*
	gets the status of the PID passed through.
	returns the exit status or termination signal.
*/

int status_temp(int pid_Status)
{
	int status = 0;

	//child process terminated normally
	if (WIFEXITED(pid_Status))
	{
		printf("Exit status is: %d \n", WEXITSTATUS(pid_Status));
	}
	//terminated by a signal (abnormally)
	else 
	{
		printf("Terminated by signal: %d \n", WTERMSIG(pid_Status));
	}
	return status;
}

/*
	function used to ignore TSTP, used for entering/exting foreground from background.
*/

void SIGTSTP_Handler(int sigNo) 
{
	//checks if foreground is off. If off, then exit foreground and set it back to 1, to signal background is on.
	if (ForegroundVar == 0)
	{
		ForegroundVar = 1;
		const char* errMessage = " Exiting foreground mode \n\n";
		write(1, errMessage, strlen(errMessage));
		fflush(stdout);
	}
	//If foreground is 1 (on), then enter foreground and set it back to 0, to signal background is off.
	else
	{
		ForegroundVar = 0;
		const char* on_message = "\n: Entering foreground mode \n";
		write(1, on_message, strlen(on_message));
		fflush(stdout);
	}

	return;
}

//Redirection of streams / commands to be executed / forking() process / signal_handling / foreground-mode testing 
void executionCode(struct sigaction sigint_handler, char ** cArray, int cArrayIndex, int * pid_Status, char * redirectInfile, char * redirectOutfile, int bgFlag)
{
	int result;
	//garbage pid value
	pid_t temp_fpid = -6969;
	pid_t currPid = -6969;
	pid_t closedpid = -6969;

	temp_fpid = fork();
	switch (temp_fpid) 
	{
		//error case
	case -1:

		perror("ERROR COULD NOT FORK");
		exit(1);

		break;

		//child case
	case 0:

		//check for no background flag (if process is foreground), for the child case
		if (bgFlag == 0)
		{
			//allowing child to be killed, by reseting the handler
			sigint_handler.sa_handler = SIG_DFL; 
			sigaction(SIGINT, &sigint_handler, NULL);
		}

		//check for if any redirection was parsed for outfile (Exploration: process and I/O)
		if (strcmp(redirectOutfile, "") != 0) 
		{
	
			int file_outDescriptor = open(redirectOutfile, O_WRONLY | O_CREAT | O_TRUNC, 0666);
			if (file_outDescriptor == -1) 
			{
				fprintf(stderr, "%s: ERROR ", cArray[0]);
				perror("");
				fflush(stderr);
				exit(1);
			}


			result = dup2(file_outDescriptor, 1);
			if (result == -1) 
			{
				perror("ERROR DUP2");
				fflush(stderr);
				exit(2);
			}

			fcntl(file_outDescriptor, F_SETFD, FD_CLOEXEC); //close file descriptor on execution (Exploration: process and I/O)
		}

		//check for if any redirection was parsed for infile (Exploration: process and I/O)
		if (strcmp(redirectInfile, "") != 0) 
		{

			int file_inDescriptor = open(redirectInfile, O_RDONLY);
			if (file_inDescriptor == -1) 
			{
				fprintf(stderr, "%s: ERROR ", cArray[0]);
				perror("");
				fflush(stderr);
				exit(1);
			}

			result = dup2(file_inDescriptor, 0);
			if (result == -1) 
			{
				perror("EROOR DUP2");
				fflush(stderr);
				exit(2);
			}

			fcntl(file_inDescriptor, F_SETFD, FD_CLOEXEC); //close file descriptor on execution Exploration: process and I/O
		}
		//if no redirection is given, redirect to /dev/null
		if (redirectOutfile == NULL)
		{
			int file_outDescriptor = open("/dev/null", O_WRONLY);
			if (file_outDescriptor == -1)
			{
				fprintf(stderr, "ERROR, cannot redirect to /dev/null \n");
				perror("");
				fflush(stderr);
				exit(1);
			}
			result = dup2(file_outDescriptor, 1);
			if (result == -1) 
			{
				perror("ERROR DUP2");
				fflush(stderr);
				exit(2);
			}
			fcntl(file_outDescriptor, F_SETFD, FD_CLOEXEC);
		}
		//if no redirection is given, redirect to /dev/null
		if (redirectInfile == NULL)
		{
			int file_inDescriptor = open("/dev/null", O_RDONLY);
			if (file_inDescriptor == -1)
			{
				fprintf(stderr, "ERROR, cannot redirect to /dev/null \n");
				perror("");
				fflush(stderr);
				exit(1);
			}
			result = dup2(file_inDescriptor, 1);
			if (result == -1)
			{
				perror("ERROR DUP2");
				fflush(stderr);
				exit(2);
			}
			fcntl(file_inDescriptor, F_SETFD, FD_CLOEXEC);
		}

		//sets the end of the command array to null, signaling that it's the end of the array
		cArray[cArrayIndex] = NULL;

		//after checking for direction send the command array in to be executed

		//executes commands. If errors executing command, print out error message
		if (execvp(cArray[0], (char * const*)cArray))
		{
			fprintf(stderr, "%s: ERROR ", cArray[0]);
			perror("");

			//flush all std streams
			fflush(stdout);
			fflush(stdin);
			fflush(stderr);
			exit(2);
		}
		break;

		//parent case (anything not a error (-1) or child process (0))
	default:

		//checks for bgFlag set to 1 and Foreground set to 1. If both are set to 1, this indicates the user wants to launch process in the background
		if (bgFlag && ForegroundVar)
		{
			//non blocking wait
			currPid = waitpid(temp_fpid, pid_Status, WNOHANG);
			printf("Background pid is : %d \n", temp_fpid);
			fflush(stdout);
		}
		//if parameters are not set, launch process in the foreground
		else 
		{
			//blocking wait to run in the foreground
			currPid = waitpid(temp_fpid, pid_Status, 0);

		}
	}
	/*
		while  pid is -1 (waiting for any child process, similar to the wait system call), non blocking wait.
		If the value of pid is greater than 0, then waitpid will wait for the child whose process ID equals pid. (Exploration: Process API - Monitoring Child Processes)
		If the value of pid is -1, then waitpid will wait for any child process, similar to the wait system call. (Exploration: Process API - Monitoring Child Processes)

		After each iteration, prints the value of the background pid closed/terminated with the status value of the termination.
	*/
	while ((closedpid = waitpid(-1, pid_Status, WNOHANG)) > 0) 
	{
		printf("Background pid %d is done: ", closedpid);
		status_temp(*pid_Status);
		fflush(stdout);
	}
}


int main() {


	char redirectOutfile[2048]; //buffer to redirect to the outfile
	char redirectInfile[2048]; //buffer to redirect to the infile
	char *cArray[2048]; //buffer to hold the array of commands
	struct sigaction sigint_handler = { { 0 } }; //initialization for signal handler
	struct sigaction sigtstp_handler = { { 0 } }; //initialization for signal handler

	//flag to indicate foreground
	int bgFlag;


	int i; //variable used to increment
	int cArrayIndex = 0; //keep track of the command index
	int tempGarbageVal = -6969; //seting a garbage value


	//handlers for to ignore ctrl + c and ctrl + z
	sigint_handler.sa_handler = SIG_IGN;
	sigfillset(&sigint_handler.sa_mask);
	sigint_handler.sa_flags = 0;
	sigaction(SIGINT, &sigint_handler, NULL);

	sigtstp_handler.sa_handler = SIGTSTP_Handler;
	sigfillset(&sigtstp_handler.sa_mask);
	sigtstp_handler.sa_flags = 0;
	sigaction(SIGTSTP, &sigtstp_handler, NULL);

	//while loop to keep parsing command line given from userinput
	while (1)
	{
		//frees the previous command array from the last command line parsed
		for (i = 0; i < cArrayIndex; ++i)
		{
			if (cArray[i] != NULL)
			{
				free(cArray[i]);
			}
		}

		char userInData[2048]; //buffer to hold user input data
		char expandPID[10]; //buffer to hold the pid expanded value
		char * token; //token ptr to keep track of parsed tokens

		cArrayIndex = 0; //set at 0; keeps track of the index of the commands in the array
		bgFlag = 0; //set bgFlag to 0

		//initialize null to be end of the array. Used to terminate a string
		userInData[0] = '\0';
		expandPID[0] = '\0';
		redirectInfile[0] = '\0';
		redirectOutfile[0] = '\0';
	

		//signals the user for a prompt
		printf(": ");
		fflush(stdout);

		//grabs user input
		fgets(userInData, 300, stdin);

		//checks for blank lines and comments
		while (( userInData[0] == NULL || userInData[0] == '#' || userInData[0] == '\0'))
		{
			printf(": ");

			fflush(stdout);
			fgets(userInData, 300, stdin);
		}


		token = strtok(userInData, " \n");

		//checks for redirects in and out and for special characters such as $ and $$
		while (token != NULL) {

			//checking for redirection to outfile
			if (strcmp(token, ">") == 0)
			{
				token = strtok(NULL, " \n");
				strcpy(redirectOutfile, token);

			}
			//checks for redirection to infile
			else if (strcmp(token, "<") == 0)
			{
				token = strtok(NULL, " \n");
				strcpy(redirectInfile, token);
			}
			//checks for if they user want to run the process in the background. Turns bgFlag on (1) signaling the user want to run in the background
			else if (strcmp(token, "&") == 0)
			{
				bgFlag = 1;
			}
			//specifically checks for the $$, by itself in the string
			else if (strcmp(token, "$$") == 0)
			{
				sprintf(expandPID, "%d", getpid()); //expands the var to receive the PID value
				cArray[cArrayIndex] = strdup(expandPID); //append to the array in place of the $$
				cArrayIndex += 1;
			}
			else
			{
				//checked instances for special characters, now to check specifically for $$ at the end of a var in the string

				char temp[2048]; //temp buffer for command arrays to be put back together if a $$ was found at the end of a string variable
				cArray[cArrayIndex] = strdup(token); //append the string var to the command array

				//if the "$$" is found at the end of the string foo$$ replace the $$ with the PID value
				if (strlen(cArray[cArrayIndex]) >= 2) 
				{
					if (cArray[cArrayIndex][strlen(cArray[cArrayIndex]) - 2] == '$' && cArray[cArrayIndex][strlen(cArray[cArrayIndex]) - 1] == '$')
					{
						//setting null-terminating string beore $$
						token[strlen(token) - 2] = '\0';

						//clears original command at array index in array
						free(cArray[cArrayIndex]);

						//copies back to command array temp string. replacing the $$ with the value of pid
						snprintf(temp, 300, "%s%d", token, getpid());

						cArray[cArrayIndex] = strdup(temp); // append PID value to end of command array, replacing previously cleared $$

					}
				}
				cArrayIndex += 1;
			}
			//reset for next token
			token = strtok(NULL, " \n");
		}


		//checks for built in functs in the beginning of the commnad line array before sending it to be executed
		if ((strcmp(cArray[0], "exit") == 0) ||(strcmp(cArray[0], "status") == 0) || (strcmp(cArray[0], "cd") == 0))
		{

			if ((strcmp(cArray[0], "exit") == 0))
			{
				userExit();
			}
			else if ((strcmp(cArray[0], "status") == 0))
			{
				status_temp(tempGarbageVal);
			}
			else if ((strcmp(cArray[0], "cd") == 0))
			{
				cd_temp(cArray, cArrayIndex);
			}
		}
		else
		{
			//After checking special characters and built in functions, send the array of command in to be executed
			executionCode(sigint_handler, cArray, cArrayIndex, &tempGarbageVal, redirectInfile, redirectOutfile, bgFlag);
		}
	}

	return 0;
}