#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#define MAXNAME 50
#define MAXUSER 50

struct user
{
    char name[MAXNAME];
    int num;
};


struct user users[MAXUSER];
//int last_user = 0;

int add_user( char* user)
{
    static int last_user = 0;
    int i = 0;
    for ( i = 0 ; i < last_user ; i++)
    {
        if( 0 == strcmp(users[i].name , user))
        {
            users[i].num++;
            return last_user;
        }
    }

    strcpy( users[last_user].name , user);
    users[last_user].num = 1;
    last_user++;
    return last_user;
}



int main()
{

    int fd[2];
    pipe(fd);
    
    pid_t id = fork();
    if(id == 0)
    {
        // child
        
        close( fd[0]);
        close( 1);
        dup2( fd[1] , 1);
        close( fd[1]); 
        
        char* arg[] = {"ps" , "-eo" , "user" , NULL};
        execvp( "ps" , arg);
    }
    else
    {
        //parrent
        close( fd[1]);
        close( 0);
        dup2( fd[0] , 0);
        close( fd[0]);

        char str[MAXNAME];
  
        int last;      
        while(gets( str))
            last = add_user( str);

        int i = 1;
        for( i = 1 ; i < last ; i++)
            printf("%s \t  %d\n" , users[i].name , users[i].num);

        
        wait(0);
        
    }


    return 0;
}









