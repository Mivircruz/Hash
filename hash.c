 #define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hash.h"

#define CAPACIDAD_INICIAL		7
#define FACTOR_DE_CARGA			0.7
#define FACTOR_REDIMENSION		2

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

typedef struct hash_iter{
  hash_campo_t* tabla;
  long int posicion;
  size_t cantidad_final_hash;
  size_t nodo_contador;
} hash_iter_t;

/* *****************************************************************
 *        			    FUNCIONES AUXILIARES
 * *****************************************************************/

//Inicializa todos los estados el LIBRE si el hash fue recién creado.
//Si se está redimensionando el hash, inicializa en LIBRE los BORRADOS.
void inicializar_estados(hash_t* hash, size_t ini){

	size_t i = 0;
	for(; i < ini; i++){
		if(hash->tabla[i].estado == BORRADO)
			hash->tabla[i].estado = LIBRE;
	}
	for(; i < hash->capacidad; i++){
		hash->tabla[i].estado = LIBRE;
  }
}

//Recorre el arreglo devolviendo la posición en la que se encuentra la clave.
//En casp de no encontrarla, devuelve -1
long int hash_recorrer(const hash_t* hash, const char* clave){

	if (!hash || !hash->cantidad)
		return -1;

	size_t indice = funcion_hash(clave, hash->capacidad);
	bool pertenece = false;

	for(size_t contador = 0; contador < hash->capacidad; contador++, indice++){

		if(indice == hash->capacidad)
				indice = 0;
		if(hash->tabla[indice].estado == LIBRE || hash->tabla[indice].estado == BORRADO)
			continue;
     	if(!strcmp(hash->tabla[indice].clave, clave)){
	        pertenece = true;
	        break;
    	}
	}
	return (pertenece) ? (long int)indice : -1;

}

long int iter_hash_recorrer(hash_iter_t* iter, size_t inicio){

	long int i;
	for(i = inicio; iter->tabla[i].estado != OCUPADO; i++);
	return i;
}

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

	long int indice = hash_recorrer(hash,clave);
	if(indice == -1)
		return NULL;

	void* a_borrar = hash->tabla[indice].dato;
	if(hash->destruir_dato)
		hash->destruir_dato(hash->tabla[indice].dato);
	free(hash->tabla[indice].clave);
	hash->tabla[indice].estado = BORRADO;
	hash->cantidad--;
	
	return a_borrar;
}

void hash_destruir(hash_t *hash){
	for(size_t i = 0; i < hash->capacidad; i++){
		if(hash->tabla[i].estado == OCUPADO){
			if(hash->destruir_dato)
				hash->destruir_dato(hash->tabla[i].dato);
			free(hash->tabla[i].clave);
		}
	}
	free(hash->tabla);
	free(hash);
}

bool hash_guardar(hash_t *hash, const char *clave, void *dato){

	size_t indice = funcion_hash( clave,hash->capacidad);

	//En el caso de que el Hash haya alcanzado el factor de carga, se redimensiona.

	if((float)hash->cantidad /(float)hash->capacidad > FACTOR_DE_CARGA){
		size_t capacidad_inicial = hash->capacidad;
		hash_campo_t* aux = realloc(hash->tabla, sizeof(hash_campo_t)* capacidad_inicial*FACTOR_REDIMENSION);
		if(!aux)
			return false;
		hash->tabla = aux;
		hash->capacidad = capacidad_inicial * FACTOR_REDIMENSION;
		inicializar_estados(hash, capacidad_inicial);
	}

	//Se procede a guardar el dato.

	for(; hash->tabla[indice].estado != LIBRE; indice++){

		if(indice == hash->capacidad)
			indice = 0;

		//Si la clave esta en uso, sobrescribe el valor sin alterar la cantidad presente.

		if(hash->tabla[indice].estado == OCUPADO){
			if(!strcmp(hash->tabla[indice].clave, clave)){
				if(hash->destruir_dato)
	    			hash->destruir_dato(hash->tabla[indice].dato);
	    		hash->tabla[indice].dato = dato;
	    		return true;
			}
		}
	}

    //Si la posicion esta libre, se guarda el dato

	hash->tabla[indice].clave = strdup(clave);
	hash->tabla[indice].dato = dato;
	hash->tabla[indice].estado = OCUPADO;
	hash->cantidad++;

	return true;
}

bool hash_pertenece(const hash_t *hash, const char *clave){

	return (hash_recorrer(hash,clave) != -1) ? true : false;
}


void *hash_obtener(const hash_t *hash, const char *clave){

	long int indice;
	if((indice = hash_recorrer(hash,clave)) == -1)
		return NULL;

	return hash->tabla[indice].dato;
}

/********************************************************
				ITERADORES EXTERNOS
*********************************************************/

hash_iter_t *hash_iter_crear(const hash_t *hash){

	if(!hash)
    	return NULL;

	hash_iter_t* hash_iter = malloc(sizeof(hash_iter_t));
	if(!hash_iter)
    	return NULL;

	hash_iter->tabla = hash->tabla;
	hash_iter->nodo_contador = 0;
	if(!hash->cantidad || hash->cantidad == 1){
		hash_iter->cantidad_final_hash = hash->cantidad;
		if(!hash->cantidad)
			hash_iter->posicion = -1;
	}
	else{
		hash_iter->cantidad_final_hash = hash->cantidad-1;
		hash_iter->posicion = iter_hash_recorrer(hash_iter,0);
	}
	return hash_iter;
}

bool hash_iter_al_final(const hash_iter_t *iter){

  return (iter->nodo_contador == iter->cantidad_final_hash) ? true : false;
}

bool hash_iter_avanzar(hash_iter_t *iter){

	if(!iter)
		return false;
	if(hash_iter_al_final(iter))
    	return false;

    iter->posicion = iter_hash_recorrer(iter, iter->posicion+1);
    iter->nodo_contador++;
	return true;
}
const char *hash_iter_ver_actual(const hash_iter_t *iter){

  if (!iter || iter->posicion == -1)
    return NULL;

  return iter->tabla[iter->posicion].clave;
}

void hash_iter_destruir(hash_iter_t* iter){
	free(iter);
}
