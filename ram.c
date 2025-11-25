#include <stdio.h>
#include "ram.h"
#include <stdlib.h>
#include <time.h>

//função que cria uma ram
 void Ram_Criar(Ram *ram, int tamanho){
  ram->memoria = (int*)malloc(tamanho * sizeof(int));
  if(ram->memoria == NULL){
    printf("Memoria insuficiente\n");
  }
  ram->tamanho = tamanho;
}

//função para liberar memoria
void Ram_Liberar(Ram *ram){
  free(ram->memoria);
  ram->memoria = NULL;
  ram->tamanho = 0;
}

//retorna valor da ram, dado um endereço
int Ram_getDado(Ram *ram, int endereco){
  if(endereco >= 0 && endereco < ram->tamanho){
    return ram->memoria[endereco];
  }
  else{
    printf("ERRO, o endereço esta fora dos limites\n");
    exit(1);
  }
}

//atribui algum valor a ram dado um endereço
int Ram_setDado(Ram *ram, int endereco, int valor){
  if(endereco >= 0 && endereco <ram->tamanho){
    ram->memoria[endereco] = valor;
    return valor; 
  }
  else{
    printf("ERRO, o endereço esta fora dos limites\n");
    exit(1);
  }
}

void gerar_RamAleatoria(Ram *ram, int valor_min, int valor_max){
  srand(time(NULL));//inicializa semente
  for(int i = 0; i < ram->tamanho; i++){
    ram->memoria[i] = valor_min +(rand() % valor_max - valor_min + 1);
  }
}

void Ram_Imprimir(Ram *ram) {
    if (ram == NULL || ram->memoria == NULL) {
        printf("RAM nao inicializada!\n");
        return;
    }

    printf("Conteudo da RAM (tamanho: %d):\n", ram->tamanho);
    printf("Endereco\tValor\n");
    printf("-----------------------\n");
    
    for (int i = 0; i < ram->tamanho; i++) {
        printf("%p[%d]\t%d\n", &ram->memoria[i], i, ram->memoria[i]);
    }
    printf("\n");
}