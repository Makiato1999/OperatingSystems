#include "linkedList.h"

LinkedList *LinkedList_Create()
{
    LinkedList *list = malloc(sizeof(LinkedList));
    memset(list, 0, sizeof(LinkedList));
    list->length = 0;
    list->header.next = NULL;
    return list;
}

void LinkedList_Destory(LinkedList *list)
{
    if (list != NULL)
    {
        free(list);
        list = NULL;
    }
}

int LinkedList_Length(LinkedList *list)
{
    if (list != NULL)
    {
        return list->length;
    }
}

int LinkedList_Insert(LinkedList *list, LinkedListNode *node, int pos)
{
    if (list == NULL || pos < 0 || node == NULL)
    {
        return -1;
        fprintf(stderr, "Wrong argument.\n");
    }
    LinkedListNode *cur = &list->header;

    int i = 0;
    while (i < pos && cur->next != NULL)
    {
        cur = cur->next;
        i++;
    }
    node->next = cur->next;
    cur->next = node;
    list->length++;
    return 0;
}

LinkedListNode *LinkedList_Get(LinkedList *list, int pos)
{
    if (list == NULL || pos < 0)
    {
        return NULL;
        fprintf(stderr, "Wrong argument.\n");
    }
    LinkedListNode *cur = &list->header;
    int i = 0;
    while (i < pos && cur->next != NULL)
    {
        cur = cur->next;
        i++;
    }
    return cur->next;
}

LinkedListNode *LinkedList_Delete(LinkedList *list, int pos)
{
    if (list == NULL || pos < 0)
    {
        return -1;
        fprintf(stderr, "Wrong argument.\n");
    }
    LinkedListNode *cur = &list->header;
    int i = 0;
    while (i < pos && cur->next != NULL)
    {
        cur = cur->next;
        i++;
    }
    LinkedListNode *r = cur->next;
    cur->next = cur->next->next;
    list->length--;
    return r;
}
