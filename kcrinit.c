/***************************************************************************************
 * Filename: kcrinit.c
 *
 * Description: Initialisation and termination procedures for the KCR simulator.
 ***************************************************************************************/

#include <kcr.h>

/***************************************************************************************
 * Name: kcr_init()
 *
 * Purpose: Allocate memory for CBs required for KCR simulator.  Set up initial CB
 *          constants.
 *
 * Parameters: IN     no_indivs - number of individuals per population.
 *             IN     no_pops - number of populations in simulation.
 *             IN     total_time - total time for the simulation.
 *             IN     start_measure_time - time to start measuring output values
 *             IN     aij_file - file containing a_ij values
 *             IN     box_width - width of box
 *             IN     box_height - height of box
 *             IN     delta_file - file containing delta parameters (local averaging radius)
 *             IN     l_val - lattice spacing
 *             IN     env_file - file containing data on the environment
 *             IN     env_weight - weighting given to the environmental layer
 *             IN     packing_term - set to 1 if there is a packing term; 0 if not
 *             IN     kappa - strength of packing 
 *
 * Returns: root_data - pointer to a CB containing all the root data for KCR.  If
 *                      any memory allocation fail then return NULL.
 *
 * Operation: Allocate memory for root_data.  Input initial values.  For each of
 *            no_indivs individuals, call into the initialisation function for that
 *            individual.
 ***************************************************************************************/
KCR_ROOT_DATA *kcr_init(unsigned short no_indivs,
                        unsigned short no_pops,
                        double total_time,
                        double start_measure_time,
                        FILE *aij_file,
                        unsigned long box_width,
                        unsigned long box_height,
						FILE *delta_file,
						double l_val,
						FILE *env_file,
						double env_weight,
						unsigned short packing_term,
						double kappa)
{
    /* Local variables */
    unsigned short curr_pop;
    KCR_POPULATION *curr_pop_cb;
    KCR_ROOT_DATA *root_data;
    unsigned short rc;
    unsigned long counter;

	/* Sanity checks. */
	assert(aij_file != NULL);
	
    /* Allocate memory for root data */
    root_data = (KCR_ROOT_DATA *)malloc(sizeof(KCR_ROOT_DATA));
	if(root_data == NULL)
	{
		fprintf(stderr,"MEMORY ALLOCATION FAILURE FOR ROOT_DATA\n");
		goto EXIT_LABEL;
	}

    /* Allocate memory for the a_ijs */
	root_data->aijs = (double *)calloc(no_pops*no_pops, sizeof(double));
	if(root_data->aijs == NULL)
	{
		fprintf(stderr,"MEMORY ALLOCATION FAILURE FOR ROOT_DATA->aijs\n");
		free(root_data);
		root_data = NULL;
		goto EXIT_LABEL;
    }

    /* Allocate memory for the deltas */
	root_data->deltas = (double *)calloc(no_pops*no_pops, sizeof(double));
	if(root_data->deltas == NULL)
	{
		fprintf(stderr,"MEMORY ALLOCATION FAILURE FOR ROOT_DATA->deltas\n");
		free(root_data->aijs);
		free(root_data);
		root_data = NULL;
		goto EXIT_LABEL;
    }

    /* Allocate memory for the environmental data */
	root_data->env_data = (double *)calloc(box_height*box_width,sizeof(double));
	if(root_data->env_data == NULL)
	{
		fprintf(stderr,"MEMORY ALLOCATION FAILURE FOR ROOT_DATA->env_data\n");
		free(root_data->aijs);
		free(root_data->deltas);
		free(root_data);
		root_data = NULL;
		goto EXIT_LABEL;
    }

    /* Initial conditions of all the variables stored on root */
    root_data->total_time = total_time;
    root_data->no_indivs = no_indivs;
    root_data->no_pops = no_pops;
    LIST_CREATE(root_data->population_list_root);
    root_data->start_measure_time = start_measure_time;
    root_data->current_time = 0;
    root_data->box_width = box_width;
    root_data->box_height = box_height;
    root_data->env_weight = env_weight;
    root_data->packing_term = packing_term;
    root_data->kappa = kappa;

    /* Set up aij-values */
    kcr_setup_array(aij_file, root_data, root_data->aijs);

    /* Set up delta-values */
    kcr_setup_array(delta_file, root_data, root_data->deltas);

    /* l_val */
    root_data->l_val = l_val;

    /* Put environmental data from file into CB */
    kcr_setup_env(env_file, root_data);

    /* Initialise populations */
    for(curr_pop = 0; curr_pop < no_pops; curr_pop++)
    {
    	curr_pop_cb = kcr_pop_init(curr_pop, root_data);
    	if(curr_pop_cb == NULL)
    	{
            fprintf(stderr,"Failed to initialise population %u\n", curr_pop);
            kcr_term(root_data);
            root_data = NULL;
            goto EXIT_LABEL;
        }
    }
    
EXIT_LABEL:
    /* Return pointer to the root data */
    return(root_data);
}

/***************************************************************************************
 * Name: kcr_pop_init()
 *
 * Purpose: Allocate memory for a population.  Set up initial CB values.
 *
 * Parameters: IN     index - index for the population
 *             IN     root_data - pointer to the per-KCR-simulator data.
 *
 * Returns: population - pointer to a CB containing all the per-population data.
 *
 * Operation: Allocate memory for population.  Add the population to the population-list
 *            rooted in root_data.  Input initial values.
 ***************************************************************************************/
KCR_POPULATION *kcr_pop_init(unsigned short index, KCR_ROOT_DATA *root_data)
{
    /* Local variables */
    KCR_POPULATION *population;
    unsigned short curr_indiv;
    KCR_INDIVIDUAL *curr_indiv_cb;

	/* Sanity checks.  The index must be between 0 and the number of individuals in the
	 * simulation.	 */
	assert(root_data != NULL);
	assert(index < root_data->no_pops);

	/* Allocate memory for the population CB */
	population = (KCR_POPULATION *)malloc(sizeof(KCR_POPULATION));
	if(population == NULL)
	{
		fprintf(stderr, "MEMORY ALLOCATION FAILURE FOR POPULATION\n");
		goto EXIT_LABEL;
	}

	/* Add population to list in root_data */
    LIST_ADD_TO_START(root_data->population_list_root,
         	          population->list_elt,
    	              population);

	/* Input initial values */
    population->index = index;

    /* Create the individual list */
    LIST_CREATE(population->individual_list_root);

    /* Initialise individuals */
    for(curr_indiv = 0; curr_indiv < root_data->no_indivs; curr_indiv++)
    {
    	curr_indiv_cb = kcr_indiv_init(curr_indiv, population, root_data);
    	if(curr_indiv_cb == NULL)
    	{
            fprintf(stderr,"Failed to initialise individual %u\n", curr_indiv);
            kcr_term(root_data);
            root_data = NULL;
            goto EXIT_LABEL;
        }
    }

EXIT_LABEL:
	/* Return pointer to the population */
	return(population);
}

/***************************************************************************************
 * Name: kcr_indiv_init()
 *
 * Purpose: Allocate memory for an individual.  Set up initial CB values.
 *
 * Parameters: IN     index - index for the individual
 *             IN     population - pointer to the population containing this individual
 *             IN     root_data - pointer to the per-KCR-simulator data.
 *
 * Returns: individual - pointer to a CB containing all the per-individual data.
 *
 * Operation: Allocate memory for individual.  Add the individual to the individual-list
 *            rooted in root_data.  Input initial values.
 ***************************************************************************************/
KCR_INDIVIDUAL *kcr_indiv_init(unsigned short index, KCR_POPULATION *population, KCR_ROOT_DATA *root_data)
{
    /* Local variables */
    KCR_INDIVIDUAL *individual;

	/* Sanity checks.  The index must be between 0 and the number of individuals in the
	 * simulation.	 */
	assert(root_data != NULL);
	assert(index < root_data->no_indivs);

	/* Allocate memory for the individual CB */
	individual = (KCR_INDIVIDUAL *)malloc(sizeof(KCR_INDIVIDUAL));
	if(individual == NULL)
	{
		fprintf(stderr, "MEMORY ALLOCATION FAILURE FOR INDIVIDUAL\n");
		goto EXIT_LABEL;
	}

	/* Add individual to list in populaiton CB */
    LIST_ADD_TO_START(population->individual_list_root,
         	          individual->list_elt,
    	              individual);

	/* Input initial values.  Put all positions to outside the box.  Correct positions will be allocated later */
    individual->index = index;
    individual->current_x_pos = root_data->box_width;
    individual->current_y_pos = root_data->box_height;

EXIT_LABEL:
	/* Return pointer to the individual */
	return(individual);
}

/***************************************************************************************
 * Name: kcr_setup_array()
 *
 * Purpose: Get array of doubles from file and store in dbl_array.
 *
 * Parameters: IN     in_file - pointer to file containing values as a matrix
 *             IN     root_data - pointer to a CB containing all the root data for KCR.
 *             IN     dbl_array - pointer to array for storing values
 *
 * Returns: rc - return code.  Either KCR_RC_OK if ok or KCR_RC_ERROR if error.
 *
 * Operation: Read in values from a file that lists a_ij-values as follows:
 *               a_11 a_12 ... a_1N
 *                  ...
 *                  ...
 *               a_N1 a_N2 ... a_NN
 *            where N=root_data->no_indivs and there is a tab (\t) between each value. 
 *            Here, a_ij is the response of individuals from population i to marks of j.
 ***************************************************************************************/
unsigned short kcr_setup_array(FILE *in_file, KCR_ROOT_DATA *root_data, double *dbl_array)
{
	/* Local variables */
	double curr_val = 0;
	char curr_char;
	char prev_char;
	unsigned long y_val = 0;
	unsigned long x_val = 0;
	double digit_after_dec = 0;
	unsigned short neg = KCR_NO;
	unsigned short rc = KCR_RC_OK;
    
	/* Sanity checks */
	assert(in_file != NULL);
	assert(root_data != NULL);
	assert(dbl_array != NULL);

    /* Get numbers from file */
    rewind(in_file);
	curr_char = getc(in_file);
	while(curr_char != EOF)
	{
		if((curr_char >= '0') && (curr_char <= '9') && (digit_after_dec == 0))
        { 
            /* This is a digit of a number */
            curr_val = curr_val*10+(double)(curr_char-'0');
        }
  		else if((curr_char >= '0') && (curr_char <= '9'))
        { 
            /* This is a digit of a number after the decimal point */
            curr_val += (double)(curr_char-'0')/pow(10,digit_after_dec);
            digit_after_dec++;
        }
        else if(curr_char == '-')
        {
            /* Negative number */
            neg = KCR_YES;
        }
        else if(curr_char == '.')
        {
            /* Decimal point. */
            digit_after_dec++;
        }
		else if((curr_char == '\n') && (prev_char >= '0') && (prev_char <= '9'))
		{
            /* End of row of numbers */
            assert(x_val < root_data->no_pops);
            assert(y_val < root_data->no_pops);
            if(neg == KCR_NO)
            {
                dbl_array[x_val+y_val*root_data->no_pops] = curr_val;
			}
			else
			{
                dbl_array[x_val+y_val*root_data->no_pops] = -curr_val;
			}
            curr_val = 0;
            y_val++;
            x_val = 0;
            digit_after_dec = 0;
            neg = KCR_NO;
        } 
		else if((curr_char == '\n') && (prev_char == '\t'))
		{
            /* End of row of numbers after a tab.  No need to store values in array.  Just
             * change x_val and y_val to reflect the fact that we are about to start a new
             * row.  */
            assert(x_val <= root_data->no_pops); /* could have x_val=no_pops due to tab */
            assert(y_val < root_data->no_pops);
            curr_val = 0;
            y_val++;
            x_val = 0;
            digit_after_dec = 0;
            neg = KCR_NO;
        } 
    	else if((curr_char == '\t') && (prev_char >= '0') && (prev_char <= '9'))
		{
            /* End of a number */
            assert(x_val < root_data->no_pops);
            assert(y_val < root_data->no_pops);
            if(neg == KCR_NO)
            {
                dbl_array[x_val+y_val*root_data->no_pops] = curr_val;
			}
			else
			{
                dbl_array[x_val+y_val*root_data->no_pops] = -curr_val;
			}
            curr_val = 0;
            x_val++;
            digit_after_dec = 0;
            neg = KCR_NO;
        }                            
        else
        {
            /* Unrecognised character.  Maybe benign.  No-op */
        }             
        prev_char = curr_char;              
    	curr_char = getc(in_file);
	}
	if((prev_char >= '0') && (prev_char <= '9'))
	{
		/* Got to the end of the file but not stored final value */
        assert(x_val < root_data->no_pops);
        assert(y_val < root_data->no_pops);
        if(neg == KCR_NO)
        {
            dbl_array[x_val+y_val*root_data->no_pops] = curr_val;
        }
		else
		{
            dbl_array[x_val+y_val*root_data->no_pops] = -curr_val;
		}
	}
	        
EXIT_LABEL:   
	/* Return */
	return(rc);
}

/***************************************************************************************
 * Name: kcr_set_init_conds()
 *
 * Purpose: Set up initial conditions for all variables
 *
 * Parameters: IN     start_file - file contining initial conditions
 *             IN     root_data - pointer to a CB containing all the root data for KCR.
 *
 * Returns: Nothing.
 *
 * Operation: Set up position data in individual CB from the start data in ROOT.
 *            Set the current_time_step in ROOT to 0.
 ***************************************************************************************/
 void kcr_set_init_conds(FILE *start_file, KCR_ROOT_DATA *root_data)
 {
	/* Local variables */
	KCR_INDIVIDUAL *curr_indiv_cb;
    KCR_POPULATION *curr_pop_cb;
	unsigned long x_val;
	unsigned long y_val;
 	unsigned long counter;
	unsigned long curr_val = 0;
	char curr_char;
	char prev_char;
	unsigned short xy_val;

	/* Sanity checks */
	assert(root_data != NULL);

    if(start_file == NULL)
    {
        /* Update positions on population and individual CBs based on random sampling. */
        curr_pop_cb = (KCR_POPULATION *)LIST_GET_FIRST(root_data->population_list_root);                        
        while(curr_pop_cb != NULL)
        {
            curr_indiv_cb = (KCR_INDIVIDUAL *)LIST_GET_FIRST(curr_pop_cb->individual_list_root);                        
            while(curr_indiv_cb != NULL)
            {
       	        curr_indiv_cb->current_x_pos = rand() % root_data->box_width;
                curr_indiv_cb->current_y_pos = rand() % root_data->box_height;

                /* Get next individual */
                curr_indiv_cb = (KCR_INDIVIDUAL *)LIST_GET_NEXT(curr_indiv_cb->list_elt);
            }
            /* Get next population */
            curr_pop_cb = (KCR_POPULATION *)LIST_GET_NEXT(curr_pop_cb->list_elt);
        }
	}
	else
	{
        /* Update positions on population and individual CBs based on file. */
    	rewind(start_file);
		curr_char = getc(start_file);
        curr_pop_cb = (KCR_POPULATION *)LIST_GET_FIRST(root_data->population_list_root);                        
        curr_indiv_cb = (KCR_INDIVIDUAL *)LIST_GET_FIRST(curr_pop_cb->individual_list_root);    
		xy_val = KCR_X;                    
		while(curr_char != EOF)
		{
			if((curr_char >= '0') && (curr_char <= '9'))
	        { 
	            /* This is a digit of a number */
	            curr_val = curr_val*10+(unsigned long)(curr_char-'0');
	        }
			else if(((curr_char == '\n') || (curr_char == '\t')) && (prev_char >= '0') && (prev_char <= '9'))
			{
	            /* End of number */
	            if(xy_val == KCR_X)
	            {
	            	/* Got an x-value */
					assert(curr_indiv_cb != NULL);
    	            curr_indiv_cb->current_x_pos = curr_val;
    	            curr_val = 0;
    	            xy_val = KCR_Y;
				}
				else
				{
					/* Got a y-value */
					assert(curr_indiv_cb != NULL);
					assert(xy_val == KCR_Y);
    	            curr_indiv_cb->current_y_pos = curr_val;					
    	            curr_val = 0;
    	            xy_val = KCR_X;
    	            
                    /* Get next individual */
                    curr_indiv_cb = (KCR_INDIVIDUAL *)LIST_GET_NEXT(curr_indiv_cb->list_elt);
    	            
    	            /* If next individual is NULL, get the next population */
    	            if(curr_indiv_cb == NULL)
    	            {
                        /* Get next population */
       					assert(curr_pop_cb != NULL);
                        curr_pop_cb = (KCR_POPULATION *)LIST_GET_NEXT(curr_pop_cb->list_elt);
                        if(curr_pop_cb != NULL)
                        {
                        	/* Get the first individual from this list */
                            curr_indiv_cb = (KCR_INDIVIDUAL *)LIST_GET_FIRST(curr_pop_cb->individual_list_root);                        
						}
						else
						{
							/* Run out of populations.  Break. */
							break;
						}
					}
				}
	        } 
	        prev_char = curr_char;              
	    	curr_char = getc(start_file);
		}
		if((prev_char >= '0') && (prev_char <= '9'))
		{
			/* Got to the end of the file but not stored final value */
            if((xy_val == KCR_X) && (curr_indiv_cb != NULL))
            {
            	/* Got an x-value */
   	            curr_indiv_cb->current_x_pos = curr_val;
   	            xy_val = KCR_Y;
			}
			else if(curr_indiv_cb != NULL) 
			{
				/* Got a y-value */
				assert(xy_val == KCR_Y);
   	            curr_indiv_cb->current_y_pos = curr_val;					
            }
		}
	}
    
    /* Set initial time in root data */
    root_data->current_time = 0;
   
    /* Return */
	return;
}

/***************************************************************************************
 * Name: kcr_term()
 *
 * Purpose: Free all memory allocated in kcr_init.
 *
 * Parameters: IN     root_data - pointer to a CB containing all the root data for KCR.
 *
 * Returns: Nothing.
 ***************************************************************************************/
void kcr_term(KCR_ROOT_DATA *root_data)
{
    /* Local variables */
    KCR_POPULATION *curr_pop_cb;

	/* Sanity checks. */
	assert(root_data != NULL);

    /* Free up populations */		
    if(LIST_EMPTY(root_data->population_list_root))
    {
    	free(root_data);
    	goto EXIT_LABEL;
    }
    curr_pop_cb = (KCR_POPULATION *)LIST_GET_FIRST(root_data->population_list_root);
    while(curr_pop_cb != NULL)
    {
        /* Delete the first element on the list.  Then free the associated control block.
         * Then get the new first element. */
        LIST_DELETE_FIRST(root_data->population_list_root);
    	kcr_pop_term(curr_pop_cb);
    	curr_pop_cb = (KCR_POPULATION *)LIST_GET_FIRST(root_data->population_list_root);
    }

    /* Free root */
	free(root_data);
        
EXIT_LABEL:
    /* Return */
    return;
}

/***************************************************************************************
 * Name: kcr_pop_term()
 *
 * Purpose: Free all memory allocated in kcr_pop_init().
 *
 * Parameters: IN     population - pointer to per-population data.
 *
 * Returns: Nothing.
 *
 * Operation: Free per-population memory.
 ***************************************************************************************/
void kcr_pop_term(KCR_POPULATION *population)
{
	/* Local variables */
    KCR_INDIVIDUAL *curr_indiv_cb;

	/* Sanity checks */
	assert(population != NULL);

    curr_indiv_cb = (KCR_INDIVIDUAL *)LIST_GET_FIRST(population->individual_list_root);
    while(curr_indiv_cb != NULL)
    {
        /* Delete the first element on the list.  Then free the associated control block.
         * Then get the new first element. */
        LIST_DELETE_FIRST(population->individual_list_root);
    	kcr_indiv_term(curr_indiv_cb);
    	curr_indiv_cb = (KCR_INDIVIDUAL *)LIST_GET_FIRST(population->individual_list_root);
    }

	/* Free up the memory allocated for the individual control block */
	free(population);

	/* Return */
	return;
}

/***************************************************************************************
 * Name: kcr_indiv_term()
 *
 * Purpose: Free all memory allocated in kcr_indiv_init().
 *
 * Parameters: IN     individual - pointer to per-individual data.
 *
 * Returns: Nothing.
 *
 * Operation: Free per-individual memory.
 ***************************************************************************************/
void kcr_indiv_term(KCR_INDIVIDUAL *individual)
{
	/* Local variables */

	/* Sanity checks */
	assert(individual != NULL);

	/* Free up the memory allocated for the individual control block */
	free(individual);

	/* Return */
	return;
}

/***************************************************************************************
 * Name: kcr_setup_env()
 *
 * Purpose: Put environmental data from file into array
 *
 * Parameters: IN     env_file - file contining environmental data
 *             IN     root_data - pointer to a CB containing all the root data for KCR.
 *
 * Returns: Nothing.
 *
 * Operation: If the file is NULL then put zeros in the environmental data array.  Else
 *            populate the array with numbers from file.
 ***************************************************************************************/
void kcr_setup_env(FILE *env_file, KCR_ROOT_DATA *root_data)
{
	/* Local variables */
	double curr_val = 0;
	char curr_char;
	char prev_char;
	unsigned long y_val;
	unsigned long x_val;
	double digit_after_dec = 0;
	unsigned short neg = KCR_NO;
	unsigned short rc = KCR_RC_OK;

	/* Sanity checks */
	assert(root_data != NULL);

  	/* Populate environmental data array with default values of zero */
   	for(y_val = 0; y_val < root_data->box_height; y_val++)
	{
       	for(x_val = 0; x_val < root_data->box_width; x_val++)
    	{
		    root_data->env_data[y_val*root_data->box_width+x_val] = 0;
	    }
	}
	x_val = 0;
	y_val = 0;
    if(env_file != NULL)
	{
		/* Populate environmental data array with values from file */
        /* Get numbers from file */
        rewind(env_file);
	    curr_char = getc(env_file);
	    while(curr_char != EOF)
	    {
    		if((curr_char >= '0') && (curr_char <= '9') && (digit_after_dec == 0))
            { 
                /* This is a digit of a number */
                curr_val = curr_val*10+(double)(curr_char-'0');
            }
      		else if((curr_char >= '0') && (curr_char <= '9'))
            { 
                /* This is a digit of a number after the decimal point */
                curr_val += (double)(curr_char-'0')/pow(10,digit_after_dec);
                digit_after_dec++;
            }
            else if(curr_char == '-')
            {
                /* Negative number */
                neg = KCR_YES;
            }
            else if(curr_char == '.')
            {
                /* Decimal point. */
                digit_after_dec++;
            }
    		else if((curr_char == '\n') && (prev_char >= '0') && (prev_char <= '9'))
    		{
                /* End of row of numbers */
                assert(x_val < root_data->box_width);
                assert(y_val < root_data->box_height);
                if(neg == KCR_NO)
                {
                    root_data->env_data[x_val+y_val*root_data->box_width] = curr_val;
    			}
    			else
    			{
                    root_data->env_data[x_val+y_val*root_data->box_width] = -curr_val;
    			}
                curr_val = 0;
                y_val++;
                x_val = 0;
                digit_after_dec = 0;
                neg = KCR_NO;
            } 
    		else if((curr_char == '\n') && (prev_char == '\t'))
    		{
                /* End of row of numbers after a tab.  No need to store values in array.  Just
                 * change x_val and y_val to reflect the fact that we are about to start a new
                 * row.  */
                assert(x_val <= root_data->box_width); /* could have x_val=box_width due to tab */
                assert(y_val < root_data->box_height);
                curr_val = 0;
                y_val++;
                x_val = 0;
                digit_after_dec = 0;
                neg = KCR_NO;
            } 
        	else if((curr_char == '\t') && (prev_char >= '0') && (prev_char <= '9'))
    		{
                /* End of a number */
                assert(x_val < root_data->box_width);
                assert(y_val < root_data->box_height);
                if(neg == KCR_NO)
                {
                    root_data->env_data[x_val+y_val*root_data->box_width] = curr_val;
    			}
    			else
    			{
                    root_data->env_data[x_val+y_val*root_data->box_width] = -curr_val;
    			}
                curr_val = 0;
                x_val++;
                digit_after_dec = 0;
                neg = KCR_NO;
            }                            
            else
            {
                /* Unrecognised character.  Maybe benign.  No-op */
            }             
            prev_char = curr_char;              
        	curr_char = getc(env_file);
    	}
    	if((prev_char >= '0') && (prev_char <= '9'))
    	{
    		/* Got to the end of the file but not stored final value */
            assert(x_val < root_data->box_width);
            assert(y_val < root_data->box_height);
            if(neg == KCR_NO)
            {
                root_data->env_data[x_val+y_val*root_data->box_width] = curr_val;
            }
    		else
    		{
                root_data->env_data[x_val+y_val*root_data->box_width] = -curr_val;
    		}
    	}
	}

	/* Return */
	return;
}
