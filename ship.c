#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdlib.h>



#define SEMNUM 5
#define SEMINSTR 15

struct Val
{
    int ship_size;
    int pass_num;
    int ladder_size;
    int trip_num;
};


int sem_init(struct sembuf* sops , struct Val val);
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
    struct sembuf sops[SEMINSTR];
    int sem_id = sem_init( sops , *val);
    

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


int sem_init(struct sembuf* sops , struct Val val)
{
    int sem_id = semget(IPC_PRIVATE, SEMNUM, 0777|IPC_CREAT);
    if(sem_id <= 0)
        exit(1);

    sops_init( sops , 0 , 0 , -1, 0); // dicrease number of trips, for ship 
    sops_init( sops , 1 , 0 , 0 , IPC_NOWAIT); // check available trip, for pass
    sops_init( sops , 2 , 1 , -1 ,0); // pass go on ladder
    sops_init( sops , 3 , 1 , 1 , 0); // pass go from  ladder 
    sops_init( sops , 4 , 2 , 0 , 0); // pass check is ladder available
    sops_init( sops , 5 , 2 , 1 , 0); // ship take up ladder
    sops_init( sops , 6 , 2 , -1, 0); // ship take down ladder
    sops_init( sops , 7 , 3 , -1, 0); // pass go from ship
    sops_init( sops , 8 , 3 , val.ship_size , 0); // ship update data of pass removed
    sops_init( sops , 9 , 4 , -1, 0); // new pass on ship
    sops_init( sops , 10, 4 , val.ship_size , 0); // ship update data of new pass

    //initial conditions
    struct sembuf init[5]; 
    sops_init( init , 0 , 0 , val.trip_num , 0);    // num of trips
    sops_init( init , 1 , 1 , val.ladder_size , 0); // size of ladder
    sops_init( init , 2 , 2 , 1 , 0);               // ladder is unavailable
    sops_init( init , 3 , 3 , 0 , 0);               // nobody must go from ship
    sops_init( init , 4 , 4 , val.ship_size , 0);   // set num of new pass
    semop( sem_id , init , 5); 

    return sem_id;
}


void sops_init( struct sembuf* sops , unsigned short op_num , unsigned short sem_num , 
                                                    short sem_op , short sem_flg)
{
    sops[op_num].sem_num = sem_num;
    sops[op_num].sem_op  = sem_op;
    sops[op_num].sem_flg = sem_flg;
}

















