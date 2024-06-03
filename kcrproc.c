/***************************************************************************************
 * Filename: kcrproc.c
 *
 * Description: General procedures for the KCR simulator.
 ***************************************************************************************/

#include <kcr.h>

/***************************************************************************************
 * Name: kcr_perform_simulation()
 *
 * Purpose: Perform the simulation.
 *
 * Parameters: IN    end_file - file for putting-out end locations
 *             IN    root_data - pointer to a CB containing all the root data for KCR.
 *
 * Returns: Nothing.
 *
 * Operation: Loop through the list of individuals, calling into the function that moves
 *            an individual and stores its position, resource and territorial cue data.  
 *            Repeat this process until root_data->total_time has passed.
 ***************************************************************************************/
void kcr_perform_simulation(FILE *end_file, KCR_ROOT_DATA *root_data)
{
	/* Local variables */
	KCR_INDIVIDUAL *curr_indiv_cb;
    KCR_POPULATION *curr_pop_cb;
    unsigned long counter;

    /* Sanity checks. Current time step should be 0. */
	assert(root_data != NULL);
	assert(root_data->current_time == 0);
	
	/* Loop through all the individuals, moving them according to the rules and 
     * updating the per-population mark information.
     * Repeat for each time step */
	while(root_data->current_time < root_data->total_time)
	{
        root_data->current_time++;
        curr_pop_cb = (KCR_POPULATION *)LIST_GET_FIRST(root_data->population_list_root);                        
        while(curr_pop_cb != NULL)
        {
            /* Go through individuals in current population, moving each */
			curr_indiv_cb = (KCR_INDIVIDUAL *)LIST_GET_FIRST(curr_pop_cb->individual_list_root);                        
            while(curr_indiv_cb != NULL)
            {
                /* Move the current individual */
                if(root_data->box_height == 1)
                {
                    kcr_move_individual1d(curr_indiv_cb, curr_pop_cb, root_data);
				}
				else
				{
                    kcr_move_individual(curr_indiv_cb, curr_pop_cb, root_data);
				}

                if((double)root_data->current_time >= root_data->start_measure_time)
                {
                	/* Print out locations of individuals */
                	printf("%i\t%i\t",curr_indiv_cb->current_x_pos,curr_indiv_cb->current_y_pos);
                	if(((double)root_data->current_time == root_data->total_time) && (end_file != NULL))
                	{
                		/* Last time step.  Print out end locations */
                		fprintf(end_file, "%i\t%i\t",curr_indiv_cb->current_x_pos,curr_indiv_cb->current_y_pos);
					}
			    }
			    
			    /* Individual cannot have moved outside the box */
                assert(curr_indiv_cb->current_x_pos >= 0);
                assert(curr_indiv_cb->current_y_pos >= 0);
                assert(curr_indiv_cb->current_x_pos < root_data->box_width);
                assert(curr_indiv_cb->current_y_pos < root_data->box_height);

                /* Get the next CB */
                curr_indiv_cb = (KCR_INDIVIDUAL *)LIST_GET_NEXT(curr_indiv_cb->list_elt);
            }

            /* Get next population */
            curr_pop_cb = (KCR_POPULATION *)LIST_GET_NEXT(curr_pop_cb->list_elt);
        }
        if((double)root_data->current_time >= root_data->start_measure_time)
        {
          	/* Gone through all populations: carriage return */
          	printf("\n");
           	if(((double)root_data->current_time == root_data->total_time) && (end_file != NULL))
           	{
           		/* Last time step.  Print out end locations */
           		fprintf(end_file, "\n");
			}
		}
    }
  
    /* Return */
    return;
}

/***************************************************************************************
 * Name: kcr_move_individual()
 *
 * Purpose: Move the individual.
 *
 * Parameters: IN/OUT individual - the individual
 *             IN     population - pointer to the population CB containing this individual
 *             IN     root_data - pointer to a CB containing all the root data for KCR.
 *
 * Returns: Nothing.
 *
 * Operation: Move individual and deposit marks
 ***************************************************************************************/
void kcr_move_individual(KCR_INDIVIDUAL *individual, 
                         KCR_POPULATION *population, 
						 KCR_ROOT_DATA *root_data)
{
	/* Local variables */
	double random;
	double up;
	double down;
	double left;
	double right;
	double sx;
	double sy;
	KCR_POPULATION *curr_pop_cb;
	KCR_INDIVIDUAL *curr_indiv_cb;
	double delta;
	double popsum;

    /* Sanity checks. */
	assert(root_data != NULL);
	assert(individual != NULL);
	assert(population != NULL);
	
    /* Calculate probabilities of moving up/down/left/right */
#ifdef KCR_PBC
	down = 1;
	up = 1;
	left = 1;
	right = 1;
#else /* KCR_PBC */
    if(individual->current_y_pos == 0)
    {
    	/* Cannot move down */
    	down = 0;
	}
	else
	{
    	/* Can move down */
		down = 1;
	}
    if(individual->current_y_pos == root_data->box_height - 1)
    {
    	/* Cannot move up */
    	up = 0;
	}
	else
	{
    	/* Can move up */
		up = 1;
	}
    if(individual->current_x_pos == 0)
    {
    	/* Cannot move left */
    	left = 0;
	}
	else
	{
    	/* Can move left */
		left = 1;
	}
    if(individual->current_x_pos == root_data->box_width - 1)
    {
    	/* Cannot move right */
    	right = 0;
	}
	else
	{
    	/* Can move right */
		right = 1;
	}
#endif /* KCR_PBC */

    /* Weights for going vertically and horizontally */
    sx = 0;
    sy = 0;
    popsum = 0;
    /* Go through populations counting number of animals within R_AA,R_AB,R_BA,R_BB of the current individual */
    curr_pop_cb = (KCR_POPULATION *)LIST_GET_FIRST(root_data->population_list_root);                        
    while(curr_pop_cb != NULL)
    {
        curr_indiv_cb = (KCR_INDIVIDUAL *)LIST_GET_FIRST(curr_pop_cb->individual_list_root);                        
        while(curr_indiv_cb != NULL)
        {
            delta = root_data->deltas[curr_pop_cb->index + population->index*root_data->no_pops];
        	if((pow(KCR_DIFF(curr_indiv_cb->current_x_pos,individual->current_x_pos,root_data->box_width)*root_data->l_val,2)+
			    pow(KCR_DIFF(curr_indiv_cb->current_y_pos,individual->current_y_pos,root_data->box_height)*root_data->l_val,2) <= pow(delta,2)) &&
			   (pow(KCR_DIFF(curr_indiv_cb->current_x_pos,individual->current_x_pos,root_data->box_width)*root_data->l_val,2)+
			    pow(KCR_DIFF(curr_indiv_cb->current_y_pos,individual->current_y_pos,root_data->box_height)*root_data->l_val,2) > 0))
			{
			    sx += (root_data->l_val*root_data->aijs[curr_pop_cb->index + population->index*root_data->no_pops]
			        *(1/(2*KCR_PI*pow(delta,2)))*KCR_DIFF(curr_indiv_cb->current_x_pos,individual->current_x_pos,root_data->box_width)/
					  sqrt(pow(KCR_DIFF(curr_indiv_cb->current_x_pos,individual->current_x_pos,root_data->box_width),2)+
			               pow(KCR_DIFF(curr_indiv_cb->current_y_pos,individual->current_y_pos,root_data->box_height),2)));
			    sy += (root_data->l_val*root_data->aijs[curr_pop_cb->index + population->index*root_data->no_pops]
			        *(1/(2*KCR_PI*pow(delta,2)))*KCR_DIFF(curr_indiv_cb->current_y_pos,individual->current_y_pos,root_data->box_height)/
					  sqrt(pow(KCR_DIFF(curr_indiv_cb->current_x_pos,individual->current_x_pos,root_data->box_width),2)+
			               pow(KCR_DIFF(curr_indiv_cb->current_y_pos,individual->current_y_pos,root_data->box_height),2)));
			}
			if((curr_indiv_cb->current_x_pos == individual->current_x_pos) && (curr_indiv_cb->current_y_pos == individual->current_y_pos))
			{
				/* Individuals are in the same place; increment popsum, storing sum of all populations at current point */
				popsum+=1/pow(root_data->l_val,2);
			}
        	curr_indiv_cb = (KCR_INDIVIDUAL *)LIST_GET_NEXT(curr_indiv_cb->list_elt);
        }
        curr_pop_cb = (KCR_POPULATION *)LIST_GET_NEXT(curr_pop_cb->list_elt);
    }

    if(root_data->packing_term == 1)
    {
    	/* We need to incorporate the packing */
    	sy /= (1+root_data->kappa*popsum);
    	sx /= (1+root_data->kappa*popsum);
	}
    sy = max(-1,min(1,sy));
    sx = max(-1,min(1,sx));
    up *= (1+sy)/4;
    down *= (1-sy)/4;
    right *= (1+sx)/4;
    left *= (1-sx)/4;
    
    /* Get a random number between 0 and up+down+left+right */
    assert(down<=1);
    assert(down>=0);
    assert(up<=1);
    assert(up>=0);
    assert(left<=1);
    assert(left>=0);
    assert(right<=1);
    assert(right>=0);
    random = ((double)rand())*(down+up+left+right)/((double)RAND_MAX);

   	/* Use this random number to determine next position */
   	if(random < down)
   	{
   		/* Move down */
#ifdef KCR_PBC
   		individual->current_y_pos = KCR_MOD(individual->current_y_pos - 1, root_data->box_height);
#else /* KCR_PBC */
   		individual->current_y_pos -= 1;
#endif /* KCR_PBC */
	}
	else if(random < down + up)
	{
   		/* Move up */
#ifdef KCR_PBC
   		individual->current_y_pos = KCR_MOD(individual->current_y_pos + 1, root_data->box_height);
#else /* KCR_PBC */
   		individual->current_y_pos += 1;
#endif /* KCR_PBC */
	}
	else if(random < down + up + left)
	{
   		/* Move left */
#ifdef KCR_PBC
   		individual->current_x_pos = KCR_MOD(individual->current_x_pos - 1, root_data->box_width);
#else /* KCR_PBC */
   		individual->current_x_pos -= 1;
#endif /* KCR_PBC */
	}
#ifdef KCR_PBC
   	else
   	{
   		/* Move right */
   		individual->current_x_pos = KCR_MOD(individual->current_x_pos + 1, root_data->box_width);
    }
#else /* KCR_PBC */
   	else if(individual->current_x_pos != root_data->box_width - 1)
   	{
   		/* Move right */
   		individual->current_x_pos += 1;
    }
#endif /* KCR_PBC */
   
    /* Return */
    return;
}

/***************************************************************************************
 * Name: kcr_move_individual1d()
 *
 * Purpose: Move the individual in a 1d environment.
 *
 * Parameters: IN/OUT individual - the individual
 *             IN     population - pointer to the population CB containing this individual
 *             IN     root_data - pointer to a CB containing all the root data for KCR.
 *
 * Returns: Nothing.
 *
 * Operation: Move individual and deposit marks
 ***************************************************************************************/
void kcr_move_individual1d(KCR_INDIVIDUAL *individual, 
                           KCR_POPULATION *population, 
						   KCR_ROOT_DATA *root_data)
{
	/* Local variables */
	double random;
	double left;
	double right;
	double sx;
	KCR_POPULATION *curr_pop_cb;
	KCR_INDIVIDUAL *curr_indiv_cb;

    /* Sanity checks. */
	assert(root_data != NULL);
	assert(individual != NULL);
	assert(population != NULL);
	
    /* Calculate probabilities of moving left/right */
#ifdef KCR_PBC
	left = 1;
	right = 1;
#else /* KCR_PBC */
    if(individual->current_x_pos == 0)
    {
    	/* Cannot move left */
    	left = 0;
	}
	else
	{
    	/* Can move left */
		left = 1;
	}
    if(individual->current_x_pos == root_data->box_width - 1)
    {
    	/* Cannot move right */
    	right = 0;
	}
	else
	{
    	/* Can move right */
		right = 1;
	}
#endif /* KCR_PBC */

    /* Weights for going horizontally */
    sx = 0;
    /* Go through populations counting number of animals within delta of the current individual */
    curr_pop_cb = (KCR_POPULATION *)LIST_GET_FIRST(root_data->population_list_root);                        
    while(curr_pop_cb != NULL)
    {
        curr_indiv_cb = (KCR_INDIVIDUAL *)LIST_GET_FIRST(curr_pop_cb->individual_list_root);                        
        while(curr_indiv_cb != NULL)
        {
        	if((KCR_DIFF(curr_indiv_cb->current_x_pos,individual->current_x_pos,root_data->box_width)*root_data->l_val <= 
			    root_data->deltas[curr_pop_cb->index + population->index*root_data->no_pops]) &&
			   (KCR_DIFF(curr_indiv_cb->current_x_pos,individual->current_x_pos,root_data->box_width)*root_data->l_val > 0))
			{
				/* Individual just to the right: increment sx */
			    sx += root_data->l_val*root_data->aijs[curr_pop_cb->index + population->index*root_data->no_pops]/(
				    4*root_data->deltas[curr_pop_cb->index + population->index*root_data->no_pops]);
			}
        	else if((KCR_DIFF(curr_indiv_cb->current_x_pos,individual->current_x_pos,root_data->box_width)*root_data->l_val >= 
			         -root_data->deltas[curr_pop_cb->index + population->index*root_data->no_pops]) &&
			        (KCR_DIFF(curr_indiv_cb->current_x_pos,individual->current_x_pos,root_data->box_width)*root_data->l_val < 0))
			{
				/* Individual just to the left: decrement sx */
			    sx -= root_data->l_val*root_data->aijs[curr_pop_cb->index + population->index*root_data->no_pops]/(
				    4*root_data->deltas[curr_pop_cb->index + population->index*root_data->no_pops]);
			}
        	curr_indiv_cb = (KCR_INDIVIDUAL *)LIST_GET_NEXT(curr_indiv_cb->list_elt);
        }
        curr_pop_cb = (KCR_POPULATION *)LIST_GET_NEXT(curr_pop_cb->list_elt);
    }
    sx = max(-1,min(1,sx));
    right *= (1+sx)/2;
    left *= (1-sx)/2;
    
    /* Get a random number between 0 and up+down+left+right */
    assert(left<=1);
    assert(left>=0);
    assert(right<=1);
    assert(right>=0);
    random = ((double)rand())*(left+right)/((double)RAND_MAX);

   	/* Use this random number to determine next position */
   	if(random < left)
	{
   		/* Move left */
#ifdef KCR_PBC
   		individual->current_x_pos = KCR_MOD(individual->current_x_pos - 1, root_data->box_width);
#else /* KCR_PBC */
   		individual->current_x_pos -= 1;
#endif /* KCR_PBC */
	}
#ifdef KCR_PBC
   	else
   	{
   		/* Move right */
   		individual->current_x_pos = KCR_MOD(individual->current_x_pos + 1, root_data->box_width);
    }
#else /* KCR_PBC */
   	else if(individual->current_x_pos != root_data->box_width - 1)
   	{
   		/* Move right */
   		individual->current_x_pos += 1;
    }
#endif /* KCR_PBC */

    /* y-positions should always be zero */
    individual->current_y_pos = 0;
   
    /* Return */
    return;
}
