
#include<stdio.h> 
  
  
int main() 
{ 
    for(int i=0;i<5;i++) // loop will run n times (n=5) 
    { 
        if(fork() == 0) 
        { 
            printf("[son] pid %d from [parent] pid %d\n",getpid(),getppid()); 
            exit(0); 
        } 
    } 

    while (1)
    {
        /* code */
    }
    
    for(int i=0;i<5;i++) // loop will run n times (n=5) 
    wait(NULL); 
      
} 
