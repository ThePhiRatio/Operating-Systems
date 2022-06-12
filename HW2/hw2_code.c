#include <stdio.h>
#include <stdlib.h>

void sleep(int) ;

struct PCB
{ struct PCB *Next_PCB ; int PID ;
  int IC ;
} ;

struct PCB *RQ, *RQT, *Current ;	// add additional pointers if needed

void Create_PCBs() ;

/*this function is called with the address of the RQ. It removes the PCB at the head of the list
  and returns a pointer to it. It updates the Head parameter to point to the next
  PCB in the list.
*/

 struct PCB *GetNextProcess(struct PCB **Head)
{
	struct PCB *temp = *Head;
	*Head = (*Head)->Next_PCB;
	return temp;
}

/*this function takes a pointer to the currently executing process and the address of the Ready
  Queue Tail. It moves the Current process to the tail of the list and updates RQT.
*/


void MvToTail(struct PCB *Current, struct PCB **RQT)
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

/* this function takes a process that has completed its execution and frees its PCB.
   it uses the free() function.
*/

void DeletePCB(struct PCB *Current)
{
	RQ = Current->Next_PCB;
	free(Current);
}

/* This function prints out the PID field of all PCBs in the list pointed to by Head.
*/



void PrintQ(struct PCB *Head)
{
	struct PCB *temp = Head;
	if(temp == NULL)
	{
		printf("%s","List is Empty.\n");
	}
	while(temp != NULL)
	{
		printf("Process ID: %d. IC: %d\n", temp->PID, temp->IC);
		temp = temp->Next_PCB;
	}
}

int main()
  { Create_PCBs() ; 
    PrintQ(RQ) ; //look at the initial state of the Q
    while(1)
	{ Current = GetNextProcess(&RQ) ; //select next process to execute and update RQ
	  Current->IC-- ; //simulate execution of process code.
	  if (Current->IC == 0) //this process has completed its execution
	     DeletePCB(Current) ; //back to the bit bucket
	  else
	     MvToTail(Current, &RQT) ; //still here? Move to tail of RQ and update RQT
          printf("NEW LIST OF READY PROCESSES\n") ;
          PrintQ(RQ) ;
          //sleep(1) ; //Marvel at what you have accomplished!! Can remove once it works.
          if (RQ == NULL)
		break ;
	}
   }




void Create_PCBs()
        {struct PCB *tmp ;
         int i ;
	 RQ = (struct PCB *) malloc (sizeof (struct PCB)) ;
         RQ->PID = 0;
         RQ->IC = 2 ;
         tmp = RQ ;
         for(i = 1; i < 10; i++)
                {tmp->Next_PCB = (struct PCB *) malloc (sizeof (struct PCB)) ;
                 tmp->Next_PCB->PID = i ;
                 tmp->Next_PCB->IC = i + 2 ; //rand returns 0 .. MAX
                 tmp->Next_PCB->Next_PCB = NULL ;
                 tmp = tmp->Next_PCB ;
                }

        RQT = tmp ;
        RQT->Next_PCB = NULL ;

        }
