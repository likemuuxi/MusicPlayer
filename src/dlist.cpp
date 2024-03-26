#include "dlist.h"

//头信息结构体的创建
dlist_t *dlist_create(int size)
{
    dlist_t *d = NULL;

    if (size < 0)
    {
        printf("dlist_create size < 0\n");
        goto err0;
    }


    //分配空间
    d = (dlist_t *)malloc(sizeof(dlist_t));
    if (NULL == d)
    {
        printf("malloc failed..\n");
        goto err0;
    }
    memset(d, 0, sizeof(dlist_t));

    //赋值
    d->p = NULL;

    d->size = size;

    d->count = 0;

    return d;
err0:
    return NULL;
}


//分配节点函数
node_t *__malloc_node(dlist_t *d, void *data)
{
    node_t *new_node = NULL;

    //参数检查
    if (NULL == d || NULL == data)
    {
        printf("__malloc_node NULL\n");
        goto err0;
    }

    //分配节点空间
    new_node = (node_t *)malloc(sizeof(node_t));
    if (NULL == new_node)
    {
        printf("malloc failed..\n");
        goto err0;
    }
    memset(new_node, 0, sizeof(node_t));

    //分配数据域空间
    new_node->data = malloc(d->size);
    if (NULL == new_node->data)
    {
        printf("malloc failed..\n");
        goto err1;
    }
    memset(new_node->data, 0, d->size);


    //赋值
    memcpy(new_node->data, data, d->size);
    new_node->prev = new_node;
    new_node->next = new_node;

    d->count++;

    return new_node;
err1:
    free(new_node);
err0:
    return NULL;
}


//头插
int dlist_prepend(dlist_t *d, void *data)
{
    node_t *new_node = NULL;
    node_t *tmp = NULL;

    if (NULL == d || NULL == data)
    {
        printf("dlist_prepend NULL\n");
        goto err0;
    }

    //分配节点空间
    new_node = __malloc_node(d, data);
    if (NULL == new_node)
    {
        goto err0;
    }

    //指向第一个节点
    tmp = d->p;

    //链表为空的时候
    if (NULL == d->p)
    {
        d->p = new_node;
    }
    else
    {
        //new节点的后继
        new_node->next = tmp;
        //new节点的前驱
        new_node->prev = tmp->prev;

        tmp->prev->next = new_node;
        tmp->prev = new_node;
    }

    //d->p指向链表的第一个节点
    d->p = new_node;


    return 0;
err0:
    return -1;
}

//头插
int dlist_prepend_v1(dlist_t *d, void *data)
{
    dlist_append(d, data);

    //d->p指向新节点
    d->p = d->p->prev;

    return 0;

}

//尾插
int dlist_append(dlist_t *d, void *data)
{
    node_t *new_node = NULL;

    node_t *tmp = NULL;

    if (NULL == d || NULL == data)
    {
        printf("dlist_prepend NULL\n");
        goto err0;
    }

    //分配节点空间
    new_node = __malloc_node(d, data);
    if (NULL == new_node)
    {
        goto err0;
    }

    //指向第一个节点
    tmp = d->p;

    if (NULL == d->p)
    {
        d->p = new_node;
    }
    else
    {
        new_node->next = tmp;
        new_node->prev = tmp->prev;
        tmp->prev->next = new_node;
        tmp->prev = new_node;

    }

    return 0;
err0:
    return -1;
}

//返回元素的个数
int dlist_count(dlist_t *d)
{
    if (NULL == d)
    {
        printf("dlist_count NULL\n");
        goto err0;
    }

    return d->count;
err0:
    return -1;
}



//遍历
int dlist_traverse(dlist_t *d, op_t my_print)
{
    node_t *tmp = NULL;

    //参数检查
    if (NULL == d || NULL == my_print || NULL == d->p)
    {
        printf("dlist_traverse NULL\n");
        goto err0;
    }

    //指向链表中第一个节点
    tmp = d->p;

    //遍历链表
    do{
        my_print(tmp->data);

        tmp = tmp->next;

    }while(tmp != d->p);

    printf("\n");



    return 0;
err0:
    return -1;
}

//反向遍历
int dlist_back_traverse(dlist_t *d, op_t my_print)
{

    node_t *tmp = NULL;

    //参数检查
    if (NULL == d || NULL == my_print)
    {
        printf("dlist_traverse NULL\n");
        goto err0;
    }

    //指向链表中最后一个节点
    tmp = d->p->prev;

    //遍历链表
    do{
        my_print(tmp->data);

        tmp = tmp->prev;

    }while(tmp != d->p->prev);

    printf("\n");

    return 0;
err0:
    return -1;

}

//销毁
int dlist_destroy(dlist_t *d)
{
    node_t *tmp = NULL;
    node_t *save = NULL;

    if (NULL == d || NULL == d->p)
    {
        printf("dlist_destroy NULL\n");
        goto err0;
    }

    //指向链表第一个节点
    tmp = d->p;

    do{

        //保存下一个节点
        save = tmp->next;

        //数据空间
        free(tmp->data);
        //节点空间
        free(tmp);

        tmp = save;


    }while(tmp != d->p);


    d->p = NULL;

    return 0;
err0:
    return -1;
}


//通过索引插入新的节点
int dlist_insert_by_index(dlist_t *d, void *data, int index)
{
    int i = 0;
    node_t *tmp = NULL;
    node_t *new_node = NULL;

    if (NULL == d || NULL == data)
    {
        printf("dlist_insert_by_index NULL\n");
        goto err0;
    }

    //索引范围检查
    if (index < 0 || index > d->count)
    {
        printf("index out of range\n");
        goto err0;
    }


    //分配节点空间
    new_node = __malloc_node(d, data);
    if (NULL == new_node)
    {
        goto err0;
    }

    //指向第一个节点
    tmp = d->p;

    //链表为空的情形
    if (NULL == d->p)
    {
        d->p = new_node;
    }
    else
    {
        //头插
        if (index == 0)
        {
            new_node->next = tmp;
            new_node->prev = tmp->prev;
            tmp->prev->next = new_node;
            tmp->prev = new_node;

            d->p = new_node;

        }
        else
        {
            for (i = 0; i < index - 1; i++)
            {
                tmp = tmp->next;
            }

            new_node->next = tmp->next;
            new_node->prev = tmp;
            tmp->next->prev = new_node;
            tmp->next = new_node;
        }

    }


    return 0;
err0:
    return -1;
}


//通过索引删除链表的结点
int dlist_delete_by_index(dlist_t *d, int index)
{

    int i = 0;
    node_t *tmp = NULL;
    node_t *save = NULL;

    if (NULL == d || NULL == d->p)
    {
        printf("dlist_delete_by_index  NULL\n");
        goto err0;
    }

    //索引范围检查
    if (index < 0 || index >= d->count)
    {
        printf("index out of range\n");
        goto err0;

    }


    tmp = d->p;

    if (0 == index)
    {
        save = tmp;

        tmp->next->prev = tmp->prev;
        tmp->prev->next = tmp->next;

        //指向链表第二个节点
        d->p = tmp->next;

    }
    else
    {
        for (i = 0; i < index - 1; i++)
        {
            tmp = tmp->next;
        }

        save = tmp->next;

        tmp->next = save->next;
        save->next->prev = tmp;
    }

    d->count--;

    if (0 == d->count)
    {
        d->p = NULL;
    }

    //释放内存
    free(save->data);
    free(save);


    return 0;
err0:
    return -1;
}



//通过key检索链表节点数据
node_t *dlist_find_by_key(dlist_t *d, void *key, cmp_t my_cmp)
{
    int i = 0;
    node_t *tmp = NULL;

    //参数检查
    if (NULL == d || NULL == key || NULL == my_cmp
        || NULL == d->p)
    {
        printf("dlist_find_by_key NULL\n");
        goto err0;
    }

    //指向第一个节点
    tmp = d->p;

    do {
        if (my_cmp(tmp->data, key) == 0)
        {
            break;
        }

        i++;

        tmp = tmp->next;
    }while(tmp != d->p);

    //找不到的情形
    if (i == d->count)
    {
        goto err0;
    }


    return tmp;
err0:
    return NULL;
}

//通过key检索链表节点序号
int node_find_by_key(dlist_t *d, void *key, cmp_t my_cmp)
{
    int i = 0;
    node_t *tmp = NULL;

    //参数检查
    if (NULL == d || NULL == key || NULL == my_cmp
        || NULL == d->p)
    {
        printf("dlist_find_by_key NULL\n");
        goto err0;
    }

    //指向第一个节点
    tmp = d->p;

    do {
        if (my_cmp(tmp->data, key) == 0)
        {
            break;
        }

        i++;

        tmp = tmp->next;
    }while(tmp != d->p);

    //找不到的情形
    if (i == d->count)
    {
        goto err0;
    }

    return i;
err0:
    return 1;
}


//排序链表 选择排序
int dlist_select_sort(dlist_t *d,  cmp_t my_cmp)
{
    node_t *tmp_i;
    node_t *tmp_j;
    node_t *tmp_k;

    void *tmp_data = NULL;


    //参数检查
    if (NULL == d || NULL == my_cmp)
    {
        printf("dlist_select_sort NULL\n");
    }


    //分配空间
    tmp_data = malloc(d->size);
    if (NULL == tmp_data)
    {
        printf("malloc failed...\n");
        goto err0;
    }
    memset(tmp_data, 0, d->size);

    tmp_i = d->p;
    do{

        tmp_k = tmp_i;

        tmp_j = tmp_i->next;
        do{

            if (my_cmp(tmp_k->data, tmp_j->data) > 0)
            {
                tmp_k = tmp_j;
            }

            tmp_j = tmp_j->next;
        }while(tmp_j != d->p);



        if (tmp_k != tmp_i)
        {
            memcpy(tmp_data, tmp_k->data, d->size);
            memcpy(tmp_k->data, tmp_i->data, d->size);
            memcpy(tmp_i->data, tmp_data, d->size);

        }


        tmp_i = tmp_i->next;

    }while(tmp_i != d->p->prev);


    //释放内存
    free(tmp_data);

    return 0;
err0:
    return -1;
}













