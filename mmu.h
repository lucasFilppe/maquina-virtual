#ifndef MMU_H
#define MMU_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define TAM_BLOCO 4
#define CUSTO_RAM 1000 // Custo alto para simular lentidão da RAM
#define CUSTO_L3 100
#define CUSTO_L2 10
#define CUSTO_L1 1

// Estrutura baseada na classe BlocoMemoria do PDF [cite: 135]
typedef struct {
    int palavras[TAM_BLOCO]; // 4 inteiros
    int endBloco;            // Tag (qual bloco da RAM é esse?)
    bool atualizado;         // Dirty bit (para Write-Back)
    int custo;               // Para o LRU
    bool valido;             // Se a linha tem dado útil
    int cacheHit;            // Contador de hits (opcional, para estatística)
} LinhaCache;

// Definição das Memórias
typedef struct {
    LinhaCache* linhas;
    int tamanho;   // Quantidade de linhas
    int hits;
    int misses;
} Cache;

typedef struct {
    LinhaCache* blocos; // A RAM agora é um vetor de Blocos
    int tamanho;        // Quantidade total de blocos
} RamBloco;

// A UCM que agrupa tudo 
typedef struct {
    Cache* l1;
    Cache* l2;
    Cache* l3;
    RamBloco* ram;
} MMU;

// Funções
MMU* MMU_criar(int sizeRAM, int sizeL1, int sizeL2, int sizeL3);
bool inserirExclusivo(Cache* c, LinhaCache* bloco, LinhaCache* vitima);
int MMU_buscar(MMU* mmu, int enderecoFisico);
void MMU_escrever(MMU* mmu, int enderecoFisico, int valor);
void MMU_liberar(MMU* mmu);

#endif