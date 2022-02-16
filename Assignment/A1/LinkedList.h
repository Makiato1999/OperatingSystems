#ifndef LINKED_LIST_H
#define LINKED_LIST_H
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

typedef struct LinkedListNodeStruct
{
    struct LinkedListNodeStruct *next;
} LinkedListNode;

typedef struct LinkedListStruct
{
    LinkedListNode header;
    int length;
} LinkedList;

LinkedList *LinkedList_Create();

void LinkedList_Destory(LinkedList *list);

int LinkedList_Length(LinkedList *list);

int LinkedList_Insert(LinkedList *list, LinkedListNode *node, int pos);

LinkedListNode *LinkedList_Get(LinkedList *list, int pos);

LinkedListNode *LinkedList_Delete(LinkedList *list, int pos);
#endif