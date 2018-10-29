#include "header.h"






int main (int argc, char* argv[]){

	int i, id, x, y;
	Armazem* list_armz, temp;
	char* end;

	list_armz = NULL;

	for (i = 1; i < argc; i+=3){
		temp = (Armazem*) malloc (sizeof(Armazem));
		id = strtol (argv[i], &end, 10);
		x = strtol (argv[i+1], &end, 10);
		y = strtol (argv[i+2], &end, 10);
		strcpy (temp->id, id);
		strcpy (temp->x, x);
		strcpy (temp->y, y);

		temp -> next = list_armz;
		list_armz = next;
	}


	//Criar pool de threads



}