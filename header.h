
#ifndef MAIN_HEADER
#define MAIN_HEADER


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <signal.h>
#include <semaphore.h>
#include <pthread.h>
#include <time.h>
#include <sys/stat.h>

#include <math.h>

#define ID_SHM_ARM 1
#define ID_SHM_EST 2
#define ID_SHM_STOCK 3
#define ID_SHM_NOMES_PRODUTOS 4
#define MAX_CHARS 50
#define MAX_LOG 150
#define INT_TO_CHAR_MAX 12
#define DESOCUPADO 0
#define OCUPADO 1
#define KEYS_FILE ".keys.txt"
#define LOG_FILE "log.txt"
#define PIPE_NAME "named_pipe"

#define DISTANCE 1

typedef struct sNomeProduto {
	char nome[MAX_CHARS];
	struct sNomeProduto * next;
} NomeProduto;

typedef struct sProduto_Qtd {
	char nome[MAX_CHARS];
	int qtd;
	struct sProduto_Qtd *next;
} Produto_Qtd;

typedef struct sArmazem {
	char nome_armazem[MAX_CHARS];
	int x, y, id, num_produtos;
	Produto_Qtd *produtos;
	struct sArmazem *next;
} Armazem;

typedef struct sEstatistica {
	int total_encomendas, total_produtos_carregados;
	int total_encomendas_entregues, total_produtos_entregues;
	double tempo_medio;
} Estatistica;

typedef struct sStock {
	char nome_produto[MAX_CHARS];
	int qtd, id_armazem;
} Stock;

typedef struct sEncomenda {
	int id;
	char *nome_produto;
	int qtd, x_final, y_final, x_arm, y_arm;
} Encomenda;

typedef struct Drone {
	double x, y;
	int id, estado;
	Encomenda *encomenda;
	pthread_mutex_t *mutex;
	pthread_cond_t *cond;
} Drone;

typedef struct sConfig {
	int x, y, num_drones, t_abast, qtd_abast, uni_tempo, num_armazens, qtd_produtos;
	NomeProduto *nomes_produtos;
	Armazem *armazens;
} Config;

typedef struct sPonteiros{
	Config *config;
	pid_t *pids, central;
} Ponteiros;

typedef struct sCleanupCentral {
	pthread_mutex_t *mutexes;
	pthread_cond_t *conds;
	Drone *drones_info;
	pthread_t *drones_id;
} CleanupCentral;



int create_file(char *nome);
int read_config(char *nome_ficheiro, Config *config);
void *create_shm_armazens(int num_armazens);
void set_shm_arm(Armazem *addr, Armazem *armazens);
void *create_shm_estatistica();
void set_shm_est(Estatistica *p);
void *create_shm_stock(int n);
void set_shm_stock(Stock *addr, Armazem *armazens);
void copia_armazem(Armazem *para, Armazem *de);
int soma_produtos_total(Armazem *p);
void clear_config(Config *c);
void set_shm_nomes_produtos(NomeProduto *shm, Config *config);
void *create_shm_nomes_produtos(int n);
void sigint_handler (int signum);
void sigint_handler_sim(int signum);
void *terminacao_controlada(void *arg);
void setup_thread(Ponteiros *ponteiros, Config *config,
						pid_t *pids, pid_t central);
void log_ecra(char *s);

void *drone_start(void *drone_info);
int drones_id_init(pthread_t *ids, Drone *info, int n);
void drones_info_init(Drone *a, pthread_mutex_t *mutexes,
							pthread_cond_t *conds, int n);
int cond_init(pthread_cond_t *a, int n);
int mutex_init(pthread_mutex_t *a, int n);
void *attach_shm(int type);
void ler_inteiros(char *str, int *d, int *t, int *x, int *y);
void sigint_handler_central(int signum);
void setup_thread_central(CleanupCentral *s, pthread_mutex_t *m,
									pthread_cond_t *c, Drone *d, pthread_t *i);
void *cleanup_central(void *arg);

void sigint_handler_armazem(int signum);



double distance(double x1, double y1, double x2, double y2);
int move_towards(double *drone_x, double *drone_y, double target_x, double target_y);

#endif
