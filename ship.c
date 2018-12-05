#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdlib.h>



#define SEMNUM 4

struct Val
{
    int ship_size;
    int pass_num;
    int ladder_size;
    int trip_num;
};


int sem_init(struct sembuf* sops);
struct Val* val_init(char* argv[]);
void sops_init( struct sembuf* sops , unsigned short op_num , unsigned short sem_num ,       
                                                    short sem_op , short sem_flg);





int main(int argc , char* argv[])
{
    if(argc != 5)
    {
        printf("Invalid usage\n");
        return 1;
    }

    struct Val* val = val_init( argv);
    struct sembuf sops[SEMNUM];
    int sem_id = sem_init( sops);
    

    return 0;
}




struct Val* val_init(char* argv[])
{
    struct Val* val = (struct Val*)calloc(1 , sizeof(struct Val));
    char* c; 
    if((val->pass_num = strtol(argv[1] , &c , 10)) <= 0 ||
       (val->trip_num = strtol(argv[2] , &c , 10)) <= 0 ||
       (val->ship_size = strtol(argv[3] , &c , 10)) <= 0 ||
       (val->ladder_size = strtol(argv[4] , &c , 10)) <= 0)
        exit(0);
    
    if(val->pass_num < val->ship_size)
        val->ship_size = val->pass_num;
    
    return val;
}


int sem_init(struct sembuf* sops)
{
    sops_init( sops , 1 , 1 , 3 , 0);


    int sem_id = semget(IPC_PRIVATE, SEMNUM, 0777|IPC_CREAT);
    if(sem_id)
        return sem_id;

    exit(1);
}


void sops_init( struct sembuf* sops , unsigned short op_num , unsigned short sem_num , 
                                                    short sem_op , short sem_flg)
{
    sops[op_num].sem_num = sem_num;
    sops[op_num].sem_op  = sem_op;
    sops[op_num].sem_flg = sem_flg;
}

















