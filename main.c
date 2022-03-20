#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINESIZE 128
#define N 100000
#define WSIZE 200
#define labelLen 4

char **arrayLabel;

int Min, label0 = 0, label1 = 1, labelSize,counter=1;
FILE *f, *g;

BOOLARRAY* BLA;
BOOLARRAY* NEXT;

NODE* G;       //point to the list of G's nodes
NODE* GF;      //point to the list of GF's nodes (first node)

NODE* V[N];  //an array pointing to the nodes via their id  (we assume no more than N new nodes will be added)

typedef struct nlist {
	int lid;
	int edgetype;
	struct nlist *lnext, *lback;
} LIST;

typedef struct node {
	int id;
	char op, label[WSIZE];
	int indegree, outdegree;
	LIST *in_edges, *out_edges;
	struct node *next, *back;
}  NODE;

typedef struct boolArray {
	int *arr;
	struct boolArray *next;
}BOOLARRAY;

BOOLARRAY* new_boolA();
BOOLARRAY* copyToNewBA(BOOLARRAY* b);
NODE* new_node();
int hex2int(char* hex);
LIST* new_list(int to, int type);
void readg();
int match(char text[], char pattern[]);
void print_dot(FILE* f, NODE* gg);
void findMaxAndMin();
void freeTheGragh(NODE* g);
void buildHashTable(NODE* g, BOOLARRAY* ba);
int** buildTable(NODE* g, BOOLARRAY* ba);
void reduceGraph(int** tab);

int main(int argc, char* argv[])
{
	int** tab;
	char filePath[128];
	printf("Path: ");
	scanf("%127s", filePath);
	g = fopen(filePath, "r");
	findMaxAndMin();
	fclose(g);
	f = fopen("NNN.dot", "w");
	g = fopen(filePath, "r");
	readg();
	print_dot(f, G);
	fclose(g);
	fclose(f);
	tab = buildTable(GF, BLA);
	reduceGraph(tab);
	f = fopen("NN.dot", "w");
	print_dot(f, G);
	fclose(f);
	return (0);
}

BOOLARRAY * new_boolA() {
	BOOLARRAY* ba;
	ba = (BOOLARRAY*)malloc(sizeof(BOOLARRAY));
	ba->arr = (int*)malloc(sizeof(int)*labelSize);
	ba->next = NULL;
	for (int i = 0; i < labelSize; i++) {
		ba->arr[i] = 0;
	}
	return ba;
}

BOOLARRAY * copyToNewBA(BOOLARRAY * b) {
	BOOLARRAY* ba = new_boolA();
	for (int i = 0; i < labelSize; i++) {
		ba->arr[i] = b->arr[i];
	}
	return ba;
}

NODE *new_node()
{
	NODE *p;
	p = (NODE *)malloc(sizeof(NODE));
	p->next = NULL;
	p->back = NULL;
	p->in_edges = NULL;
	p->out_edges = NULL;
	p->indegree = p->outdegree = 0;
	p->op = 'G';
	return(p);
}


int hex2int(char *hex) {
	int val = 0;
	while (*hex) {
		// get current character then increment
		char byte = *hex++;
		// transform hex character to the 4bit equivalent number, using the ascii table indexes
		if (byte >= '0' && byte <= '9') byte = byte - '0';
		else if (byte >= 'a' && byte <= 'f') byte = byte - 'a' + 10;
		else if (byte >= 'A' && byte <= 'F') byte = byte - 'A' + 10;
		// shift 4 to make space for new digit, and add the 4 bits of the new digit
		val = (val << 4) | (byte & 0xF);
	}
	return val;
}

LIST *new_list(int to, int type)
{
	LIST *l;
	l = (LIST *)malloc(sizeof(LIST));
	l->lnext = NULL;
	l->lback = NULL;
	l->lid = to;
	l->edgetype = type;  //no type
	return(l);
}


void readg()
{
	int b, v, r, i, j, n, k, fr, to, le, indLabel;
	char c = '1';
	char line[LINESIZE];
	char name[WSIZE];
	char id[LINESIZE];
	int position, lastpos, lastlastpos;
	NODE *p;
	LIST *l, *ll, *t, *tt;
	r = 0;
	b = 0;
	le = 0;
	indLabel = 2;
	// TODO --- done 

	V[0] = new_node();
	V[0]->id = 0;
	strcpy(V[0]->label, "0");
	strcpy(arrayLabel[0], "0");
	V[1] = new_node();
	V[1]->id = 1;
	strcpy(V[1]->label, "1");
	strcpy(arrayLabel[1], "1");
	//
	while (c != EOF)
	{
		i = 0;
		while ((c = getc(g)) != '\n' && c != EOF)// read line from the file
		{
			line[i] = c; i++;
			if (i == LINESIZE)printf("LSIZE too small\n");
		}
		line[i] = '\0';

		r = sscanf(line, "{ rank = same; \" %s", name);
		if ((r > 0) && (name[0] != 'C' || name[1] != 'O' || name[2] != 'N' || name[3] != 'S' || name[4] != 'T'))
		{
			b = 1;
			printf("name=%s\n", name);
			strcpy(arrayLabel[indLabel], name);
			sprintf(name, "%d", indLabel);
			indLabel++;
		}
		else {
			r = sscanf(line, "\"%x\";", &n);
			//printf("%s l = %d b=%d r=%d\n",line,n,b,r);
			if (b && (r > 0))
			{
				n -= Min;
				printf("id=%d \n", n);
				p = new_node();
				p->id = n;
				strcpy(p->label, name);
				if (le == 0) {
					GF = G = p; V[p->id] = p; le++;
				}
				else {
					G->back = p; V[p->id] = p; p->next = G; G = p;
				}
			}
			else b = 0;
		}
		r = sscanf(line, "\"%x\" -> \"%x\" [style = dashed];", &fr, &to);  //read negative edges
		if (r > 1 && (strstr(line, "[style = dashed];") != NULL)) {//TODO
			fr -= Min;
			if (to != label0 && to != label1)
				to -= Min;
			else {
				if (to == label0)
					to = 0;
				else
					to = 1;
			}
			printf("\"%x\" -> \"%x\" [style = dashed];\n", fr, to);
			l = new_list(to, 0);
			l->lnext = V[fr]->out_edges;
			if (V[fr]->out_edges != NULL) V[fr]->out_edges->lback = l;
			V[fr]->out_edges = l;
			V[fr]->outdegree++;
			t = new_list(fr, 0);
			t->lnext = V[to]->in_edges;
			if (V[to]->in_edges != NULL) V[to]->in_edges->lback = t;
			V[to]->in_edges = t;
			V[to]->indegree++;
		}
		else
		{
			r = sscanf(line, "\"%x\" -> \"%x\";", &fr, &to);  //read positive edges
			if (r > 1 && (line[0] != '"' || line[1] != 'F' || line[2] != '0' || line[3] != '"')) {
				fr -= Min;
				if (to != label0 && to != label1)
					to -= Min;
				else {
					if (to == label0)
						to = 0;
					else
						to = 1;
				}
				printf("\"%x\" -> \"%x\" \n", fr, to);
				l = new_list(to, 1);
				l->lnext = V[fr]->out_edges;
				if (V[fr]->out_edges != NULL) V[fr]->out_edges->lback = l;
				V[fr]->out_edges = l;
				V[fr]->outdegree++;
				t = new_list(fr, 1);
				t->lnext = V[to]->in_edges;
				if (V[to]->in_edges != NULL) V[to]->in_edges->lback = t;
				V[to]->in_edges = t;
				V[to]->indegree++;
			}
		}
	}
}

int match(char text[], char pattern[]) {
	int c, d, e, text_length, pattern_length, position = -1;
	//printf("MATCHING %s in %s\n",pattern,text);
	text_length = strlen(text);
	pattern_length = strlen(pattern);

	if (pattern_length > text_length) {
		return -1;
	}

	for (c = 0; c <= text_length - pattern_length; c++) {
		position = e = c;

		for (d = 0; d < pattern_length; d++) {
			if (pattern[d] == text[e]) {
				e++;
			}
			else {
				break;
			}
		}
		if (d == pattern_length) {
			return position;
		}
	}

	return -1;
}


//print dot graph
void print_dot(FILE *f, NODE *gg)
{
	int node = 0, edge = 0;
	NODE *p;
	LIST *l;
	fprintf(f, "digraph G{\n");

	p = gg;
	while (p != NULL)
	{
		fprintf(f, "%d [label = \"%s\"]\n", p->id, arrayLabel[atoi(p->label)]);
		p = p->next;
		node++;
	}
	p = gg;
	while (p != NULL)
	{
		l = p->out_edges;
		while (l != NULL)
		{
			edge++;
			if (l->edgetype == 0) {
				fprintf(f, "%d -> %d [label = \"n.%s\"]\n", p->id, l->lid, arrayLabel[atoi(p->label)] /* V[p->id]->type*/);
			}
			else {
				fprintf(f, "%d -> %d [label = \"%s\"]\n", p->id, l->lid, arrayLabel[atoi(p->label)] /* V[p->id]->type*/);
			}
			l = l->lnext;
		}
		p = p->next;
	}
	fprintf(f, "}\n");
	printf("The nodes is %d   The edges is %d\n", node, edge);
}

void findMaxAndMin() {
	int b, r, i, n, max, min, label, size;
	char c = '1';
	char line[LINESIZE];
	char name[WSIZE];
	size = 2;
	r = 0;
	b = 0;
	max = 0;
	min = 100000;
	label = 0;
	while (c != EOF)
	{
		i = 0;
		while ((c = getc(g)) != '\n' && c != EOF)// read line from the file
		{
			line[i] = c; i++;
			if (i == LINESIZE)printf("LSIZE too small\n");
		}
		line[i] = '\0';

		r = sscanf(line, "{ rank = same; \" %s", name);
		if ((r > 0) && (name[0] != 'C' || name[1] != 'O' || name[2] != 'N' || name[3] != 'S' || name[4] != 'T'))
		{
			b = 1;
			size++;
		}
		else {
			r = sscanf(line, "\"%x\";", &n);
			//printf("%s l = %d b=%d r=%d\n",line,n,b,r);
			if (b && (r > 0))
			{
				if (n > max)
					max = n;
				if (min > n)
					min = n;
			}
			else b = 0;
		}

		r = sscanf(line, "\" %s\" [label = \"0\"];", name);
		if (r > 0 && (strstr(line, "[label = \"0\"];") != NULL)) {
			label0 = (int)strtol(name, NULL, 16);
		}
		else
			r = 0;
		r = sscanf(line, "\" %s\" [label = \"1\"];", name);
		if (r > 0 && (strstr(line, "[label = \"1\"];") != NULL)) {
			label1 = (int)strtol(name, NULL, 16);
		}
		else
			r = 0;
	}

	arrayLabel = malloc(size * sizeof(char*));
	for (int i = 0; i < size; i++)
		arrayLabel[i] = malloc((labelLen + 1) * sizeof(char));

	Min = min - 3;
	max = max + 1 - 1;
	labelSize = (size * 2 + 3);
}

void freeTheGragh(NODE * g) {
	NODE *p;
	LIST* l;
	LIST* ll;
	p = g;
	while (p != NULL)
	{
		g = p->next;
		l = p->out_edges;
		while (l != NULL) {
			ll = l->lnext;
			free(l);
			l = ll;
		}
		l = p->in_edges;
		while (l != NULL) {
			ll = l->lnext;
			free(l);
			l = ll;
		}
		free(p);
		p = g;
	}
}

void buildHashTable(NODE* g,BOOLARRAY* ba) {
	NODE* p = g;
	LIST *l = p->out_edges;
	BOOLARRAY* b = ba;
	BOOLARRAY* bb = copyToNewBA(ba);
	int checkp = 0, checkn = 0;
	if (atoi(p->label) == 1 || atoi(p->label) == 0) {
		b->arr[atoi(p->label)] = 1;
	}
	while (l != NULL) {
		printf("counter is %d\n", counter);
		if (l->edgetype == 0)// if the label is negative
		{
			/*if (checkn == 0) {
				checkn = 1;
				counter++;
			}*/
			ba->arr[atoi(p->label) * 2 - 1] = 1;
			buildHashTable(V[l->lid], ba);
		}
		else// if the label is positive
		{
			if (checkp == 0) {
				checkp = 1;
				counter++;
				NEXT->next = bb;
				NEXT = bb;
			}
			bb->arr[atoi(p->label) * 2 - 2] = 1;
			buildHashTable(V[l->lid], bb);
		}
		l = l->lnext;
	}
}

int** buildTable(NODE* g, BOOLARRAY* ba) {
	ba = new_boolA();
	BOOLARRAY *b;
	int i;
	NEXT = ba;
	buildHashTable(g, ba);
	freeTheGragh(G);
	int **tab = (int**)malloc(sizeof(int*)*(counter-1));
	for (i = 0; i < counter; i++) {
		tab[i] = (int*)malloc(sizeof(int)*labelSize);
	}
	NEXT = ba;
	for (int j = 0; j < counter-1; j++) {
		for (i = 0; i < labelSize; i++) {
			tab[j][i] = ba->arr[i];
			printf("%d ", ba->arr[i]);
		}
		printf("\n", counter);
		b = ba;
		ba = ba->next;
	}
	while (NEXT != NULL) {
		ba = NEXT;
		NEXT = NEXT->next;
		free(ba->arr);
		free(ba);
	}
	return tab;
}

void reduceGraph(int ** tab) {
	int i, j, k;
	int sum = 0, check = 0;;
	for (i = 0; i < counter-1; i++) {
		// check if the path end with 0
		if (tab[i][0] == 1)
			tab[i][labelSize-1] = -1;
		else {
			// check if the path contain variable and the negative
			for (j = 2; j < (labelSize-1)/2; j ++) {
				if (tab[i][j * 2 - 1] == 1 && tab[i][j * 2 - 2] == 1) {
					tab[i][labelSize - 1] = -1;
					check = 1;
				}
				//count the number of vairables in the path
				if (tab[i][j * 2 - 1] == 1 || tab[i][j * 2 - 2] == 1)
					sum++;
			}
			//if we need the path we write the number of the vairables in the last node in the array
			if (check == 1) {
				check = 0;
			}
			else {
				tab[i][labelSize - 1] = sum;
			}
			sum = 0;
		}
	}

	for (i = 0; i < counter-1; i++) {
		sum=0;
		if (tab[i][labelSize - 1] != -1) {
			for (k = i + 1; k < counter; k++) {
				if (tab[k][labelSize - 1] != -1) {
					for (j = 2; j < labelSize - 1; j++) {
						if (tab[i][j] == 1 && tab[k][j] == 1) {
							sum++;
						}
					}
					if (tab[i][labelSize - 1] < tab[k][labelSize - 1]) {
						if (tab[i][labelSize - 1] == sum) {
							tab[k][labelSize - 1] = -1;
						}
					}
					else {
						if (tab[k][labelSize - 1] == sum) {
							tab[i][labelSize - 1] = -1;
							break;
						}
					}
				}
			}
		}
	}

	printf("\n");
	for (i = 0; i < counter-1; i++) {
		for (j = 0; j < labelSize; j++) {
			printf("%d ", tab[i][j]);
		}
		printf("\n");
	}

	// make a reduced table
	sum = 0;
	for (i = 0; i < counter-1; i++) {
		if (tab[i][labelSize - 1] != -1)
			sum++;
	}
	int **tab1 = (int**)malloc(sizeof(int*)*(sum));
	for (i = 0; i < sum; i++) {
		tab1[i] = (int*)malloc(sizeof(int)*labelSize-3);
	}
	k = 0;
	printf("\n");
	for (i = 0; i < counter-1; i++) {
		if (tab[i][labelSize - 1] != -1) {
			for (j = 2; j < labelSize - 1; j++) {
				tab1[k][j - 2] = tab[i][j];
				printf("%d ", tab[i][j]);
			}
			printf("\n");
			k++;
		}
	}

	// free the old table
	for (i = 0; i < counter-1; i++) {
		free(tab[i]);
	}

	tab = tab1;

}