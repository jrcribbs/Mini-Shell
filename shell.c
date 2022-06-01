#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_BUFFER_SIZE 80

void sigint_handler(int sig){
	write(1,"\nTerminating through signal handler\n",36); 
	exit(0);
}

	
// function for built in commands, history handled in main()
int builtIns(char** token){
	int numCmds = 3;
	char* builtInCmds[numCmds];
	int i = 0;
	int hit = 0;
	
	builtInCmds[0] = "exit";
	builtInCmds[1] = "cd";
	builtInCmds[2] = "help";
	
	// loop through array looking for a match
	for(i; i < numCmds; i++){
		if(strcmp(token[0], builtInCmds[i])==0){
			hit = i + 1;
			continue;
		}
	}	
	if(hit == 0){ // not in built-ins
		return 0;	
	} else if(hit == 1){ // exit
		printf("Goodbye!\n");
		exit(0);
	} else if (hit == 2){ // cd
		chdir(token[1]);
		return hit;
	} else if(hit == 3){ // help
		printf("Welcome to Shell Junior!\n");
		printf("Here are all the built-in commands you can use:\n");
		printf("exit\n");
		printf("cd\n");
		printf("help\n");
		printf("history\n");
		return hit;
	}
}

// function to execute simple system commands
void sysExe(char** parsed){
	pid_t pid = fork();

	if(pid == -1){
		printf("Forking failed");
		return;
	} else if(pid == 0){ // try to run in fork, throw error if it doesn't work
		if(execvp(parsed[0],parsed) < 0){
			printf("Command not found\n");
		}
		exit(0);
	} else {
		wait(NULL);
		return;
	}
}
// function for pipe commands
void pipeExe(char** cmdPipe1, char** cmdPipe2){
	int pipeFD[2];
	
	pipe(pipeFD);
	
	// forking first command
	pid_t pipe1 = fork();

	if (pipe1 < 0){
		printf("Error Forking 1st Pipe Program\n");
		return;
	} else if (pipe1 == 0){
		// child 1 execute
		close(pipeFD[0]);
		dup2(pipeFD[1], STDOUT_FILENO);
		close(pipeFD[1]);
		
		if(execvp(cmdPipe1[0], cmdPipe1) < 0){
			printf("Error executing 1st Pipe Program\n");
			exit(0);
		}
		exit(0);
	} else {
		wait(NULL);
		// parent forking second command
		pid_t pipe2 = fork();

		if(pipe2 < 0){
			printf("Error Forking 2nd Pipe Program\n");
			return;
		}
		// child 2 execute
		if(pipe2 == 0){
			close(pipeFD[1]);
			dup2(pipeFD[0], STDIN_FILENO);
			close(pipeFD[0]);
			
			if(execvp(cmdPipe2[0],cmdPipe2) < 0){
				printf("Error executing 2nd Pipe Program\n");
				exit(0);
			}
		exit(0);
		}else{
			// parent waiting
			wait(NULL);
		}
	}
}

int main(){
	char line[MAX_BUFFER_SIZE];
	char *token;
	char *parsed[MAX_BUFFER_SIZE];
	char *pipe;
	char *pipe1;
	char *pipe2;
	char *pipes[MAX_BUFFER_SIZE];
	char *cmdPipe1[MAX_BUFFER_SIZE];
	char *cmdPipe2[MAX_BUFFER_SIZE];
	char *history[MAX_BUFFER_SIZE];
	int counter = 0;
	int i;
	
	// ctrl+c to escape
	signal(SIGINT, sigint_handler);
	
	
    // A loop that runs forever.
    while(1){
		i = 0;

		//shell prompt 
    	printf("mini-shell>");
		// Read in 1 line of text
        // The line is coming from 'stdin' standard input
        fgets(line,MAX_BUFFER_SIZE,stdin);
		
		// check if no input
		if(strcmp(line,"\n")==0){
			continue;
		}
		// remove newline char
		char *line2 = strstr(line, "\n");
		if(line2 != NULL){
			strncpy(line2, "\0",1);
		}
		
		// add to history
		history[counter] = strdup(line);

		// check for pipe
		pipe = strstr(line,"|");
			if(pipe != NULL){
				// if pipe found split into two arrays: before and after pipe
				pipe = strtok(line,"|");
				while(pipe !=NULL){
					pipes[i]=strdup(pipe);
					i++;
					pipe = strtok(NULL,"|");
				}		 
				pipes[i]=NULL;
				i = 0;
				// parsed pipe array 1
				pipe1 = strtok(pipes[0], " ");
					while(pipe1 !=NULL){
						cmdPipe1[i]=strdup(pipe1);
						i++;
						pipe1 = strtok(NULL, " ");
				}
				cmdPipe1[i]=NULL;
				// parsed pipe array 2
				int i = 0;
				pipe2 = strtok(pipes[1], " ");
					while(pipe2 != NULL){
						cmdPipe2[i]=strdup(pipe2);
						i++;
						pipe2 = strtok(NULL, " ");
					}
				cmdPipe2[i] = NULL;
				// pass two parsed arrays to pipeExe 
				pipeExe(cmdPipe1, cmdPipe2);
			}else{
	
			// if no pipe procede as normal
			// parsing through line   
			token = strtok(line, " ");
				while(token != NULL) {
					parsed[i]=strdup(token);
					i++;	
					token = strtok(NULL, " ");
				}
				parsed[i]=NULL;
				// checking if history was called
				if(strcmp(parsed[0], "history")==0){
					int x = 0;
					for(x; x <= counter; x++){
						printf("%s\n",history[x]);	
					}
				continue;
				}
				// else
				if(builtIns(parsed)==0){
					sysExe(parsed);
				}
			}
		counter++;
		}
  return 0;
}