#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>

#define NUM_CHILD 5

int doSomething(void)
{
    int ret;
    srand(getpid());
    ret = (rand() % 256);
    printf("HIJO: PID = %d, valor = %d \n", getpid(), ret);
    return ret;
}

int main()
{
    pid_t pidC;
    int status;
    for (int i = 0; i < NUM_CHILD; i++)
    {
        pidC = fork();
        if (pidC > 0)
            continue;
        else if (pidC == 0)
        {
           /*  doSomething(); */
            exit(doSomething());
        }
        else
        {
            printf(" -- Error --");
        }
    }

    for (int i = 0; i < NUM_CHILD; i++)
    {
        /* code */
       pidC = wait(&status);
       printf("PADRE>> PID: %d, hijo de PID = %d terminado, st = %d \n", getpid(), pidC, WEXITSTATUS(status));
    }
    

    while (1)
    {
        sleep(10);
    }
    

    return 0;
}