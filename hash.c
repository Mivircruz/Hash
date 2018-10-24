#include <stdio.h>
#include <stdlib.h>
#include "hash.h"
#include "lista.h"

#define CAPACIDAD_INICIAL		7

struct hash{
	lista_t* tabla;
	size_t cantidad;
	size_t capacidad;
	hash_destruir_dato_t hash_destruir_dato;
};

hash_t* hash_crear(hash_destruir_dato_t destruir_dato){

	hash_t* hash = malloc(sizeof(hash_t));
	if(!hash)
		return NULL;

	hash->tabla = malloc(sizeof(lista_t)*CAPACIDAD_INICIAL);
	if(!hash->tabla){
		free(hash);
		return NULL;
	}
	hash->capacidad = CAPACIDAD_INICIAL;
	hash->cantidad = 0;
}