 #define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hash.h"

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

	if(!hash_pertenece(hash,clave))
		return NULL;

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

		if( *(hash->tabla[indice]).clave == *clave){// se agrego "*" porque asi se compara lo apuntado
			a_borrar = hash->tabla[indice].dato;
			hash->destruir_dato(hash->tabla[indice].dato);
			hash->tabla[indice].estado = BORRADO;
			break;
		}
	}
	return a_borrar;
}


void hash_destruir(hash_t *hash){
	for(size_t i = 0; i < hash->capacidad; i++){
		hash->destruir_dato(hash->tabla[i].dato);
	}
	free(hash->tabla);
	free(hash);
}

//Seccion de juancito
bool hash_guardar(hash_t *hash, const char *clave, void *dato){
	if (!hash)
		return false;
	char* a_guardar = strdup(clave);
	size_t pos_guardado = funcion_hash( clave,hash->capacidad);//coloca en pos_guardado el lugar donde guardara dato
	//FALTA cambiar el tamanno del HASH si el ALFA es > 0.7
	size_t alfa = ( ( hash_cantidad(hash) / hash->capacidad) * 10 );
	if (alfa < 10){
		//perfectamente modularizable
		size_t viejo_tamannio = hash->capacidad;
		size_t nuevo_tamannio = (hash->capacidad * 2);
		hash->tabla = realloc(hash->tabla, sizeof(hash_campo_t)* nuevo_tamannio);
		hash->capacidad = nuevo_tamannio;
		inicializar_estados(hash, viejo_tamannio);
	}
	//Redimensionar el hash a una nueva longitud
	while(true){ // Re turbio este loop, segu con un DO WHILE pasa
		if (hash->tabla[pos_guardado].estado == LIBRE){
			//guardar el dato
			hash->tabla[pos_guardado].clave = a_guardar;
			hash->tabla[pos_guardado].dato = dato;
			hash->tabla[pos_guardado].estado = OCUPADO;
			hash->cantidad++;
			return true;
		}
		else{
			pos_guardado++;
			if (pos_guardado == hash->capacidad)
				pos_guardado = 0;
		}
	}
	return false;//Nunca deberia salir por aqui
}

void *hash_obtener(const hash_t *hash, const char *clave){
	if (!hash || !hash->cantidad)
		return NULL;

    //Metodo rudimientario

	void* a_obtener = NULL;
	size_t indice = funcion_hash(clave, hash->capacidad);
	size_t inicio = indice-1; // SI inicio es -1 que pasa?
  //Recorrido rudimentario por el hash
	bool vuelta_completa = false;
		for(; !vuelta_completa; indice++){
      if( *(hash->tabla[indice]).clave == *clave){//Se agrego "*" porque sino se comparan PUNTEROS y no el contenido
        a_obtener = hash->tabla[indice].dato;
        break;
			if(indice == inicio)
				vuelta_completa = true;
			if(indice == hash->capacidad)
				indice = 0;
			}
		}
	return a_obtener;
}

bool hash_pertenece(const hash_t *hash, const char *clave){
	return (hash_obtener(hash,clave)) ? true : false;
}
