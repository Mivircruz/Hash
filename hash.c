#include <stdio.h>
#include <stdlib.h>
#include "hash.h"
#include "lista.h"

#define CAPACIDAD_INICIAL		7

//Función de hashing

size_t funcion_hash(const char* cadena, size_t hash_capacidad){

	size_t valor_hash;
	for(valor_hash = 0; *cadena != '\0'; cadena++)
		valor_hash = *cadena + 11 * valor_hash;
	return valor_hash % hash_capacidad;
}

/* *****************************************************************
 *             DEFINICIÓN DE  LAS ESTRUCTURAS HASH E ITERADOR
 * *****************************************************************/

struct hash{
	lista_t* tabla;
	size_t cantidad;
	size_t capacidad;
	hash_destruir_dato_t destruir_dato;
};

/* *****************************************************************
 *                   PRIMITIVAS DEL HASH
 * *****************************************************************/

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

