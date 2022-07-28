#include  <stdio.h>
#include  <sys/types.h>
#include  <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64

pid_t backgroud_pid[1000] ; 
pid_t parallel_pid[1000] ;
int BackgroundNo, parallel_num, ip = 0, itr=0 ; 

/* Splits the string by space and returns the array of tokens
*
*/
char **tokenize(char *line)
{
  char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
  char *token = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
  int i, tokenIndex = 0, tokenNo = 0;

  for(i =0; i < strlen(line); i++){

    char readChar = line[i];

    if (readChar == ' ' || readChar == '\n' || readChar == '\t'){
      token[tokenIndex] = '\0';
      if (tokenIndex != 0){
	tokens[tokenNo] = (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
	strcpy(tokens[tokenNo++], token);
	tokenIndex = 0; 
      }
    } else {
      token[tokenIndex++] = readChar;
    }
  }
 
  free(token);
  tokens[tokenNo] = NULL ;
  return tokens;
}

void execute(char **args, int ip)
{
	int tokenNo = 0 ;
	for(;args[tokenNo]!=NULL;tokenNo++) ; 

	/* Exit command  */
	if(!strcmp(args[0], "exit")){
		for(int i=0;i<BackgroundNo;i++) kill(backgroud_pid[i], 9) ; 
		kill(getpid(), 9) ; 
	}
	
	/* Is it a backgroud function? */
	int bg = 0 ;
	if(!strcmp(args[tokenNo-1], "&")){
		bg = 1 ;
		args[tokenNo-1] = NULL ;
	} 

	/* Non Build-in Functions */
	if(!strcmp(args[0], "cd")) 
	{
		if(chdir(args[1]) == -1)
		{
			fprintf(stderr, "Incorrect Command\n") ; 
		} 
		return ; 
	}

	/* Build-in Functions */
	int rc = fork() ;
	if(bg) {backgroud_pid[BackgroundNo] = rc ; BackgroundNo++ ;} 
	if(ip) {parallel_pid[parallel_num] = rc ; parallel_num++ ; }
	if(bg && !ip) { setpgid(0,0) ; }
	if(rc < 0)
	{
		fprintf(stderr, "fork failed") ; 
		exit(1) ; 
	} else if(rc == 0)
	{
		if(getpgid(rc)!=getpgid(getppid())) signal(SIGINT, SIG_IGN) ; 
		else signal(SIGINT, SIG_DFL) ; 

		int res = execvp(*args + 0, args) ; 
		if(res == -1)
		{
			fprintf(stderr, "Incorrect Command\n") ; 
			if(kill(getpid(), 9))
			{
				fprintf(stderr, "Couldn't kill child process.") ; 
			}
			return ; 
		}  
	}
	else if(!bg)
	{
		waitpid(rc, NULL, 0) ;  
	}
}

char* replace(const char* s)
{
	char *res = malloc(MAX_INPUT_SIZE*sizeof(char)) ; 
	char *resp = res ; 

	*res = 0 ; 

	while (*s)
	{
		if(!strncmp(s, "&&&", 3))
		{
			ip = 1; 
			strcat(resp, "& &&") ; 
			s += 3 ; 
			resp += 4 ; 
		} 
		else
		{
			*resp = *s ; 
			resp++ ; 
			s++ ; 
		}
	}
	*resp = 0 ; 
	return res ; 
}

void sighandler(int signum)
{
	printf("\n") ; 
	itr = 1 ; 
}

void childhandler(int signum)
{
	waitpid(-1, NULL, WNOHANG) ;
}

int main(int argc, char* argv[]) {
	signal(SIGINT, sighandler) ;
	signal(SIGCHLD, childhandler) ; 

	char  line[MAX_INPUT_SIZE];            
	char  **tokens;              
	int i;

	FILE* fp;
	if(argc == 2) {
		fp = fopen(argv[1],"r");
		if(fp < 0) {
			printf("File doesn't exists.");
			return -1;
		}
	}

	while(1) {
		/* BEGIN: TAKING INPUT */
		bzero(line, sizeof(line));
		if(argc == 2) { // batch mode
			if(fgets(line, sizeof(line), fp) == NULL) { // file reading finished
				break;	
			}
			line[strlen(line) - 1] = '\0';
		} else { // interactive mode
			printf("$ ");
			scanf("%[^\n]", line);
			getchar();
		}
		/* printf("Command entered: %s (remove this debug output later)\n", line); */
		/* END: TAKING INPUT */

		line[strlen(line)] = '\n'; //terminate with new line
		
		/* Adjusting for &&& */
		ip = 0 ;
		char *lines = replace(line) ; 

		tokens = tokenize(lines);

       //do whatever you want with the commands, here we just print them

		/* for(i=0;tokens[i]!=NULL;i++){
			printf("found token %s (remove this debug output later)\n", tokens[i]);
		} */
		if(itr) itr = 0 ; 

		int start = 0;
		int x = 0 ; 
		for(int i = 0 ;tokens[i]!=NULL;i++)
		{
			if(itr) break ; 
			x++ ; 
			if(!strcmp(tokens[i], "&&"))
			{
				char** args = (char**) malloc((i -start+1)*sizeof(char*)) ; 

				for(int j=start;j<i;j++)
				{
					args[j-start] = tokens[j] ; 
				}
				
				args[i-start] = NULL ; 
				execute(args, ip) ; 
				start = i+1 ; 
			}
		}
		
		if(itr) {itr=0; goto nearend ;} 

		if(x != start){
			char** args = (char**) malloc((x-start+1)*sizeof(char*)) ; 

			for(int j=start;j<x;j++) {
				args[j-start] = tokens[j] ; 
			} 
			args[x-start] = NULL ;
			execute(args, ip) ; 
		}

		if(itr) itr = 0 ; 

		nearend:
		if(ip)
		{
			for(int i=0;i<parallel_num;i++)
			{
				waitpid(parallel_pid[i], NULL,0) ; 
			}
			parallel_num = 0 ; 
		}

		// Freeing the allocated memory	
		for(i=0;tokens[i]!=NULL;i++){
			free(tokens[i]);
		} 
		free(tokens);
	}
	return 0;
}
