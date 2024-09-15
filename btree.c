#include "btree.h"
#include <string.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

char* generateRandomFilename(char* path){
	int pathLen=0;
	int exists=0;

	while(path[pathLen] != '\0'){
		pathLen++;
	}

	char *result = (char*) malloc(sizeof(char)*(FILENAME_SIZE+pathLen+1));
	char filename[FILENAME_SIZE];

	strcpy(result, path);

	//Abro o diretório do caminho que será armazenado o arquivo
	DIR *d;
	struct dirent *dir;
	d = opendir(path);

	do{
		rewinddir(d);
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

		//Concatena com a extensão do arquivo
		strcat(filename, extension);

		if (d) {
			while ((dir = readdir(d)) != NULL) {
				//Confere se já existe um nome igual ao gerado no momento
				if(strcmp(filename, dir->d_name) == 0){
					sleep(1);
					exists=1;
				}
			}
		}
	}while(exists != 0);

	closedir(d);

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
	//Libera memória alocada para cada nome de arquivo filho
	for(int i=0; i < 2*t; i++){
		free(root->children[i]);
	}
	free(root->children);
	free(root);
}

//Cria um nó de árvore b na memória principal
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

//Cria uma árvore b, que guarda o nome do arquivo raiz, seu próprio nome de arquivo e seu t
BTree* createBTree(int T, char* rootFilename){
	BTree* tree = (BTree*) malloc(sizeof(BTree));

	tree->filename = generateRandomFilename(treesPath);
	tree->root = (char*) malloc(sizeof(char)*(FILENAME_SIZE+NODEPATH_LEN+1));
	tree->t = T;

	strcpy(tree->root, rootFilename);

	return tree;
}

//Grava no disco um nó
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

//Grava no disco uma árvore b
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

//Lê do disco o nó de uma árvore b
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

//Lê do disco uma árvore b
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

int binarySearch(int arr[], int ini, int end, int elem){
	if(end >= ini){
		//Pega o valor centrar do array
		int mid = ini + (end-ini) / 2;

		if(arr[mid] == elem)
			return mid;

		if(arr[mid] > elem)
			return binarySearch(arr, ini, mid-1, elem);

		return binarySearch(arr, mid+1, end, elem);
	}
	return ini;
}

char* search(BTreeNode* root, int elem, int *pos){
	int i = 0;
	BTreeNode *child = NULL;
	i = binarySearch(root->keys, 0, root->n-1, elem);

	if(i <= root->n && elem == root->keys[i]){
		char* result = (char*)malloc(sizeof(char)*(FILENAME_SIZE+NODEPATH_LEN+1));
		strcpy(result, root->filename);
		//Retorna para a variável pos a posição em que foi encontrada o elemento
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

	//Copia para z todas as chaves que estão depois da mediana em y
	for(int j=0; j < t-1; j++){
		z->keys[j] = y->keys[j+t];
	}

	if (y->leaf == 0){
		//Copia para z todos os filhos que estão depois da mediana em y
		for(int j=0; j < t; j++){
			strcpy(z->children[j], y->children[j+t]);
		}
	}
	y->n = t-1;

	//Abre uma vaga para um filho na posição i+1 empurrando todos os filhos para a direita
	for(int j=(x->n); j > i; j--){
		strcpy(x->children[j+1], x->children[j]);
	}
	strcpy(x->children[i+1], z->filename);

	//Abre uma vaga para uma chave na posição i empurrando todas as chaves para a direita
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
		//Abre um vaga na posição i+1 para colocar elem
		while(i >= 0 && elem < root->keys[i]){
			root->keys[i+1] = root->keys[i];
			i--;
		}
		root->keys[i+1] = elem;
		(root->n)++;
		diskWrite(root);
	}else{
		//Encontra o nome do arquivo filho em que deve ser inserido
		while(i >= 0 && elem < root->keys[i]){
			i--;
		}
		i++;
		BTreeNode* child = NULL;

		diskRead(root->children[i], &child);
		//Se o filho estiver cheio split o filho
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
	//Se a raiz da árvore estiver cheia, é realizado um split e a raiz é atualizada
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

	i = binarySearch(root->keys, 0, root->n-1, elem);

	if(root->keys[i] == elem){
		if(root->leaf == 1){
			//Case 1
			for(int j=i; j < (root->n)-1;j++){
				root->keys[j] = root->keys[j+1];
			}
			(root->n)--;
			diskWrite(root);
		}else{
			diskRead(root->children[i], &leftChild);
			if(leftChild->n >= t){
				//Case 2a

				num = deletePredecessor(leftChild);
				root->keys[i] = num;
				diskWrite(root);
			}else{
				diskRead(root->children[i+1], &rightChild);
				if(rightChild->n >= t){
					//Case 2b

					num = deleteSucessor(rightChild);
					root->keys[i] = num;
					diskWrite(root);
				}else{
					//Case 2c
					//Adiciona a chave na mediana de leftChild
					leftChild->keys[t-1] = root->keys[i];

					//Concatenar vetor de chaves do ńo filho da esquerda com o da direita
					for(int j=0; j < rightChild->n; j++){
						leftChild->keys[t+j] = rightChild->keys[j];
					}

					//Concatena os filhos da direita no da esquerda
					for(int j=0; j < rightChild->n+1; j++){
						strcpy(leftChild->children[t+j],rightChild->children[j]);
					}
					leftChild->n = 2*t-1;

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
					delete(leftChild, elem);
				}
				freeNode(rightChild);
			}
			freeNode(leftChild);
		}
	}else{
		if(root->leaf == 1){
			printf("%d Not found\n", elem);
			return;
		}
		diskRead(root->children[i], &leftChild);
		if(leftChild->n == t-1){
			if(i > 0){
				diskRead(root->children[i-1], &leftSib);
				if(leftSib->n >= t){
					//Case 3a
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
					diskWrite(leftSib);
				}
			}

			if(i+1 <= root->n && flag == 0){
				diskRead(root->children[i+1], &rightSib);
				if(rightSib->n >= t){
					//Case 3a
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
				diskWrite(rightSib);
			}

			if(flag == 0){
				if(i+1 <= root->n){
					//Case 3b
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
					remove(rightSib->filename);
				}else{
					//Case 3b
					//Utiliza chave na posicao i da raiz como mediana do filho da esquerda
					leftChild->keys[t-1] = root->keys[i-1];

					//Reorganiza vetor de chaves do nó
					for(int j=i; j < (root->n)-1; j++){
						root->keys[j] = root->keys[j+1];
					}

					(root->n)--;

					//Reorganiza vetor de filhos do nó
					strcpy(root->children[root->n], leftChild->filename);

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
					remove(leftSib->filename);
				}
			}
			diskWrite(root);
			diskWrite(leftChild);

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
	if((*root)->n == 0 && strcmp((*root)->children[0], "") != 0){
		printf("here\n");
		BTreeNode *temp = *root;
		diskRead((*root)->children[0], root);

		//Remove o arquivo da antiga raiz
		remove(temp->filename);
		free(temp);
	}
}

char* chooseTree(){
	char* result = (char*)malloc(sizeof(char)*(FILENAME_SIZE+TREEPATH_LEN+2));
	char filename[FILENAME_SIZE+1];

	strcpy(result, treesPath);

	int i=0, found=0;

	//Abre o diretório onde são armazenados os arquivos das árvores
	DIR *d;
	struct dirent *dir;
	d = opendir(treesPath);

	while(found == 0){
		i=0;
		if (d) {
			while ((dir = readdir(d)) != NULL) {
				if(strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0){
					printf("%s\n", dir->d_name);
				}
			}
		}
		printf("Enter the filename: ");
		fgets(filename, FILENAME_SIZE+1, stdin);

		//Remove o \n inserido pelo fgets
		while(filename[i] != '\n'){
			i++;
		}
		filename[i] = '\0';

		//Verifica se foi inserido um nome já existente
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
			strcpy(filename, "");
		}
	}

	strcat(result, filename);

	closedir(d);
	return result;
}

void printBTree(BTreeNode* root, int level) {
	BTreeNode* child = NULL;

    if (root != NULL) {
        int i;
        if (!root->leaf) {
			diskRead(root->children[root->n], &child);
            printBTree(child, level + 1);
        }
        for (i = root->n - 1; i >= 0; i--) {
            for (int j = 0; j < level; j++) {
                printf("    ");  // Indentação para mostrar a profundidade
            }
            printf("%d\n", root->keys[i]);

            if (!root->leaf) {
				diskRead(root->children[i], &child);
                printBTree(child, level + 1);
            }
        }
		freeNode(child);
    }
}

void createTreeTest(BTreeNode** x){
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

	printf("\n");
	printf("%s\n", (*x)->filename);
}

void runTestScript(BTreeNode** x){
	*x = createNode(t, 1);
    printf("\n========= INICIANDO TESTE - ARVORE B COM T = 2 =========\n\n");

    printf("\n========= INSERINDO 25 =========\n");
	insertCLRS(x, 25);
	printBTree((*x), 0);
    printf("\n========= INSERINDO 10 =========\n");
	insertCLRS(x, 10);
	printBTree((*x), 0);
    printf("\n========= INSERINDO 30 =========\n");
	insertCLRS(x, 30);
	printBTree((*x), 0);
    printf("\n========= INSERINDO 5 =========\n");
	insertCLRS(x, 5);
	printBTree((*x), 0);
    printf("\n========= INSERINDO 15 =========\n");
	insertCLRS(x, 15);
	printBTree((*x), 0);
    printf("\n========= INSERINDO 20 =========\n");
	insertCLRS(x, 20);
	printBTree((*x), 0);
    printf("\n========= INSERINDO 1 =========\n");
	insertCLRS(x, 1);
	printBTree((*x), 0);
    printf("\n========= INSERINDO 35 =========\n");
	insertCLRS(x, 35);
	printBTree((*x), 0);
    printf("\n========= INSERINDO 40 =========\n");
	insertCLRS(x, 40);
	printBTree((*x), 0);
    printf("\n========= INSERINDO 2 =========\n");
	insertCLRS(x, 2);
	printBTree((*x), 0);
    printf("\n========= INSERINDO 26 =========\n");
	insertCLRS(x, 26);
	printBTree((*x), 0);
    printf("\n========= INSERINDO 28 =========\n");
	insertCLRS(x, 28);
	printBTree((*x), 0);
    printf("\n========= INSERINDO 11 =========\n");
	insertCLRS(x, 11);
	printBTree((*x), 0);
    printf("\n========= INSERINDO 12 =========\n");
	insertCLRS(x, 12);
	printBTree((*x), 0);
    printf("\n========= REMOVENDO 2 (caso 1)=========\n");
	deleteCLRS(x, 2);
	printBTree((*x), 0);
    printf("\n========= REMOVENDO 35 (caso 2a) =========\n");
	deleteCLRS(x, 35);
	printBTree((*x), 0);
    printf("\n========= REMOVENDO 25 (caso 2b) =========\n");
	deleteCLRS(x, 25);
	printBTree((*x), 0);
    printf("\n========= REMOVENDO 30 (caso 2c) =========\n");
	deleteCLRS(x, 30);
	printBTree((*x), 0);
    printf("\n========= REMOVENDO 20 (caso 3a) =========\n");
	deleteCLRS(x, 20);
	printBTree((*x), 0);
    printf("\n========= REMOVENDO 15 (caso 3a) =========\n");
	deleteCLRS(x, 15);
	printBTree((*x), 0);
    printf("\n========= REMOVENDO 1 (caso 1) =========\n");
	deleteCLRS(x, 1);
	printBTree((*x), 0);
    printf("\n========= REMOVENDO 5 (caso 3b) =========\n");
	deleteCLRS(x, 5);
	printBTree((*x), 0);
    printf("\n========= REMOVENDO 40 (caso 3b) =========\n");
	deleteCLRS(x, 40);
	printBTree((*x), 0);
    printf("\n========= ENCERRANDO TESTE =========\n");

	printf("\n");
	printf("%s\n", (*x)->filename);
}

int initialMenu(){
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

int mainMenu(){
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
