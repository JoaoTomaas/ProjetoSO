/*#includ (...) */

typedef struct sProduto{
	char* nome;
	int qtd;
	struct sProduto * next;
}Produto;


typedef struct sArmazem{
	int id_armz;
	int x, y;
	struct sArmazem * next;
}Armazem;


/*var stock -> armazena a lista ligada de prods*/
