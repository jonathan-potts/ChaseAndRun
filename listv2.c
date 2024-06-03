/***************************************************************************************
 * Filename: listv2.c
 *
 * Description: Functions for list v2 operations (list v2 is like the usual list but 
 *              also contains a pointer to the previous element).
 ***************************************************************************************/

#include <listv2.h>

/***************************************************************************************
 * Name: list_v2_get_first()
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
void *list_v2_get_first(LIST_V2_ROOT root)
{
	/* Local variables */
	void *data;

	TRACE_START("list_v2_get_first")
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
 * Name: list_v2_get_next()
 *
 * Purpose: Get data stored in next element in list after a given element.
 *
 * Parameters: IN    element - given list element
 *
 * Returns: Pointer to data contained in next element.  NULL if none is present.
 *
 * Operation: If the next list element is NULL then return NULL.  Otherwise, return a
 *            pointer to the data stored in the next list element.
 ***************************************************************************************/
void *list_v2_get_next(LIST_V2_ELT element)
{
	/* Local variables */
	void *data;

	TRACE_START("list_v2_get_next")

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

/***************************************************************************************
 * Name: list_v2_get_previous()
 *
 * Purpose: Get data stored in previous element in list after a given element.
 *
 * Parameters: IN    element - given list element
 *
 * Returns: Pointer to data contained in previous element.  NULL if none is present.
 *
 * Operation: If the previous list element is NULL then return NULL.  Otherwise, return a
 *            pointer to the data stored in the previous list element.
 ***************************************************************************************/
void *list_v2_get_previous(LIST_V2_ELT element)
{
	/* Local variables */
	void *data;

	TRACE_START("list_v2_get_previous")

    /* Sanity checks. */
    
    if(element.previous == NULL)
    {
        /* Next element is NULL.  Set data to NULL. */
        TRACE_TEXT("Previous element is NULL")
        data = NULL;
    }
    else 
    {
        /* Return the data stored in the previous element */   
        TRACE_TEXT_VAL("Previous element is %p", element.previous)      
        TRACE_TEXT_VAL("Data in previous element is stored at %p", (element.previous)->data)
        data = (void *)(element.previous)->data;
    }
     
    TRACE_END
    /* Return */
    return(data);     
}

/***************************************************************************************
 * Name: list_v2_delete_current()
 *
 * Purpose: Delete given element.
 *
 * Parameters: IN     root - the root of the list
 *             IN     element - given list element
 *
 * Returns: Nothing
 *
 * Operation: Delete the element from the list.  Reset next/previous pointers as necessary.
 ***************************************************************************************/
void list_v2_delete_current(LIST_V2_ROOT *root, LIST_V2_ELT element)
{
	/* Local variables */

	TRACE_START("list_v2_delete_current")

    /* Sanity checks. */
    
    if(element.previous != NULL)
    {
        /* Set the next-of-the-previous to the next-of-the-current */   
        TRACE_TEXT_VAL("Previous element is %p", element.previous)      
        TRACE_TEXT_VAL("Data in previous element is stored at %p", (element.previous)->data)
	    (element.previous)->next = element.next; 
    }
    else
    {
        /* The next element must be the root element of the list */
        *root = element.next;
    }
     
    if(element.next != NULL)
    {
        /* Set the previous-of-the-next to the previous-of-the-current */   
        TRACE_TEXT_VAL("Next element is %p", element.next)      
        TRACE_TEXT_VAL("Data in next element is stored at %p", (element.next)->data)
	    (element.next)->previous = element.previous; 
    }
     
    TRACE_END
    /* Return */
    return;     
}

/***************************************************************************************
 * Name: list_v2_add_to_start()
 *
 * Purpose: Add element to start of list.
 *
 * Parameters: IN     root - the root of the list
 *             IN     element - given list element
 *             IN     data - data to store in list
 *
 * Returns: Nothing
 *
 * Operation: Add the element to start of list.  Reset next/previous pointers as necessary.
 ***************************************************************************************/
void list_v2_add_to_start(LIST_V2_ROOT *root, LIST_V2_ELT *element, void *data)
{
	/* Local variables */

	TRACE_START("list_v2_add_to_start")

    /* Sanity checks. */
    assert(root != NULL);
    assert(element != NULL);
    
	element->data = data; 
	element->next = *root; 
	element->previous = NULL; 
     
    if(element->next != NULL)
    {
        /* Set the previous-of-the-next to the previous-of-the-current */   
        TRACE_TEXT_VAL("Next element is %p", element->next)      
        TRACE_TEXT_VAL("Data in next element is stored at %p", (element->next)->data)
	    (element->next)->previous = element; 
    }
	*root = element;
     
    TRACE_END
    /* Return */
    return;     
}

