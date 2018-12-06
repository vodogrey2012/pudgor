#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>



#define SEMNUM 7
#define SEMINSTR 20

struct Val
{
    int ship_size;
    int pass_num;
    int ladder_size;
    int trip_num;
};


int sem_init( struct sembuf* sops , struct Val val);
struct Val* val_init( char* argv[]);
void sops_init( struct sembuf* sops , unsigned short op_num , unsigned short sem_num ,
                                                    short sem_op , short sem_flg);
void passanger( struct sembuf sops[] , int sem_id , int sem_pas_id , int sem_pas_num);
void ship( struct sembuf sops[] , int sem_id , int sem_pas_id , struct Val val);
int sem_pass_init(struct Val val);
void wait_while_trip(int sem_pas_id , int sem_pas_num);
void tell_pass_that_trip_is_finished(int sem_pas_id , struct Val val);
void set_wait_while_trip( int sem_pas_id , int sem_pas_num);



int main(int argc , char* argv[])
{
    if(argc != 5)
    {
        printf("Usage: <pass_num> <trip_num> <ship_size> <ladder_size>\n");
        return 1;
    }

    struct Val* val = val_init( argv);
    struct sembuf sops[SEMINSTR];
    int sem_id = sem_init( sops , *val);
    int sem_pass_id = sem_pass_init( *val);    

    int i = 0;
    pid_t pid;

    for( i = 0 ; i < val->pass_num ; i++)
    {
        pid = fork();
        if(!pid)
        {
            passanger( sops , sem_id , sem_pass_id , i);
            return 0;
        }
    }
    sleep(1); 
    ship( sops , sem_id , sem_pass_id , *val);        

    while(wait(NULL) == -1);
    semctl( sem_id , SEMNUM , IPC_RMID);    

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
    {
        perror("Semget in init error");
        exit(0);
    }

    sops_init( sops , 0 , 0 , -1, 0); // dicrease number of trips, for ship 
    sops_init( sops , 1 , 0 , 0 , IPC_NOWAIT); // check available trip, for pass
    sops_init( sops , 2 , 1 , -1 ,0); // pass go on ladder
    sops_init( sops , 3 , 2 , 0 , 0); // pass check is ladder available
    sops_init( sops , 4 , 1 , 1 , 0); // pass go from  ladder 
    sops_init( sops , 5 , 2 , 1 , 0); // ship take up ladder
    sops_init( sops , 6 , 2 , -1, 0); // ship take down ladder
    sops_init( sops , 7 , 3 , -1, 0); // pass go from ship
    sops_init( sops , 8 , 3 , val.ship_size , 0); // ship update data of pass removed
    sops_init( sops , 9 , 4 , -1, 0); // new pass on ship
    sops_init( sops , 10, 4 , val.ship_size , 0); // ship update data of new pass
    sops_init( sops , 11, 3 , 0 , 0); // all pass go away from ship
    sops_init( sops , 12, 4 , 0 , 0); // ship is full
    sops_init( sops , 13, 5 , -1, 0); // pass reserved on ship
    sops_init( sops , 14, 5 , 0 , 0); // all ship is reserved
    sops_init( sops , 15, 5 , val.ship_size , 0); //update reserve data
    sops_init( sops , 16, 5 , val.pass_num , 0); //on exit
    sops_init( sops , 17, 6 , -1 , IPC_NOWAIT);  //pass is gone
    sops_init( sops , 18, 6 , 0 , 0); 
    sops_init( sops , 19, 6 , val.pass_num , 0);
 
    //initial conditions
    struct sembuf init[7]; 
    sops_init( init , 0 , 0 , val.trip_num , 0);    // num of trips
    sops_init( init , 1 , 1 , val.ladder_size , 0); // size of ladder
    sops_init( init , 2 , 2 , 1 , 0);               // ladder is unavailable
    sops_init( init , 3 , 3 , 0 , 0);               // nobody must go from ship
    sops_init( init , 4 , 4 , val.ship_size , 0);   // set num of new pass
    sops_init( init , 5 , 5 , val.ship_size , 0);   // set num of places reserved
    if(semop( sem_id , init , 6))
    {
        perror("Semop in init error");
        exit(0);
    }

    return sem_id;
}

int sem_pass_init(struct Val val)
{
    int sem_id = semget(IPC_PRIVATE, val.pass_num , 0777|IPC_CREAT);
    if(sem_id <= 0)
    {
        perror("Semget in sem_pas_init error");
        exit(0);
    }
    return sem_id;
}

void sops_init( struct sembuf* sops , unsigned short op_num , unsigned short sem_num ,
                                                    short sem_op , short sem_flg)
{
    sops[op_num].sem_num = sem_num;
    sops[op_num].sem_op  = sem_op;
    sops[op_num].sem_flg = sem_flg;
}


void passanger( struct sembuf sops[] , int sem_id , int sem_pas_id , int sem_pas_num)
{
    int status = 0; // 0 - pass on land , 1 - pass on ship
    
    pid_t pid = getpid();
    while(1)
    {
        do
        {

        if( !status)
        {
            semop( sem_id , &sops[13] , 1);
        }
                

        printf("Passanger %d on %s and waits for ladder\n" , pid , 
                                                status 
                                                ? "ship" 
                                                : "land, booked a place on ship");
  
        fflush(NULL);

        semop( sem_id , &sops[2] , 2);
        // On ladder
        if( !semop( sem_id , &sops[17] , 1))
            {
                printf("Passanger %d left from ladder and gone.\n" , pid);
                fflush(NULL);
                semop( sem_id , &sops[4] , 1);
                return;
            }


        printf("Passanger %d on ladder\n" , pid);
        fflush(NULL);

        status = (status + 1) % 2;
        // Not on ladder

        printf("Passanger %d on %s\n" , pid , status 
                                              ? "ship" 
                                              : "land");
        fflush(NULL);

        semop( sem_id , &sops[4] , 1);
 
        if( status)
        {
            set_wait_while_trip( sem_pas_id , sem_pas_num);
            semop( sem_id , &sops[9] , 1);
        }
        else
            semop( sem_id , &sops[7] , 1);
        }
        while( !status);
        
        wait_while_trip( sem_pas_id , sem_pas_num);
    }
}


void wait_while_trip( int sem_pas_id , int sem_pas_num)
{
    struct sembuf buf = {sem_pas_num , 0 , 0};
    semop( sem_pas_id , &buf , 1);
}


void set_wait_while_trip( int sem_pas_id , int sem_pas_num)
{
    struct sembuf buf = {sem_pas_num , 1 , 0};
    semop( sem_pas_id , &buf , 1);
}


void ship( struct sembuf sops[] , int sem_id , int sem_pas_id , struct Val val)
{
    int cur_trip = 0;
    int is_trip_last = 0;
    do
    {
        cur_trip++;
        printf("\n==================\nNumber of ship filling: %d\n" , cur_trip);
        fflush(NULL);
        tell_pass_that_trip_is_finished( sem_pas_id , val);
 
        printf("Ladder is available\n");
        fflush(NULL);

        semop( sem_id , &sops[6] , 1);

        semop( sem_id , &sops[14], 1);
        semop( sem_id , &sops[11], 1);
        semop( sem_id , &sops[12], 1);

        printf("Ship is full\n");
        printf("Ladder is unavailable\n");
        fflush(NULL);   
        semop( sem_id , &sops[5] , 1);
        
        //trip
        printf("---TRIP---\n");
        fflush(NULL);
        sleep(1);
       
        semop( sem_id , &sops[0] , 1 ); 
        if( !semop( sem_id , &sops[1] , 1))
            break;        
 
        //update data about pass
        semop( sem_id , &sops[8] , 1);
        semop( sem_id , &sops[10], 1);
        semop( sem_id , &sops[15], 1);
    }
    while(1);
    semop( sem_id , &sops[19], 1);
    semop( sem_id , &sops[16], 1);
    tell_pass_that_trip_is_finished( sem_pas_id , val);
    semop( sem_id , &sops[6] , 1);
    semop( sem_id , &sops[18], 1);
    printf("IT WAS THE LAST TRIP, SHIP IS GONE\n");
    fflush(NULL);

}

void tell_pass_that_trip_is_finished(int sem_pas_id , struct Val val)
{
    int i = 0;
    
    for( i = 0 ; i < val.pass_num ; i++)
        semctl( sem_pas_id , i , SETVAL , 0);
}









