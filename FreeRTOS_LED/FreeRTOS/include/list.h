/*
 * FreeRTOS Kernel V10.4.3
 * list.h - Linked list implementation used by the scheduler
 */

#ifndef LIST_H
#define LIST_H

#include <stdlib.h>
#include "FreeRTOS.h"

struct xLIST_ITEM
{
    TickType_t              xItemValue;
    struct xLIST_ITEM *     pxNext;
    struct xLIST_ITEM *     pxPrevious;
    void *                  pvOwner;
    struct xLIST *          pvContainer;
};
typedef struct xLIST_ITEM ListItem_t;

struct xMINI_LIST_ITEM
{
    TickType_t              xItemValue;
    struct xLIST_ITEM *     pxNext;
    struct xLIST_ITEM *     pxPrevious;
};
typedef struct xMINI_LIST_ITEM MiniListItem_t;

typedef struct xLIST
{
    UBaseType_t     uxNumberOfItems;
    ListItem_t *    pxIndex;
    MiniListItem_t  xListEnd;
} List_t;

/* Macros */
#define listSET_LIST_ITEM_OWNER( pxListItem, pxOwner ) \
    ( ( pxListItem )->pvOwner = ( void * ) ( pxOwner ) )

#define listGET_LIST_ITEM_OWNER( pxListItem ) \
    ( ( pxListItem )->pvOwner )

#define listSET_LIST_ITEM_VALUE( pxListItem, xValue ) \
    ( ( pxListItem )->xItemValue = ( xValue ) )

#define listGET_LIST_ITEM_VALUE( pxListItem ) \
    ( ( pxListItem )->xItemValue )

#define listGET_ITEM_VALUE_OF_HEAD_ENTRY( pxList ) \
    ( ( ( pxList )->xListEnd ).pxNext->xItemValue )

#define listGET_HEAD_ENTRY( pxList ) \
    ( ( ( pxList )->xListEnd ).pxNext )

#define listGET_NEXT( pxListItem ) \
    ( ( pxListItem )->pxNext )

#define listGET_END_MARKER( pxList ) \
    ( ( ListItem_t const * ) ( &( ( pxList )->xListEnd ) ) )

#define listLIST_IS_EMPTY( pxList ) \
    ( ( BaseType_t ) ( ( pxList )->uxNumberOfItems == ( UBaseType_t ) 0 ) )

#define listCURRENT_LIST_LENGTH( pxList ) \
    ( ( pxList )->uxNumberOfItems )

#define listGET_OWNER_OF_NEXT_ENTRY( pxTCB, pxList )                       \
{                                                                            \
    List_t * const pxConstList = ( pxList );                                \
    ( pxConstList )->pxIndex = ( pxConstList )->pxIndex->pxNext;            \
    if( ( void * ) ( pxConstList )->pxIndex ==                              \
        ( void * ) &( ( pxConstList )->xListEnd ) )                         \
    {                                                                        \
        ( pxConstList )->pxIndex = ( pxConstList )->pxIndex->pxNext;        \
    }                                                                        \
    ( pxTCB ) = ( pxConstList )->pxIndex->pvOwner;                          \
}

#define listGET_OWNER_OF_HEAD_ENTRY( pxList ) \
    ( ( &( ( pxList )->xListEnd ) )->pxNext->pvOwner )

#define listIS_CONTAINED_WITHIN( pxList, pxListItem ) \
    ( ( BaseType_t ) ( ( pxListItem )->pvContainer == ( void * ) ( pxList ) ) )

#define listLIST_ITEM_CONTAINER( pxListItem ) \
    ( ( pxListItem )->pvContainer )

#define listLIST_IS_INITIALISED( pxList ) \
    ( ( pxList )->xListEnd.xItemValue == portMAX_DELAY )

/* Functions */
void vListInitialise( List_t * const pxList );
void vListInitialiseItem( ListItem_t * const pxItem );
void vListInsert( List_t * const pxList, ListItem_t * const pxNewListItem );
void vListInsertEnd( List_t * const pxList, ListItem_t * const pxNewListItem );
UBaseType_t uxListRemove( ListItem_t * const pxItemToRemove );

#endif /* LIST_H */
