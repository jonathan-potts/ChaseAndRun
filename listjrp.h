/***************************************************************************************
 * Filename: listjrp.h
 *
 * Description: Header file containing CBs, macros and preprocessor definitions
 *              used for list manipulation.
 ***************************************************************************************/

#ifndef __LISTJRP_H_
#define __LISTJRP_H_

/***************************************************************************************
 * Includes
 ***************************************************************************************/
#include <windows.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <trace.h>

/***************************************************************************************
 * Control blocks
 ***************************************************************************************/

/***************************************************************************************
 * Name: LIST_ELT
 *
 * Purpose: An element in a list.
 ***************************************************************************************/
typedef struct list_elt
{
	/***********************************************************************************
	 * Pointer to data.
	 ***********************************************************************************/
	void *data;

	/***********************************************************************************
	 * Pointer to next element.
	 ***********************************************************************************/
	struct list_elt *next;

} LIST_ELT;

/***************************************************************************************
 * Name: LIST_ROOT
 *
 * Purpose: Root of a list.  Points to the first element in the list.
 ***************************************************************************************/
typedef LIST_ELT * LIST_ROOT;

/***************************************************************************************
 * Function declarations.
 ***************************************************************************************/

/***************************************************************************************
 * list.c
 ***************************************************************************************/
void *list_get_first(LIST_ROOT);
void *list_get_next(LIST_ELT);

/***************************************************************************************
 * Macros
 ***************************************************************************************/

/***************************************************************************************
 * Create list.
 *
 * Parameters: ROOT - root of list
 ***************************************************************************************/
#define LIST_CREATE(ROOT) (ROOT) = NULL

/***************************************************************************************
 * Add element to start of list.
 *
 * Parameters: ROOT - root of the list
 *             ELT  - element of list to add
 *             DATA - pointer to the data pointed to by the list element
 ***************************************************************************************/
#define LIST_ADD_TO_START(ROOT, ELT, DATA) \
	 ((ELT).data = (DATA)); \
	 ((ELT).next = (ROOT)); \
	 ((ROOT) = &(ELT))

/***************************************************************************************
 * Delete first element from list.
 *
 * Parameters: ROOT - root of list
 ***************************************************************************************/
#define LIST_DELETE_FIRST(ROOT) \
	 assert((ROOT) != NULL); \
	 ((ROOT) = (ROOT)->next)

/***************************************************************************************
 * Get data stored in next element in list after a given element.
 *
 * Parameters: ELT - given list element
 *
 * Returns: Pointer to data contained in next element.  NULL if none is present.
 ***************************************************************************************/
#define LIST_GET_NEXT(ELT) list_get_next(ELT)

/***************************************************************************************
 * Get data stored in first element of given list.
 *
 * Parameters: ROOT - root of list
 *
 * Returns: Pointer to data stored in first element.  NULL if none is present.
 ***************************************************************************************/
#define LIST_GET_FIRST(ROOT) list_get_first(ROOT)

/***************************************************************************************
 * Check that a list is empty.
 ***************************************************************************************/
#define LIST_EMPTY(ROOT) ((ROOT) == NULL)

#endif /* __LISTJRP_H_ */
