#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>




struct msgbuf
{ 
    long mtype;
};



void runer( int msgid , int runer);
void judge( int msgid , int num_of_runers);


int main( int argc , char* argv[])
{
    if( argc != 2)
    {
        printf("Use: <number_of_runers>\n");
        return 0;
    }
    char* c;
    int num_of_runers = strtol( argv[1] , &c , 10);

    int msgid = 0;
    if ((msgid = msgget(IPC_PRIVATE, 0777|IPC_CREAT)) == -1) 
    { 
        perror("Msgget in int\n");
    }

    int i = 0;
    for( i = 0 ; i < num_of_runers ; i++)
    {
        if(!fork())
        {
            runer(msgid , i + 1);
            return 0;
        }
    }
    
    judge(msgid , num_of_runers);
    
    wait(0);
    
    return 0;
}


void runer( int msgid , int runer)
{
    struct msgbuf start;
    struct msgbuf finish = {runer + 1};
    
    msgrcv( msgid , &start , 0 , runer , 0);  // wait prev runer
     
    //run
    printf("Runer %d: start -> " , runer);
    fflush(NULL);
    sleep(1);
    printf("finish\n");
    fflush(NULL);
    
    msgsnd( msgid , &finish , 0 , 0);
}



void  judge( int msgid , int num_of_runers)
{
    struct msgbuf jstart  = {1};
    struct msgbuf jfinish;
    struct timeval time_start , time_finish;
    
    printf("JUDGE IS READY\n");
    fflush(NULL);

    gettimeofday( &time_start , NULL);
    msgsnd( msgid , &jstart , 0 , 0);
    // run
    msgrcv ( msgid , &jfinish, 0 , num_of_runers + 1 , 0);
    gettimeofday( &time_finish , NULL);
    
    printf("FINISH\n");
    fflush(NULL);
    
    printf("Runtime %ld.%06ld\n" , (time_finish.tv_sec - time_start.tv_sec) , 
                                   (time_finish.tv_usec - time_start.tv_usec));
    fflush(NULL);
}




















