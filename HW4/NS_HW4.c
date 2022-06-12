#include <stdio.h>
#include <stdlib.h>

void sleep(int) ;

struct PCB
{ struct PCB *Next_PCB ; 
  int PID ;
  int IC ;
} ;

struct Semaphore 
 { int count ;
    struct PCB *SemQ ;
 } ;


/* functions used for assignment */
int Wait(struct Semaphore *, struct PCB *) ;
void Signal (struct Semaphore *) ;
struct PCB *GetNextProcess(struct PCB **) ;
void MovetoTail(struct PCB *Current, struct PCB **RQT) ;
void PrintQ(struct PCB *Head) ;
void Create_PCBs() ;

/*Global Variables*/

struct PCB *RQ, *RQT, *Current ;	// add additional pointers if needed
struct Semaphore Sem ;

int main()
  { int blocked ;
    Sem.count = 0 ;
    Sem.SemQ = NULL ;
    Create_PCBs() ; 
    while(1)
	{ Current = GetNextProcess(&RQ) ; //select next process to execute and update RQ
	  printf("Current Proc is %d\n",Current->PID) ;
	  int j = rand() % 4 ; //you will want to play with this!
	  if (j < 3 )
		{ printf("CALLING WAIT with PID %d\n", Current->PID) ;
		  blocked = Wait(&Sem, Current) ;
		  if (blocked)
			printf("Process Got blocked on wait Q. Bummer!\n") ;
		  else
			printf("Process NOT blocked on wait Q. Yabadabadoo!!!\n") ;
			
		}
	  else
		{printf("Calling Signal with PID %d\n", Current->PID) ;
		Signal(&Sem) ;	  
		blocked = 0 ;
		}
	
	  if (!blocked)
		{
	  	 MovetoTail(Current, &RQT) ;	
		 if (RQ == NULL)
			RQ = RQT ;
		}
	  
	  printf("Calling PrintQ for the ReadyQ\n") ;
	  PrintQ(RQ) ;
	  printf("Calling PrintQ for the SemaphoreQ\n") ;
	  PrintQ(Sem.SemQ) ;
          
	  sleep(1) ; //Marvel at what you have accomplished!! Can remove once it works.
          
	if (RQ == NULL)
    		break ;
  }
 }

int Wait(struct Semaphore *Sem, struct PCB *Current)
        { struct PCB *tmp ;
	  Sem->count--;				//dec count
	  if(Sem->count < 0)			//move to SemQ tail if count < 0
	  {
	  	if(Sem->SemQ == NULL)		//If there is nothing on the SemQ
		{
			Sem->SemQ = Current;	//Place process on SemQ tail
		}
		else				//if there is stuff in the SemQ
		{	
			tmp = Sem->SemQ;
			while(tmp->Next_PCB != NULL)	//go through SemQ
			{
				tmp = tmp->Next_PCB;
			}
			Current->Next_PCB = NULL;	//set Current->Next equal to null and place at tail of SemQ
			tmp->Next_PCB = Current;
		}
		printf("In Wait with PID %d Sem.Count = %d\n", Current->PID, Sem->count);
		return 1;				//return 1 b/c process is blocked
	  }
	  else
	  {	
		printf("In Wait with  PID %d  Sem.Count = %d\n", Current->PID, Sem->count) ;
		return 0;				//return 0 because process isn't blocked
	  }
        }

void Signal(struct Semaphore *Goo)
        {struct PCB *tmp ;
         printf("In Signal. Count is %d\n", Goo->count) ;
	 Goo->count++;
	 if(Goo->count <= 0)
	 {
		if(Goo->SemQ != NULL)
		{
			tmp = Goo->SemQ;
			Goo->SemQ = Goo->SemQ->Next_PCB;
			MovetoTail(tmp,&RQT);
		}
	 }
        }

void Create_PCBs()
        {struct PCB *tmp ;
         int i ;
	 RQ = (struct PCB *) malloc (sizeof (struct PCB)) ;
         RQ->PID = 0;
         tmp = RQ ;
         for(i = 1; i < 10; i++)
                {tmp->Next_PCB = (struct PCB *) malloc (sizeof (struct PCB)) ;
                 tmp->Next_PCB->PID = i ;
                 tmp->Next_PCB->Next_PCB = NULL ;
                 tmp = tmp->Next_PCB ;
                }

        RQT = tmp ;
        RQT->Next_PCB = NULL ;
        }

struct PCB *GetNextProcess(struct PCB **Head)
  {struct PCB *tmp ;
   tmp = *Head ;
   if (tmp == NULL)
        {printf("No more processes to get fool!\n") ;
         exit(0) ;
        }
   *Head = (*Head)->Next_PCB ;
   tmp->Next_PCB = NULL ;
   return(tmp) ;
  }


/*this function takes a pointer to the currently executing process and the address of the Ready
  Queue Tail. It moves the Current process to the tail of the list and updates RQT.
*/        
         
void MovetoTail(struct PCB *Current, struct PCB **RQT)
  {
   (*RQT)->Next_PCB = Current ;
   *RQT = Current ; 
   (*RQT)->Next_PCB = NULL ;
   if (RQ == NULL)
	RQ = *RQT ;
  }          


void PrintQ(struct PCB *Head)
 {struct PCB *tmp ;
  tmp = Head ;
  if (tmp == NULL)
        {
         printf("Ready Queue is empty!\n") ;
         return ;
        }
  while(tmp != NULL)
        {printf("Process ID %d. \n", tmp->PID) ;
         tmp = tmp->Next_PCB ;
        }
}
