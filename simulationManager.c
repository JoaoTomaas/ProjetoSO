#include "header.h"


int main()
{
	void *shm_addr;
	pid_t pid;
	FILE *fp;
	int x, y, num_drones, t_abast, qtd_abast, uni_tempo, num_armazens;
	NomeProduto *nomes_produtos;
	Armazem *armazens;

	/*	Variáveis para teste
	NomeProduto *tmp_nome_produto;
	Armazem *tmp_armazem;
	Produto_Qtd *tmp_prod_qtd;
	*/

	nomes_produtos = NULL;
	armazens = NULL;

	if (create_file("keys.txt") < 0)
		return -1;

	shm_addr = create_shm();
	if (shm_addr == (void *)-1)
		return -1;

	/*Lê config.txt*/
	read_config("config.txt", &nomes_produtos, &armazens,
					&x, &y,
					&num_drones,
					&t_abast, &qtd_abast, &uni_tempo,
					&num_armazens);

	return 0;
}





int read_config(char *nome_ficheiro, NomeProduto **nomes_produtos,
					Armazem **armazens, int *x, int *y, int *num_drones,
					int *t_abast, int *qtd_abast, int *uni_tempo,
					int *num_armazens)

{
	char str_buffer[MAX_CHARS], *token;
	NomeProduto *tmp_nome_produto;
	Armazem *tmp_armazem;
	Produto_Qtd *tmp_prod_qtd;
	FILE *fp;
	int i;

	if ( !(fp = fopen(nome_ficheiro, "r")))
		return -1;

	fscanf(fp, "%d, %d\n", x, y);

	fgets(str_buffer, MAX_CHARS, fp);

	token = strtok(str_buffer, ", ");
	while (token)
	{
		tmp_nome_produto = (NomeProduto *)malloc(sizeof(NomeProduto));
		strcpy(tmp_nome_produto->nome, token);
		tmp_nome_produto->next = *nomes_produtos;
		*nomes_produtos = tmp_nome_produto;
		token = strtok(NULL, ", ");
	}
	(*nomes_produtos)->nome[strlen((*nomes_produtos)->nome)-1] = '\0';

	fscanf(fp, "%d\n%d, %d, %d", num_drones, t_abast, qtd_abast, uni_tempo);

	fscanf(fp, "%d\n", num_armazens);

	for (i=0; i<*num_armazens; i++)
	{
		tmp_armazem = (Armazem *)malloc(sizeof(Armazem));
		tmp_armazem->produtos = NULL;
		tmp_armazem->next = *armazens;
		*armazens = tmp_armazem;

		fscanf(fp, "%s ", tmp_armazem->nome_armazem);
		fscanf(fp, "xy: %d, %d prod: ", &tmp_armazem->x, &tmp_armazem->y);

		fgets(str_buffer, MAX_CHARS, fp);
		token = strtok(str_buffer, ", ");
		while (token)
		{
			tmp_prod_qtd = (Produto_Qtd *)malloc(sizeof(Produto_Qtd));
			strcpy(tmp_prod_qtd->nome, token);
			token = strtok(NULL, ", ");
			tmp_prod_qtd->qtd = strtol(token, NULL, 10);
			tmp_prod_qtd->next = tmp_armazem->produtos;
			tmp_armazem->produtos = tmp_prod_qtd;
			token = strtok(NULL, ", ");
		}
	}
	fclose(fp);
}




void *create_shm()
{
	key_t shm_key;
	int shm_id;
	void * shm_addr;

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
	if (!(fp = fopen(nome, "w")))
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
