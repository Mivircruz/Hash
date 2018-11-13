 #define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hash.h"

#define CAPACIDAD_INICIAL		97
#define CAPACIDAD_MINIMA		13
#define FACTOR_DE_CARGA			0.7
#define FACTOR_REDIMENSION		2


//Función de hashing

size_t funcion_hash(const char* cadena, size_t hash_capacidad){

	size_t valor_hash;
	for(valor_hash = 0; *cadena != '\0'; cadena++)
		valor_hash = *cadena + 11 * valor_hash;
	return valor_hash % hash_capacidad;
}

typedef enum{
	REDIMENSION_AGRANDAR = 1,
	REDIMENSION_ACHICAR  = 0
}redimension_t;


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
	size_t posicion;
	size_t cantidad_final_hash;
} hash_iter_t;


/* *****************************************************************
 *        			    FUNCIONES AUXILIARES
 * *****************************************************************/

//Inicializa todos los estados el LIBRE si el hash fue recién creado.
//Si se está redimensionando el hash, inicializa en LIBRE los BORRADOS.
void inicializar_estados(hash_t* hash){

	for(size_t i = 0; i < hash->capacidad; i++)
		hash->tabla[i].estado = LIBRE;
}

//Recorre el arreglo devolviendo la posición en la que se encuentra la clave.
//En casp de no encontrarla, devuelve la capacidad del hash.
size_t hash_buscar_clave(const hash_t* hash, const char* clave){

	if (!hash || !hash->cantidad)
		return hash->capacidad;

	size_t indice = funcion_hash(clave, hash->capacidad);
	bool pertenece = false;

	for(; hash->tabla[indice].estado != LIBRE; indice++){

		if(indice == hash->capacidad)
				indice = 0;
		if(hash->tabla[indice].estado == BORRADO)
			continue;

		if(!strcmp(hash->tabla[indice].clave, clave)){
			pertenece = true;
			break;
		}

	}
	return (pertenece) ? indice : hash->capacidad;

}

bool _guardar(hash_t* hash, const char* clave, void* dato){

	//Si la clave esta en uso, sobrescribe el valor sin alterar la cantidad presente.
	size_t posicion_clave = hash_buscar_clave(hash, clave);
	if(posicion_clave != hash->capacidad){
		if(hash->destruir_dato)
			hash->destruir_dato(hash->tabla[posicion_clave].dato);
		hash->tabla[posicion_clave].dato = dato;
		return true;
	}

	//Si la posicion esta libre, se guarda el dato
	else{
		size_t indice = funcion_hash(clave, hash->capacidad);

		for(; hash->tabla[indice].estado != LIBRE; indice++){

			if(indice == hash->capacidad)
				indice = 0;
		}
		hash->tabla[indice].clave = strdup(clave);
		hash->tabla[indice].dato = dato;
		hash->tabla[indice].estado = OCUPADO;
		hash->cantidad++;

	}
	
	return true;
}

//Busca el siguiente elemento ocupado en el hash. 
//Si no lo encuentra, devuelve la capacidad del hash.
size_t hash_siguiente_ocupado(hash_iter_t* iter, size_t inicio){

	if(inicio >= iter->cantidad_final_hash)
    	return iter->cantidad_final_hash;

    size_t i;
	for(i = inicio; i < iter->cantidad_final_hash; i++){
		if(iter->tabla[i].estado == OCUPADO)
			break;
	}
	return i;
}

bool hash_a_redimensionar(hash_t* hash, redimension_t redimension){

  	if(!hash->tabla)
    	return false;

    size_t capacidad_nueva;
    size_t capacidad_vieja = hash->capacidad;
    
    if(redimension)
		capacidad_nueva = capacidad_vieja * FACTOR_REDIMENSION;
	else{
		if(hash->capacidad <= CAPACIDAD_MINIMA)
			return true;
		capacidad_nueva = capacidad_vieja / FACTOR_REDIMENSION;
	}

	hash_campo_t* aux = malloc(sizeof (hash_campo_t) * capacidad_nueva);
	if(!aux)
		return NULL;
	
	hash_campo_t* tabla_vieja = hash->tabla;
	hash->tabla = aux;
	hash->capacidad = capacidad_nueva;
	hash->cantidad = 0;
 	inicializar_estados(hash);
	for(size_t i = 0; i < capacidad_vieja; i++){
		if(tabla_vieja[i].estado == OCUPADO) {
			_guardar(hash, tabla_vieja[i].clave, tabla_vieja[i].dato);
			free(tabla_vieja[i].clave);
		}
	}
	free(tabla_vieja);
	return true;

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
	inicializar_estados(hash);
	return hash;
}

size_t hash_cantidad(const hash_t *hash){
	return hash->cantidad;
}

void *hash_borrar(hash_t *hash, const char *clave){

	size_t indice = hash_buscar_clave(hash,clave);
	if(indice == hash->capacidad)
		return NULL;

	free(hash->tabla[indice].clave);
	hash->tabla[indice].estado = BORRADO;
	hash->cantidad--;

	return hash->tabla[indice].dato;
}

void hash_destruir(hash_t *hash){

	if(hash_cantidad(hash)){
		for(size_t i = 0; i < hash->capacidad; i++){

			if(hash->tabla[i].estado == OCUPADO){
				if(hash->destruir_dato)
					hash->destruir_dato(hash->tabla[i].dato);
				free(hash->tabla[i].clave);
			}
		}
	}
	free(hash->tabla);
	free(hash);
}

bool hash_guardar(hash_t *hash, const char *clave, void *dato){

	//En el caso de que el Hash haya alcanzado el factor de carga o su capacidad supere en 4 a la cantidad de elementos almacenados, se redimensiona.

	if(((float)hash->cantidad /(float)hash->capacidad) > FACTOR_DE_CARGA){
		if(!hash_a_redimensionar(hash, REDIMENSION_AGRANDAR))
			return false;
	}
	if(hash->cantidad <= hash->capacidad/4){
		if(!hash_a_redimensionar(hash, REDIMENSION_ACHICAR))
			return false;
	}

	//Se procede a guardar el dato.

	return _guardar(hash, clave, dato);
	
}

bool hash_pertenece(const hash_t *hash, const char *clave){

	return (hash_buscar_clave(hash,clave) != hash->capacidad) ? true : false;
}


void *hash_obtener(const hash_t *hash, const char *clave){

	long int indice;
	if((indice = hash_buscar_clave(hash,clave)) == hash->capacidad)
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
	hash_iter->cantidad_final_hash = hash->capacidad;
	if(hash->cantidad)
		hash_iter->posicion = hash_siguiente_ocupado(hash_iter,0);
	else
		hash_iter->posicion = hash->capacidad;
	return hash_iter;
}

bool hash_iter_al_final(const hash_iter_t *iter){

	if(iter->posicion == iter->cantidad_final_hash)
		return true;
	return false;
}

bool hash_iter_avanzar(hash_iter_t *iter){

	if(!iter)
		return false;
	if(hash_iter_al_final(iter))
		return false;
	
	iter->posicion = hash_siguiente_ocupado(iter, iter->posicion+1);
	return (iter->posicion == iter->cantidad_final_hash) ? false : true;
}
const char *hash_iter_ver_actual(const hash_iter_t *iter){

	if (!iter || iter->posicion == iter->cantidad_final_hash)
		return NULL;

	return iter->tabla[iter->posicion].clave;
}

void hash_iter_destruir(hash_iter_t* iter){
	free(iter);
}
