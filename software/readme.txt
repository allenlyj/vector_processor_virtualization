This is the simulator used to evaluate the efficiency of the register management algorithm as described in the paper.

Usage: compile the simulator using "make", then run "virt_sim number_of_toss".
The simulator will randomly toss requests/release command with reasonable register length and numbers.
Every time a miss occured due to fragmentation, it will be reported with the registers structure
At the end, the detailed missing report will be generated, fragmentation existing time will be measured in unit of tosses.
