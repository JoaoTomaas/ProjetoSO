#include "header.h"

/**************************
*
*	verificar produtos no read_config
*
*	Implementar os SIGNAL HANDLERS
*	Implementar o PIPE
*	LOG -- alterar para mandar tsmbem para o ecrã
*	Estatísticas
*
*	Testar se o drone se movimenta corretamente; funçao start_drone
*
*********	O sinal só pode ser recebido depois de ter as shm e armazens e central criados	*******
*
*
*		TERMINAÇÃO CONTROLADA NÃO FUNCIONA
*
*
**********************/

pthread_mutex_t mutex;
pthread_cond_t cond;
pthread_t tid;

int main()
{
	Config config;
	pid_t pid, *pid_armazens, pid_central;
	Armazem *shm_addr_arm;
	Estatistica *shm_addr_est;
	Stock *shm_addr_stock;
	NomeProduto *shm_addr_nomes_produtos;
	char buffer[MAX_CHARS], log_buffer[MAX_LOG];
	int i;
	sigset_t set;
	Ponteiros ponteiros;

	/*	Variáveis para teste
	char w2[SEGMENT_SIZE];
	NomeProduto *tmp_nome_produto;
	Produto_Qtd *tmp_prod_qtd;
	*/

	/*	Initialize 'mutex' and 'cond'	*/
	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&cond, NULL);

	/*Signal Handling*/
	sigfillset (&set);
	sigdelset (&set, SIGINT);
	sigdelset (&set, SIGUSR1);
	sigprocmask (SIG_SETMASK, &set, NULL);


	/*Cria ficheiro keys.txt e log.txt	*/
	if (create_file(KEYS_FILE))
		return -1;
	if (create_file(LOG_FILE))
		return -1;

	/*	escreve para o log e ecrã o início do programa	*/
	log_ecra("Inicio do programa");

	/*Lê config.txt*/
	if (read_config("config.txt", &config) == -1)
		return -1;

	/*		Alocação da SHM para os armazens*/
	shm_addr_arm = (Armazem *)create_shm_armazens(config.num_armazens);
	if (shm_addr_arm == (Armazem *)((void *)(-1)))
		return -1;

	/*	Alocação da SHM para as estatísticas, estrutura Estatistica	*/
	shm_addr_est = (Estatistica *)create_shm_estatistica();
	if (shm_addr_est == (Estatistica *)((void *)(-1)))
		return -1;

	/*	Alocação da shm para o stock de todos os armazens*/
	shm_addr_stock = (Stock *)create_shm_stock(soma_produtos_total(config.armazens));
	if (shm_addr_stock == (Stock *)((void *)(-1)))
		return -1;

	/*	Alocação da shm para os nomes dos produtos	*/
	shm_addr_nomes_produtos = (NomeProduto *)create_shm_nomes_produtos(config.qtd_produtos);
	if (shm_addr_nomes_produtos == (NomeProduto *)((void *)(-1)))
		return -1;

	/*	Inicializar a shm das estatísticas	*/
	set_shm_est(shm_addr_est);

	/*	Inicializar a shm dos armazéns	*/
	set_shm_arm(shm_addr_arm, config.armazens);

	/*	Inicializar shm dos stocks */
	set_shm_stock(shm_addr_stock, config.armazens);

	/*	Inicializar shm dos nomes dos produtos	*/
	set_shm_nomes_produtos(shm_addr_nomes_produtos, &config);


	/*	"Apagar" variável "armazens"	*/
	/*	Não funciona, ainda	*/
	/*********************clear_armazens(&config);*/
	/**/


	/*	Criar processos armazens	*/
	pid_armazens = (pid_t *)malloc(config.num_armazens * sizeof(pid_t));
	for (i=0; i<config.num_armazens; i++)
	{
		pid = fork();
		if (pid == 0)
		{
			sprintf(buffer, "%d", i);
			execl("armazem.out", "armazem.out", buffer, (char *)NULL);
		}
		if (pid < 0)
			return -1;

		pid_armazens[i] = pid;
		sprintf(log_buffer, "Criacao do armazem %d", pid);
		log_ecra(log_buffer);
	}

	/*	Cria processo Central
	pid = fork();
	if (pid == 0)
	{
		sprintf(buffer, "%d %d %d %d", config.num_drones, config.uni_tempo, config.x, config.y);
		execl("central.out", "central.out", buffer, (char *)NULL);
	}
	else if (pid < 0)
		return -1;
	else{
		pid_central = pid;
	}*/


	/*	Setup para a thread responsável pelo cleanup após SIGINT	*/
	setup_thread(&ponteiros, &config, pid_armazens, pid_central);

	if (pthread_create(&tid, NULL, terminacao_controlada, (void *)&ponteiros))
		return -1;

		/**/

	/* SIGNAL HANDLING (start)*/
	signal (SIGINT, sigint_handler_sim); /*CTRL+C handler*/
	/* SIGNAL HANDLING (finish)*/

	while(1);

return 0;
}

void log_ecra(char *s)
{
	FILE *fp;
	time_t seconds;
	struct tm curr_time;

	fp = fopen(LOG_FILE, "a");

	seconds = time(NULL);
	curr_time = *localtime(&seconds);

	fprintf(fp, "%d:%d:%d %s\n", curr_time.tm_hour, curr_time.tm_min, curr_time.tm_sec, s);
	printf("%d:%d:%d %s\n", curr_time.tm_hour, curr_time.tm_min, curr_time.tm_sec, s);

	fclose(fp);
}

void setup_thread(Ponteiros *ponteiros, Config *config,
						pid_t *pids, pid_t central)
{
	ponteiros->config = config;
	ponteiros->pids = pids;
	ponteiros->central = central;
}

void *terminacao_controlada(void *arg)
{
	int i, id;
	Ponteiros *ponteiros;
	key_t key;

	ponteiros = (Ponteiros *)arg;

	/*	Não há ninguém a competir por este mutex	*/
	pthread_mutex_lock(&mutex);

	while(1)
		pthread_cond_wait(&cond, &mutex);

	/*	Cleanup	*/
	/*	Ainda tenho o mutex	*/

	//	Mandar sinal para terminar central
	////kill(ponteiros->central, SIGINT);
	//	Mandar sinal para terminar processos armazem
	for (i=0; i<ponteiros->config->num_armazens; ++i)
		kill(*(ponteiros->pids + i), SIGINT);


	/*	Cleanup simulation manager	*/

	/*************	PIPE??	********/
	/*close (fd);
	unlink (PIPE_NAME);*/

	/*	Desnecessário por causa de _exit()	*/
	//shmdt (ponteiros->arm);
	//shmdt (ponteiros->stock);
	//shmdt (ponteiros->est);
	//shmdt(ponteiros->nome_produto);
	/**/

	/*	Marca as shm para destruição	*/
	for (i=ID_SHM_ARM; i<=ID_SHM_NOMES_PRODUTOS; ++i)
	{
		key = ftok(KEYS_FILE, i);
		id = shmget(key, 0, 0);
		shmctl(id, IPC_RMID, NULL);
	}

	//clear_config(ponteiros->config);

	log_ecra("Fim do prgrama");
	_exit(0);	/*	_exit() fecha as shared memory attached	*/
}

void sigint_handler_sim(int signum)
{
	char c;

	signal(signum, SIG_IGN); /*Ignorar dentro do handler porque pode ser acionado enquanto o handler está a processar o sinal anterior*/

	printf("\n CTRL+C pressed. Do you want to exit? ");
	c = getchar();

	if (c == 'y' || c == 'Y'){
		/*Fechar e libertar os recursos necessários*/
		/*Liga a variável de condição*/
		pthread_mutex_lock(&mutex);
		pthread_cond_signal(&cond);
		pthread_mutex_unlock(&mutex);
		pthread_join(tid, NULL);
		exit(0);
	}
	else{
		signal(SIGINT, sigint_handler_sim); /*Reinstanlar o sinal handler para que seja possível apanhar este sinal se for acionado de novo*/
	}
}

void set_shm_nomes_produtos(NomeProduto *shm, Config *config)
{
	int i;
	NomeProduto *tmp;

	tmp = config->nomes_produtos;
	for (i=0; i<config->qtd_produtos; ++i)
	{
		strcpy(shm[i].nome, tmp->nome);
		shm[i].next = NULL;
		tmp = tmp->next;
	}

	/*
	strcpy(shm[i].nome, "EOF");
	shm[i].next = NULL;
	*/
}

void *create_shm_nomes_produtos(int n)
{
	key_t key;
	int id;

	if ((key = ftok(KEYS_FILE, ID_SHM_NOMES_PRODUTOS)) == -1)
		return (void *)(-1);

	if ((id = shmget(key, (n+1)*sizeof(NomeProduto), IPC_CREAT|0600)) == -1)
		return (void *)(-1);

	return shmat(id, NULL, 0);

}


void clear_config(Config *c)
{
	Produto_Qtd *pq;
	Armazem *arm;
	NomeProduto *np;

	while(c->armazens)
	{
		while(c->armazens->produtos)
		{
			pq = c->armazens->produtos;
			c->armazens->produtos = c->armazens->produtos->next;
			free(pq);
		}
		arm = c->armazens;
		c->armazens = c->armazens->next;
		free(arm);
	}

	while(c->nomes_produtos)
	{
		np = c->nomes_produtos;
		c->nomes_produtos = c->nomes_produtos->next;
		free(np);
	}

}


/*Funçao privada	*/
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

	if ((key = ftok(KEYS_FILE, ID_SHM_STOCK)) == -1)
		return (void *)(-1);

	if ((id = shmget(key, (n+1)*sizeof(Stock), IPC_CREAT|0600)) == -1)
		return (void *)(-1);

	return shmat(id, NULL, 0);
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
	addr[i].id_armazem = -1;
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
	shm_addr[i].id = -1;
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

	key = ftok(KEYS_FILE, ID_SHM_EST);
	if(key == -1)
		return (void *)(-1);

	shm_id = shmget(key, sizeof(Estatistica), IPC_CREAT|0600);
	if (shm_id == -1)
		return (void *)(-1);


	return shmat(shm_id, NULL, 0);
}


int read_config(char *nome_ficheiro, Config *config)

{
	char str_buffer[MAX_CHARS], *token;
	NomeProduto *tmp_nome_produto;
	Armazem *tmp_armazem;
	Produto_Qtd *tmp_prod_qtd;
	FILE *fp;
	int i, conta;

	config->nomes_produtos = NULL;
	config->armazens = NULL;

	if ( !(fp = fopen(nome_ficheiro, "r")))
		return -1;

	/*	Ler x y	*/
	fscanf(fp, "%d, %d\n", &config->x, &config->y);

	/*	Criar lista ligada com nomes dos produtos*/
	fgets(str_buffer, MAX_CHARS, fp);
	conta = 0;
	token = strtok(str_buffer, ", ");
	while (token)
	{
		++conta;
		tmp_nome_produto = (NomeProduto *)malloc(sizeof(NomeProduto));
		strcpy(tmp_nome_produto->nome, token);
		tmp_nome_produto->next = config->nomes_produtos;
		config->nomes_produtos = tmp_nome_produto;
		token = strtok(NULL, ", ");
	}
	config->nomes_produtos->nome[strlen(config->nomes_produtos->nome)-1] = '\0';
	config->qtd_produtos = conta;
	/*	Ler mais dados	*/
	fscanf(fp, "%d\n%d, %d, %d", &config->num_drones, &config->t_abast,
											&config->qtd_abast, &config->uni_tempo);

	/*	Ler numero de armazéns*/
	fscanf(fp, "%d\n", &config->num_armazens);

	for (i=0; i<config->num_armazens; i++)
	{
		tmp_armazem = (Armazem *)malloc(sizeof(Armazem));
		tmp_armazem->id = i;
		tmp_armazem->num_produtos = 0;
		tmp_armazem->produtos = NULL;
		tmp_armazem->next = config->armazens;
		config->armazens = tmp_armazem;

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
			tmp_prod_qtd->qtd = (int) strtol(token, NULL, 10);
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

	if ((shm_key = ftok(KEYS_FILE, ID_SHM_ARM)) == -1)
		return (void *)(-1);

	if ((shm_id = shmget(shm_key, (num_armazens+1)*sizeof(Armazem), IPC_CREAT|0600)) == -1)
		return (void *)(-1);

	return shmat(shm_id, NULL, 0);
}


int create_file(char *nome)
{
	FILE *fp;

	if (!(fp = fopen(nome, "w")))
		return -1;

	if (fclose(fp) == EOF)
		return -1;

	return 0;
}
