
#ifndef BITREE_H_
#define BITREE_H_
typedef struct node {
	struct node *lchild;
	struct node *rchild;
	int16_t iniRol;
	int16_t col;
}bitnode, *bitree;

#ifndef BITREE_C_
extern bitree creatTree(int16_t col, int16_t iniRol);
extern void printBitree(bitree bt, void(*p)(int16_t, int16_t));
extern void delBitree(bitree t);
#endif 
#endif // !BITREE_H_
