 #define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hash.h"

#define CAPACIDAD_INICIAL		7
#define FACTOR_REDIMENSION		0.7

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
    hash->tabla[i].clave = NULL;//Para que cuando al recorrer al destruir, el FREE no explote
  }
}

//Compara el estado del hash con el factor de redimension. Devuelve true si
//se debe redimensionar el hash.
bool hash_a_redimensionar(hash_t* hash){

	float factor_crecimiento = (float)hash->cantidad /(float)hash->capacidad;
	return (factor_crecimiento < FACTOR_REDIMENSION) ? false : true;

}

//Recorre el arreglo devolviendo la posición en la que se encuentra la clave.
//En casp de no encontrarla, devuelve -1
int recorrer_hash(const hash_t* hash, const char* clave){

	if (!hash || !hash->cantidad)
		return -1;

	size_t indice = funcion_hash(clave, hash->capacidad);
	bool pertenece = false;

	for(size_t contador = indice; contador < hash->capacidad; contador++, indice++){

		if(indice == hash->capacidad)
				indice = 0;
		if(hash->tabla[indice].estado == LIBRE || hash->tabla[indice].estado == BORRADO)
			continue;
     	if(!strcmp(hash->tabla[indice].clave, clave)){
	        pertenece = true;
	        break;
    	}
	}
	return (pertenece) ? indice : -1;

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

		if(!strcmp(hash->tabla[indice].clave, clave)){
			a_borrar = hash->tabla[indice].dato;
			if(hash->destruir_dato)
				hash->destruir_dato(hash->tabla[indice].dato);
			hash->tabla[indice].estado = BORRADO;
      //free(hash->tabla[indice].clave); // Borro la clave, para que la necesito?
      //hash->tabla[indice].clave = NULL; // Para luego pasarlo de formasegura a FREE
			hash->cantidad--;
			break;
		}
	}
	return a_borrar;
}


void hash_destruir(hash_t *hash){
	for(size_t i = 0; i < hash->capacidad; i++){
    /*if (hash->destruir_dato)
		  hash->destruir_dato(hash->tabla[i].dato);*/
    free(hash->tabla[i].clave);
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
	bool redimensionar = hash_a_redimensionar(hash);
  if (redimensionar){
		//perfectamente modularizable
		size_t viejo_tamannio = hash->capacidad;
		size_t nuevo_tamannio = (hash->capacidad * 2);
		hash->tabla = realloc(hash->tabla, sizeof(hash_campo_t)* nuevo_tamannio);
		hash->capacidad = nuevo_tamannio;
		inicializar_estados(hash, viejo_tamannio);
	}

	//Redimensionar el hash a una nueva longitud
	while(true){ // Re turbio este loop, seguro con un DO WHILE pasa
    //Si la posicion esta libre guarda el dato
		if (hash->tabla[pos_guardado].estado == LIBRE){
			//guardar el dato
			hash->tabla[pos_guardado].clave = a_guardar;
			hash->tabla[pos_guardado].dato = dato;
			hash->tabla[pos_guardado].estado = OCUPADO;
			hash->cantidad++;
			return true;
		}
    //Si la clave esta en uso, sovreeescribo el valor sin alterar la cantidad presente
    else if (!strcmp( (hash->tabla[pos_guardado].clave) , clave)) {
      //free(hash->tabla[pos_guardado].clave); //Esto borra la vieja clave que hace perder memoria y la reemplaza por la nueva que ES LA MISMA en contenido
      hash->tabla[pos_guardado].clave = a_guardar;
      hash->tabla[pos_guardado].dato = dato;
      hash->tabla[pos_guardado].estado = OCUPADO;
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

bool hash_pertenece(const hash_t *hash, const char *clave){

	return (recorrer_hash(hash,clave) != -1) ? true : false;
}


void *hash_obtener(const hash_t *hash, const char *clave){

	int indice;
	if((indice = recorrer_hash(hash,clave)) == -1)
		return NULL;

	return hash->tabla[indice].dato;
}
