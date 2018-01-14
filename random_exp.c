# include "random_exp.h"

// Random toss to decide request 
void do_toss (toss *my_seed, int num_thread)
{
	int ran;
	if(num_thread <3 ) // When no thread occupied, always do request
	{
		my_seed->req = 1;
	}
	else
	{
		my_seed->thread_id = rand()%num_thread;  // Only toss for id to be released when a thread is occupied
		if(num_thread == 4)
			my_seed->req =0;
		else
			my_seed->req = rand()%2;  
	}

	// Always toss for VL, num_reg just in case no thread occupied
	ran = rand() % 3;							
	switch (ran)
	{
	case 0: 
		my_seed->vector_length = 16;
		my_seed->num_reg = rand()%32 +1;
		break;
	case 1:
		my_seed->vector_length = 32;
		my_seed->num_reg = rand()%32 +1;
		break;
	case 2:
		my_seed->vector_length = 64;
		my_seed->num_reg = rand()%16 +1;
		break;
	}
}

// Measure current fragmentation
void measure_frag (vp_control *vp, frag_state *frag)
{
	frag->fra_32 = vp->avail_16/2 - vp->avail_32;
	frag->fra_64 = vp->avail_16/4 - vp->avail_64;
}

// Rearrange the occupied thread id index arrays
void rearrange (int *a)
{
	int b[4];
	int i,j;
	for(i=0;i<4;i++)
	{
		b[i] = -1;
	}

	for(i=0,j=0;i<4;i++)
	{
		if(a[i] != -1)
		{
			b[j]=a[i];
			j++;
		}
	}

	for(i=0;i<4;i++)
	{
		a[i]=b[i];
	}
}