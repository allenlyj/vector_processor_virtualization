#include "vp_control_int.h"
#include <stdio.h>

//extern vp_control vp_amin;
#ifndef __PLATFORM_H_

// Display the thread virtual to physical name mapping
static void vp_tlb(vp_control *vp)
{
	int i,j;
	printf("\t\t  Thread_0\t Thread_1\t Thread_2\t Thread_3\t\n");
	printf("Vector Length:    %8d\t %8d\t %8d\t %8d\t\n",vp->thread_len[0], vp->thread_len[1],vp->thread_len[2],vp->thread_len[3]);
	printf("Num of Registers: %8d\t %8d\t %8d\t %8d\t\n",vp->thread_num[0], vp->thread_num[1],vp->thread_num[2],vp->thread_num[3]);
	printf("\n**************Register virtual name to physical name mapping**************\n");
	for (i = 0; i<32; i++)
	{
		printf("R[%2d] mapped to:  ",i);
		for (j=0; j<4; j++)
		{
			printf("%8d\t ",vp->tlb_table[i][j]);
		}
		printf("\n");
	}
}

// Show the basic block usage pattern of reg_16, Reg_32, and Reg_64 usage; This function gives better view than the vp_pattern
static void vp_pattern (vp_control *vp)
{
	int i;
	vp_reg *reg;

	printf("Reg_16 usage:");
	reg = vp->reg_16;
	for (i=0; i<64; i++)
		printf("_%d",reg[i].used);
	printf("\n");

	printf("Reg_32 usage:");
	reg = vp->reg_32;
	for (i=0; i<32; i++)
		printf("___%d",reg[i].used);
	printf("\n");

	printf("Reg_64 usage:");
	reg = vp->reg_64;
	for (i=0; i<16; i++)
		printf("_______%d",reg[i].used);
	printf("\n");
	printf("*****************************************\n");

}

// Display access que content
static void vp_que (vp_control *vp)
{
	vp_reg *current;

	// Show all available reg_16 in que
	printf("Available reg_16 in que are:\n");
	for (current=vp->head_16;current!=NULL;current=current->next)
		{printf("reg_16[%d]\t",current->rname);}

	// Show all available reg_32 in que
	printf("\nAvailable reg_32 in que are:\n");
	for (current=vp->head_32;current!=NULL;current=current->next)
		{printf("reg_32[%d]\t",current->rname);}

	// Show all available reg_64 in que
	printf("\nAvailable reg_64 in que are:\n");
	for (current=vp->head_64;current!=NULL;current=current->next)
		{printf("reg_64[%d]\t",current->rname);}
	printf("\n*****************************************\n");
}



// Display the vp resource status and TLB mapping
void vp_stat (vp_control *vp)
{
	printf("Potentially %d Reg_16 available\n",vp->avail_16);
	printf("Potentially %d Reg_32 available\n",vp->avail_32);
	printf("Potentially %d Reg_64 available\n",vp->avail_64);
	printf("*****************************************\n");
	vp_pattern (vp);
	vp_tlb (vp); 
	vp_que (vp);
}
#endif

/****************************************************************/
// Initialize the vp
void vp_initialize (vp_control *vp)
{
	int i;

	// All registers are potentially available
	vp->avail_16 = 64;
	vp->avail_32 = 32;
	vp->avail_64 = 16;

	// Only reg_64 are in que initially
	vp->in_que_64 = 16;

	// Connect all reg_64 into que
	// reg_64 que does not need the prev pointer
	vp->head_64 = vp->reg_64;
	for (i=0; i<16; i++)
	{
		vp->reg_64[i].rname = i;
		vp->reg_64[i].in_que = 1;
		if (i!=15)
			vp->reg_64[i].next = &(vp->reg_64[i+1]);
		if (i!=0)
			vp->reg_64[i].prev = &(vp->reg_64[i-1]);
	}

	// Initialize all regs_32
	for (i=0; i<32; i++)
	{
		vp->reg_32[i].rname = i;
	}

	// Initialize all regs_16
	for (i=0; i<64; i++)
	{
		vp->reg_16[i].rname = i;
	}
#ifndef __PLATFORM_H_
	printf("Vector Processor Initialization Completed!\n");
	printf("*****************************************\n");
#endif
}
/****************************************************************/


/* See if the vp has enough resource for accepting a thread
asking for reg_n registers of reg_l, return id 0 to 3 if accepted,
return 5 if access denied due to all thread being occupied,
return 4 if denied due to insufficient*/
static int process_id (int reg_len, int reg_num, vp_control *vp)
{
	int pid;

	// Check if all threads are occupied, if so then deny
	for (pid=0; pid<4; pid++)
	{
		if (vp->thread_len[pid] == 0)
			break;
	}
	if (pid>3)
	{
#ifndef __PLATFORM_H_
		printf("All threads occupied!\n");
#endif
		return 5;
	}

	// Check if resource enough to allow thread, if not then deny
	switch (reg_len)
		{
			case 16:
			if (reg_num>vp->avail_16)
			{
#ifndef __PLATFORM_H_
				//printf("Not enought Reg_16 for use!\n");
#endif
				return 4;
			}
			break;

			case 32:
			if (reg_num>vp->avail_32)
			{
#ifndef __PLATFORM_H_
				//printf("Not enought Reg_32 for use!\n");
#endif
				return 4;
			}
			break;

			case 64:
			if (reg_num>vp->avail_64)
			{
#ifndef __PLATFORM_H_
				//printf("Not enought Reg_64 for use!\n");
#endif
				return 4;
			}
			break;

			default:
#ifndef __PLATFORM_H_
			printf("Please use registers length of 16, 32, or 64!\n");
#endif
			return 4;
		}
	return pid; // Return the found available PID
}

// Break n registers of length 4 into registers of length 2
static void break_64 (int num, vp_control *vp)
{
	int i,j;

	// Loop num times breaking reg_64
	for (i = 0; i<num; i++)
	{
		// Find the register to break
		vp_reg *current = vp->head_64;
		int rname = current->rname;

		// Take the register out of the que, label as used
		vp->head_64 = current->next;
		if (current->next != NULL)
		{
			current->next->prev = NULL;
			current->next = NULL;
		}
		current->used = 1;
		current->in_que = 0;

		// Update reg_64 availability in vp
		vp->avail_64 -= 1;
		vp->in_que_64 -= 1;

		// Link the obtained regs_32 into que, and label them in que
		for (j=0; j<2;j++)
		{
			// Identify the register's own address
			vp_reg *new_guy = &(vp->reg_32[2*rname+j]);

			// Link the register into the que while checking if que is originally empty. If not empty, the first in que register's prev pointer has to be linked to the new register entering the que
			if ((new_guy->next=vp->head_32)!=NULL)
				new_guy->next->prev = new_guy;
			vp->head_32 = new_guy;
			// The first one in que's prev must be NULL, it will later be used to decide if it is the first one in que
			new_guy->prev = NULL;
			new_guy->in_que = 1;
		}

		// Update reg_32 availability in vp access que
		vp->in_que_32 += 2;
	}
}


// Break n registers of length 2 into registers of length 1
static void break_32 (int num,vp_control *vp)
{
	int i,j;

	/* Check if there is enough reg_32 to break, if not then
	break some reg_64 */
	int remain = num - vp->in_que_32;
	if (remain>0)
	{
		break_64((remain+1)/2,vp);
	}

	// Start breaking reg_32 like breaking reg_64
	for (i=0; i<num; i++)
	{
		// find the register to break
		vp_reg *current= vp->head_32;
		int rname = current->rname;

		// Take the register out of que and label as used
		vp->head_32 = current->next;
		if (current->next != NULL)
		{
			current->next->prev = NULL;
			current->next = NULL;
		}
		current->used = 1;
		current->in_que = 0;

		// Update reg_32 availability in vp
		vp->avail_32 -= 1;
		vp->in_que_32 -= 1;

		// Link the obtained regs_16 into que, and label them in que
		for (j=0; j<2;j++)
		{
			// Identify the register's own address
			vp_reg *new_guy = &(vp->reg_16[2*rname+j]);

			// Link the register into the que while checking if que is originally empty. If not empty, the first in que register's prev pointer has to be linked to the new register entering the que
			if ((new_guy->next=vp->head_16)!=NULL)
				new_guy->next->prev = new_guy;
			vp->head_16 = new_guy;
			// The first one in que's prev must be NULL, it will later be used to decide if it is the first one in que
			new_guy->prev = NULL;
			new_guy->in_que = 1;
		}

		// Update reg_16 availability in vp access que
		vp->in_que_16 += 2;
	}
}

// Void function, just for pointer passing
static void create_64 (int num,vp_control *vp)
{
	;
}

// Create num reg_32s in the que, call break_64 as needed
static void create_32 (int num,vp_control *vp)
{
	int remain = num - vp->in_que_32;
	if (remain>0)
	{
		break_64((remain+1)/2,vp);
	}
}

// Create num reg_16s in the que, call break_32 as needed
static void create_16 (int num,vp_control *vp)
{
	int remain = num - vp->in_que_16;
	if (remain>0)
	{
		break_32((remain+1)/2,vp);
	}
}

// Access num registers of length 16 for process pid
static void access_reg_16 (int num, int pid, vp_control *vp)
{
	int i;
	// loop num times to give reg_16s to thread pid
	for (i=0; i<num; i++)
	{
		vp_reg *current;
		int rname;

		// Find the first reg_16 in que
		current = vp->head_16;
		rname = current->rname;

		// Remove register from access que and label as used, if que has more than one registers, the second one's prev pointer has to be set to NULL
		if ((vp->head_16 = current->next)!=NULL)
			current->next->prev = NULL;
		current->prev = NULL;
		current->next = NULL;
		current->used = 1;
		current->in_que = 0;

		// Write the result into tlb and update vp resource availability
		tlb_write(rname,pid,i,vp);
		vp->avail_16 --;
		vp->in_que_16 --;
	}

}

// Access int1 registers of length 32 for process int2
static void access_reg_32 (int num,int pid,vp_control *vp)
{
	int i,j;
	// loop num times to give reg_32s to thread pid
	for (i=0; i<num; i++)
	{
		vp_reg *current;
		int rname;

		// Find the first reg_32 in que
		current = vp->head_32;
		rname = current->rname;

		// Remove register from access que and label as used, if que has more than one registers, the second one's prev pointer has to be set to NULL
		if ((vp->head_32 = current->next)!=NULL)
			current->next->prev = NULL;
		current->prev = NULL;
		current->next = NULL;
		current->used = 1;
		current->in_que = 0;

		// Label child reg_16s as used
		for (j=0; j<2; j++)
		{
			vp->reg_16[2*rname+j].used = 1;
		}

		// Write the result into tlb and update vp resource availability (Available child regs also reduced)
		tlb_write(rname,pid,i,vp);
		vp->avail_32 --;
		vp->in_que_32 --;
		vp->avail_16 -= 2;
	}
}


// Access int1 registers of length 64 for process int2
static void access_reg_64 (int num,int pid,vp_control *vp)
{
	int i,j;
	// loop num times to give reg_64s to thread pid
	for (i=0; i<num; i++)
	{
		vp_reg *current;
		int rname;

		// Find the first reg_64 in que
		current = vp->head_64;
		rname = current->rname;

		// Remove register from access que and label as used, if que has more than one registers, the second one's prev pointer has to be set to NULL
		if ((vp->head_64 = current->next)!=NULL)
			current->next->prev = NULL;
		current->prev = NULL;
		current->next = NULL;
		current->used = 1;
		current->in_que = 0;

		// Label child reg_32s as used
		for (j=0; j<2; j++)
		{
			vp->reg_32[2*rname+j].used = 1;
		}

		// Label child reg_16s as used
		for (j=0; j<4; j++)
		{
			vp->reg_16[4*rname+j].used = 1;
		}

		// Write the result into tlb and update vp resource availability (Available child regs also reduced)
		tlb_write(rname,pid,i,vp);
		vp->avail_64 --;
		vp->in_que_64 --;
		vp->avail_32 -= 2;
		vp->avail_16 -= 4;
	}
}


// Write register name "rname" into process "pid"s lookup entry "entry"
static void tlb_write (int rname,int pid,int entry,vp_control *vp)
{
	// Real function waiting to be implemented

	vp->tlb_table[entry][pid]=rname;
#ifdef __PLATFORM_H_
	int tlb_signal;
	tlb_signal = entry | (pid<<5) | (rname<<7);
	putfslx(tlb_signal,0,FSL_DEFAULT);
#endif
	// printf("Thread[%d] reg[%d] now has physical name reg[%d]\n",pid,entry,rname);
}


// Request to the vp for a thread using r_num registers of length r_len
int thread_register (int r_len, int r_num, vp_control *vp)
{
	int pid;
	void (*create) (int, vp_control *);
	void (*access) (int, int, vp_control *);

	if ((pid = process_id(r_len,r_num,vp))>3)
	{
#ifndef __PLATFORM_H_
		//printf("Thread access denied! Please try later!\n");
#endif
		return pid;
	}
	//printf("Found available pid %d\n",pid);

	vp->thread_num[pid]=r_num;
	vp->thread_len[pid]=r_len;
	switch(r_len)
	{
		// When vector length = 16
		case 16:
		create = &create_16;
		access = &access_reg_16;
		break;

		// When vector length = 32
		case 32:
		create = &create_32;
		access = &access_reg_32;
		break;

		// When vector length = 64
		default:
		access = &access_reg_64;
		create = &create_64;
	}
	(*create)(r_num,vp);
	(*access)(r_num,pid,vp);
	return pid;
}
/****************************************************************/


// Recover a reg_64 whose physicl name is rname, only update its own availability in vp resource list, not children
static void recover_64 (int rname, vp_control *vp)
{
	// Put register back into que
	if((vp->reg_64[rname].next = vp->head_64)!= NULL)
		vp->head_64->prev = &(vp->reg_64[rname]);
	vp->head_64 = &(vp->reg_64[rname]);
	vp->head_64->prev = NULL;

	// Update register status
	vp->reg_64[rname].used = 0;
	vp->reg_64[rname].in_que = 1;

	// Update vp resource list
	vp->avail_64++;
	vp->in_que_64++;
}

// Release a reg_64whose physicl name is rname, calls recover_64, also updates its children
static void release_64 (int rname, vp_control *vp)
{
	int i;

	// Recover the register back into que
	recover_64 (rname,vp);

	// Since the register is directly released, its children all become available
	for (i=0; i<4; i++)
	{
		vp->reg_16[4*rname+i].used=0;
	}

	for (i=0; i<2; i++)
	{
		vp->reg_32[2*rname+i].used=0;
	}
	vp->avail_16 += 4;
	vp->avail_32 += 2;
}


// Recover a reg_32 whose physicl name is rname, only update its own availability in vp resource list, not children. My also update its parent availability due to combining with its buddy
static void recover_32 (int rname, vp_control *vp)
{
	// Find the register and its buddy
	vp_reg *self = &(vp->reg_32[rname]);
	vp_reg *buddy = &(vp->reg_32[rname^1]);

	// Whether combined with buddy or not, there will be one more reg_32 potentially available
	vp->avail_32++;
	self->used = 0; // Register itself will become unused anyway

	// Check if buddy in que, if so, combine both to form a reg_64, register will not go into que and buddy will be taken from que
	if (buddy->in_que==1)
	{
		// Combined reg_64 recovered
		recover_64(rname/2,vp);

		// Remove buddy from que
		buddy->in_que=0;

		// If buddy is originally first in que, then pass its next pointer to head, otherwise its prev register's next gets its next pointer
		if (buddy->prev == NULL)
			vp->head_32 = buddy->next;
		else
			buddy->prev->next = buddy->next;


		// If buddy is not the last in que, its next register's prev gets its prev pointer, otherwise nothing happens
		if (buddy->next != NULL)
			buddy->next->prev = buddy->prev;
		
		buddy->next = NULL;
		buddy->prev = NULL;

		// No register is added into que but one is removed
		vp->in_que_32--;
	}

	// If buddy not in que, the register itself will go into the access que
	else
	{
		// Link the register into the que while checking if que is originally empty. If not empty, the first in que register's prev pointer has to be linked to the new register entering the que
		if ((self->next=vp->head_32)!=NULL)
			self->next->prev = self;
		vp->head_32 = self;

		// The first one in que's prev must be NULL, it will later be used to decide if it is the first one in que
		self->prev = NULL;
		self->in_que = 1;
		// One register is added into the que
		vp->in_que_32++;
	}
}


// Release a reg_32 whose physicl name is rname, calls recover_32, also updates its children
static void release_32 (int rname, vp_control *vp)
{
	int i;

	// Recover the register back into que
	recover_32 (rname,vp);

	// Since the register is directly released, its children all become available
	for (i=0; i<2; i++)
	{
		vp->reg_16[2*rname+i].used = 0;
	}

	vp->avail_16 += 2;
}


// Realse reg_16 is actually the same as recover reg_16 since reg_16 does not have children. Recovering reg_16 is similar to recovering reg_32, which involves possible binding with its buddy
static void release_16 (int rname, vp_control *vp)
{
	// Find the register and its buddy
	vp_reg *self = &(vp->reg_16[rname]);
	vp_reg *buddy = &(vp->reg_16[rname^1]);

	// Whether combined with buddy or not, there will be one more reg_16 potentially available
	vp->avail_16++;
	self->used = 0; // Register itself will become unused anyway

	// Check if buddy in que, if so, combine both to form a reg_64, register will not go into que and buddy will be taken from que
	if (buddy->in_que==1)
	{
		// Combined reg_64 recovered
		recover_32(rname/2,vp);

		// Remove buddy from que
		buddy->in_que=0;

		// If buddy is originally first in que, then pass its next pointer to head, otherwise its prev register's next gets its next pointer
		if (buddy->prev == NULL)
			vp->head_16 = buddy->next;
		else
			buddy->prev->next = buddy->next;

		// If buddy is not the last in que, its next register's prev gets its prev pointer, otherwise nothing happens
		if (buddy->next != NULL)
			buddy->next->prev = buddy->prev;
		
		buddy->next = NULL;
		buddy->prev = NULL;

		// No register is added into que but one is removed
		vp->in_que_16--;
	}

	// If buddy not in que, the register itself will go into the access que
	else
	{
		// Link the register into the que while checking if que is originally empty. If not empty, the first in que register's prev pointer has to be linked to the new register entering the que
		if ((self->next=vp->head_16)!=NULL)
			self->next->prev = self;
		vp->head_16 = self;

		// The first one in que's prev must be NULL, it will later be used to decide if it is the first one in que
		self->prev = NULL;
		self->in_que = 1;
		// One register is added into the que
		vp->in_que_16++;
	}
}

// Terminate the thread No. pid
void thread_release (int pid, vp_control *vp)
{
	int i;
	void (*release) (int, vp_control *);
	// Check the thread's vector length and call appropriate release functions
	switch(vp->thread_len[pid])
	{
		case 16:
		release = &release_16;
		break;

		case 32:
		release = &release_32;
		break;

		default:
		release = &release_64;
	}

	// Find all physical name of registers in the thread and release them
	for (i=0; i<vp->thread_num[pid]; i++)
		{
			int *virtual_name = &vp->tlb_table[i][pid];
			(*release)(*virtual_name,vp);
			*virtual_name = 0;
		}
	vp->thread_len[pid] = 0;
	vp->thread_num[pid] = 0;
}
