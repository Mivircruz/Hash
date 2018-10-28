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
 *        			    FUNCIONES AUXILIARES
 * *****************************************************************/

void inicializar_estados(hash_t* hash, size_t ini){

	size_t i = 0;
	for(; i < ini; i++){
		if(hash->tabla[i].estado == BORRADO)
			hash->tabla[i].estado = LIBRE;
	}
	for(; i < hash->capacidad; i++)
		hash->tabla[i].estado = LIBRE;
}

/* *****************************************************************
 *             DEFINICIÓN DE  LAS ESTRUCTURAS HASH E ITERADOR
 * *****************************************************************/
typedef enum{
	LIBRE,
	OCUPADO,
	BORRADO
}estado_t;

typedef struct hash_campo{
	char* clave;
	void* dato;
	estado_t estado;
}hash_campo_t;

struct hash{
	size_t cantidad;
	size_t capacidad;
	hash_campo_t* tabla;
	hash_destruir_dato_t destruir_dato;
};

/* *****************************************************************
 *                   PRIMITIVAS DEL HASH
 * *****************************************************************/

hash_t* hash_crear(hash_destruir_dato_t destruir_dato){

	hash_t* hash = malloc(sizeof(hash_t));
	if(!hash)
		return NULL;

	hash->tabla = malloc(sizeof(hash_campo_t)*CAPACIDAD_INICIAL);
	if(!hash->tabla){
		free(hash);
		return NULL;
	}
	hash->capacidad = CAPACIDAD_INICIAL;
	hash->cantidad = 0;
	hash->destruir_dato = destruir_dato;
	inicializar_estados(hash,0);
	return hash;
}

size_t hash_cantidad(const hash_t *hash){
	return hash->cantidad;
}

void *hash_borrar(hash_t *hash, const char *clave){

	size_t indice = funcion_hash(clave, hash->capacidad);
	size_t inicio = indice-1;
	void* a_borrar = NULL; 
	bool vuelta_completa = false;

	for(; !vuelta_completa; indice++){

		if(indice == inicio)
			vuelta_completa = true;

		if(indice == hash->cantidad+1){
			indice = 0;
		}

		if((hash->tabla[indice]).clave == clave){
			a_borrar = hash->tabla[indice].dato;
			hash->destruir_dato(hash->tabla[indice].dato);
			hash->tabla[indice].estado = BORRADO;
			break;
		}
	}
	return a_borrar;
}

bool hash_pertenece(const hash_t *hash, const char *clave){

	bool pertenece = false;
	if((void* dato = hash_borrar(hash, clave))){
		pertenece = true;
		hash_guardar(hash, clave);
	}
	return pertenece;
}


void hash_destruir(hash_t *hash){
	for(size_t i = 0; i < hash->capacidad; i++){

	}
}

//Seccion de juancito
bool hash_guardar(hash_t *hash, const char *clave, void *dato){
	if (!hash)
		return false;
	return false;
}

void *hash_obtener(const hash_t *hash, const char *clave){
	if (!hash)
		return NULL;
	return NULL;
}

