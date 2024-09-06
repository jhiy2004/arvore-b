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

int diskWrite(char *name, BTreeNode* node){
	FILE* file = fopen(name, "wb+");
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

	if(y->n == 2*t-1){
		z->n = t-1;

		for(int j=0; j < t-1; j++){
			z->keys[j] = y->keys[j+t];
		}
	}else{
		z->n = t-2;

		for(int j=0; j < t-2; j++){
			z->keys[j] = y->keys[j+t];
		}
	}

	if (y->leaf == 0){
		printf("Não folha\n");
		for(int j=0; j < t; j++){
			printf("%d\n", j);
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

	printf("X: [");
	for(int j=0; j < x->n; j++){
		printf("%d,", x->keys[j]);
	}
	printf("]\n");

	printf("Y: [");
	for(int j=0; j < y->n; j++){
		printf("%d,", y->keys[j]);
	}
	printf("]\n");
	
	printf("Z: [");
	for(int j=0; j < z->n; j++){
		printf("%d,", z->keys[j]);
	}
	printf("]\n");

	diskWrite(y->filename, y);
	diskWrite(z->filename, z);
	diskWrite(x->filename, x);

	freeNode(y);
	freeNode(z);
}

void insertNotFull(BTreeNode* root, int elem){
	int i = root->n-1;
	if(root->leaf == 1){
		printf("------------------------\n");
		printf("Chave: %d\n", root->keys[0]);
		printf("Elem: %d\n", elem);
		printf("------------------------\n");
		while(i >= 0 && elem < root->keys[i]){
			root->keys[i+1] = root->keys[i];
			i--;
		}
		root->keys[i+1] = elem;
		(root->n)++;
		diskWrite(root->filename, root);
	}else{
		printf("Else\n");
		while(i >= 0 && elem < root->keys[i]){
			i--;
		}
		i++;
		printf("I: %d\n", i);
		BTreeNode* child = NULL;

		diskRead(root->children[i], &child); 
		if(child->n == 2*t-1){
			printf("Quebrou\n");
			splitChild(root, i);
			if(elem > root->keys[i]){
				i++;
				freeNode(child);
				diskRead(root->children[i], &child); 
			}
		}

		insertNotFull(child, elem);
		freeNode(child);
	}
}

BTreeNode* insertCLRS(BTreeNode** root, int elem){
	insertNotFull(*root, elem);

	if ((*root)->n == 2*t - 1){
		printf("CLRS\n");
		sleep(1);
		BTreeNode* s = createNode(t, 0);
		strcpy(s->children[0],(*root)->filename);
		splitChild(s, 0);
		*root = s;
		insertNotFull(*root, elem);
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
			free(temp);
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

	printf("\n");
	printf("%s\n", (*x)->filename);
}

void testeLerArvore(BTreeNode** x, char* str){
	diskRead(str, x);
	printTree(*x);
}

int main(){
	t=3;

	BTreeNode* root=NULL;
	//testeCriarArvore(&root);
	testeLerArvore(&root, "./files/B4N5AjU7v4HTuM2EF5KJ.bin");

	return 0;
}
