#ifndef RAM_H
#define RAM_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define TAM_BLOCO 4  // Cada bloco tem 4 inteiros

// Estrutura do Bloco (Baseada na classe BlocoMemoria do PDF)
typedef struct {
    int palavras[TAM_BLOCO]; // Dados: vetor de 4 inteiros
    int endBloco;            // Endereço do bloco (Tag)
    int atualizado;          // Dirty bit (para write-back)
    int custo;               // Para LRU
    int valido;              // Se tem dado real
} BlocoMemoria;

// A RAM agora é um vetor de BLOCOS, não de inteiros
typedef struct {
    BlocoMemoria* blocos; // Vetor de blocos
    int tamanho;          // Quantidade TOTAL de blocos (não de palavras)
} Ram;

// Funções Atualizadas
void Ram_Criar(Ram *ram, int qtdBlocos);
void Ram_Liberar(Ram *ram);

// Note: Não precisamos mais de getDado/setDado aqui para a CPU.
// A MMU é quem vai acessar a RAM agora. Mas deixamos funções auxiliares
// para a MMU usar internamente se quiser.

void Ram_Imprimir(Ram *ram);

#endif