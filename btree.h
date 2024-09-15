#ifndef BTREE_H
#define BTREE_H

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

char* generateRandomFilename(char* path);
void freeNode(BTreeNode* root);

BTreeNode* createNode(int T, int leaf);
BTree* createBTree(int T, char* rootFilename);

int diskWrite(BTreeNode* node);
int writeTree(BTree* tree);

void diskRead(char *name, BTreeNode **dest);
void readTree(char *name, BTree **dest);

int binarySearch(int arr[], int ini, int end, int elem);
char* search(BTreeNode* root, int elem, int *pos);

void splitChild(BTreeNode* x, int i);
void insertNotFull(BTreeNode* root, int elem);
void insertCLRS(BTreeNode** root, int elem);

int deletePredecessor(BTreeNode* root);
int deleteSucessor(BTreeNode* root);
void delete(BTreeNode* root, int elem);
void deleteCLRS(BTreeNode** root, int elem);

char* chooseTree();
void printBTree(BTreeNode* root, int level);
void createTreeTest(BTreeNode** x);
void runTestScript(BTreeNode** x);
int initialMenu();
int mainMenu();

#include "btree.c"
#endif
