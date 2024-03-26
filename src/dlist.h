#ifndef __DLIST_H__
#define __DLIST_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef int(*op_t)(void*);
typedef int(*cmp_t)(void *data, void *key);


//双向循环链表节点类型
typedef struct _node_t{
    //数据域
    void *data;

    //指针域
    struct _node_t *prev; //前驱指针
    struct _node_t *next; //后继指针
}node_t;


//双向循环链表头信息结构体
typedef struct _dlist_t{
    node_t *p; //指向双向循环链表的第一个节点
    int size; //数据域大小
    int count; //节点个数
}dlist_t;


//头信息结构体的创建
dlist_t *dlist_create(int size);
//头插
int dlist_prepend(dlist_t *d, void *data);
//头插
int dlist_prepend_v1(dlist_t *d, void *data);
//尾插
int dlist_append(dlist_t *d, void *data);
//返回元素的个数
int dlist_count(dlist_t *d);

//通过key检索链表节点数据
node_t *dlist_find_by_key(dlist_t *d, void *key, cmp_t my_cmp);

//通过key检索链表节点序号
int node_find_by_key(dlist_t *d, void *key, cmp_t my_cmp);

//通过索引插入新的节点
int dlist_insert_by_index(dlist_t *d, void *data, int index);

//通过索引删除链表的结点
int dlist_delete_by_index(dlist_t *d, int index);


//根据关键字排序
int dlist_select_sort(dlist_t *d,  cmp_t my_cmp);

//遍历
int dlist_traverse(dlist_t *d, op_t my_print);
//反向遍历
int dlist_back_traverse(dlist_t *d, op_t my_print);
//销毁
int dlist_destroy(dlist_t *d);



#endif/*__DLIST_H__*/
