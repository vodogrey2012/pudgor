#include "stdio.h"
#include "stdlib.h"
#include <unistd.h>
#include <string.h>


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

int main()
{
struct arg* arg;
while(1)
{
        printf("InspectorShell$ ");
	fflush(NULL);
        int num_arg = 0;
        arg = read_string();

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
	char* string;
	int x = scanf("%m[^\n]" , &string);
	getchar();			// get \n;
	//printf("%d\n" , x);
	if(x == 0)
		return NULL;
	else	
		return parser(NULL , string);
}


struct arg* parser(struct arg* a , char* string)
{
struct arg* arg = next_arg_init(a);
char c;
int shift = 0;
int was_command = 0;
for(; strlen(string) != 0 ; string++)
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
		arg = parser(arg , ++string);
		break;
	}
	else if((*string == '|') && !was_command)
	{
		printf("empty command\n");
		fflush(NULL);
		return NULL;
	}
	
	else if(!was_command)
	{
	//	printf("command: ");
		shift = sscanf(string , "%m[^ ^|^\n]" , &(arg->command));
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

if(!was_command)
	return NULL;
return arg;
}


