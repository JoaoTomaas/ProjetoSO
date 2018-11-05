#include "header.h"


/*
*	1) drones_id_init() tem que saber as coordenadas maximas do "terreno"
*	2) central não compila bem porque nao conhece move_towards()
*	3) desruir named pipe
*
*
*/

int uni_tempo;
pthread_mutex_t mutex;
pthread_cond_t cond;

int main(int argc, char **argv)
{
	int num_drones, x_max, y_max, i;

	Armazem 		*addr_shm_arm;
	Stock 		*addr_shm_stock;
	Estatistica *addr_shm_est;
	NomeProduto	*addr_shm_nomes_produtos;
	pthread_mutex_t *mutexes;
	pthread_cond_t *conds;
	Drone *drones_info;
	pthread_t *drones_id;
	sigset_t set;
	pthread_t tid;
	CleanupCentral cleanup_central_struct;

	ler_inteiros(argv[1], &num_drones, &uni_tempo, &x_max, &y_max);

	/*	Inicializa as globais	*/
	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&cond, NULL);

	/*	Alocação para os mutexes, conds, info, ids*/
	mutexes = (pthread_mutex_t *)malloc(num_drones*sizeof(pthread_mutex_t));
	conds = (pthread_cond_t *)malloc(num_drones*sizeof(pthread_cond_t));
	drones_info = (Drone *)malloc(num_drones*sizeof(Drone));
	drones_id = (pthread_t *)malloc(num_drones*sizeof(pthread_t));

	/*	Attach todas as shm (armazens, stock, estatisticas, nomes_produtos)	*/
	addr_shm_arm = (Armazem *)attach_shm(ID_SHM_ARM);
	addr_shm_stock = (Stock *)attach_shm(ID_SHM_STOCK);
	addr_shm_est = (Estatistica *)attach_shm(ID_SHM_EST);
	addr_shm_nomes_produtos = (NomeProduto *)attach_shm(ID_SHM_NOMES_PRODUTOS);

	/*
	*	Criação de um mutex e de uma variável de condição para cada drone
	*	Inicialização das estrutura Drone
	*	Criação das threads
	*/

	if (!mutex_init(mutexes, num_drones))
		return -1;
	if (!cond_init(conds, num_drones))
		return -1;
	drones_info_init(drones_info, mutexes, conds, num_drones);
	if (!drones_id_init(drones_id, drones_info, num_drones))
		return -1;


	/*	Setup da thread responsável pela terminação controlada	*/
	setup_thread_central(&cleanup_central_struct, mutexes, conds,
								drones_info, drones_id);
	pthread_create(&tid, NULL, cleanup_central, (void *)&cleanup_central_struct);

	/* SIGNAL HANDLING (start)*/
	sigfillset (&set);
	sigdelset (&set, SIGINT);
	sigdelset (&set, SIGUSR1);
	sigprocmask (SIG_SETMASK, &set, NULL);

	signal (SIGINT, sigint_handler_central); /*CTRL+C handler*/
	/* SIGNAL HANDLING (finish)*/

	/*	Criação do named pipe	*/
	if ((mkfifo(PIPE_NAME, 0600) < 0))
		return -1;

	/*	Pronto para receber encomendas do pipe e mete-a em "encomenda"	*/

return 0;
}

void setup_thread_central(CleanupCentral *s, pthread_mutex_t *m,
									pthread_cond_t *c, Drone *d, pthread_t *i)
{
		s->mutexes = m;
		s->conds = c;
		s->drones_info = d;
		s->drones_id = i;
}

void sigint_handler_central(int signum)
{
	pthread_mutex_lock(&mutex);
	pthread_cond_signal(&cond);
	pthread_mutex_unlock(&mutex);
}

void *cleanup_central(void *arg)
{
	CleanupCentral *s = (CleanupCentral *)arg;

	pthread_mutex_lock(&mutex);
	while(1)
		pthread_cond_wait(&cond, &mutex);

	free(s->mutexes);
	free(s->conds);
	free(s->drones_info);
	free(s->drones_id);

	/*	PIPE	cleanup	*/
	unlink(PIPE_NAME);

	_exit(0);	/*Faz o dettach das shm que já estão marcadas para destruição pelo SimulationManager	*/
}






void ler_inteiros(char *str, int *d, int *t, int *x, int *y)
{
	char *token;
	token = strtok(str, " ");
	*d = (int)strtol(token, NULL, 10);
	token = strtok(NULL, " ");
	*t = (int)strtol(token, NULL, 10);
	token = strtok(NULL, " ");
	*x = (int)strtol(token, NULL, 10);
	token = strtok(NULL, " ");
	*y = (int)strtol(token, NULL, 10);
}

void *drone_start(void *drone_info)
	/*	a variável "uni_tempo" não existe aqui, mas deve	*/

	/*	este código deve estar num while(TRUE)	*/
{
	Drone *info;
	info = (Drone *)drone_info;

	pthread_mutex_lock(info->mutex);


	/* Espera por uma Encomenda*/
	while (!info->encomenda)
		pthread_cond_wait(info->cond, info->mutex);

	/*	Existe uma encomenda*/
	info->estado = OCUPADO;

	/*	Vai até ao armazém*/
	while(move_towards(&info->x, &info->y,
							info->encomenda->x_arm, info->encomenda->y_arm
							)
			)
	{
		/*********************************************************
		**	A função move_towards já atualiza a estrutra Drone	****
		**	com as coordenadas atuais corretas	********************
		**********************************************************/
		pthread_mutex_unlock(info->mutex);
		sleep(uni_tempo);
		pthread_mutex_unlock(info->mutex);
	}
	/*	Drone chegou ao armazém
	*	Ainda tenho o mutex
	*/


	pthread_exit(NULL);
}


int drones_id_init(pthread_t *ids, Drone *info, int n)
{
	int i;
	for (i=0; i<n; ++i)
	{
		if (pthread_create(ids+i, NULL, drone_start, (void *)(info+i)))
			return 0;
	}
	return 1;
}

void drones_info_init(Drone *a, pthread_mutex_t *mutexes,
							pthread_cond_t *conds, int n)
{
	int i;
	for (i=0; i<n; ++i)
	{
		a[i].x = a[i].y = 0.0;
		a[i].id = i;
		a[i].estado = DESOCUPADO;
		a[i].encomenda = NULL;
		a[i].mutex = mutexes + i;
		a[i].cond = conds + i;
	}
}

int cond_init(pthread_cond_t *a, int n)
{
	while(n--)
	{
		if (pthread_cond_init(a+n, NULL))
			return 0;
	}

	return 1;
}

int mutex_init(pthread_mutex_t *a, int n)
{
	while(n--)
	{
		if (pthread_mutex_init(a+n, NULL))
			return 0;
	}

	return 1;
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


double distance(double x1, double y1, double x2, double y2){
     return sqrtf( (x2-x1)*(x2-x1) + (y2-y1)*(y2-y1) );
}

/*
 * Advances the drone 1 step towards the target point.
 *
 * You should provide the address for the coordinates of your drone,
 * in order for them to be updated, according to the following cases
 *
 *
 * returns  1 - in case it moved correctly towards the target
 *				drone_x, drone_y will be updated
 *
 * returns  0 - in case it moved and reached the target
 *				drone_x, drone_y will be updated
 *
 * returns -1 - in case it was already in the target
 *				drone_x, drone_y are NOT updated
 * returns -2 - in case there is an error
 *				drone_x, drone_y are NOT updated
 */

int move_towards(double *drone_x, double *drone_y, double target_x, double target_y){

	// if one of the coords is negative, there is an error.
	if( *drone_x < 0 ||  *drone_y < 0 ||  target_x < 0 ||  target_y < 0){
		return -2;
	}

	// if it is on the target, does not move
	if( (*drone_x == target_x) && (*drone_y ==  target_y)){
		return -1;
	}

	// if distance < 1, move to target, return 0
	if( distance(*drone_x, *drone_y,  target_x,  target_y) <= 1){
		*drone_x = target_x;
		*drone_y = target_y;
		return 0;
	}


	// obtain the angle, usign arc tangent.
	double angle = atan2(target_y - *drone_y, target_x - *drone_x);


    *drone_x = (*drone_x) + (cos(angle) * DISTANCE);
    *drone_y = (*drone_y) + (sin(angle) * DISTANCE);


    return 1;
}
