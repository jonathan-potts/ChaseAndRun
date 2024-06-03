/***************************************************************************************
 * Filename: kcrmain.c
 *
 * Description: Contains main entry-point into KCR simulator.
 ***************************************************************************************/

#include <kcr.h>

/***************************************************************************************
 * Name: main()
 *
 * Purpose: Main function into KCR
 *
 * Parameters: (see usage statement below)
 *
 * Returns: zero
 ***************************************************************************************/
int main(int argc, char** argv)
{
	/* Local variables. */	
	double total_time;
    unsigned short no_indivs;
    unsigned short no_pops;
    unsigned long box_width;
    unsigned long box_height;
    KCR_ROOT_DATA *root_data;
	KCR_INDIVIDUAL *curr_indiv_cb;
	KCR_POPULATION *curr_pop_cb;
    unsigned long curr_arg;
    FILE *aij_file;
    FILE *start_file;
    FILE *end_file;
    double start_measure_time;
    unsigned int rseed;
    FILE *delta_file;
    double l_val;
    time_t current_time;
    char *c_time_string;
    FILE *env_file;
    double env_weight;
    FILE *mark_resp_file;
    unsigned short packing_term;
    double kappa;
 
    /* If no arguments then print usage statement */
	if(argc == 1)
	{
		printf("Usage: kcr.exe [-i <number-of-individuals> (default = 4)]\n");
		printf("               [-p <number-of-populations> (default = 2)]]\n");
		printf("               [-tt <total-time> (default = 100000)]\n");
		printf("               [-smt <start-measure-time> (default = 0)]\n");
		printf("               [-af <aij-file>]\n");
		printf("               [-bw <box-width> (default = 100)]\n");
		printf("               [-bh <box-height> (default = 100)]\n");
		printf("               [-df <delta-file>]\n");
		printf("               [-l <lattice spacing> (default = 0.1)]\n");
		printf("               [-r <random seed> (default = 0)]\n");
		printf("               [-ew <environment-weighting> (default = 0)]\n");
		printf("               [-sf <start-file> (default = NULL)]\n");
		printf("               [-ef <end-file> (default = NULL)]\n");
		printf("               [-edf <environmental-data-file> (default = NULL)]\n");
		printf("               [-pck <packing-term> (default = 0)]\n");
		printf("               [-kap <kappa> (default = 1)]\n");
		goto EXIT_LABEL;
	}
	
	/* Default values */
	no_indivs = 4;
	no_pops = 2;
	total_time = 100000;
    aij_file = NULL;
    start_measure_time = 0;
    box_width = 50;
    box_height = 50;
	rseed = 0;
	env_weight = 0;
	l_val = 0.1;
	start_file = NULL;
	end_file = NULL;
	env_file = NULL;
    mark_resp_file = NULL;
    kappa = 1;
	
	/* Process arguments */
    for(curr_arg = 1; curr_arg < argc; curr_arg++)
    {
        if(!strcmp(argv[curr_arg], "-i"))
        {
            /* No of individuals per population */                      
         	no_indivs = atoi(argv[++curr_arg]);
        }
        else if(!strcmp(argv[curr_arg], "-p"))
        {
            /* No of populations */                      
         	no_pops = atoi(argv[++curr_arg]);
        }
        else if(!strcmp(argv[curr_arg], "-tt"))
        {
            /* Total time */
        	total_time = atof(argv[++curr_arg]);
        }
        else if(!strcmp(argv[curr_arg], "-smt"))
        {
            /* Time to start measuring */ 
         	start_measure_time = atof(argv[++curr_arg]);
        }
        else if(!strcmp(argv[curr_arg], "-af"))
        {
            /* File containing a_ij values: detail the extent to which animals of 
			 * population i move towards or away from marks of population j */
        	aij_file = fopen(argv[++curr_arg],"r");
        }
        else if(!strcmp(argv[curr_arg], "-sf"))
        {
            /* File containing start locations.  Contains one row of the form: 
			 *   x_00 y_00 x_01 y_01 ... x_0N y_0N x_10 y_10 x_11 y_11 ... x_1N y_1N ... ... x_n0 y_n0 x_n1 y_n1 ... x_nN y_nN
             * where n is the number of populations and N the number of individuals per population */
        	start_file = fopen(argv[++curr_arg],"r");
        }
        else if(!strcmp(argv[curr_arg], "-ef"))
        {
            /* File for putting end locations in */
        	end_file = fopen(argv[++curr_arg],"w");
        }
        else if(!strcmp(argv[curr_arg], "-edf"))
        {
            /* File containing environmental data */
        	env_file = fopen(argv[++curr_arg],"r");
        }
        else if(!strcmp(argv[curr_arg], "-bw"))
        {
            /* Box width */ 
         	box_width = atol(argv[++curr_arg]);
        }
        else if(!strcmp(argv[curr_arg], "-bh"))
        {
            /* Box height */ 
         	box_height = atol(argv[++curr_arg]);
        }
        else if(!strcmp(argv[curr_arg], "-df"))
        {
            /* File storing delta parameter values (spatial averaging radius) */ 
         	delta_file = fopen(argv[++curr_arg],"r");
        }
        else if(!strcmp(argv[curr_arg], "-l"))
        {
            /* lattice spacing */ 
         	l_val = atof(argv[++curr_arg]);
        }
        else if(!strcmp(argv[curr_arg], "-ew"))
        {
            /* Weighting given to the environmental layer */ 
         	env_weight = atof(argv[++curr_arg]);
        }
        else if(!strcmp(argv[curr_arg], "-r"))
        {
            /* Random seed */ 
         	rseed = atoi(argv[++curr_arg]);
        }
        else if(!strcmp(argv[curr_arg], "-pck"))
        {
            /* Packing constant */ 
         	packing_term = atoi(argv[++curr_arg]);
        }
        else if(!strcmp(argv[curr_arg], "-kap"))
        {
            /* Strength of packing constant */ 
         	kappa = atof(argv[++curr_arg]);
        }
        else
        {
            /* Unrecognised parameter */
            fprintf(stderr,"Error: unrecognised parameter: %s\n", argv[curr_arg]);
            goto EXIT_LABEL;
        }
	}
                       
	/* Check a_ij file exists.  Else exit. */
	if(aij_file == NULL)
	{
        fprintf(stderr, "Error: no file for storing a_ij values\n");
        goto EXIT_LABEL;
    }
	
	/* Initialise random seed. */
	if(rseed == 0)
	{
     	srand(time(NULL));
	}
	else
	{
        srand(rseed);
	}

	/* Initialisation: Enter values into CBs and allocate memory where necessary */
    root_data = kcr_init(no_indivs,
                         no_pops,
                         total_time,
                         start_measure_time,
                         aij_file,
                         box_width,
                         box_height,
						 delta_file,
						 l_val,
						 env_file,
						 env_weight,
						 packing_term,
						 kappa);
		
	if(root_data == NULL)
	{
		printf("Memory allocation failure\n");
		goto EXIT_LABEL;
	}

	/* Close the various files */
	if(aij_file != NULL)
	{
		fclose(aij_file);
	}

    kcr_set_init_conds(start_file, root_data);
    current_time = time(NULL);
    c_time_string = ctime(&current_time);
    fprintf(stderr,"Initial conditions set up on %s", c_time_string);                 
    kcr_perform_simulation(end_file, root_data);
    current_time = time(NULL);
    c_time_string = ctime(&current_time);
    fprintf(stderr,"Simulation finished on %s", c_time_string);                 
	
	/* Free memory allocated */
	kcr_term(root_data);

EXIT_LABEL:
	TRACE_END
	return(0);
}
