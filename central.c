#include "header.h"

#define PIPE_NAME "namedpipe" /*Mudar este define para o header*/

//sync resources
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
sem_t threads_limit;


void sigint_handler (int signum){
	char c;

	signal(signum, SIG_IGN); /*Ignorar dentro do handler 
	porque pode ser pressionado enquanto o handler está a 
	processar o sinal anterior*/
	
	printf("\n CTRL+C pressed. Do you want to exit? ");
	c = getchar();

	if (c == 'y' || c == 'Y'){
		printf("Exiting...\n");
		/*Fechar e libertar os recursos necessários*/
		exit(0);
	}
	else
		signal(SIGINT, sigint_handler); /*Reinstanlar o sinal handler
		para qye seja possível apanhar este sinal se for acionado de novo*/
}


void *thread_work (){
	/*printf("Vou testar até me cansar, toda a noiteeeeee\n");*/

	while(1){
		sem_wait (&threads_limit); /*Semaforo usado para saber se podemos aceder a esta zona*/

		pthread_mutex_lock (&mutex);


	}




	return 0;
}



int main (int argc, char* argv[]){

	int n_drones = 4; /*Variavel de teste*/
	pthread_t threadp [n_drones]; /*Esta variavel depois vai ter que ser recebida de outra forma*/
	int th_ids [n_drones];
	int j;

	signal (SIGINT, sigint_handler); /*CTRL+C handler*/
	

	sem_init (&threads_limit, 0, n_drones); /*Mudar o número de drones para uma constante*/

	/*Criar pool de threads
	1. Numero de threads a criar vai ser igual ao numero de drones*/

	for (j = 0; j < n_drones; j++){
		th_ids[j] = j;
		if (pthread_create (&threadp[j], NULL, thread_work, &th_ids[j]) != 0){
			perror("Erro na criacao da thread");
			exit(0);
		}
		printf("Thread %d criada\n", j);
	}


	/*(EM FALTA) Terminar a pool de threads*/


	/*(EM FALTA) Ler e escrever na SHM*/


	//Create named pipe
	if ((mkfifo(PIPE_NAME,O_CREAT|O_EXCL|0600) < 0) && (errno != EEXIST)){
		perror ("Cannot create named pipe\n");
		exit(0);
	}
	else{
		printf("Named pipe created successfuly\n");
	}


	//SIGINT handler
	/*
	signal (SIGABRT, SIG_IGN);
	signal (SIGILL, SIG_IGN);
	signal (SIGSEGV, SIG_IGN);
	signal (SIGTERM, SIG_IGN);*/


}
