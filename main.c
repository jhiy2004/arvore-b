#include <string.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define FILENAME_SIZE 33

int t;
const char *path = "./files/";
const char *extension = ".bin";

typedef struct BTreeNode{
	char *filename;
	int n;
	int *keys;
	char **children;
	int leaf;
} BTreeNode;

char* generateRandomFilename(){
	srand(time(NULL));
	char *result = (char*) malloc(sizeof(char)*FILENAME_SIZE);
	char filename[25];

	strcpy(result, path);
	
	DIR *d;
	struct dirent *dir;
	d = opendir("./files/");

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
	result[FILENAME_SIZE-1] = '\0';

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
	node->filename = generateRandomFilename();

	for(int i=0; i < 2*T; i++){
		node->children[i] = (char*)malloc(FILENAME_SIZE*sizeof(char));
		strcpy(node->children[i], "");
	}
	return node;
}

int diskWrite(BTreeNode* node){
	FILE* file = fopen(node->filename, "wb+");
	if(file == NULL){
		printf("ERRO\n");
		return 0;
	}

	//Grava o valor de n no arquivo
	fwrite(&node->n, sizeof(int), 1, file);
	
	//Grava o nome do arquivo no arquivo
	for(int i=0; i < FILENAME_SIZE; i++){
		fwrite(&node->filename[i], sizeof(char), 1, file);
	}

	//Grava os valores das chaves no arquivo
	for(int i=0; i < node->n;i++){
		fwrite(&node->keys[i], sizeof(int), 1, file);
	}

	//Grava os nomes dos arquivos filhos no arquivo
	for(int i=0; i < 2*t; i++){
		for(int j=0; j < FILENAME_SIZE; j++){
			fwrite(&node->children[i][j], sizeof(char), 1, file);
		}
	}

	//Grava se o nó é folha ou não no arquivo
	fwrite(&node->leaf, sizeof(int), 1, file);

	fclose(file);
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
	for(int i=0; i < FILENAME_SIZE; i++){
		fread(&(*dest)->filename[i], sizeof(char), 1, file);
	}

	//Lê os valores das chaves no nó
	for(int i=0; i < (*dest)->n;i++){
		fread(&(*dest)->keys[i], sizeof(int), 1, file);
	}

	//Lê os nomes dos arquivos filhos no nó
	for(int i=0; i < 2*t; i++){
		for(int j=0; j < FILENAME_SIZE; j++){
			fread(&(*dest)->children[i][j], sizeof(char), 1, file);
		}
	}

	//Lê se o nó é folha ou não no nó
	fread(&(*dest)->leaf, sizeof(int), 1, file);

	fclose(file);
}

/*
BTreeNode* search(BTreeNode* root, int elem){
	int i = 1;
	BTreeNode *child = NULL;
	while(i <= root->n && elem > root->keys[i]){
		i++;
	}
	if(i <= root->n && elem == root->keys[i]){
		return root;
	}else if(root->leaf == 1){
		return NULL;
	}else{
		diskRead(root->children[i], child);	
	}
	return search(child, elem);
}
*/

void splitChild(BTreeNode* x, int i){
	printf("Split!\n");
	BTreeNode* y = NULL;
	BTreeNode* z = NULL;

	diskRead(x->children[i], &y);

	sleep(1);
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

	printf("X[%d]: [", x->n);
	for(int j=0; j < x->n; j++){
		printf("%d,", x->keys[j]);
	}
	printf("]\n");

	printf("Y[%d]: [", y->n);
	for(int j=0; j < y->n; j++){
		printf("%d,", y->keys[j]);
	}
	printf("]\n");
	
	printf("Z[%d]: [", z->n);
	for(int j=0; j < z->n; j++){
		printf("%d,", z->keys[j]);
	}
	printf("]\n");

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
		sleep(1);
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

	printf("ROOT[%d]: [", root->n);
	for(int j=0; j < root->n; j++){
		printf("%d,", root->keys[j]);
	}
	printf("]\n");

	while(i < root->n && root->keys[i] < elem){
		i++;
	}

	if(root->keys[i] == elem){
		if(root->leaf == 1){
			if(root->n-1 >= t-1){
				//Case 1
				for(int j=i; j < t;j++){
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
					(leftSib->n)--;

					//Adiciona nova chave ao filho que cotém elem
					for(int j=(leftChild->n)-1; j > 0; j--){
						leftChild->keys[j+1] = leftChild->keys[j];
					}
					leftChild->keys[0] = root->keys[i];
					(leftChild->n)++;

					root->keys[i] = num;

					printf("ROOT[%d]: [", root->n);
					for(int j=0; j < root->n; j++){
						printf("%d,", root->keys[j]);
					}
					printf("]\n");

					printf("LEFTSIB[%d]: [", leftSib->n);
					for(int j=0; j < leftSib->n; j++){
						printf("%d,", leftSib->keys[j]);
					}
					printf("]\n");

					printf("CHILD[%d]: [", leftChild->n);
					for(int j=0; j < leftChild->n; j++){
						printf("%d,", leftChild->keys[j]);
					}
					printf("]\n");

				}
			}

			if(i+1 <= root->n && flag == 0){
				diskRead(root->children[i+1], &rightSib);
				if(rightSib->n >= t){
					printf("a\n");
					flag=1;
					//Remove elemento mais a esquerda do vetor chaves do irmão da esquerda do nó que contém elem
					num = rightSib->keys[0];
					(rightSib->n)--;

					//Adiciona nova chave ao filho que contém elem
					rightSib->keys[rightSib->n] = root->keys[i];
					(rightSib->n)++;

					root->keys[i] = num;
				}
			}

			if(flag == 0){
				printf("b\n");
				if(i+1 <= root->n){
					//Utiliza chave na posicao i da raiz como mediana do filho da esquerda
					leftChild->keys[t-1] = root->keys[i];
					(root->n)--;

					//Reorganiza vetor de chaves do nó
					for(int j=i; j < (root->n)-1; j++){
						root->keys[j] = root->keys[j+1];
					}
					
					//Reorganiza vetor de filhos do nó
					for(int j=i; j < (root->n); j++){
						strcpy(root->children[j],root->children[j+1]);
					}

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
					//Utiliza chave na posicao i da raiz como mediana do filho da esquerda
					leftChild->keys[t-1] = root->keys[i-1];
					(root->n)--;

					//Reorganiza vetor de chaves do nó
					for(int j=i; j < (root->n)-1; j++){
						root->keys[j] = root->keys[j+1];
					}
					
					//Reorganiza vetor de filhos do nó
					for(int j=i; j < root->n; j++){
						strcpy(root->children[j],root->children[j+1]);
					}

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

				printf("ROOT[%d]: [", root->n);
				for(int j=0; j < root->n; j++){
					printf("%d,", root->keys[j]);
				}
				printf("]\n");

				printf("CHILDS_ROOT[%d]: [", (root->n)+1);
				for(int j=0; j < (root->n)+1; j++){
					printf("%s,", root->children[j]);
				}
				printf("]\n");

				printf("LEFTCHILD[%d]: [", leftChild->n);
				for(int j=0; j < leftChild->n; j++){
					printf("%d,", leftChild->keys[j]);
				}
				printf("]\n");

				printf("CHILDS_LEFT[%d]: [", (leftChild->n)+1);
				for(int j=0; j < (leftChild->n)+1; j++){
					printf("%s,", leftChild->children[j]);
				}
				printf("]\n");

			}
			freeNode(rightSib);
			freeNode(leftSib);
			exit(1);
		}

		delete(leftChild, elem);
		freeNode(leftChild);
	}
}

/*
void deleteCLRS(BTreeNode** root, int elem){

}
*/


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

int main(int argc, char* argv[]){
	t=3;
	BTreeNode* root=NULL;

	if(argc == 1){
		testeCriarArvore(&root);
	}else{
		testeLerArvore(&root, argv[2]);
		if(strcmp(argv[1], "print") == 0){
			printTree(root);
		}
		if(strcmp(argv[1], "del") == 0){
			delete(root, atoi(argv[3]));
		}
	}

	return 0;
}
