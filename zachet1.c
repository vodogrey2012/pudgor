#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <semaphore.h>
#include <sys/mman.h>
#include <string.h>
#include <sys/types.h>
#include <sys/shm.h>


#define SEMNAME "/SEM"
#define SHMNAME "/SHM124"


struct _monitor
{
    sem_t* busy;
    char* adr;
};


int activity(struct _monitor* monitor, int act , char c);
struct _monitor* monitor_init();

int main()
{
    struct _monitor* monitor = monitor_init();  
   
    char* str;
    char str1[] =  "Hello world\n";
    char str2[] =  "Goodby world\n";

    // is it one
    str = activity(monitor , 1 , 0)
        ? str1
        : str2;

    //write str
    int i = 0;
    for(i = 0 ; i < strlen(str) ; i++)
    {
        sleep(1);
        activity(monitor , 0 ,str[i]);
    }
    
    // say that is is done
    activity(monitor , 2 , 0);

    return 0;
}

int activity(struct _monitor* monitor , int act , char c)
{
    int i = 0;
    int ret = 0;
    int num_of_active = 0;
    sem_wait(monitor->busy);
        // critical section
        if(act == 0)
        {
            printf("%c" , c);
            fflush(NULL);
        }

        else if(act == 1)
        {
            memcpy(&num_of_active , monitor->adr , sizeof(int)); 
            num_of_active++;
            memcpy(monitor->adr , &num_of_active , sizeof(int));
            
            if(num_of_active == 1)
            {   
                ret = 1;
            }
            else
            {   
                ret = 0;
            }
        }
        else if(act == 2)
        {
            memcpy(&num_of_active , monitor->adr , sizeof(int));
            num_of_active--;
            memcpy(monitor->adr , &num_of_active , sizeof(int));
        }

    sem_post(monitor->busy);
    return ret;
}



struct _monitor* monitor_init()
{
    struct _monitor* monitor = (struct _monitor*)calloc(1 , sizeof(struct _monitor));
    monitor->busy = sem_open (SEMNAME , O_CREAT | O_RDWR , 0777 , 1);
    sem_unlink(SEMNAME);

    int shmid = shm_open (SHMNAME , O_RDWR | O_CREAT , 0777);
    int p = ftruncate(shmid , 128);

    monitor->adr = mmap(NULL, 128 , PROT_WRITE|PROT_READ, MAP_SHARED, shmid, 0);
    return monitor;
}

