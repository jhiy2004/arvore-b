#include "btree.h"

int main(int argc, char* argv[]){
	int opc=0;
	int key=0;
	int pos=0;
	BTreeNode* root = NULL;
	BTree* tree = NULL;
	char* filename = NULL;
	char* searchResult = NULL;

	srand(time(NULL));

	//Se receber algum argumento e for create, criar um árvore padrão para testes
	if(argc != 1){
		if(strcmp(argv[1], "create") == 0){
			t = 2;
			createTreeTest(&root);

			tree = createBTree(t, root->filename);
			strcpy(tree->root, root->filename);
			writeTree(tree);
			printf("Tree name: %s\n", tree->filename);
			return 0;
		}
	}

	opc = initialMenu();
	switch(opc){
		case 1:
			//Criar nova árvore
			do{
				printf("Enter the value of t: ");
				scanf("%d", &t);
			}while(t < 2);

			root = createNode(t, 1);
			tree = createBTree(t, root->filename);

			break;
		case 2:
			//Abrir árvore existente
			getchar();
			filename = chooseTree();
			printf("%s\n", filename);

			readTree(filename, &tree);
			t = tree->t;

			diskRead(tree->root, &root);

			break;
		case 3:
			return 0;
			break;
	}

	while(1){
		opc = mainMenu();
		switch(opc){
			case 1:
				// Imprimir árvore
				printBTree(root, 0);
				break;
			case 2:
				// Inserir na árvore
				printf("Insert a key: ");
				scanf("%d", &key);
				insertCLRS(&root, key);
				break;
			case 3:
				// Remover da árvore
				printf("Delete a key: ");
				scanf("%d", &key);
				deleteCLRS(&root, key);
				break;
			case 4:
				// Buscar na árvore
				printf("Search a key: ");
				scanf("%d", &key);
				searchResult =  search(root, key, &pos);
				if(searchResult != NULL){
					printf("%d Found at file %s at position %d\n", key, searchResult, pos);

					//Liberar a memória da busca
					free(searchResult);
					searchResult = NULL;
				}else{
					printf("%d Not found\n", key);
				}
				break;
			case 5:
				// Sair do programa
				strcpy(tree->root, root->filename);
				writeTree(tree);
				printf("Tree name: %s\n", tree->filename);
				return 0;
				break;
		}
	}

	return 0;
}
