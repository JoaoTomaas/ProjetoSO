#include "header.h"


void terminacao_controlada (){
	int i, n_drones = 4; //Variavel de teste

	for (i = 0; i < n_drones; i++){
		pthread_join (threadp[i], NULL);
		printf("Thread %d joined\n", i);
	}

	//Terminar central

	//Terminar processos armazem

	//Terminar simulation manager


	close (fd);
	unlink (PIPE_NAME);

	shmdt (shm_addr_arm);
	shmctl (shm_arm_id, IPC_RMID, NULL);

	shmdt (shm_addr_stock);
	shmctl (shm_stock_id, IPC_RMID, NULL);

	shmdt (shm_addr_est);
	shmctl (shm_est_id, IPC_RMID, NULL);

	pthread_mutex_destroy (&mutex); //Para destroir o mutex
	
	printf("Exiting...\n");
}


void sigint_handler (int signum){
	char c;

	signal(signum, SIG_IGN); /*Ignorar dentro do handler porque pode ser acionado enquanto o handler está a processar o sinal anterior*/
	
	printf("\n CTRL+C pressed. Do you want to exit? ");
	c = getchar();

	if (c == 'y' || c == 'Y'){
		/*Fechar e libertar os recursos necessários*/
		terminacao_controlada();
		exit(0);
	}
	else{
		signal(SIGINT, sigint_handler); /*Reinstanlar o sinal handler para que seja possível apanhar este sinal se for acionado de novo*/
	}
}


int main()
{
	pid_t pid;
	int x, y, num_drones, t_abast, qtd_abast, uni_tempo, num_armazens;
	NomeProduto *nomes_produtos;
	Armazem *armazens, *shm_addr_arm;
	Estatistica *shm_addr_est;
	Stock *shm_addr_stock;
	char id_str[INT_TO_CHAR_MAX];

	int i;
	/*	Variáveis para teste
	char w2[SEGMENT_SIZE];
	NomeProduto *tmp_nome_produto;
	Produto_Qtd *tmp_prod_qtd;
	*/

	/* SIGNAL HANDLING (start)*/
	sigset_t set;
	sigfillset (&set);
	sigdelset (&set, SIGINT);
	sigdelset (&set, SIGUSR1);
	sigprocmask (SIG_SETMASK, &set, NULL);

	signal (SIGINT, sigint_handler); /*CTRL+C handler*/
	/* SIGNAL HANDLING (finish)*/

	nomes_produtos = NULL;
	armazens = NULL;

	/*Cria ficheiro keys.txt*/
	if (create_file("keys.txt") < 0)
		return -1;

	/*Lê config.txt*/
	read_config("config.txt", &nomes_produtos, &armazens,
					&x, &y,
					&num_drones,
					&t_abast, &qtd_abast, &uni_tempo,
					&num_armazens);

	/*		Alocação da SHM para os armazens*/
	shm_addr_arm = (Armazem *)create_shm_armazens(num_armazens);
	if (shm_addr_arm == (Armazem *)((void *)(-1)))
		return -1;

	/*	Alocação da SHM para as estatísticas, estrutura Estatistica	*/
	shm_addr_est = (Estatistica *)create_shm_estatistica();
	if (shm_addr_est == (Estatistica *)((void *)(-1)))
		return -1;

	/*	Alocação da shm para o stock de todos os armazens*/
	shm_addr_stock = (Stock *)create_shm_stock(soma_produtos_total(armazens));
	if (shm_addr_stock == (Stock *)((void *)(-1)))
		return -1;

	/*	Inicializar a shm das estatísticas	*/
	set_shm_est(shm_addr_est);

	/*	Inicializar a shm dos armazéns	*/
	set_shm_arm(shm_addr_arm, armazens);

	/*	Inicializar shm dos stocks */
	set_shm_stock(shm_addr_stock, armazens);


	/*	"Apagar" variável "armazens"	*/
	clear_armazens(&armazens);


	/*	Criar processos armazens	*/
	for (i=0; i < num_armazens; i++)
	{
		pid = fork();
		if (pid == 0)
		{
			sprintf(id_str, "%d", i);
			execl("armazem.out", "armazem.out", id_str,(char *)NULL);
		}
	}

	/*	Cria processo Central	*/
	pid = fork();
	if (pid == 0)
		execl("central.out", "central.out", (char *)NULL);

	return 0;
}

void clear_armazens(Armazem **p)
{
	Produto_Qtd *pq, *tmp;
	Armazem *arm;
	while(*p)
	{
		tmp = (*p)->produtos;
		while(tmp)
		{
			pq = tmp;
			tmp = tmp->next;
			free(pq);
		}
		arm = *p;
		(*p) = (*p)->next;
		free(arm);
	}
	*p = NULL;
}

int soma_produtos_total(Armazem *p)
/*	Vai contar quantos nomes de produtos há em cada armazém e somar	*/
{
	int c;
	c = 0;
	while(p)
	{
		c += p->num_produtos;
		p = p->next;
	}

	return c;
}

void *create_shm_stock(int n)
{
	key_t key;
	int id;
	void *addr;

	if ((key = ftok("keys.txt", ID_SHM_STOCK)) == -1)
		return (void *)(-1);

	if ((id = shmget(key, n*sizeof(Stock), IPC_CREAT|0600)) == -1)
		return (void *)(-1);

	if ((addr = shmat(id, NULL, 0)) == (void *)(-1))
		return (void *)(-1);

	return addr;
}

void set_shm_stock(Stock *addr, Armazem *armazens)
{
	Produto_Qtd *tmp;
	int i, id_atual;

	i = 0;

	while (armazens)
	{
		id_atual = armazens->id;
		tmp = armazens->produtos;
		while (tmp)
		{
			addr[i].id_armazem = id_atual;
			strcpy(addr[i].nome_produto, tmp->nome);
			addr[i].qtd = tmp->qtd;

			++i;
			tmp = tmp->next;
		}
		armazens = armazens->next;
	}

/*
	while(armazens)
	{
		addr[i].id_armazem = armazens->id;
		tmp = armazens->produtos;

		while(tmp)
		{
			strcpy(addr[i].nome_produto, tmp->nome);
			addr[i].qtd = tmp->qtd;
			tmp = tmp->next;
			++i;
		}
		armazens = armazens->next;
	}
	*/
}

void set_shm_arm(Armazem *shm_addr, Armazem *armazens)
{
	int i;
	i = 0;

	while(armazens)
	{
		copia_armazem(shm_addr+i, armazens);
		++i;
		armazens = armazens->next;
	}
}

void copia_armazem(Armazem *para, Armazem *de)
{
	strcpy(para->nome_armazem, de->nome_armazem);
	para->x = de->x;	para->y = de->y;
	para->num_produtos = de->num_produtos;
	para->produtos = NULL;
	para->next = NULL;
}


void set_shm_est(Estatistica *p)
{
	p->total_encomendas = 0;
	p->total_produtos_carregados = 0;
	p->total_encomendas_entregues = 0;
	p->total_produtos_entregues =0;
	p->tempo_medio = 0.0;
}

void *create_shm_estatistica()
{
	key_t key;
	int shm_id;
	void *shm_addr;

	key = ftok("./keys.txt", ID_SHM_EST);
	if(key == -1)
		return (void *)(-1);

	shm_id = shmget(key, sizeof(Estatistica), IPC_CREAT|0600);
	if (shm_id == -1)
		return (void *)(-1);

	shm_addr = shmat(shm_id, NULL, 0);
	if (shm_addr == (void *)(-1))
		return (void *)(-1);

	return shm_addr;
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

	/*	Ler x y	*/
	fscanf(fp, "%d, %d\n", x, y);

	/*	Criar lista ligada com nomes dos produtos*/
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

	/*	Ler mais dados	*/
	fscanf(fp, "%d\n%d, %d, %d", num_drones, t_abast, qtd_abast, uni_tempo);

	/*	Ler numero de armazéns*/
	fscanf(fp, "%d\n", num_armazens);

	for (i=0; i<*num_armazens; i++)
	{
		tmp_armazem = (Armazem *)malloc(sizeof(Armazem));
		tmp_armazem->id = i;
		tmp_armazem->num_produtos = 0;
		tmp_armazem->produtos = NULL;
		tmp_armazem->next = *armazens;
		*armazens = tmp_armazem;

		fscanf(fp, "%s ", tmp_armazem->nome_armazem);
		fscanf(fp, "xy: %d, %d prod: ", &tmp_armazem->x, &tmp_armazem->y);

		fgets(str_buffer, MAX_CHARS, fp);
		token = strtok(str_buffer, ", ");
		while (token)
		{
			++tmp_armazem->num_produtos;
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
	return 0;
}




void *create_shm_armazens(int num_armazens)
{
	key_t shm_key;
	int shm_id;
	void *shm_addr;

	if ((shm_key = ftok("./keys.txt", ID_SHM_ARM)) == -1)
		return (void *)(-1);

	if ((shm_id = shmget(shm_key, num_armazens*sizeof(Armazem), IPC_CREAT|0600)) == -1)
		return (void *)(-1);

	if ((shm_addr = shmat(shm_id, NULL, 0)) == (void *)(-1))
		return (void *)(-1);

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
