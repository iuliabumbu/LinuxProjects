#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/types.h>
#include <pthread.h>
#include <sys/wait.h>
#include "a2_helper.h"

typedef struct
{
	int process;
	int thread;
	pthread_mutex_t * lock8_1;
	pthread_mutex_t * lock8_2;
	pthread_mutex_t * lock12;
	pthread_mutex_t * lock5_3;
	pthread_mutex_t * lock8_3;
	pthread_mutex_t * lock5_1;
	pthread_cond_t * cond;
	pthread_cond_t * cond2;
	sem_t * logSem6;
	sem_t * logSem1;
	sem_t * logSem2;
	sem_t * barrier;
	sem_t **semOne;
	sem_t **semTwo;
}
th_struct;

pthread_mutex_t lock;
void *thread_fn2_3(void *param)
{
	th_struct *t = (th_struct*) param;

	if (t->process == 8 && t->thread == 3)
	{
		sem_wait(*t->semOne);
	}

	if (t->process == 8 && t->thread == 1)
	{
		pthread_mutex_lock(t->lock8_1);
	}

	info(BEGIN, t->process, t->thread);

	if (t->process == 8 && t->thread == 2)
	{
		pthread_mutex_unlock(t->lock8_1);
		pthread_mutex_lock(t->lock8_2);
	}

	info(END, t->process, t->thread);

	if (t->process == 8 && t->thread == 3)
	{
		sem_post(*t->semTwo);
	}

	if (t->process == 8 && t->thread == 1)
		pthread_mutex_unlock(t->lock8_2);

	return NULL;
}
int count = 0;
int ok = 0;

void *thread_fn2_4(void *param)
{
	th_struct *t = (th_struct*) param;


/*	pthread_mutex_lock(t->lock12);
	while(t->thread != 12 && ok == 0){
		pthread_cond_wait(t->cond, t->lock12);
	}
	pthread_mutex_unlock(t->lock12);


    pthread_mutex_lock(t->lock12);
	if(t->thread == 12 && ok == 0){
		ok = 1;
		pthread_cond_broadcast(t->cond);
	}
	pthread_mutex_unlock(t->lock12);

	sem_wait(t->logSem6);

	info(BEGIN, t->process, t->thread);

	sem_wait(t->logSem1);

	pthread_mutex_lock(t->lock12);
	count++;
	pthread_mutex_unlock(t->lock12);

	if (count % 6 == 0 )
	{
		sem_post(t->barrier);
	}

	sem_post(t->logSem1);
	sem_wait(t->barrier);

	info(END, t->process, t->thread);

	sem_post(t->barrier);

	sem_post(t->logSem6);
	*/
    
	sem_wait(t->logSem6);

    info(BEGIN, t->process, t->thread);

    pthread_mutex_lock(t->lock12);
	if(t->thread == 43 || t->thread == 44 || t->thread == 45 || t->thread == 46 || t->thread == 47 || t->thread == 12)
	{
		count++;

		if(count == 6)
		{
			ok = 1;
			pthread_cond_signal(t->cond2);
	    }

		if(t->thread == 43 || t->thread == 44 || t->thread == 45 || t->thread == 46 || t->thread == 47)
		{
		    pthread_cond_wait(t->cond, t->lock12);
		}

		else if(t->thread == 12)
		{
			if(ok == 0)
			{
			   pthread_cond_wait(t->cond2, t->lock12);
			}
			pthread_cond_broadcast(t->cond);
		}			
	}
	pthread_mutex_unlock(t->lock12);

    info(END, t->process, t->thread);

	sem_post(t->logSem6);


	return NULL;
}

void *thread_fn2_5(void *param)
{
	th_struct *t = (th_struct*) param;

	if (t->process == 5 && t->thread == 3)
	{
		info(BEGIN, t->process, t->thread);
		info(END, t->process, t->thread);
		sem_post(*t->semOne);
	}
	else if (t->process == 5 && t->thread == 1)
	{
		sem_wait(*t->semTwo);
		info(BEGIN, t->process, t->thread);
		info(END, t->process, t->thread);
	}
	else
	{
		info(BEGIN, t->process, t->thread);
		info(END, t->process, t->thread);
	}

	return NULL;
}

void creeazaThreaduri(pthread_t tids[], th_struct params[], int nr_threads)
{
	for (int i = 0; i < nr_threads; i++)
		if (nr_threads == 4)
		{
			pthread_create(&tids[i], NULL, thread_fn2_3, &params[i]);
		}
	else if (nr_threads == 47)
	{
		pthread_create(&tids[i], NULL, thread_fn2_4, &params[i]);
	}
	else if (nr_threads == 5)
	{
		pthread_create(&tids[i], NULL, thread_fn2_5, &params[i]);
	}

	for (int i = 0; i < nr_threads; i++)
		pthread_join(tids[i], NULL);
}

pid_t pid2, pid3, pid4, pid5, pid6, pid7, pid8, pid9;

pid_t creeazaProces(int id, th_struct params, th_struct params2[], th_struct params5[])
{
	pid_t pid;
	if ((pid = fork()) == 0)
	{
		info(BEGIN, id, 0);

		if (id == 2)
		{
			pid4 = creeazaProces(4, params, params2, params5);
			pid5 = creeazaProces(5, params, params2, params5);

			pthread_t threads[47];
			creeazaThreaduri(threads, params2, 47);

			waitpid(pid4, NULL, 0);
			waitpid(pid5, NULL, 0);
		}
		else if (id == 5)
		{
			pid6 = creeazaProces(6, params, params2, params5);
			pid7 = creeazaProces(7, params, params2, params5);

			pthread_t threads[5];
			creeazaThreaduri(threads, params5, 5);

			waitpid(pid6, NULL, 0);
			waitpid(pid7, NULL, 0);
		}
		else if (id == 7)
		{
			pid9 = creeazaProces(9, params, params2, params5);
			waitpid(pid9, NULL, 0);
		}

		info(END, id, 0);
		exit(0);
	}
	else
		return pid;
}

int main()
{

	init();

	info(BEGIN, 1, 0);
	pthread_mutex_t lock8_1 = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_t lock8_2 = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_t lock12 = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_t lock5_3 = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_t lock8_3 = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_t lock5_1 = PTHREAD_MUTEX_INITIALIZER;
	pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
	pthread_cond_t cond2 = PTHREAD_COND_INITIALIZER;
	pthread_mutex_init(&lock, 0);
	sem_t logSem6;
	sem_t barrier;
	sem_t logSem1;
	sem_t logSem2;
	sem_t *semOne = sem_open("semOne", O_CREAT, 0644, 0);
	sem_t *semTwo = sem_open("semTwo", O_CREAT, 0644, 0);

	if (sem_init(&logSem6, 0, 6) != 0)
	{
		perror("Could not init the semaphore");
		return -1;
	}
	if (sem_init(&barrier, 0, 0) != 0)
	{
		perror("Could not init the semaphore");
		return -1;
	}
	if (sem_init(&logSem1, 0, 1) != 0)
	{
		perror("Could not init the semaphore");
		return -1;
	}
	if (sem_init(&logSem2, 0, 1) != 0)
	{
		perror("Could not init the semaphore");
		return -1;
	}

	th_struct params, params2[47], params5[5];

	params.barrier = &barrier;
	params.logSem1 = &logSem1;
	params.logSem2 = &logSem2;
	params.logSem6 = &logSem6;
	params.lock5_1 = &lock5_1;
	params.lock5_3 = &lock5_3;
	params.lock8_1 = &lock8_1;
	params.lock8_2 = &lock8_2;
	params.lock8_3 = &lock8_3;
	params.cond = &cond;
	params.cond2 = &cond2;
	params.lock12 = &lock12;
	params.semOne = &semOne;
	params.semTwo = &semTwo;

	for (int i = 0; i < 47; i++)
	{
		params2[i].barrier = &barrier;
		params2[i].logSem1 = &logSem1;
		params2[i].logSem2 = &logSem2;
		params2[i].logSem6 = &logSem6;
		params2[i].lock5_1 = &lock5_1;
		params2[i].lock5_3 = &lock5_3;
		params2[i].lock8_1 = &lock8_1;
		params2[i].lock8_2 = &lock8_2;
		params2[i].lock8_3 = &lock8_3;
		params2[i].cond = &cond;
		params2[i].cond2 = &cond2;
		params2[i].lock12 = &lock12;
		params2[i].semOne = &semOne;
		params2[i].semTwo = &semTwo;
		params2[i].process = 2;
		params2[i].thread = i + 1;
	}

	for (int i = 0; i < 5; i++)
	{
		params5[i].barrier = &barrier;
		params5[i].logSem1 = &logSem1;
		params5[i].logSem2 = &logSem2;
		params5[i].logSem6 = &logSem6;
		params5[i].lock5_1 = &lock5_1;
		params5[i].lock5_3 = &lock5_3;
		params5[i].lock8_1 = &lock8_1;
		params5[i].lock8_2 = &lock8_2;
		params5[i].lock8_3 = &lock8_3;
		params5[i].cond = &cond;
		params5[i].cond2 = &cond2;
		params5[i].lock12 = &lock12;
		params5[i].semOne = &semOne;
		params5[i].semTwo = &semTwo;
		params5[i].process = 5;
		params5[i].thread = i + 1;
	}

	pid2 = creeazaProces(2, params, params2, params5);
	pid3 = creeazaProces(3, params, params2, params5);
	pid8 = creeazaProces(8, params, params2, params5);

	pthread_t threads[4];
	th_struct params4[4];

	for (int i = 0; i < 4; i++)
	{
		params4[i].barrier = &barrier;
		params4[i].logSem1 = &logSem1;
		params4[i].logSem2 = &logSem2;
		params4[i].logSem6 = &logSem6;
		params4[i].lock5_1 = &lock5_1;
		params4[i].lock5_3 = &lock5_3;
		params4[i].lock8_1 = &lock8_1;
		params4[i].lock8_2 = &lock8_2;
		params4[i].lock8_3 = &lock8_3;
		params4[i].cond = &cond;
		params4[i].cond2 = &cond2;
		params4[i].lock12 = &lock12;
		params4[i].semOne = &semOne;
		params4[i].semTwo = &semTwo;
		params4[i].process = 8;
		params4[i].thread = i + 1;
	}

	pthread_mutex_lock(params.lock8_1);
	pthread_mutex_lock(params.lock8_2);
	pthread_mutex_lock(params.lock12);
	pthread_mutex_lock(params.lock5_3);
	pthread_mutex_lock(params.lock8_3);
	pthread_mutex_lock(params.lock5_1);
	creeazaThreaduri(threads, params4, 4);
	waitpid(pid2, NULL, 0);
	waitpid(pid3, NULL, 0);
	waitpid(pid8, NULL, 0);
	info(END, 1, 0);

	sem_destroy(&logSem6);
	sem_destroy(&barrier);
	sem_destroy(&logSem1);
	sem_destroy(&logSem2);
	sem_close(semOne);
	sem_unlink("semOne");
	sem_close(semTwo);
	sem_unlink("semTwo");
	pthread_cond_destroy(&cond);
	pthread_cond_destroy(&cond2);
	pthread_mutex_destroy(&lock8_1);
	pthread_mutex_destroy(&lock8_2);
	pthread_mutex_destroy(&lock12);
	pthread_mutex_destroy(&lock5_3);
	pthread_mutex_destroy(&lock8_3);
	pthread_mutex_destroy(&lock5_1);
	return 0;
}