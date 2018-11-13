#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdlib.h>
#include <unistd.h>


void* sausager(void* p);
void* saucer(void* p);
void* checker(void* p);





struct _monitor
{
	char pizza[4];
	int num_pizza;
	char need_to_check[50][4];
	int num_to_check;
	pthread_mutex_t busy;
	pthread_cond_t new_pizza;
	void (*put_elem)(struct _monitor* monitor , char c);
	int (*check_pizza)(struct _monitor* monitor);
};


int check_pizza(struct _monitor* monitor)
{
	int i;
	int ret = 0;
	while(i < monitor->num_pizza)
	{
//		printf("checker\n");
		pthread_mutex_lock(&(monitor->busy));
		if(monitor->num_to_check > 0)
		{
			if((monitor->need_to_check[monitor->num_to_check][0]+
			    monitor->need_to_check[monitor->num_to_check][1]+
			    monitor->need_to_check[monitor->num_to_check][2]+
			    monitor->need_to_check[monitor->num_to_check][3]) 
			    != 'k' + 's' + 's')
				 ret++;
			i++;
			monitor->num_to_check--;
		}
		else
			pthread_cond_wait(&(monitor->new_pizza) , &(monitor->busy));

                pthread_mutex_unlock(&(monitor->busy));
	}
	return ret;
}

void put_elem(struct _monitor* monitor , char c)
{
int n , i;
for(n = 0 ; n < monitor->num_pizza ; n++)
{
//	printf("%c before_critical_section\n" , c);
	fflush(NULL);
	pthread_mutex_lock(&(monitor->busy));
//	printf("%c inside\n" , c);
	fflush(NULL);	
	
	i = 0;	
	while(0 != monitor->pizza[i])
		i++;

//	printf("-%d-" , i);	
	monitor->pizza[i] = c;

	if(i == 2)
	{
		monitor->num_to_check++;

		monitor->need_to_check[monitor->num_to_check][0] = monitor->pizza[0];
		monitor->need_to_check[monitor->num_to_check][1] = monitor->pizza[1];
		monitor->need_to_check[monitor->num_to_check][2] = monitor->pizza[2];
		monitor->need_to_check[monitor->num_to_check][3] = monitor->pizza[3];
		
//		printf("%s\n" , monitor->pizza);
		fflush(NULL);
		monitor->pizza[0] = 0;
		monitor->pizza[1] = 0;
		monitor->pizza[2] = 0;
		monitor->pizza[3] = 0;

		pthread_cond_broadcast(&(monitor->new_pizza));

//		printf("signaled\n");
		fflush(NULL);
	}
	else
	{
//              printf("%c wait\n" , c);
                fflush(NULL);
	//	sleep(1);
                pthread_cond_wait(&(monitor->new_pizza) , &(monitor->busy));
//              printf("get_sig\n");
                fflush(NULL);
        }

	pthread_mutex_unlock(&(monitor->busy));
}
}



int main(int argc, char** argv)
{
	struct _monitor monitor;

	char* l;
        monitor.num_pizza = strtoll(argv[1] , &l , 10);
	monitor.put_elem = &put_elem;
	monitor.check_pizza = &check_pizza;
	monitor.num_to_check = 0;
	pthread_t id_sauce, id_sausage_1, id_sausage_2, id_checker;

	monitor.num_to_check = 0;

	pthread_mutex_init(&(monitor.busy) , NULL);

	pthread_create(&id_sauce , NULL , &saucer , &monitor);    
	pthread_create(&id_sausage_1 , NULL , &sausager , &monitor);
	pthread_create(&id_sausage_2 , NULL , &sausager , &monitor);
	pthread_create(&id_checker , NULL , &checker , &monitor);

	
	pthread_join(id_checker , NULL);
	pthread_join(id_sauce , NULL);
	pthread_join(id_sausage_1 , NULL);
	pthread_join(id_sausage_2 , NULL);
	

	return 0;
}

void* sausager(void* p)
{
	struct _monitor* monitor = (struct _monitor*) p;
	monitor->put_elem(monitor , 'k');
}

void* saucer(void* p)
{
        struct _monitor* monitor = (struct _monitor*) p;
        monitor->put_elem(monitor , 's');
}

void* checker(void* p)
{
        struct _monitor* monitor = (struct _monitor*) p;
        if(monitor->check_pizza(monitor))
	{
		printf("GOOD PIZZA\n");
	}
	else
	{
		printf("BAD PIZZA!\n");
	}
	fflush(NULL);
}


