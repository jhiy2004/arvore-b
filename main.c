#include <string.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define FILENAME_SIZE 25
#define NODEPATH_LEN 8
#define TREEPATH_LEN 13

int t = 0;
char *nodesPath = "./files/";
char *treesPath = "./files/trees/";
char *extension = ".bin";

typedef struct BTreeNode{
	char *filename;
	int n;
	int *keys;
	char **children;
	int leaf;
} BTreeNode;

typedef struct BTree{
	char *filename;
	char *root;
	int t;
} BTree;

char* generateRandomFilename(char* path){
	int pathLen=0;

	while(path[pathLen] != '\0'){
		pathLen++;
	}

	char *result = (char*) malloc(sizeof(char)*(FILENAME_SIZE+pathLen+1));
	char filename[FILENAME_SIZE];

	strcpy(result, path);
	
	DIR *d;
	struct dirent *dir;
	d = opendir(path);

	while(1){
		for(int i=0; i < 20; i++){
			int rd_num, rd_category;
			rd_category = rand() % 3;
			switch(rd_category){
				case 0:
					//Escolhe um número aleatório
					rd_num = rand() % (48 - 57 + 1) + 48;
					break;
				case 1:
					//Escolhe uma letra maiúscula aleatória
					rd_num = rand() % (65 - 90 + 1) + 65;
					break;
				case 2:
					//Escolhe uma letra maiúscula aleatória
					rd_num = rand() % (97 - 122 + 1) + 97;
					break;
			}
			filename[i] = rd_num;
		}
		filename[20] = '\0';
		strcat(filename, extension);

		if (d) {
			while ((dir = readdir(d)) != NULL) {
				if(strcmp(filename, dir->d_name) == 0){
					sleep(1);
					continue;
				}
			}
			closedir(d);
			break;
		}
	}

	strcat(result, filename);
	result[FILENAME_SIZE+pathLen-1] = '\0';

	return result;
}

void freeNode(BTreeNode* root){
	if(root == NULL){
		return;
	}

	free(root->keys);
	free(root->filename);
	for(int i=0; i < 2*t; i++){
		free(root->children[i]);
	}
	free(root->children);
	free(root);
}

BTreeNode* createNode(int T, int leaf){
	BTreeNode* node = (BTreeNode*) malloc(sizeof(BTreeNode));
	node->n = 0;
	node->keys = (int*)malloc((2*T - 1) * sizeof(int));
	node->children = (char**) malloc((2*T) * sizeof(char*));
	node->leaf = leaf;
	node->filename = generateRandomFilename(nodesPath);

	for(int i=0; i < 2*T; i++){
		node->children[i] = (char*)malloc((FILENAME_SIZE+NODEPATH_LEN+1)*sizeof(char));
		strcpy(node->children[i], "");
	}
	return node;
}

BTree* createBTree(int T, char* rootFilename){
	BTree* tree = (BTree*) malloc(sizeof(BTree));

	tree->filename = generateRandomFilename(treesPath);
	tree->root = (char*) malloc(sizeof(char)*(FILENAME_SIZE+NODEPATH_LEN+1));
	tree->t = T;

	strcpy(tree->root, rootFilename);

	return tree;
}

int diskWrite(BTreeNode* node){
	if(node == NULL){
		return 0;
	}

	FILE* file = fopen(node->filename, "wb+");
	if(file == NULL){
		printf("ERRO\n");
		return 0;
	}

	//Grava o valor de n no arquivo
	fwrite(&node->n, sizeof(int), 1, file);
	
	//Grava o nome do arquivo no arquivo
	for(int i=0; i < FILENAME_SIZE+NODEPATH_LEN+1; i++){
		fwrite(&node->filename[i], sizeof(char), 1, file);
	}

	//Grava os valores das chaves no arquivo
	for(int i=0; i < node->n;i++){
		fwrite(&node->keys[i], sizeof(int), 1, file);
	}

	//Grava os nomes dos arquivos filhos no arquivo
	for(int i=0; i < 2*t; i++){
		for(int j=0; j < FILENAME_SIZE+NODEPATH_LEN+1; j++){
			fwrite(&node->children[i][j], sizeof(char), 1, file);
		}
	}

	//Grava se o nó é folha ou não no arquivo
	fwrite(&node->leaf, sizeof(int), 1, file);

	fclose(file);

	return 1;
}

int writeTree(BTree* tree){
	if(tree == NULL){
		return 0;
	}

	FILE* file = fopen(tree->filename, "wb+");
	if(file == NULL){
		printf("ERRO\n");
		return 0;
	}

	//Grava o nome do arquivo da árvore
	for(int i=0; i < FILENAME_SIZE+TREEPATH_LEN+1; i++){
		fwrite(&tree->filename[i], sizeof(char), 1, file);
	}

	//Grava o nome do arquivo raiz
	for(int i=0; i < FILENAME_SIZE+NODEPATH_LEN+1; i++){
		fwrite(&tree->root[i], sizeof(char), 1, file);
	}

	//Grava o t da árvore
	fwrite(&tree->t, sizeof(int), 1, file);

	fclose(file);

	return 1;
}

void diskRead(char *name, BTreeNode **dest){
	FILE* file = fopen(name, "rb");

	if(file == NULL){
		printf("[ERRO]\n");
		return;
	}

	*dest = createNode(t, 0);

	//Lê o valor de n no nó 
	fread(&(*dest)->n, sizeof(int), 1, file);
	
	//Lê o nome do arquivo no nó
	for(int i=0; i < FILENAME_SIZE+NODEPATH_LEN+1; i++){
		fread(&(*dest)->filename[i], sizeof(char), 1, file);
	}

	//Lê os valores das chaves no nó
	for(int i=0; i < (*dest)->n;i++){
		fread(&(*dest)->keys[i], sizeof(int), 1, file);
	}

	//Lê os nomes dos arquivos filhos no nó
	for(int i=0; i < 2*t; i++){
		for(int j=0; j < FILENAME_SIZE+NODEPATH_LEN+1; j++){
			fread(&(*dest)->children[i][j], sizeof(char), 1, file);
		}
	}

	//Lê se o nó é folha ou não no nó
	fread(&(*dest)->leaf, sizeof(int), 1, file);

	fclose(file);
}

void readTree(char *name, BTree **dest){
	FILE* file = fopen(name, "rb");

	if(file == NULL){
		printf("[ERRO]\n");
		return;
	}

	*dest = createBTree(t, "");

	//Lê o nome do arquivo na árvore
	for(int i=0; i < FILENAME_SIZE+TREEPATH_LEN+1; i++){
		fread(&(*dest)->filename[i], sizeof(char), 1, file);
	}

	//Lê o nome do arquivo raiz da árvore
	for(int i=0; i < FILENAME_SIZE+NODEPATH_LEN+1; i++){
		fread(&(*dest)->root[i], sizeof(char), 1, file);
	}

	//Lê o valor de t da árvore
	fread(&(*dest)->t, sizeof(int), 1, file);

	fclose(file);
}

char* search(BTreeNode* root, int elem, int *pos){
	int i = 0;
	BTreeNode *child = NULL;
	while(i < root->n && elem > root->keys[i]){
		i++;
	}
	if(i <= root->n && elem == root->keys[i]){
		char* result = (char*)malloc(sizeof(char)*(FILENAME_SIZE+NODEPATH_LEN+1));
		strcpy(result, root->filename);
		*pos = i;
		return result;
	}else if(root->leaf == 1){
		return NULL;
	}else{
		diskRead(root->children[i], &child);	
	}
	return search(child, elem, pos);
}

void splitChild(BTreeNode* x, int i){
	BTreeNode* y = NULL;
	BTreeNode* z = NULL;

	diskRead(x->children[i], &y);

	z = createNode(t, y->leaf);

	z->n = t-1;

	for(int j=0; j < t-1; j++){
		z->keys[j] = y->keys[j+t];
	}

	if (y->leaf == 0){
		for(int j=0; j < t; j++){
			strcpy(z->children[j], y->children[j+t]);
		}
	}
	y->n = t-1;
	for(int j=(x->n); j > i; j--){
		strcpy(x->children[j+1], x->children[j]);
	}
	strcpy(x->children[i+1], z->filename);
	for(int j=(x->n)-1; j > (i-1); j--){
		x->keys[j+1] = x->keys[j];
	}
	x->keys[i] = y->keys[t-1];
	x->n = x->n+1;

	diskWrite(y);
	diskWrite(z);
	diskWrite(x);

	freeNode(y);
	freeNode(z);
}

void insertNotFull(BTreeNode* root, int elem){
	int i = root->n-1;
	if(root->leaf == 1){
		while(i >= 0 && elem < root->keys[i]){
			root->keys[i+1] = root->keys[i];
			i--;
		}
		root->keys[i+1] = elem;
		(root->n)++;
		diskWrite(root);
	}else{
		while(i >= 0 && elem < root->keys[i]){
			i--;
		}
		i++;
		BTreeNode* child = NULL;

		diskRead(root->children[i], &child); 
		if(child->n == 2*t-1){
			splitChild(root, i);
			if(elem > root->keys[i]){
				i++;
			}
		}

		freeNode(child);
		diskRead(root->children[i], &child); 

		insertNotFull(child, elem);
		freeNode(child);
	}
}

void insertCLRS(BTreeNode** root, int elem){
	if ((*root)->n == 2*t - 1){
		BTreeNode* s = createNode(t, 0);
		strcpy(s->children[0],(*root)->filename);
		splitChild(s, 0);
		*root = s;
		insertNotFull(*root, elem);
	}else{
		insertNotFull(*root, elem);
	}
}

int deletePredecessor(BTreeNode* root){
	BTreeNode* child = NULL;
	int num=0;

	if(root->leaf == 1){
		//Armazena em num o maior valor do vetor keys
		num = root->keys[(root->n)-1];
		(root->n)--;
		diskWrite(root);

		return num;
	}else{
		//Lê o nó mais a direita
		diskRead(root->children[root->n], &child);
		num = deletePredecessor(child);

		freeNode(child);
		return num;
	}
}

int deleteSucessor(BTreeNode* root){
	BTreeNode* child = NULL;
	int num=0;

	if(root->leaf == 1){
		//Armazena em num o menor valor do vetor keys
		num = root->keys[0];
		
		// Organiza o vetor
		for(int i=0; i < (root->n)-1; i++){
			root->keys[i] = root->keys[i+1];
		}

		(root->n)--;
		diskWrite(root);

		return num;
	}else{
		//Lê o nó mais a esquerda
		diskRead(root->children[0], &child);
		num = deleteSucessor(child);

		freeNode(child);
		return num;
	}
}

void delete(BTreeNode* root, int elem){
	int i=0, num=0, flag=0;
	BTreeNode* leftChild = NULL;
	BTreeNode* rightChild = NULL;
	BTreeNode* leftSib = NULL;
	BTreeNode* rightSib = NULL;

	while(i < root->n && root->keys[i] < elem){
		i++;
	}

	if(root->keys[i] == elem){
		if(root->leaf == 1){
			if(root->n-1 >= t-1){
				//Case 1
				for(int j=i; j < (root->n)-1;j++){
					root->keys[j] = root->keys[j+1];
				}
				(root->n)--;
			}
			diskWrite(root);
		}else{
			diskRead(root->children[i], &leftChild);
			if(leftChild->n >= t){
				printf("Case 2a: \n");

				num = deletePredecessor(leftChild);
				root->keys[i] = num;
				diskWrite(root);
			}else{
				diskRead(root->children[i+1], &rightChild);
				if(rightChild->n >= t){
					printf("Case 2b: \n");

					num = deleteSucessor(rightChild);
					root->keys[i] = num;
					diskWrite(root);
				}else{
					printf("Case 2c: \n");

					//Concatenar vetor de chaves do ńo filho da esquerda com o da direita
					for(int j=0; j < rightChild->n; j++){
						leftChild->keys[(leftChild->n)+j] = rightChild->keys[j];
					}
					leftChild->n += rightChild->n;

					//Reorganizar o vetor de chaves do nó
					for(int j=i; j < (root->n)-1; j++){
						root->keys[j] = root->keys[j+1];
					}

					//Reorganizar os filhos do nó
					for(int j=i+1; j < root->n; j++){
						strcpy(root->children[j], root->children[j+1]);
					}

					(root->n)--;
					
					if(remove(rightChild->filename) == 0){
						diskWrite(root);
						diskWrite(leftChild);
					}
				}
				freeNode(rightChild);
			}
			freeNode(leftChild);
		}
	}else{
		printf("I: %d\n", i);
		diskRead(root->children[i], &leftChild);
		if(leftChild->n == t-1){
			printf("Case 3");
			if(i > 0){
				diskRead(root->children[i-1], &leftSib);
				if(leftSib->n >= t){
					printf("a\n");
					flag=1;
					//Remove elemento mais a direita do vetor chaves do irmão da esquerda do nó que contém elem
					num = leftSib->keys[(leftSib->n)-1];

					//Reorganiza filhos do nó
					for(int j=leftChild->n; j >= 0; j--){
						strcpy(leftChild->children[j+1], leftChild->children[j]);
					}

					//Transfere o filho do nó removido para a subárvore que contém o elem
					strcpy(leftChild->children[0], leftSib->children[leftSib->n]);
					(leftSib->n)--;

					//Adiciona nova chave ao filho que contém elem
					for(int j=(leftChild->n)-1; j >= 0; j--){
						leftChild->keys[j+1] = leftChild->keys[j];
					}
					leftChild->keys[0] = root->keys[i-1];
					(leftChild->n)++;

					root->keys[i-1] = num;
				}
			}

			if(i+1 <= root->n && flag == 0){
				diskRead(root->children[i+1], &rightSib);
				if(rightSib->n >= t){
					printf("a\n");
					flag=1;
					//Remove elemento mais a esquerda do vetor chaves do irmão da direita do nó que contém elem
					num = rightSib->keys[0];

					//Transfere o filho do nó removido para a subárvore que contém o elem
					strcpy(leftChild->children[(leftChild->n)+1], rightSib->children[0]);
					(rightSib->n)--;

					//Reorganiza vetor chaves do irmão da direita
					for(int j=0; j < rightSib->n; j++){
						rightSib->keys[j] = rightSib->keys[j+1];
					}

					//Reorganiza filhos do irmão da direita
					for(int j=0; j < (rightSib->n)+1; j++){
						strcpy(rightSib->children[j], rightSib->children[j+1]);
					}

					//Adiciona nova chave ao filho que contém elem
					leftChild->keys[leftChild->n] = root->keys[i];
					(leftChild->n)++;

					//Sobe o número coletado do irmão da direita para o pai
					root->keys[i] = num;

				}
			}

			if(flag == 0){
				if(i+1 <= root->n){
					printf("b\n");
					//Utiliza chave na posicao i da raiz como mediana do filho da esquerda
					leftChild->keys[t-1] = root->keys[i];

					//Reorganiza vetor de chaves do nó
					for(int j=i; j < (root->n)-1; j++){
						root->keys[j] = root->keys[j+1];
					}
					
					//Reorganiza vetor de filhos do nó
					for(int j=i+1; j < (root->n); j++){
						strcpy(root->children[j],root->children[j+1]);
					}
					(root->n)--;

					//Concatena vetor de chaves do ńo filho da esquerda com o seu irmão da direita
					for(int j=0; j < t-1; j++){
						leftChild->keys[j+t] = rightSib->keys[j];
					}
					leftChild->n = 2*t-1;
					
					//Concatena vetor de filhos do ńo filho da esquerda com o seu irmão da direita
					for(int j=0; j < t; j++){
						strcpy(leftChild->children[j+t],rightSib->children[j]);
					}
				}else{
					printf("b\n");
					//Utiliza chave na posicao i da raiz como mediana do filho da esquerda
					leftChild->keys[t-1] = root->keys[i-1];

					//Reorganiza vetor de chaves do nó
					for(int j=i; j < (root->n)-1; j++){
						root->keys[j] = root->keys[j+1];
					}
					
					//Reorganiza vetor de filhos do nó
					for(int j=i; j < root->n; j++){
						strcpy(root->children[j],root->children[j+1]);
					}
					(root->n)--;

					//Reorganiza os elementos existentes do vetor para depois da mediana
					for(int j=0; j < t-1; j++){
						leftChild->keys[j+t] = leftChild->keys[j];
					}

					//Insere elementos do irmão da esquerda no vetor de chaves do nó
					for(int j=0; j < t-1; j++){
						leftChild->keys[j] = leftSib->keys[j];
					}
					leftChild->n = 2*t-1;
					
					//Reorganiza o vetor de filhos do nó
					for(int j=0; j < t; j++){
						strcpy(leftChild->children[j+t],leftChild->children[j]);
					}

					//Insere nós filhos do irmão da esquerda
					for(int j=0; j < t; j++){
						strcpy(leftChild->children[j],leftSib->children[j]);
					}
				}
			}
			diskWrite(root);
			diskWrite(leftChild);
			diskWrite(leftSib);
			diskWrite(rightSib);

			freeNode(rightSib);
			freeNode(leftSib);
		}

		delete(leftChild, elem);
		freeNode(leftChild);
	}
}

void deleteCLRS(BTreeNode** root, int elem){
	delete(*root, elem);

	//Troca o ponteiro da raiz da árvore caso após a remoção tenha ficado sem elementos;
	if((*root)->n == 0){
		BTreeNode *temp = *root;
		diskRead((*root)->children[0], root);

		free(temp);
	}
}

char* chooseTree(){
	char* result = (char*)malloc(sizeof(char)*(FILENAME_SIZE+TREEPATH_LEN+2));
	char filename[FILENAME_SIZE+1];

	strcpy(result, treesPath);

	int i=0, found=0;
	DIR *d;
	struct dirent *dir;
	d = opendir(treesPath);

	while(found == 0){
		if (d) {
			while ((dir = readdir(d)) != NULL) {
				printf("%s\n", dir->d_name);
			}
		}
		printf("Enter the filename: ");
		getchar();
		fgets(filename, FILENAME_SIZE+1, stdin);

		while(filename[i] != '\n'){
			i++;
		}
		filename[i] = '\0';

		printf("%s\n", filename);

		rewinddir(d);
		if (d) {
			while ((dir = readdir(d)) != NULL) {
				if(strcmp(filename, dir->d_name) == 0){
					found=1;
					break;
				}
			}
		}
		if(found == 0){
			rewinddir(d);
			printf("Not found\n");
		}
	}

	strcat(result, filename);
	
	closedir(d);
	return result;
}

void imprimirArvoreB(BTreeNode* root, int nivel) {
	BTreeNode* child = NULL;

    if (root != NULL) {
        int i;
        if (!root->leaf) {
			diskRead(root->children[root->n], &child);
            imprimirArvoreB(child, nivel + 1);
        }
        for (i = root->n - 1; i >= 0; i--) {
            for (int j = 0; j < nivel; j++) {
                printf("    ");  // Indentação para mostrar a profundidade
            }
            printf("%d\n", root->keys[i]);

            if (!root->leaf) {
				diskRead(root->children[i], &child);
                imprimirArvoreB(child, nivel + 1);
            }
        }
		freeNode(child);
    }
}
BTreeNode* printTree(BTreeNode* root){
	printf("----------------\n");
	printf("Size: %d\n", root->n);
	printf("Filename: %s\n", root->filename);
	printf("Keys: ");
	for(int i=0; i < root->n;i++){
		printf("%d,", root->keys[i]);
	}
	printf("\n");
	printf("Childs: ");
	for(int i=0; i < root->n+1; i++){
		if(strcmp(root->children[i],"") != 0){
			printf("%s\n", root->children[i]);
		}
	}
	printf("\n");
	printf("Leaf: %d\n", root->leaf);

	for(int i=0; i < root->n+1; i++){
		if(strcmp(root->children[i],"") != 0){
			BTreeNode* temp = NULL;
			diskRead(root->children[i], &temp);
			printTree(temp);
			freeNode(temp);
		}
	}
}

void testeCriarArvore(BTreeNode** x){
	*x = createNode(t, 1);

	insertCLRS(x, 5);
	insertCLRS(x, 1);
	insertCLRS(x, 7);
	insertCLRS(x, -1);
	insertCLRS(x, 9);
	insertCLRS(x, 10);
	insertCLRS(x, 6);
	insertCLRS(x, 15);
	insertCLRS(x, 17);
	insertCLRS(x, 25);
	insertCLRS(x, 40);
	insertCLRS(x, 2);
	insertCLRS(x, 3);
	insertCLRS(x, 4);
	insertCLRS(x, 11);
	insertCLRS(x, 12);
	insertCLRS(x, 13);
	insertCLRS(x, 16);
	insertCLRS(x, 0);
	insertCLRS(x, 80);
	insertCLRS(x, 75);
	insertCLRS(x, 93);
	insertCLRS(x, 100);

	printf("\n");
	printf("%s\n", (*x)->filename);
}

void testeLerArvore(BTreeNode** x, char* str){
	diskRead(str, x);
}

int menu_inicial(){
	int opc=0;
	do{
		printf("========================== B-Tree ==========================\n");
		printf("= 1 - Create new tree                                      =\n");
		printf("= 2 - Open a existing tree                                 =\n");
		printf("= 3 - Exit                                                 =\n");
		printf("============================================================\n");
		printf("Enter your option: ");
		scanf("%d", &opc);
	}while(opc < 1 || opc > 3);
	
	return opc;
}

int menu_principal(){
	int opc = 0;
	do{
		printf("========================== B-Tree ==========================\n");
		printf("= 1 - Print B-Tree                                         =\n");
		printf("= 2 - Insert element                                       =\n");
		printf("= 3 - Delete element                                       =\n");
		printf("= 4 - Search element                                       =\n");
		printf("= 5 - Exit                                                 =\n");
		printf("============================================================\n");
		printf("Enter your option: ");
		scanf("%d",&opc);
	}while(opc < 1 || opc > 5);

	return opc;
}
int main(int argc, char* argv[]){
	int opc=0;
	int key=0;
	int pos=0;
	BTreeNode* root = NULL;
	BTree* tree = NULL;
	char* filename = NULL;
	char* searchResult = NULL;

	srand(time(NULL));

	if(argc != 1){
		if(strcmp(argv[1], "create") == 0){
			t = 3;
			testeCriarArvore(&root);

			tree = createBTree(t, root->filename);
			strcpy(tree->root, root->filename);
			writeTree(tree);
			printf("Tree name: %s\n", tree->filename);
			return 0;
		}
		if(strcmp(argv[1], "print") == 0){
			printTree(root);
		}
		if(strcmp(argv[1], "del") == 0){
			deleteCLRS(&root, atoi(argv[3]));
			printf("Raiz: %s\n", root->filename);
		}
	}

	opc = menu_inicial();
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
		opc = menu_principal();
		switch(opc){
			case 1:
				// Imprimir árvore
				imprimirArvoreB(root, 0);
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
				}
				break;
			case 5:
				// Sair do programa
				strcpy(tree->root, root->filename);
				writeTree(tree);
				return 0;
				break;
		}
	}

	return 0;
}
