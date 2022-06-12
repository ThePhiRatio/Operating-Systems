#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int IDs[4] ;
int BigA[1024][1024] ;
pthread_t tid[4] ;
void *Worker(void *) ;
int Final_Dist[10] ;

int main()
 {
   //change seed for random each time.
   srand(time(0));
   int Seed = rand();


   int i,j,k ;
   for(i = 0 ; i < 10 ; i++)
    Final_Dist[i] = 0 ;
  
   for(k = 0; k < 4 ; k++)
         { IDs[k] = k ;
         j = pthread_create(&(tid[k]), NULL, Worker,(void *) &(IDs[k])) ; //note type cast
         if (j != 0)
            {printf("Pthread create Failed. Gonna Bail!!\n") ;
             exit(0) ;
            }
        }
  
   for(k = 0 ; k < 4; k++)
      pthread_join(tid[k], NULL) ;
  
   for(i = 0 ; i < 10 ; i++)
      printf("Final_Dist[%d] = %d\n", i, Final_Dist[i])  ;

   //Total the sum and see if it is bounded correctly.
   int sum = 0;
   for(int l = 0; l < 10; l++)
   {
	sum += Final_Dist[l];
   }
   if(sum == 1024*1024)
   {
   	printf("The sum is bounded correctly.\n");
	printf("Sum: %d, 1024*1024: %d, Difference: %d\n",sum,1024*1024,(1024*1024)-sum);
   }
   else
   {	
	printf("Crap! Math Broke!\n"); 
   	printf("Sum: %d, 1024*1024: %d, Difference: %d\n",sum,1024*1024,(1024*1024)-sum);
   }	
 }
 

void *Worker(void *My_ID)
    {   
	   int my_count[10] ;
           int my_id = *((int *)My_ID) ;
        //perform thread operations
	   for(int q = 0; q < 10; q++)
	   	my_count[q] = 0;	

	   int min = my_id * 256;
	   int max = min + 256;

	   for(int row = min; row < max; row++)
	   {
		   for(int col = 0; col < 1024; col++)
		   {	
			BigA[row][col] = rand() % 10;
			my_count[BigA[row][col]] += 1;
	           }
	   }

	   for(int f = 0; f < 10; f++)
	   {
		   //acccessing global variable
		   Final_Dist[f] += my_count[f];
	   }
    }
