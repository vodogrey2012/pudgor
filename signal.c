#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

      
#define READY SIGUSR1
#define BIT0 SIGUSR2
#define BIT1 SIGUSR1 


// SIGUSR1 - 1 bit; answer from parent, that it is ready to get new signal
// SIGUSR2 - 0 bit



void send_char(char c , sigset_t waitmask , sigset_t actmask);
void parent(pid_t pid , sigset_t waitmask , sigset_t actmask);
void child(int file , sigset_t waitmask , sigset_t actmask);
int open_file(char* name);
void sig_bit(int signo);
int out_char();
void print_char(char out_char);
void sig_empty(int sigio);
void parent_sent(int sigio);
void signal_init(sigset_t waitmask , sigset_t actmask);

int main(int argc , char* argv[])
{
	int file;

	sigset_t waitmask , actmask;
	signal_init(waitmask , actmask);

	pid_t pid = fork();
	
	
	if(pid)
	{
		parent(pid , waitmask , actmask);
	}
	else
	{
		//file = open_file(argv[1]);
		child(file , waitmask , actmask);
		//close(file);
	}
	wait(0);
	return 0;	


}

void signal_init(sigset_t waitmask , sigset_t actmask)
{
	sigemptyset(&actmask);

        sigemptyset(&waitmask);
        sigaddset(&waitmask , BIT0);
        sigaddset(&waitmask , BIT1);

        sigprocmask(SIG_SETMASK , &waitmask , NULL);

        signal(READY , parent_sent);
}



int first_parent_signal = 0;

void parent_sent(int sigio){
	first_parent_signal = 1;
};


void sig_empty(int sigio){
};



void send_char(char c , sigset_t waitmask , sigset_t actmask) // for child
{
        signal(BIT1 , sig_empty);
        signal(BIT0 , sig_empty);


	int i = 128;
	pid_t pid = getppid();

	signal(READY , sig_empty);

	for(i = 128 ; i > 0 ; i /= 2) 
	{
	
		if(first_parent_signal)
			first_parent_signal = 0;
		else
			sigsuspend(&actmask);

        	if (i & c)
		{
        		kill(pid, BIT1);
		}
        	else 
		{
            		kill(pid, BIT0);
		}
		

	}
}


void child(int file , sigset_t waitmask , sigset_t actmask)
{
	char str[] = "hello\n";
	int i = 0;
	for( i = 0 ; str[i] ; i++)
		send_char(str[i] , waitmask , actmask);
}

char bit = 0;

void parent(pid_t pid , sigset_t waitmask , sigset_t actmask)
{
	signal(BIT0 , sig_bit);
        signal(BIT1 , sig_bit);

	while(1)
	{
		

		kill(pid , READY); 
		
		sigsuspend(&actmask);

		if(!out_char())
			break;
	}

}

void sig_bit(int signo)
{
	if(signo == BIT0)
		bit = 0;
	else if(signo == BIT1)
		bit = 1;
	else 
	{
		printf("invalid signal");
		fflush(NULL);
		exit(0);
	}

}

int out_char()
{
	static int out_char = 0;
	static int counter = 128;

	if(bit == 0)
	{
		counter/=2;
	}
	else
	{
		out_char += counter;
    		counter /= 2;
	}
	
	if(!counter)
	{
		print_char(out_char);
		counter = 128;
		if(out_char)
			out_char = 0;
		else 
			return 0;	
	}

	return 1;	
}

void print_char(char out_char)
{
	printf("%c" , out_char);
	fflush(NULL);
}
















