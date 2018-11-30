#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

      
#define READY SIGUSR1
#define BIT0 SIGUSR2
#define BIT1 SIGUSR1 
#define LENSTR 1

// SIGUSR1 - 1 bit; answer from parent, that it is ready to get new signal
// SIGUSR2 - 0 bit


char bit = 0;
int first_parent_signal = 0;



void send_char(char c , sigset_t waitmask , sigset_t actmask);
void parent(pid_t pid , sigset_t waitmask , sigset_t actmask);
void child(int file , sigset_t waitmask , sigset_t actmask);
void sig_bit(int signo);
int out_char();
int open_file(char* file_name);
void print_char(char out_char);
void sig_empty(int sigio);
void parent_sent(int sigio);
void signal_init(sigset_t waitmask , sigset_t actmask);


void parent_sent(int sigio){
        first_parent_signal = 1;
};


void sig_empty(int sigio){
};


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
		file = open_file(argv[1]);
		child(file , waitmask , actmask);
		close(file);
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


void child(int file , sigset_t waitmask , sigset_t actmask)
{
        char* str = (char*)calloc(LENSTR , sizeof(char));
	signal(READY , sig_empty);	
	int a = 1 , i; 
	while(a > 0)
        {
		a = read(file , str , LENSTR);
        	for(i = 0 ; i < a ; i++)
			send_char(str[i] , waitmask , actmask);
	}
		
}
 

void parent(pid_t pid , sigset_t waitmask , sigset_t actmask)
{
        signal(BIT0 , sig_bit);
        signal(BIT1 , sig_bit);

        while(1)
        {


                kill(pid , READY);
		alarm(1);
                sigsuspend(&actmask);

                if(!out_char())
                        break;
        }

}


void send_char(char c , sigset_t waitmask , sigset_t actmask) 
{
	int i = 128;
	pid_t pid = getppid();


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
		write(STDOUT_FILENO , &out_char , 1); 
		counter = 128;
		out_char = 0;
		fflush(NULL);
	}
	
	return 1;	
}


int open_file(char* file_name)
{
	int file_discriptor = open(file_name , O_RDONLY);
	if(file_discriptor == -1)
	{
		printf("Invalid file name");
		exit(0);
	}
	return file_discriptor;
}














