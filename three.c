#include <stdio.h>
#include<stdlib.h>

typedef struct Node
{
    int data;
    struct Node *lchild,*rchild;
} BiTNode, *BiTree;
int N;
BiTree CreateBiTree(int *nodelist,int position)
{
    BiTree p;
    if ((nodelist[position]==0)||(position>N))
    {
        return NULL;
    }else{
        p=(BiTree)malloc(sizeof(BiTNode));
        p->data=nodelist[position];
        p->lchild=CreateBiTree(nodelist,2*position);
        p->rchild=CreateBiTree(nodelist,2*position+1);
        return p;
    }
    
}
int Get_Value(int oper,int oper1,int oper2)
{
    switch ((char)oper)
    {
    case '*':
        return oper1*oper2;
        break;
    case '/':
        return oper1/oper2;
        break;
    case '+':
        return oper1+oper2;
        break;
    case '-':
        return oper1-oper2;
        break;
    }
}

int Calculate(BiTree T)
{
    int oper1=0;
    int oper2=0;
    if ((T->lchild==NULL)&&(T->rchild==NULL))
    {
        return T->data-'0';
    }else{
        oper1=Calculate(T->lchild);
        oper2=Calculate(T->rchild);
        return Get_Value(T->data,oper1,oper2);
    }
    
}

int main()
{
    BiTree T=NULL;
    int cal_result;
    int nodelist[8]={' ','*','+','-','1','2','6','3'};
    N=7;
    T=CreateBiTree(nodelist,1);
    cal_result=Calculate(T);
    printf("Calculate result is [%d]\n", cal_result);
    return 0;
}