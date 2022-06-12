#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "Vars.h"

struct Semaphore Forks[5];
struct Semaphore Doorman;

void Create_PCBs() ;
int ParseOp1Reg (char*);
void LoadPrograms() ;
void LoadProgram(int, struct PCB **) ;
void LoadProgramsDoor() ;
void LoadProgramDoor(int, struct PCB **) ;
void RestoreState(struct PCB *) ;
int  ExecuteProc(struct PCB *) ;
void DeletePCB(struct PCB *) ;
void MvToTail(struct PCB *, struct PCB **) ;
void SaveState(struct PCB **) ;
void PrintQ(struct PCB*) ;
struct PCB* GetNextProcess(struct PCB **) ;

int OS_Trap(char * , struct PCB *);
int Wait(struct PCB* , struct Semaphore*);
int Signal(struct Semaphore *);
int GetPID(struct PCB *);


//Must declare Opcode Functions. Otherwise, end up with a boat load of
//highly annoying compiler warnings!


/*These variables are associated with the implementation of the VM*/
int i, j, k ;
int ProgSize ;
char input_line [7] ;

/*I made these global to make implementation a bit easier.
  You do not have to.
*/

struct PCB *RQ, *tmp, *RQT, *Current ;

int Max_Line = 0;

int isDoor;
int timeslice;

int main()
{
  /*basically same as before. You are creating 5 philosopher processes (with identical code),
    and creating two separate PB programs: one with the array of fork semaphores and one where the doorman
    Is added.

    Make sure you initialize the semaphores!!
  */	
	timeslice = 0;
	isDoor = 0;
	printf("Do you wish to run with the doorman? 0 = No, 1 = Yes.\n");
	scanf("%d",&isDoor);
	switch(isDoor){
		case 0: printf("Running without doorman.\n"); break;
		case 1: printf("Running with doorman.\n"); break;
		default: printf("You damn fool! That wasn't one of the options!\n"); 
	}
	sleep(2);
	if(isDoor == 0)
	{

		printf("Do you wish to have a Random IC? 0 = No, 1 = Yes.(No IC = 1)\n");
		scanf("%d",&timeslice);
				
		switch(isDoor){
			case 0: printf("Running with IC = 1.\n"); break;
			case 1: printf("Running with random IC.\n"); break;
			default: printf("You damn fool! That wasn't one of the options!\n"); 
			}
	}


	//initalize forks
	for(int i = 0; i < 5; i++)
	{
		Forks[i].count = 1;
		Forks[i].SemQ = NULL;//Forks[i].SemQ = (struct PCB *)malloc(sizeof(struct PCB));
	}

	//initialize doorman
	Doorman.count = 4;
	Doorman.SemQ = NULL;

	Create_PCBs() ;
	
	//load fork pb
	if(isDoor == 0)
	{
		LoadPrograms();
	}
	//load doorman pb
	else if(isDoor == 1)
	{
		LoadProgramsDoor();
	}
	else
	{
		exit(0);
	}

while(1)
          {

                Current = GetNextProcess(&RQ) ;
                RestoreState(Current) ;
		if(timeslice == 0 && isDoor == 0)
		{
			Current->IC = 1;
		}
		else if (timeslice == 1 || isDoor == 1)
		{
			Current->IC = (rand() % 200) + 5;
		}
		else
		{
			exit(0);
	  	}
                printf("CURRENT PID %d, IC %d\n", Current->PID, Current->IC) ;
                int Completed = ExecuteProc(Current) ;
              
                if (Completed == -1)
                      {printf("Current Process is Blocked on Semaphore.\n") ;
                       SaveState(&Current) ;
                       }
                
                if(Completed == 0)
                      {SaveState(&Current) ;
                       printf("Moving PID %d to TAIL\n", Current->PID) ;
                       MvToTail(Current, &RQT) ;
                       printf("RQT is %d\n", RQT->PID) ;
                       if(RQ == NULL)
                          RQ = RQT ;
                      }

                if (Completed == 1)
                      {
                       printf("Removing PID %d\n", Current->PID) ;
                       DeletePCB(Current);
                      }

                PrintQ(RQ) ;
	       // sleep(1);
                if (RQ == NULL)
                        break ;
        }
    return(0) ;
 }

 //NOTE: Any general purpose registers can be used to hold the system call number and
 //OP1 of the Trap Instruction. However, I assume R0 will always be used to hold the system
 //call number and R1 will be Op1 of the Trap instruction.

  /* New Functions */

  /* OS_Trap is called by the process when it executes Opcode 36.
    It returns a value of 1 if the calling process is blocked and 0 if it is
    not blocked. It performs the following actions:
  
    1) Determines the system call being made.
    2) Calls function Wait, Signal, or GetPID depending on the requested operation. It
       passes the address of the semaphore to be operated on in the case of Wait and
       Signal, and the struct PCB * in case of GetPID.
    3) Returns the value returned by Wait, Signal, or GetPID.
  */


    int OS_Trap(char *IR, struct PCB *Current)
      {
	      switch (RRegs[0]) {
		      case 0:
			 if(RRegs[1] == 0)
			 {	 printf("Calling wait on fork %d and using philisopher %d.\n",ACC,Current->PID);
		      	 	return Wait(Current, &Forks[ACC]);
			 }
			 else if(RRegs[1] == 1)
			 {
				 printf("Calling wait on Door and using philisopher %d.\n",Current->PID);
				 return Wait(Current, &Doorman);
			 }
		      case 1:
			if(RRegs[1] == 0)
			{	printf("Calling signal on fork %d and using philisopher %d.\n",ACC,Current->PID);
			 	return Signal(&Forks[ACC]);
			}
			else if (RRegs[1] == 1)
			{
				printf("***********Calling signal on doorman and using philisopher %d.\n",Current->PID);
				return Signal(&Doorman);
			}
		      case 2:
			 return GetPID(Current);
	      }
      }

  /* Performs basic wait operation on the semaphore parameter.
     Decrements the count variable, and, if it is less than 0, places
    the PCB of the caller on Set->Sem_Queue, and returns a 1 indicating
     the process is blocked on the semaphore. Otherwise, it returns a 0.*/
  
   
int Wait(struct PCB *Current, struct Semaphore *Sem)
    { 
	  struct PCB *temp ;
	  Sem->count--;				//dec count
	  if(Sem->count < 0)			//move to SemQ tail if count < 0
	  {
	  	if(Sem->SemQ == NULL)		//If there is nothing on the SemQ
		{
			Sem->SemQ = Current;	//Place process on SemQ tail
		}
		else				//if there is stuff in the SemQ
		{	
			temp = Sem->SemQ;
			while(temp->Next_PCB != NULL)	//go through SemQ
			{
				temp = temp->Next_PCB;
			}
			temp->Next_PCB = Current;
		}
		Current->Next_PCB = NULL;	//set Current->Next equal to null and place at tail of SemQ
		printf("In Wait with PID %d Sem.Count = %d Fork index = %d\n", Current->PID, Sem->count,ACC);
		return 1;				//return 1 b/c process is blocked
	  }
	  else
	  {	
		printf("In Wait with  PID %d  Sem.Count = %d Fork index = %d\n", Current->PID, Sem->count,ACC) ;
		return 0;				//return 0 because process isn't blocked
	  }
        }

  /*Signal performs the basic signal operation on a semaphore. It increments the count
    variable, and, if it is <= 0, it picks the PCB from the head of the semaphore
    Queue and places it on the Ready Queue. It always returns a 0.
  */
   
    int Signal(struct Semaphore *Sem)
     {	 
	 struct PCB *tmp ;
         printf("In Signal. Count is %d, Fork used: %d\n", Sem->count,ACC) ;
	 Sem->count++;
	 if(Sem->count <= 0)
	 {
		if(Sem->SemQ != NULL)
		{
			tmp = Sem->SemQ;
			Sem->SemQ = Sem->SemQ->Next_PCB;
			MvToTail(tmp,&RQT);
			if(RQ == NULL)
			{
				RQ = RQT;
			}
			printf("RQ PID = %d.\n",RQ->PID);
		}
	 }
	 return 0;
        }
 
  /*GetPID places PID of process in Register R1. While the programmer can specify any
    register, it is a lot simpler to always put it in R1.
     Always returns a 0
  */
     int GetPID(struct PCB *Current)
      {
	      RRegs[1] = Current->PID;
	      return 0;
      }

     void Create_PCBs()
	{    RQ = (struct PCB *) malloc (sizeof (struct PCB)) ;
         RQ->PID = 0;
         RQ->IC = (rand() % 200) + 5 ;
         tmp = RQ ;
         for(i = 1; i < 5; i++)
                {tmp->Next_PCB = (struct PCB *) malloc (sizeof (struct PCB)) ;
                 tmp->Next_PCB->PID = i ;

                 tmp->Next_PCB->IC = (rand() % 200) + 5 ; //rand returns 0 .. MAX
                 tmp->Next_PCB->Next_PCB = NULL ;
                 tmp = tmp->Next_PCB ;
                }

        RQT = tmp ;
        RQT->Next_PCB = NULL ;

	}

void LoadPrograms() {
	struct PCB *tmp = RQ;
	for (i = 0; i < 5; i++)
	{
		LoadProgram(i, &tmp);
		printf("LimitReg = %d. IC = %d\n", tmp->LimitReg, tmp->IC);
		tmp = tmp -> Next_PCB;
	}
}

void LoadProgram(int PID, struct PCB **tmp)
  	{
         int i, fp ;
         int program_line = 100 * PID ;
         (*tmp)->BaseReg  = program_line ;
         (*tmp)->LimitReg = program_line + 100;
         fp = open("forks.PB", O_RDONLY) ; //always check the return value.
         printf("Open is %d\n", fp) ;

         if (fp < 0) //error in read
                {printf("Could not open file\n");
                 exit(0) ;
                }

        int ret = read (fp, input_line, 7 ) ; //returns number of characters read`

        while (1)
                {
                 if (ret <= 0) //indicates end of file or error
                        break ; //breaks out of infinite loop

                 printf("Copying Program line %d into memory\n", program_line) ;
                 for (i = 0; i < 6 ; i++)
                        {
                         memory[program_line][i] = input_line[i] ;
                         printf("%c ", memory[program_line][i]) ;
                        }
                 printf("\n") ;

                ret = read (fp, input_line, 7 ) ;
                program_line++ ; //now at a new line in the prog
               }

  	 printf("Read in Code. Closing File\n") ;
  	 close(fp) ;
 	}


void LoadProgramsDoor() {
	struct PCB *tmp = RQ;
	for (i = 0; i < 5; i++)
	{
		LoadProgramDoor(i, &tmp);
		printf("LimitReg = %d. IC = %d\n", tmp->LimitReg, tmp->IC);
		tmp = tmp -> Next_PCB;
	}
}

void LoadProgramDoor(int PID, struct PCB **tmp)
  	{
         int i, fp ;
         int program_line = 100 * PID ;
         (*tmp)->BaseReg  = program_line ;
         (*tmp)->LimitReg = program_line + 100;
         fp = open("doorman.PB", O_RDONLY) ; //always check the return value.
         printf("Open is %d\n", fp) ;

         if (fp < 0) //error in read
                {printf("Could not open file\n");
                 exit(0) ;
                }

        int ret = read (fp, input_line, 7 ) ; //returns number of characters read`

        while (1)
                {
                 if (ret <= 0) //indicates end of file or error
                        break ; //breaks out of infinite loop

                 printf("Copying Program line %d into memory\n", program_line) ;
                 for (i = 0; i < 6 ; i++)
                        {
                         memory[program_line][i] = input_line[i] ;
                         printf("%c ", memory[program_line][i]) ;
                        }
                 printf("\n") ;

                ret = read (fp, input_line, 7 ) ;
                program_line++ ; //now at a new line in the prog
               }

  	 printf("Read in Code. Closing File\n") ;
  	 close(fp) ;
 	}


       

/*	This function returns the PCB at the head of the RQ and updates
	RQ to point to the next PCB in the list
*/

	struct PCB *GetNextProcess(struct PCB **RQ)
	{
		struct PCB *temp = *RQ;
		*RQ = (*RQ)->Next_PCB;
		return temp;
	}
       

/*	Deletes the PCB (using free) */

	void DeletePCB(struct PCB *Current)
	{
		RQ = Current->Next_PCB;
		free(Current);
	}       

/*	This function places the PCB pointed to by Current at the tail of the
	Ready Queue and updates the RQT pointer.
*/

	void MvToTail (struct PCB *Current, struct PCB **RQT)
	{
		(*RQT)->Next_PCB = Current;
		Current->Next_PCB = NULL;
		*RQT = Current;
		//this is to allow us to execute the last process in the queue.
		if(RQ == NULL)	
		{
			RQ = Current->Next_PCB;
		}
	}


/*	Prints out the elements of a linked list */

	void PrintQ(struct PCB *Head)
	{
		struct PCB *temp = Head;
		//this allows us to see if we have executed all the PCBs.
		if(temp == NULL)
		{
			printf("%s","List is Empty. DEADLOCK!\n");
		}
		while(temp != NULL)
		{
			printf("Process ID: %d. IC: %d\n", temp->PID, temp->IC);
			temp = temp->Next_PCB;
		}
	}


/*	This function restores the state of the process that is set to begin its
	execution
*/

	void RestoreState(struct PCB *NextProc)
	{
		//restore all info from local PCB to global variables
		PC = NextProc->PC;
		PRegs[0] = NextProc->P0;
		PRegs[1] = NextProc->P1;
		PRegs[2] = NextProc->P2;
		PRegs[3] = NextProc->P3;
		RRegs[0] = NextProc->R0;
		RRegs[1] = NextProc->R1;
		RRegs[2] = NextProc->R2;
		RRegs[3] = NextProc->R3;
		PSW[0] = NextProc->PSW[0];
		PSW[1] = NextProc->PSW[1];
		ACC = NextProc->ACC;

		//DO NOT SAVE THESE! Restoring is necessary.
		BaseRegister = NextProc->BaseReg;
		LimitRegister = NextProc->LimitReg;
	}

/*	This function saves the state of the VM into the PCB of the process that
	just completed its "time slice"
*/

	void SaveState(struct PCB **PrevProc)
	{
		//save all PCB global variables to local PCB
		(*PrevProc)->PC = PC;
		(*PrevProc)->P0 = PRegs[0];
		(*PrevProc)->P1 = PRegs[1];
		(*PrevProc)->P2 = PRegs[2];
		(*PrevProc)->P3 = PRegs[3];
		(*PrevProc)->R0 = RRegs[0];
		(*PrevProc)->R1 = RRegs[1];
		(*PrevProc)->R2 = RRegs[2];
		(*PrevProc)->R3 = RRegs[3];
		(*PrevProc)->PSW[0] = PSW[0];
		(*PrevProc)->PSW[1] = PSW[1];
		(*PrevProc)->ACC = ACC;
	}
