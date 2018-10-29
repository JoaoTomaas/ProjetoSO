
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define ID_SHM 5
#define MAX_CHARS 50
#define SEGMENT_SIZE 50

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
	int x, y;
	Produto_Qtd *produtos;
	struct sArmazem *next;
}Armazem;



int create_file(char *nome);
int read_config(char *nome_ficheiro, NomeProduto **nomes_produtos,
					Armazem **armazens, int *x, int *y, int *num_drones,
					int *t_abast, int *qtd_abast, int *uni_tempo,
					int *num_armazens);
void *create_shm();
