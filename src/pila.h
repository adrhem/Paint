#ifndef OPERACIONES_DE_PILA_H_INCLUDED
#define OPERACIONES_DE_PILA_H_INCLUDED
#include "pila.h"
//TODO
//CREAR PILA A PATA
typedef struct punto{
    int x,y, direccion;
}Punto;

typedef struct n{
    Punto p;
    struct n *ant;
}nodo;

typedef struct pila{
    nodo *tope;
}Pila;

int vacia(Pila p){
    return (p.tope==NULL);
}

Punto pop(Pila *p){
    //printf("Pop Pila: %d Tope: %p\n",p, p->tope);
    Punto punto = p->tope->p;
    nodo *aux = p->tope;
    p->tope=p->tope->ant;
    free(aux);
    return punto;
}

void inicializaPila(Pila *p){
    p->tope = NULL;
}

void insertar(Pila *p, int x, int y, int d){
    nodo *n = (nodo *)malloc(sizeof(nodo));
    n->p.x=x;
    n->p.y=y;
    n->p.direccion=d;
    if(!vacia(*p)){
        n->ant=p->tope;
    }else{
        n->ant=NULL;
    }
    p->tope = n;
}

#endif
