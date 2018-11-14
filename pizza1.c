#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdlib.h>
#include <unistd.h>

#define PILE_LEN 50

void* sausager(void* p);
void* saucer(void* p);
void* checker(void* p);





struct _monitor
{
	char pizza[4];
	int num_pizza;
	char need_to_check[PILE_LEN][4];
	int num_to_check;
	pthread_mutex_t busy;
	pthread_cond_t pizza_is_ready;
	void (*put_elem)(struct _monitor* monitor , char c);
	int (*check_pizza)(struct _monitor* monitor);
	void (*pizza_to_check_pile)(struct _monitor* monitor);
};


int check_pizza(struct _monitor* monitor)
{
	pthread_mutex_lock(&(monitor->busy));
	int ret = 0;
	
	if(monitor->num_to_check == 0)
		pthread_cond_wait(&(monitor->pizza_is_ready) , &(monitor->busy));

	if((monitor->need_to_check[monitor->num_to_check][0]+
    	    monitor->need_to_check[monitor->num_to_check][1]+
	    monitor->need_to_check[monitor->num_to_check][2]+
	    monitor->need_to_check[monitor->num_to_check][3]) != ('k' + 's' + 's'))
		ret++;
	
	monitor->num_to_check--;

        pthread_mutex_unlock(&(monitor->busy));
	return ret;
}

void pizza_to_check_pile(struct _monitor* monitor)
{
	monitor->num_to_check++;
	int i = 0;
	for(i = 0 ; i < 3 ; i++)
	{
        	monitor->need_to_check[monitor->num_to_check][i] = monitor->pizza[i];
                monitor->pizza[i] = 0;
	}
}

void put_elem(struct _monitor* monitor , char c)
{
	pthread_mutex_lock(&(monitor->busy));
	
	int i = 0;	
	while(0 != monitor->pizza[i])
		i++;

	monitor->pizza[i] = c;

	if(i == 2)
	{
		monitor->pizza_to_check_pile(monitor);
		pthread_cond_broadcast(&(monitor->pizza_is_ready));
	}
	else
	{
                pthread_cond_wait(&(monitor->pizza_is_ready) , &(monitor->busy));
        }

	pthread_mutex_unlock(&(monitor->busy));
}


void monitor_init(struct _monitor* monitor , int num_pizza)
{
	monitor->num_pizza = num_pizza;
	monitor->put_elem = &put_elem;
        monitor->check_pizza = &check_pizza;
        monitor->pizza_to_check_pile = &pizza_to_check_pile;

	monitor->num_to_check = 0;

	pthread_mutex_init(&(monitor->busy) , NULL);
	pthread_cond_init(&(monitor->pizza_is_ready) , NULL);

        int i = 0;
        for(i = 0 ; i < 3 ; i++)
                monitor->pizza[i] = 0;

}

int main(int argc, char** argv)
{
	struct _monitor monitor;

	char* l;
        int num_pizza = strtoll(argv[1] , &l , 10);


	monitor_init(&monitor , num_pizza);

	pthread_t id_sauce, id_sausage_1, id_sausage_2, id_checker;

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
	int i = 0;
	for(i = 0 ; i < monitor->num_pizza ; i++)
		monitor->put_elem(monitor , 'k');
}

void* saucer(void* p)
{
        struct _monitor* monitor = (struct _monitor*) p;
        int i = 0;
	for(i = 0 ; i < monitor->num_pizza ; i++)
		monitor->put_elem(monitor , 's');
}

void* checker(void* p)
{
        struct _monitor* monitor = (struct _monitor*) p;
	int i = 0 , ans = 0;
	for(i = 0 ; i < monitor->num_pizza ; i++)
		ans += monitor->check_pizza(monitor);

        if(ans)
	{
		printf("GOOD PIZZA\n");
	}
	else
	{
		printf("BAD PIZZA!\n");
	}
	fflush(NULL);
}
