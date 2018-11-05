#include "header.h"

pthread_mutex_t mutex;
pthread_cond_t cond;

int main(int argc, char **argv)
{
	int id;
	Stock *shm_stock;
	sigset_t set;

	/*	Lê o id deste armazem	*/
	id = (int)strtol(argv[1], NULL, 10);

	/*	Initialize mutex and cond */
	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&cond, NULL);

	/*	Attach da shm do stock*/
	shm_stock = (Stock *)attach_shm(ID_SHM_ARM);


	/* SIGNAL HANDLING (start)*/
	sigfillset (&set);
	sigdelset (&set, SIGINT);
	sigdelset (&set, SIGUSR1);
	sigprocmask (SIG_SETMASK, &set, NULL);

	signal (SIGINT, sigint_handler_armazem); /*CTRL+C handler*/
	/* SIGNAL HANDLING (finish)*/


return 0;
}

void sigint_handler_armazem(int signum)
{
	printf("End armzem\n");
	_exit(0);	/*	Faz o detach da shm	*/
}


void *attach_shm(int type)
{
	key_t key;
	int id;

	if ((key = ftok(KEYS_FILE, type)) == -1)
		return (void *)(-1);

	if ((id = shmget(key, 0, 0)) == -1)
		return (void *)(-1);


	return shmat(id, NULL, 0);
}
