/*
Samarth Soni                2019A7PS1274H
Aditya Pratap Singh Tomar   2019A7PS0127H
Gaurvit Kumar               2019A7PS1278H
Ojashvi Tarunabh            2019A7PS0025H
Om Agarwal                  2019A7PS0052H
Bharadwaz Rushi Dabbiru     2019A7PS0111H
Saransh Dwivedi             2019A7PS0173H
*/
#include <stdbool.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/stat.h>
#include<pthread.h>
#include<sys/mman.h>
#include<stdint.h>
#include<time.h>
#include<wait.h>

#define PAGESIZE 4096
#define ll unsigned long long
#define READ 0
#define WRITE 1

/*
    Structure of a node in Ready Queue.
*/

struct node *head,*tail;
typedef struct node{
    int pid;
    struct node *next;
}node;

/*
    File descriptors for communicaiton between child and master processes
    through pipes.
*/
int fd1[3][2],fd2[3][2];

/*
    Process IDs for the Child processes.
*/ 
int pid1, pid2, pid3;

/*
    Shared memory:
    fin_flag : to signal the completion of a child process.
    sch_flag : to signal work or sleep commands to the child process.
    exec_time : to provide execution time of the work thread to the master process.
*/
uint8_t *fin_flag;
uint8_t *sch_flag;
struct timespec *exec_time;
int allow[3];  
int n1, n2, n3;

/*
    To store the computational results of the C1 and C3 processes.
*/
ll result1=0ll,result3=0ll;

/* 
    Intialising mutex and conditionals variables for parameters of thread scheduling 
    using signal and wait condition.
*/
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond2 = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex3 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond3 = PTHREAD_COND_INITIALIZER;


/*
    FUNCTIONS FOR USING READY QUEUE
*/
/*
    Creates/ Initializes the ready queue.
*/
node *createnode(int val){
    node *temp=(node *)malloc(sizeof(node));
    temp->pid=val;
    temp->next=NULL;
    return temp;
}

/*
    Gets the key value (process id) of the top node in the ready queue.
*/
int extract_front(node *ref){
    return ref->pid;
}

/*
    Removes the top node of the ready queue and puts it at the end.
*/
void dequeue(){
    node *temp=head;

    if(head!=NULL){
        if(head->next==NULL){
            return ;
        }
        head=head->next;
    }
    if(tail!=NULL){
        tail->next=temp;
        if(temp!=NULL){
            tail=tail->next;
            tail->next=NULL;
        }
        
    }
}

/*
    FUNCTIONS PERFORMED BY WORK THREADS OF THE CHILD PROCESSES
*/
/*
    C1 Work function: Outputs the sum of numbers from 1 to
    a given number "n1".
*/
void * c1_work(void * param)
{             
    clockid_t threadClockId1;
    pthread_getcpuclockid(pthread_self(), &threadClockId1);

    result1 = 0ll;

    for(int i = 0; i <=n1; i++){

        // putting the thread in waiting state if it is not allowed to run.
        if(allow[0] == 0){
            pthread_cond_wait(&cond1, &mutex1);
        }    
        result1+=i; 
    }

    // setting up the finish signal as true at the end of the process.
    fin_flag[0] = 1;
   
    // Measuring the execution time of this thread.
    clock_gettime(threadClockId1, &exec_time[0]);
}

/*
    C2 Work function: Reads data from a file and print its contents 
    on the screen.
*/
void *c2_work(void * param)
{            

    clockid_t threadClockId2;
    pthread_getcpuclockid(pthread_self(), &threadClockId2);

    char *filename = "Readme1.txt";
    FILE *fp = fopen(filename, "r");
    if (fp == NULL)
    {
        printf("Error: could not open file %s", filename);
    }

    const unsigned MAX_LENGTH = 256;
    char buffer[256];

    int cnt=0;

    while (cnt<n2){

        fgets(buffer, MAX_LENGTH, fp);
        // putting thread in waiting state if not allowed to run.
        if(!allow[1]){
            
            pthread_cond_wait(&cond2, &mutex2);
        }
        printf("%s", buffer);
        cnt++;
    }

    // close the file
    fclose(fp);
    
    //setting up finish signal as true at the end of the process.
    fin_flag[1] = 1;

    // Measuring the execution time of this thread.
    clock_gettime(threadClockId2, &exec_time[1]);
}

/*
    C3 Work function: Reads data (numbers) from the file and computes
    thier sum.
*/
void *c3_work()
{                         

    clockid_t threadClockId3;
    pthread_getcpuclockid(pthread_self(), &threadClockId3);
    
    result3 = 0;
    char *filename = "Readme2.txt";
    FILE *fp = fopen(filename, "r");
    if (fp == NULL)
    {
        printf("Error: could not open file %s", filename);
    }

    const unsigned MAX_LENGTH = 256;
    char buffer[256];

    while (fgets(buffer, MAX_LENGTH, fp))
    {   
        // putting the thread in waiting state if not allowed to run.
        if(!allow[2]){
            pthread_cond_wait(&cond3, &mutex3);
        }
        int x;
        sscanf(buffer,"%d",&x);
        result3 += x;
        
    }

    // close the file
    fclose(fp);

    //setting up finish signal as true at the end of the process.
    fin_flag[2] = 1;

    // Measuring the execution time of this thread.
    clock_gettime(threadClockId3, &exec_time[2]);
    
}

int main()
{

    /*
        Preparing the shared memory.
    */
    sch_flag=mmap(NULL, 3*PAGESIZE,                                                     // used in scheduling (signaling wait/work)
                                PROT_READ | PROT_WRITE, 
                                MAP_SHARED| MAP_ANONYMOUS,-1,0);

    fin_flag=mmap(NULL, 3*PAGESIZE,                                                     // used in signaling the end of the work
                                PROT_READ | PROT_WRITE, 
                                MAP_SHARED| MAP_ANONYMOUS,-1,0);
    
    exec_time=mmap(NULL, 3*sizeof(struct timespec),                                     // used in recording execution time of the processes
                                PROT_READ | PROT_WRITE, 
                                MAP_SHARED| MAP_ANONYMOUS,-1,0);

    sch_flag[1] = 0; sch_flag[0] = 0; sch_flag[2] = 0;
    fin_flag[1] = 0; fin_flag[0] = 0; fin_flag[2] = 0; 

    /*
        Creating pipes:
        p1, p2, p3: Pipes for communication from master process to child process (sending the argumments)
        q1, q2, q3: Pipes for communication from child process to master process (sending the output values)
    */
    int p1 = pipe(fd1[0]);
    int p2 = pipe(fd1[1]);
    int p3 = pipe(fd1[2]);

    int q1 = pipe(fd2[0]);
    int q2 = pipe(fd2[1]);
    int q3 = pipe(fd2[2]);

    // Creating first child process
    pid1 = fork();
    if(pid1 == 0){              // Process C1
        
        /*
            Reading the value of "n1" sent by Master process.
        */
        read(fd1[0][READ], &n1, sizeof(int));
        
        /*
        Creating a thread to perform the work (work thread).
        The initial thread performs monitoring and communication with the Master process (monitor thread).
        */
        pthread_t tid1; 
        pthread_create(&tid1, NULL, c1_work, NULL);

        
        // Monitor thread for C1

        struct timespec tim,tim2;
        tim.tv_sec=0;
        tim.tv_nsec=100L;

        /*
            MONITORING PROCESS:
            The monitor thread regularly check the value of the shared memory and relays this information
            to the work thread (frequency: 10 Mhz).
            The work thread, if, sees the allow variable as "0", it pauses.
            If the monitor thread sees the allow varaible as "1", it resumes the work thread.
        */
        while(!fin_flag[0]){
            allow[0] = sch_flag[0];

            if(allow[0] == 1)
                pthread_cond_signal(&cond1);

            nanosleep(&tim, &tim2);
        }

        pthread_join(tid1,NULL);

        /*
            Finally, the output of the work thread is sent back to the Master process via pipe. 
        */
        write(fd2[0][WRITE],&result1,sizeof(result1));
        
    }
    else
    {
        // Creating Second child process
        pid2 = fork();
        if(pid2 == 0){          // Process C2
            
            /* 
                Reading the value of "n2" sent by Master process.
            */
            read(fd1[1][READ], &n2, sizeof(int));
            
            /*
                Creating a thread to perform the work (work thread).
                The initial thread performs monitoring and communication with the Master process (monitor thread).
            */
            pthread_t tid2; 
            pthread_create(&tid2, NULL, c2_work, NULL);

            // Monitor Thread

            struct timespec tim, tim2;
            tim.tv_sec = 0;
            tim.tv_nsec = 100L;

            /*
                MONITORING PROCESS:
                The monitor thread regularly check the value of the shared memory and relays this information
                to the work thread (frequency: 10 Mhz).
                The work thread, if, sees the allow variable as "0", it pauses.
                If the monitor thread sees the allow varaible as "1", it resumes the work thread.
            */
            while(!fin_flag[1]){
                allow[1] = sch_flag[1];

                if(allow[1] == 1)
                    pthread_cond_signal(&cond2);
                nanosleep(&tim, &tim2);
            }
            
            pthread_join(tid2,NULL);

            char buff[50]="Done Printing";
            /*
                Finally, the output of the work thread is sent back to the Master process via pipe. 
            */
            write(fd2[1][WRITE],buff,strlen(buff)+1);
        }
        else
        {
            // Creating Third child process.
            pid3 = fork();
            if(pid3 == 0){                  // Process C3

                /* 
                    Reading the value of "n2" sent by Master process.
                */
                read(fd1[2][READ], &n3, sizeof(int));

                /*
                    Creating a thread to perform the work (work thread).
                    The initial thread performs monitoring and communication with the Master process (monitor thread).
                */
                pthread_t tid3; 
                pthread_create(&tid3, NULL, c3_work, NULL);

                // Monitor Thread

                struct timespec tim, tim2;
                tim.tv_sec = 0;
                tim.tv_nsec = 100L;

                /*
                    MONITORING PROCESS:
                    The monitor thread regularly check the value of the shared memory and relays this information
                    to the work thread (frequency: 10 Mhz).
                    The work thread, if, sees the allow variable as "0", it pauses.
                    If the monitor thread sees the allow varaible as "1", it resumes the work thread.
                */
                while(!fin_flag[2]){
                    allow[2] = sch_flag[2];

                    if(allow[2] == 1)
                        pthread_cond_signal(&cond3);
                    nanosleep(&tim, &tim2);
                }
                pthread_join(tid3,NULL);
                
                /*
                    Finally, the output of the work thread is sent back to the Master process via pipe. 
                */
                write(fd2[2][WRITE],&result3,sizeof(result3));
            }
            else
            {
                // MASTER PROCESS

                /*
                    Getting the input for n1, n2, and n3.
                    time_quantum is the time quantum used in Round Robin Scheduling (in nanoseconds).
                    algo is the selection of the scheduling algorithm (1 for FCFS, 2 for Round Robin).
                */
                int in1, in2, in3;
                int time_quantum;
                int algo = 0;
                printf("Enter the value of n1: ");
                scanf("%d",&in1);
                printf("Enter the value of n2: ");
                scanf("%d",&in2);
                printf("Enter the value of n3: ");
                scanf("%d",&in3);
                
                printf("Select the Scheduling Algorithm.\nPress \"1\" for FCFS or press \"2\" for Round Robin: ");
                scanf("%d", &algo);

                // ROUND ROBIN SCHEDULING ALGORITHM.

                if(algo == 2){
                    printf("Performing Round Robin Scheduling.\n");
                    printf("Enter the value of time quantum(in nanoseconds):");
                    scanf("%d", &time_quantum);

                    // Sending the value of n1, n2, n3 to the respective child processes.
                    write(fd1[0][WRITE], &in1, sizeof(int));
                    write(fd1[1][WRITE], &in2, sizeof(int));
                    write(fd1[2][WRITE], &in3, sizeof(int));

                    // Time structure to strore the Turn-around Time of the processes.
                    double diff[3];
                    struct timespec start[3];
                    struct timespec end[3];

                    // Creating the Ready Queue.
                    head = NULL;
                    tail = NULL;
                    for(int i = 1; i <= 3; i++){
                        node *cur = createnode(i);
                        if(head == NULL){
                            head = cur;
                            tail = cur;
                        }
                        else
                        {
                            tail->next = cur;
                            tail = tail->next;
                        }
                    }

                    // Intializing the same start time for all the processes arriving in the Ready Queue.
                    clock_gettime(CLOCK_REALTIME, &start[0]);
                    clock_gettime(CLOCK_REALTIME, &start[1]);
                    clock_gettime(CLOCK_REALTIME, &start[2]);

                    // Scheduling Starts using Round Robin Algorithm ...
                    while(fin_flag[0] == 0 || fin_flag[1] == 0 || fin_flag[2] == 0){
                    
                    
                    // extracting the process which is to be put into running state.
                        int pro = extract_front(head);
                        struct node *temp = head;
                        
                        /*
                            Checking if the process is already completed.
                            If it is not:
                            The process is allowed to continue its execution for a given time quantum.
                            Then it is again paused.
                        */
                        if(!fin_flag[pro-1]){

                        //    printf("doing %d\n", pro);
                            
                            sch_flag[pro-1] = 1;
                            
                            struct timespec tim, tim2;
                            tim.tv_sec = 0;
                            tim.tv_nsec = time_quantum;
                        
                        //    printf("Halt\n");
                            
                            nanosleep(&tim, &tim2);
                            sch_flag[pro-1] = 0;
                        } 
                        /*
                            If the process is complete:
                            It is taken out of the queue.
                            And its Finish time is recorded.
                        */
                        if(fin_flag[pro-1])
                        {
                            clock_gettime(CLOCK_REALTIME, &end[pro-1]);

                            // Make the head pointer point to next process and make the current next to NULL
                            temp = head->next;
                            head->next = NULL;
                            head = temp;
                            
                        }
                        /*
                            Putting back the latest run process back into ready queue.
                        */
                        else
                        {
                            dequeue();
                        }
                    }

                    /*
                        Finally after the execution of all the processes, their Turn-around time and Execution Time is calculated.
                    */
                    diff[0] = (double)(end[0].tv_sec - start[0].tv_sec) + (((double)(end[0].tv_nsec - start[0].tv_nsec))/1000000000.0);

                    diff[1] = (double)(end[1].tv_sec - start[1].tv_sec) + (((double)(end[1].tv_nsec - start[1].tv_nsec))/1000000000.0);

                    diff[2] = (double)(end[2].tv_sec - start[2].tv_sec) + (((double)(end[2].tv_nsec - start[2].tv_nsec))/1000000000.0);

                    double burst1 = ((double)exec_time[0].tv_nsec)/1000000000;

                    double burst2 = ((double)exec_time[1].tv_nsec)/1000000000;

                    double burst3 = ((double)exec_time[2].tv_nsec)/1000000000;

                    double wait[3];
                    wait[0]=diff[0]-burst1;  wait[1]=diff[1]-burst2; wait[2]=diff[2]-burst3;

                    /*
                        The master process recieves the output values from the child processes.
                    */
                    ll op1,op3;
                    char ans[50];

                    read(fd2[0][READ],&op1,sizeof(op1));
                    read(fd2[2][READ],&op3,sizeof(op3));
                    read(fd2[1][READ],ans,50);

                    /*
                        Displaying the Results in Console.
                    */
                    printf("Ouput of C1: %llu\n",op1);
                    printf("TAT of is: %f    ",diff[0]);
                    printf("Exec Time: %f\n", burst1);
                    printf("Wait Time: %f\n", wait[0]);
                    printf("Ouput of C2: %s\n",ans);
                    printf("TAT of is: %f    ",diff[1]);
                    printf("Exec time: %f\n", burst2);
                    printf("Wait Time: %f\n", wait[1]);
                    printf("Ouput of C3: %llu\n",op3);
                    printf("TAT of is: %f    ",diff[2]);
                    printf("Exec time: %f\n", burst3);
                    printf("Wait Time: %f\n", wait[2]);

                }
                else if(algo == 1)   
                {

                // ROUND ROBIN SCHEDULING ALGORITHM.

                    printf("Performing FCFS Scheduling\n");
                    
                    // clock_t start[3];
                    // clock_t end[3];
                    double diff[3];

                    // Time structure to strore the Turn-around Time of the processes.
                    struct timespec start[3];
                    struct timespec end[3];

                    // Sending the value of n1, n2, n3 to the respective child processes.
                    write(fd1[0][WRITE], &in1, sizeof(int));
                    write(fd1[1][WRITE], &in2, sizeof(int));
                    write(fd1[2][WRITE], &in3, sizeof(int));
                    
                    // Intializing the same start time for all the processes arriving in the Ready Queue.
                    clock_gettime(CLOCK_REALTIME, &start[0]);
                    clock_gettime(CLOCK_REALTIME, &start[1]);
                    clock_gettime(CLOCK_REALTIME, &start[2]);

                    // Scheduling Starts using FCFS Algorithm ...
                   
                    // Running process C1
                    sch_flag[0] = 1;
                    while(fin_flag[0] != 1){}
                    // Recording the Finish Time for process C1
                    clock_gettime(CLOCK_REALTIME, &end[0]);

                    // Running process C2
                    sch_flag[1] = 1;
                    while(fin_flag[1] != 1){}
                    // Recording the Finish Time for process C2
                    clock_gettime(CLOCK_REALTIME, &end[1]);

                    // Running process C3
                    sch_flag[2] = 1;
                    while(fin_flag[2] != 1){}
                    // Recording the Finish time for process C3
                    clock_gettime(CLOCK_REALTIME, &end[2]); 

                    pid_t w=waitpid(pid3,NULL,0);
                    
                    /*
                        Finally after the execution of all the processes, their Turn-around time and Execution Time is calculated.
                    */
                    diff[0]=(double)(end[0].tv_sec - start[0].tv_sec) +(((double)(end[0].tv_nsec - start[0].tv_nsec))/1000000000.0);

                    diff[1]=(double)(end[1].tv_sec - start[1].tv_sec) +(((double)(end[1].tv_nsec - start[1].tv_nsec))/1000000000.0);

                    diff[2]=(double)(end[2].tv_sec - start[2].tv_sec) +(((double)(end[2].tv_nsec - start[2].tv_nsec))/1000000000.0);

                    double burst1 = ((double)exec_time[0].tv_nsec)/1000000000;

                    double burst2 = ((double)exec_time[1].tv_nsec)/1000000000;

                    double burst3 = ((double)exec_time[2].tv_nsec)/1000000000;

                    double wait[3];
                    wait[0]=diff[0]-burst1;  wait[1]=diff[1]-burst2; wait[2]=diff[2]-burst3;

                    /*
                        The master process recieves the output values from the child processes.
                    */
                    ll op1,op3;
                    char ans[50];
                    read(fd2[0][READ],&op1,sizeof(op1));
                    read(fd2[2][READ],&op3,sizeof(op3));
                    read(fd2[1][READ],ans,50);

                    /*
                        Displaying the Results in Console.
                    */
                    printf("Ouput of C1: %llu\n",op1);
                    printf("TAT of is: %f\t",diff[0]);
                    printf("Exec Time: %f\t", burst1);
                    printf("Wait Time: %f\n", wait[0]);
                    printf("Ouput of C2: %s\n",ans);
                    printf("TAT of is: %f\t",diff[1]);
                    printf("Exec time: %f\t", burst2);
                    printf("Wait Time: %f\n", wait[1]);
                    printf("Ouput of C3: %llu\n",op3);
                    printf("TAT of is: %f\t",diff[2]);
                    printf("Exec time: %f\t", burst3);
                    printf("Wait Time: %f\n", wait[2]);

                }
                else
                {
                    printf("Error: Invalid input.\n");
                }  
            }
        }
    }
}