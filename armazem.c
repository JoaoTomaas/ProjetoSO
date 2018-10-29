#include "header.h"




int main (int argc, char* argv[]){
	
	int i, id_armz, qtd;
	NomeProduto* stock, temp;
	char * end;


	id_armz = strtol(argv[1], &end, 10); /*ID unico de cada armazem -> Ver se está a converter bem*/
	stock = NULL; /*Para que a var fique a apontar para NULL*/


	for (i = 2; i < argc; i+=2){
		temp = (NomeProduto*) malloc (sizeof(NomeProduto));
		strcpy (temp->nome, argv[i]);
		qtd = strtol(argv[i+1], &end, 10);
		strcpy (temp->qtd, qtd);

		temp -> next = stock;
		stock = next;
	}




}