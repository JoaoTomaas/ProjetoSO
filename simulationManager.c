#include "header.h"


int main()
{
	void * shm_addr;
	pid_t pid;
	FILE * fp;
	int x_max, y_max, num_armazens,x, y;
	NomeProduto * nomes_produtos;
	Armazem * armazens, *looper;

	nomes_produtos = NULL;
	armazens = NULL;

	if (create_file("keys.txt"))
		return -1;

	shm_addr = create_shm();
	if (shm_addr == (void *)-1)
		return -1;

	/*Lê config.txt*/
	read_config("config.txt", &x_max, &y_max, &nomes_produtos, &num_drones, &temp_abast,
					&qtd, &unidades_tempo, &num_armazens);

	if (!(fp = fopen("config.txt", 'r')))
		return -1;
	for (i=0; i<num_armazens; i++)
	{
		//	ler x y
		//criar lista ligada de Produto
		pid = fork();
		if (pid == 0)
			exec("armazem.c", "id_armazem (i)", prod, quant, ..., (char*)NULL);
		else
			wait(NULL);
	}
	fclose(fp);

	//Cria Central
	exec("Central", "lista ligada de Armazem");

}



void * create_shm()
{
	key_t shm_key,
	int shm_id;
	void * shmaddr;

	if ((shm_key = ftok("./keys.txt", ID_SHM)) == -1)
		return (void *)-1;

	if ((shm_id = shmget(shm_key, SEGMENT_SIZE, IPC_CREAT|0600)) == -1)
		return (void *)-1;

	if ((shm_addr = shmat(shm_id, NULL, 0)) == (void *) -1)
		return (void *)-1;


	return shm_addr;
}


int create_file(char *nome)
{
	FILE *fp;
	if (!(fp = fopen("keys.txt", 'w')))
	{
		/*printf("Erro ao criar ficheiro 'keys.txt'\n");*/
		return -1;
	}
	if (fclose(fp))
	{
		/*printf("Erro ao fechar ficheiro 'keys.txt'\n");*/
		return -1;
	}
	return 0;
}
