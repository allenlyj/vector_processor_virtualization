#include <stdio.h>
#include <stdlib.h>
//#define __PLATFORM_H_  // Comment this line when running on standard pc

#ifndef AMIN_VP_OP
#define AMIN_VP_OP

#define __vp_req(length,number) thread_register(length,number,&vp_amin);
#define __vp_rel(thread_id) thread_release(thread_id,&vp_amin);
#define __vp_stat vp_stat(&vp_amin);
#define __vp_init vp_initialize(&vp_amin);

typedef struct vp_control vp_control;
typedef struct vp_reg vp_reg;

/* Stands for each register in the vp, totally 16 reg_64,
32 reg_32 and 64 reg_64 in the vp. Originally only reg_64
are available in the access que, to get smaller registers, 
reg_64 needs to be broken and obtained smaller registers 
are put into their own access que. The pointer next is 
used to implement the access que. The pointer prev is used
for recovering registers, when a buddy is found to be combined,
it has to be removed from the que and it is not necessarily at
the head of the que. */
struct vp_reg 
{
	int rname;
	int in_que, used;
	vp_reg *prev, *next;
};

/* The structure containing all the information of the vp:
available_n stands for the number of registers of length n 
potentially available in the vp (including all registers that 
can be obtained by breaking larger registers). In_que_n stands
for number of registers of length n that are currently in the 
access que that can be directly claimed. */
struct vp_control 
{
	vp_reg reg_16[64], reg_32[32], reg_64[16];
	vp_reg *head_16, *head_32, *head_64;
	int avail_16, avail_32, avail_64;
	int in_que_16, in_que_32, in_que_64;
	int thread_len[4];
	int thread_num[4];
	int tlb_table[32][4];
};

/* The actual struct containing the shared vp information */
vp_control vp_amin;

#ifndef __PLATFORM_H_
/****************************************************************/
// Display the thread virtual to physical name mapping
static void vp_tlb(vp_control *vp);

// Show the basic block usage pattern and Reg_32, Reg_64 usage
static void vp_pattern (vp_control *vp);


// Display access que content
static void vp_que (vp_control *vp);

// Display the vp resource status and TLB mapping
void vp_stat (vp_control *vp);
/* vp_stat() is the external user accessible function, it will call
vp_tlb(),vp_pattern(), and vp_que */
/****************************************************************/
#endif



/****************************************************************/
// Initialize the vp
void vp_initialize (vp_control *vp);
/* vp_initialize() is the external user accessible function to
initialize vp's resource management data structure */
/****************************************************************/


/****************************************************************/
/* See if the vp has enough resource for accepting a thread
asking for reg_n registers of reg_l, return id 0 to 3 if accepted,
return 4 if access denied */
static int process_id (int reg_len, int reg_num, vp_control *vp);

// Break n registers of length 4 into registers of length 2
static void break_64 (int num, vp_control *vp);

// Break n registers of length 2 into registers of length 1
static void break_32 (int num, vp_control *vp);

// Void function, just for pointer passing
static void create_64 (int num, vp_control *vp);

// Create num reg_32s in the que, call break_64 as needed
static void create_32 (int num, vp_control *vp);

// Create num reg_16s in the que, call break_32 as needed
static void create_16 (int num, vp_control *vp);

// Access num registers of length 16 for process pid
static void access_reg_16 (int num, int pid, vp_control *vp);

// Access num registers of length 32 for process pid
static void access_reg_32 (int num, int pid, vp_control *vp);

// Access num registers of length 64 for process pid
static void access_reg_64 (int num, int pid, vp_control *vp);

// Write register name "rname" into process "pid"s lookup entry "entry"
static void tlb_write (int rname, int pid, int entry,vp_control *vp);

// Request to the vp for a thread using r_num registers of length r_len
int thread_register (int r_len, int r_num, vp_control *vp);
/* thread_register is external user accessible function that uses all the
internal functions, to see if a thread can fit into the vp. If access is
denied either due to all 4 thread occupied or not sufficient registers, 4
is returned which stands for denial. If access is granted, a pid is given
to the thread, vp registers are arranged to accomodate the thread and 
name translation is written into the tlb of thread pid, also pid is returned
to external user, for later use of free up the resource when the thread
completes. */
/****************************************************************/

// Recover a reg_64 whose physicl name is rname, only update its own availability in vp resource list, not children
static void recover_64 (int rname, vp_control *vp);

// Release a reg_64whose physicl name is rname, calls recover_64, also updates its children 
static void release_64 (int rname, vp_control *vp);

// Recover a reg_32 whose physicl name is rname, only update its own availability in vp resource list, not children. My also update its parent availability due to combining with its buddy
static void recover_32 (int rname, vp_control *vp);

// Release a reg_32 whose physicl name is rname, calls recover_32, also updates its children 
static void release_32 (int rname, vp_control *vp);

// Realse reg_16 is actually the same as recover reg_16 since reg_16 does not have children. Recovering reg_16 is similar to recovering reg_32, which involves possible binding with its buddy
static void release_16 (int rname, vp_control *vp);

// Terminate the thread No. pid
void thread_release (int pid, vp_control *vp);
/* Thread_release is the external user accessible function used to terminate
currently in vp. It will find the vector length and mapping table of the 
thread and call the corresponding release_n functions to release the thread
registers usage back to the vp, and wipe out the thread's tlb table and reset
the thread's pid as available. */

#endif












