#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

int main(void)
{
    int fd[2];
    pid_t pidC;
    char buf[10] = "";
    int num;

    pipe(fd); /* TODO: error manamgent */
    pidC = fork();

    switch (pidC)
    {
    case 0: /* HIJO */
        close(fd[0]);             /* Descripto de leer */
        write(fd[1], "abcde", 5); /* 5 leng de buffer */
        close(fd[1]);             /* Cierro escritura */
        exit(0);
        break;
    case -1:
        /* TOSO: error */
        break;
    default:                                 /* Padre */
        close(fd[1]);                        /* Descripto de escritura */
        num = read(fd[0], buf, sizeof(buf)); /* 5 leng de buffer ! num : numero de bits que  se han leido */
        printf("Padre lee %d bytes: %s \n", num, buf);
        close(fd[0]); /* Cierre desacripto de lectura */
        /* exit(0); */
        break;
    }
    return 0;
}