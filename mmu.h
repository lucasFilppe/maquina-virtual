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

// Estrutura unificada: serve para Linhas de Cache e Blocos de RAM
typedef struct {
    int palavras[TAM_BLOCO]; // 4 inteiros
    int endBloco;            // Tag (qual bloco da RAM/HD é esse?)
    bool atualizado;         // Dirty bit (para Write-Back ou Swap do HD)
    int custo;               // Para o LRU/LFU
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
    int tamanho;        // Quantidade total de blocos (limite físico da RAM)
    int hits;           // NOVO: Estatísticas da RAM
    int misses;         // NOVO: Estatísticas da RAM
} RamBloco;

// A UCM que agrupa tudo 
typedef struct {
    Cache* l1;
    Cache* l2;
    Cache* l3;
    RamBloco* ram;
    
    // ==========================================
    // 💽 ADIÇÕES PARA O TP3 (DISCO RÍGIDO)
    // ==========================================
    FILE* hd;        // Ponteiro para o arquivo binário (hd.bin)
    long acessos_hd; // Contador para medir o tráfego de I/O
} MMU;

// Funções
MMU* MMU_criar(int sizeRAM, int sizeL1, int sizeL2, int sizeL3);
bool inserirExclusivo(Cache* c, LinhaCache* bloco, LinhaCache* vitima);

// A RAM agora precisa de funções de busca e substituição idênticas às da Cache
int buscarEmRam(RamBloco* ram, int tagBloco);
int obterIndiceParaSubstituirRAM(RamBloco* ram);

int MMU_buscar(MMU* mmu, int enderecoFisico);
void MMU_escrever(MMU* mmu, int enderecoFisico, int valor);
void MMU_liberar(MMU* mmu);

#endif