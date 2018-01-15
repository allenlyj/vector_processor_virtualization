#include "vp_control.h"
#include "random_exp.h"
#include <time.h>

int main(int argc, char* argv[])
{
	int i,temp;  // Loop counter and temp variable
	int num_req = 0, req_suc = 0;
	int t_num;   // Number of threads occupied
	int frag_miss;  // Number of misses due to fragmentation
	int frag_32_max = 0,frag_64_max = 0;  // Maximum number of fragmented registers ever reached
	int thread[4];  // Threads numbers being occupied. First occupied always in thread[0] and second after, thanks to the rearrange function
	int frag_32_time[16], frag_64_time[16];  // Recording the existing time of fragmentation
	toss my_toss;  // The random toss to decide req or release, number of registers and VL
	frag_state frag_status = {0,0};  // The current number of fragmented registers after every random experiment
	if (argc<2)
	{
		printf("Please specify number of random toss");
		return 1;
	}
	__vp_init;
	srand(time(NULL));

	for(i=0;i<4;i++)
	{
		thread[i]=-1;
	}

	for(i=0;i<16;i++)
	{
		frag_32_time[i]=frag_64_time[i]=0;
	}

	for (i=0,t_num=0,frag_miss=0; i<atoi(argv[1]); i++)
	{
		do_toss(&my_toss,t_num);
		//printf("req is %d\n",my_toss.req);
		if(my_toss.req == 1) // If toss is a request, do the request
		{
			num_req++;
			if( (temp = __vp_req(my_toss.vector_length,my_toss.num_reg)) < 4)  // If request suceeded, update thread id and thread number
			{
				thread[t_num] = temp;
				t_num++;
				req_suc++;
				//printf("requested thread %2d\n",temp);
			}
			else  // If request fails, analyze cause
			{
				switch(my_toss.vector_length)
				{
				case 32:
					if (vp_amin.avail_16/2 >= my_toss.num_reg)
						frag_miss++;
					break;
				case 64:
					if (vp_amin.avail_16/4 >= my_toss.num_reg)
						frag_miss++;
					break;
				default:
					break;
				}
			}
		}

		else  // If toss is a release, then release 
		{
			__vp_rel(thread[my_toss.thread_id]);
			//printf("released thread %2d\n",thread[my_toss.thread_id]);
			thread[my_toss.thread_id] = -1; // When thread id released, it goes back to -1
			t_num--;
			rearrange(thread);  // Rearrange the index array of occupied thread id, to insure occupied thread id always continuously occupies the first few indexes
		}

		measure_frag(&vp_amin, &frag_status);
		if (t_num<4)
		{
			if (frag_status.fra_32>frag_32_max)
			{
				frag_32_max = frag_status.fra_32;
				__vp_stat;
				printf("This is the %dth toss\n\n\n",i);
			}
			if (frag_status.fra_64>frag_64_max)
			{
				frag_64_max = frag_status.fra_64;
				__vp_stat;
				printf("This is the %dth toss\n\n\n",i);
			}
		}
		frag_32_time[frag_status.fra_32]++; 
		frag_64_time[frag_status.fra_64]++;
		
		
		//printf("Frag_32 = %2d\tFrag_64 = %2d\tThread%2d\n",frag_status.fra_32,frag_status.fra_64,t_num);
	}
	printf("Maximum R32 frag is %2d. Maximum R64 frag is %2d\n %d requests, %d successes,missed %d times due to fragmentation.\n",frag_32_max,frag_64_max,num_req,req_suc,frag_miss);

	// Print fragmented reg_32 results
	printf("Frag_32");
	for (i=0;i<8;i++)
	{
		printf("\t\t%d",i);
	}
	printf("\n");
	printf("Time");
	for (i=0;i<8;i++)
	{
		printf("\t\t%d",frag_32_time[i]);
	}
	printf("\n");
	printf("\n");
	printf("Frag_32");
	for (i=8;i<16;i++)
	{
		printf("\t\t%d",i);
	}
	printf("\n");
	printf("Time");
	for (i=8;i<16;i++)
	{
		printf("\t\t%d",frag_32_time[i]);
	}
	printf("\n");
	printf("\n");

	// Print fragmented reg_64 results
	printf("Frag_64");
	for (i=0;i<8;i++)
	{
		printf("\t\t%d",i);
	}
	printf("\n");
	printf("Time");
	for (i=0;i<8;i++)
	{
		printf("\t\t%d",frag_64_time[i]);
	}

	printf("\n");
	printf("\n");
	printf("Frag_64");
	for (i=8;i<16;i++)
	{
		printf("\t\t%d",i);
	}
	printf("\n");
	printf("Time");
	for (i=8;i<16;i++)
	{
		printf("\t\t%d",frag_64_time[i]);
	}
	printf("\n");
	return 0;
}