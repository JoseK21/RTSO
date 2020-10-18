#ifndef _STRUCT_H
#define _STRUCT_H

//Martian struct//
typedef struct mnodo
{                               //mnodo es el nombre de la estructura
    int x;                      // Position x
    int y;                      // Position y
    float period;               // Weight
    float energy;               // Speed
    pthread_t id;               // Thread Id
    struct mnodo *next_martian; //El puntero siguiente para recorrer la lista enlazada
} martian;

#endif