/***************************************************************************************
 * Filename: list.c
 *
 * Description: Functions for list operations.
 ***************************************************************************************/

#include <listjrp.h>

/***************************************************************************************
 * Name: list_get_first()
 *
 * Purpose: Get data stored in first element of given list.
 *
 * Parameters: IN    root - root of list
 *
 * Returns: Pointer to data stored in first element.  NULL if none is present.
 *
 * Operation: If the root is NULL then return NULL.  Otherwise, return a pointer to the 
 *            data stored in the first list element.
 ***************************************************************************************/
void *list_get_first(LIST_ROOT root)
{
	/* Local variables */
	void *data;

	TRACE_START("list_get_first")
	TRACE_TEXT_VAL("Root is %p", root)

    /* Sanity checks. */
    
    if(root == NULL)
    {
        /* Root is NULL.  Set data to NULL. */
        TRACE_TEXT("Root is NULL")
        data = NULL;
    }
    else 
    {
        /* Return the data stored in the root */         
        TRACE_TEXT_VAL("Found CB %p", root->data)
        data = (void *)root->data;
    }
     
    TRACE_END
    /* Return */
    return(data);     
}
 
/***************************************************************************************
 * Name: list_get_next()
 *
 * Purpose: Get data stored in next element in list after a given element.
 *
 * Parameters: IN    element - given list element
 *
 * Returns: Pointer to data contained in next element.  NULL if none is present.
 *
 * Operation: If the next list element is NULL then return NULL.  Otherwise, return a
 *            pointer to the data stored in the first list element.
 ***************************************************************************************/
void *list_get_next(LIST_ELT element)
{
	/* Local variables */
	void *data;

	TRACE_START("list_get_next")

    /* Sanity checks. */
    
    if(element.next == NULL)
    {
        /* Next element is NULL.  Set data to NULL. */
        TRACE_TEXT("Next element is NULL")
        data = NULL;
    }
    else 
    {
        /* Return the data stored in the next element */   
        TRACE_TEXT_VAL("Next element is %p", element.next)      
        TRACE_TEXT_VAL("Data in next element is stored at %p", (element.next)->data)
        data = (void *)(element.next)->data;
    }
     
    TRACE_END
    /* Return */
    return(data);     
}
