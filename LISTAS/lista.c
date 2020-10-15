#include <stdio.h>
#include <stdlib.h>

typedef struct mnodo
{ //mnodo es el nombre de la estructura
    int valor;
    int power;
    struct mnodo *sig; //El puntero siguiente para recorrer la lista enlazada
} martian;             //martian es el tipo de dato para declarar la estructura

typedef martian *tpuntero; //Puntero al tipo de dato martian para no utilizar punteros de punteros

void insertarEnLista(tpuntero *cabeza, int e, int w);
void imprimirLista(tpuntero cabeza);
void borrarLista(tpuntero *cabeza);

int main()
{
    int e;
    int w;
    tpuntero cabeza; //Indica la cabeza de la lista enlazada, si la perdemos no podremos acceder a la lista
    cabeza = NULL;   //Se inicializa la cabeza como NULL ya que no hay ningun nodo cargado en la lista

    printf("Ingrese elementos, -1 para terminar: ");
    scanf("%d %d", &e, &w);

    while (e != -1)
    {
        insertarEnLista(&cabeza, e, w);
        printf("Ingresado correctamente");
        printf("\n");
        printf("Ingrese elementos, -1 para terminar: ");
        scanf("%d %d", &e, &w);
    }

    printf("\nSe imprime la lista cargada: ");
    imprimirLista(cabeza);

    printf("\nSe borra la lista cargada\n");
    borrarLista(&cabeza);

    /*  printf("\n");
    system("PAUSE"); */

    return 0;
}

void insertarEnLista(tpuntero *cabeza, int e, int w)
{
    tpuntero nuevo;                  //Creamos un nuevo nodo
    nuevo = malloc(sizeof(martian)); //Utilizamos malloc para reservar memoria para ese nodo
    nuevo->valor = e;                //Le asignamos el valor ingresado por pantalla a ese nodo
    nuevo->power = w;                //Le asignamos el valor ingresado por pantalla a ese nodo
    nuevo->sig = *cabeza;            //Le asignamos al siguiente el valor de cabeza
    *cabeza = nuevo;                 //Cabeza pasa a ser el ultimo nodo agregado
}

void imprimirLista(tpuntero node)
{
    while (node != NULL)
    {                                                                  //Mientras node no sea NULL
        printf("\nValor: %d - Power: %d\n", node->valor, node->power); //Imprimimos el valor del nodo
        node = node->sig;                                              //Pasamos al siguiente nodo
    }
}

void borrarLista(tpuntero *cabeza)
{
    tpuntero actual; //Puntero auxiliar para eliminar correctamente la lista

    while (*cabeza != NULL)
    {                             //Mientras cabeza no sea NULL
        actual = *cabeza;         //Actual toma el valor de cabeza
        *cabeza = (*cabeza)->sig; //Cabeza avanza 1 posicion en la lista
        free(actual);             //Se libera la memoria de la posicion de Actual (el primer nodo), y cabeza queda apuntando al que ahora es el primero
    }
}