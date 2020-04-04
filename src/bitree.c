#define BITREE_C_
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "scl_alg.h"
#include "scl_deco.h"
#include "bitree.h"
/*构造深度N的二叉树*/
bitree creatTree(int16_t col, int16_t iniRol) {//col父亲所在列，col-1这次newTree,col-2这次newTree的儿子所在列，col = A 代表A是根节点所在列，A-1是第一级所在列
	if (col == 0) {//这个判断条件导致输入N+1，及下边col-1,col-2,0是叶子所在列，要映射fg的列,行数均从0开始
		return NULL;
	}
	else {
		bitree newTree;//不要用new，是关键字
		newTree = (bitree)malloc(sizeof(bitnode));
		if (newTree == NULL) {
			printf("fail to malloc nod -- col:%d,iniRol:%d", col, iniRol);
			exit(1);
		}
		else {
			newTree->iniRol = iniRol;
			newTree->col = col - 1;//根节点是M列数，（第一次传入M+1）树的列数标定，与f,g，llr对列数的约定相一致
			newTree->lchild = creatTree(col - 1, iniRol);//左节点起始行总是继承父亲行，此处col-1使递归走向终结
			newTree->rchild = creatTree(col - 1, iniRol + *(difNodTab + col - 2));//R节点起始行与列数有关，注意这是递归意义下的，此处col-1使递归走向终结
		}
	return newTree;
	}
}
/*以二叉树的形式解码N个比特*/
void printBitree(bitree bt, void(*p)(int16_t, int16_t)) {//注意void(*p)参数数据类型
	if (bt != NULL) {
		if (bt->col != M)//根节点 不操作
			(*p)(bt->iniRol, bt->col);//前序，对节点f或g
		//printf("%d%d\n", bt->iniRol, bt->col);
		printBitree(bt->lchild, fnod);
		printBitree(bt->rchild, gnod);
	}
}
/*释放空间*/
void delBitree(bitree t) {
	if (t != NULL){
		delBitree(t->lchild);
		delBitree(t->rchild);
		free(t);
	}
}