
/*	ifndef	*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

#define ID_SHM_ARM 1
#define ID_SHM_EST 2
#define ID_SHM_STOCK 3
#define MAX_CHARS 50
#define INT_TO_CHAR_MAX 12
#define PIPE_NAME "namedpipe" /*Última edição*/

typedef struct sNomeProduto {
	char nome[MAX_CHARS];
	struct sNomeProduto * next;
}NomeProduto;

typedef struct sProduto_Qtd {
	char nome[MAX_CHARS];
	int qtd;
	struct sProduto_Qtd *next;
}Produto_Qtd;

typedef struct sArmazem {
	char nome_armazem[MAX_CHARS];
	int x, y, id, num_produtos;
	Produto_Qtd *produtos;
	struct sArmazem *next;
}Armazem;

typedef struct sEstatistica {
	int total_encomendas, total_produtos_carregados;
	int total_encomendas_entregues, total_produtos_entregues;
	double tempo_medio;
}Estatistica;

typedef struct sStock {
	char nome_produto[MAX_CHARS];
	int qtd, id_armazem;
}Stock;



int create_file(char *nome);
int read_config(char *nome_ficheiro, NomeProduto **nomes_produtos,
					Armazem **armazens, int *x, int *y, int *num_drones,
					int *t_abast, int *qtd_abast, int *uni_tempo,
					int *num_armazens);
void *create_shm_armazens(int num_armazens);
void set_shm_arm(Armazem *addr, Armazem *armazens);
void *create_shm_estatistica();
void set_shm_est(Estatistica *p);
void *create_shm_stock(int n);
void set_shm_stock(Stock *addr, Armazem *armazens);
void copia_armazem(Armazem *para, Armazem *de);
int soma_produtos_total(Armazem *p);
void clear_armazens(Armazem **p);
