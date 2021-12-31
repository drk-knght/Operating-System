# Operating-System

## Description of Master and Child process

Created one master process, M which spawns 3 child processes, C1, C2, and C3.

• C1 is a compute-intensive process which adds n1 numbers in the range 1 to 1 million.

• C2 is an I/O intensive process which reads n2 numbers (range from 1 to 1 million) from a text file and prints them to the console.Each number is present in a separate line in the file. After printing all the numbers, C2 sends the message “Done Printing” to M using an ordinary pipe.

• C3 is both compute and I/O intensive which reads n3 numbers (range from 1 to 1 million) from a text file and adds them up.

• C1 and C3 communicate the results of their operations to M using pipes. M prints these results on the console.

• Note that 3 separate pipes are to be used. The text files along with the contents will be provided during the demo. Create your own file while testing your code.




## Scheduling Algorithms

In M, the scheduling algorithms FCFS and Round Robin (RR) with a fixed time quantum were performed. 

• For RR, the time quantum is given as user input. Assume the order of process creation is C1, C2 and C3. C1, C2 and C3 are to be scheduled only after all the 3 processes have been created by M. 

• The emulation of scheduling is to be done using the sleep() function. 

• M will use shared memory segments with C1, C2 and C3 to communicate if C1, C2, C3 should sleep or wake up to perform their designated tasks. 

• Within each of C1, C2 and C3, use one thread to do the task (mentioned earlier) and another to monitor communications from M and accordingly put the task thread to sleep, or wake it up. 

• All the actions are performed assuming a uniprocessor environment. 

• At a time only one of C1, C2 or C3 can be awake. Note that, when M tells a process (either C1 or C2 or C3) to sleep, only the task thread should sleep. 

• The monitor thread should remain awake in order to monitor communications from M. The frequency at the which the monitor thread checks for communication from M is 100Mhz.

• The choice of the scheduling algorithm will be given as user input. n1, n2 and n3 constitute the process workload and are also given as user inputs. You should be able to repeat for different sizes of workload for performance analysis. The values of n1, n2 and n3 will range between 25 and one million.
