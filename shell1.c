#include "stdio.h"
#include "stdlib.h"
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>


#define STRLEN 256
#define ARGNUM 50

struct arg
{
        char* command;
        char** com_arg;
	int num_com_arg;
        struct arg* next;
};

struct arg* next_arg_init(struct arg* cur_arg);
struct arg* read_string();
struct arg* parser(struct arg* arg , char* string);
void execution(struct arg* arg);
void wait_all(struct arg* arg);
void destroy_arg(struct arg* arg);

int main()
{
struct arg* arg;
while(1)
{
        printf("InspectorShell$ ");
        arg = read_string();
	if(arg == NULL)
		continue;

	//printf("%s %s\n" , arg->com_arg[0] , arg->com_arg[1] );

	execution(arg);
	wait_all(arg);
	destroy_arg(arg);

	//sleep(1);
}
return 0;
}


struct arg* next_arg_init(struct arg* cur_arg)
{
        struct arg* next_arg = (struct arg*)calloc(1 , sizeof(struct arg));
        next_arg->com_arg = (char**)calloc(ARGNUM , sizeof(char*));
        next_arg->command = NULL;
        next_arg->next = NULL;
	next_arg->num_com_arg = 0;
 
	if(cur_arg != NULL)
                cur_arg->next = next_arg;
        
	return next_arg;
}


struct arg* read_string()
{
	char* string = calloc(STRLEN , sizeof(char));
	
	char* c = fgets(string, STRLEN, stdin);
	if(c == NULL)
		exit(0);
	/*
	int x = scanf("%m[^\n]" , &string);
	if(x = EOF)
		exit(0);
	getchar();
	//printf("%d\n" , x);*/
	if(string[0] == '\n')
	{
		free(string);
		return NULL;
	}
	else if(!strcmp(string , "exit\n"))
	{
		free(string);
		exit(0);
	}
	else
	{
		struct arg* arg = parser(NULL , string);
		free(string);
		return arg;
	}
}


struct arg* parser(struct arg* a , char* string)
{
struct arg* arg = next_arg_init(a);
char c;
int shift = 0;
int was_command = 0;
for( ; strlen(string) != 1 ; string++)
{
	if(*string == ' ')
	{
	//	printf("probel\n");
		continue;
	}

	else if((*string == '|') && was_command)
	{
	//	printf("new command\n");
		next_arg_init(arg);
		if(NULL == parser(arg , ++string))
			return NULL;
		break;
	}
	else if((*string == '|') && !was_command)
	{
		//printf("empty command\n");
		return NULL;
	}
	
	else if(!was_command)
	{
	//	printf("command: ");
		sscanf(string , "%m[^ ^|^\n]" , &(arg->command));
		sscanf(string , "%m[^ ^|^\n]" , &(arg->com_arg[arg->num_com_arg]));
		(arg->num_com_arg)++;
		string += strlen(arg->command) - 1;
		was_command = 1;
	//	printf("%s\n" , arg->command);
	}
	else
	{
	//	printf("argument: ");
		sscanf(string , "%m[^ ^|^\n]" , &(arg->com_arg[arg->num_com_arg]));
		string += strlen(arg->com_arg[arg->num_com_arg]) - 1;
	//	printf("%s\n" , arg->com_arg[arg->num_com_arg]);
		(arg->num_com_arg)++;

	}

	fflush(NULL);

}

if(!was_command )
	return NULL;
return arg;
}



void execution(struct arg* arg)
{
pid_t id;
int fd[2] , com_fd = -1;
int is_first = 1;
do
{
	if(!is_first)
		arg = arg->next;
	pipe(fd);
	id = fork();
	if(id == 0)
	{
	//child
		close(fd[0]);
		if(!is_first)
		{
			close(0);
			dup2(com_fd , 0);
			close(com_fd);
		}
		if(arg->next != NULL)
		{
			close(1);
			dup2(fd[1] , 1);
			close(fd[1]);
		}
//		printf("%s %s\n" , arg->com_arg[0] , arg->com_arg[1]);
		fflush(NULL);	
		execvp(arg->command , arg->com_arg);
	}	
	else
	{
	//parent
		if(com_fd != -1)
			close(com_fd);
		com_fd = fd[0];
		close(fd[1]);
	}
	is_first = 0;

}
while(arg->next != NULL);

}





void wait_all(struct arg* arg)
{
wait(0);
while(arg->next != NULL)
{
	wait(0);
	arg = arg->next;
}
}


void destroy_arg(struct arg* arg)
{
	if(arg->next != NULL)
		destroy_arg(arg->next);
	for(int i = 0 ; i < arg->num_com_arg ; i++)
		free(arg->com_arg[i]);
	
	free(arg->command);

	if(arg->next != NULL)
		free(arg->next);

}







 
