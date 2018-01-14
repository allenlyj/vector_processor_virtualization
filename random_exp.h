# ifndef __RANDOM_EXP
# define __RANDOM_EXP
# include <stdlib.h>
# include <stdio.h>
# include "vp_control_int.h"

typedef struct random_req toss;
typedef struct fra_state frag_state;

struct random_req
{
	int req;
	int vector_length;
	int num_reg;
	int thread_id;
};

struct fra_state
{
	int fra_32;
	int fra_64;
};


void do_toss (toss *my_seed, int num_thread);
void measure_frag (vp_control *vp, frag_state *frag);
void rearrange (int *a);

#endif 