#include "mmu.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// ============================================================================
// CONFIGURAÇÃO DA POLÍTICA DE SUBSTITUIÇÃO
// ============================================================================
// 0 = LRU (Least Recently Used)
// 1 = LFU (Least Frequently Used)
#define POLITICA 0

/**
 * Função auxiliar para criar e inicializar uma Cache.
 */
Cache *criarCache(int tamanho)
{
    Cache *c = (Cache *)malloc(sizeof(Cache));
    c->linhas = (LinhaCache *)malloc(sizeof(LinhaCache) * tamanho);
    c->tamanho = tamanho;
    c->hits = 0;   
    c->misses = 0; 

    for (int i = 0; i < tamanho; i++)
    {
        c->linhas[i].valido = false;     
        c->linhas[i].custo = 0;          
        c->linhas[i].atualizado = false; 
    }
    return c;
}

/**
 * Função Principal de Inicialização da MMU
 */
MMU *MMU_criar(int sizeRAM, int sizeL1, int sizeL2, int sizeL3)
{
    MMU *mmu = (MMU *)malloc(sizeof(MMU));

    // --- Configuração da RAM ---
    mmu->ram = (RamBloco *)malloc(sizeof(RamBloco));
    mmu->ram->tamanho = sizeRAM;
    mmu->ram->blocos = (LinhaCache *)malloc(sizeof(LinhaCache) * sizeRAM);

    for (int i = 0; i < sizeRAM; i++)
    {
        mmu->ram->blocos[i].endBloco = i;
        for (int j = 0; j < TAM_BLOCO; j++)
        {
            mmu->ram->blocos[i].palavras[j] = 0;
        }
    }

    // --- Configuração das Caches ---
    mmu->l1 = criarCache(sizeL1); 
    mmu->l2 = criarCache(sizeL2); 
    mmu->l3 = criarCache(sizeL3); 

    if (POLITICA == 0)
        printf("Politica Ativa: LRU (Least Recently Used)\n");
    else
        printf("Politica Ativa: LFU (Least Frequently Used)\n");

    return mmu;
}

// ============================================================================
// LÓGICA DE GERENCIAMENTO DE SUBSTITUIÇÃO
// ============================================================================

void atualizaCustos(Cache *c, int indiceAcessado)
{
    if (POLITICA == 0) // LRU
    {
        for (int i = 0; i < c->tamanho; i++)
        {
            if (i == indiceAcessado)
            {
                c->linhas[i].custo = 0;
            }
            else if (c->linhas[i].valido)
            {
                c->linhas[i].custo++;
            }
        }
    }
    else // LFU
    {
        c->linhas[indiceAcessado].custo++;
    }
}

int obterIndiceParaSubstituir(Cache *c)
{
    for (int i = 0; i < c->tamanho; i++)
    {
        if (!c->linhas[i].valido)
            return i;
    }

    int indiceEscolhido = 0;

    if (POLITICA == 0) // LRU
    {
        int maiorCusto = -1;
        for (int i = 0; i < c->tamanho; i++)
        {
            if (c->linhas[i].custo > maiorCusto)
            {
                maiorCusto = c->linhas[i].custo;
                indiceEscolhido = i;
            }
        }
    }
    else // LFU
    {
        int menorFrequencia = c->linhas[0].custo;
        for (int i = 1; i < c->tamanho; i++)
        {
            if (c->linhas[i].custo < menorFrequencia)
            {
                menorFrequencia = c->linhas[i].custo;
                indiceEscolhido = i;
            }
        }
    }
    return indiceEscolhido;
}

// Insere e retorna vítima se houve substituição
// Retorna true se houve vítima
bool inserirExclusivo(Cache *c, LinhaCache *bloco, LinhaCache *vitima)
{
    int idx = obterIndiceParaSubstituir(c);
    bool houveVitima = c->linhas[idx].valido;

    if (houveVitima)
    {
        *vitima = c->linhas[idx]; // guarda vítima
    }

    c->linhas[idx] = *bloco;
    c->linhas[idx].valido = true;

    if (POLITICA == 0)
        atualizaCustos(c, idx);
    else
        c->linhas[idx].custo = 1; // Reseta frequência na nova inserção (LFU)

    return houveVitima;
}

// ============================================================================
// BUSCA E MAPEAMENTO
// ============================================================================

int buscarEmCache(Cache *c, int tagBloco)
{
    for (int i = 0; i < c->tamanho; i++)
    {
        if (c->linhas[i].valido && c->linhas[i].endBloco == tagBloco)
        {
            atualizaCustos(c, i); 
            return i;             
        }
    }
    return -1; 
}

/**
 * Função Principal de Leitura (Cascata Exclusiva L1 -> L2 -> L3 -> RAM)
 */
int MMU_buscar(MMU *mmu, int enderecoFisico)
{
    int tagBloco = enderecoFisico / TAM_BLOCO;
    int offset = enderecoFisico % TAM_BLOCO;
    int idx;

    // =========================
    // 🔵 1) L1
    // =========================
    idx = buscarEmCache(mmu->l1, tagBloco);
    if (idx != -1)
    {
        mmu->l1->hits++;
        return mmu->l1->linhas[idx].palavras[offset];
    }
    mmu->l1->misses++;

    // =========================
    // 🟡 2) L2
    // =========================
    idx = buscarEmCache(mmu->l2, tagBloco);
    if (idx != -1)
    {
        mmu->l2->hits++;

        LinhaCache bloco = mmu->l2->linhas[idx];
        mmu->l2->linhas[idx].valido = false; // Remove da L2

        LinhaCache vitimaL1;
        if (inserirExclusivo(mmu->l1, &bloco, &vitimaL1))
        {
            LinhaCache vitimaL2;
            if (inserirExclusivo(mmu->l2, &vitimaL1, &vitimaL2))
            {
                LinhaCache vitimaL3;
                if (inserirExclusivo(mmu->l3, &vitimaL2, &vitimaL3))
                {
                    mmu->ram->blocos[vitimaL3.endBloco] = vitimaL3;
                }
            }
        }
        return bloco.palavras[offset];
    }
    mmu->l2->misses++;

    // =========================
    // 🟠 3) L3
    // =========================
    idx = buscarEmCache(mmu->l3, tagBloco);
    if (idx != -1)
    {
        mmu->l3->hits++;

        LinhaCache bloco = mmu->l3->linhas[idx];
        mmu->l3->linhas[idx].valido = false; // Remove da L3

        LinhaCache vitimaL1;
        if (inserirExclusivo(mmu->l1, &bloco, &vitimaL1))
        {
            LinhaCache vitimaL2;
            if (inserirExclusivo(mmu->l2, &vitimaL1, &vitimaL2))
            {
                LinhaCache vitimaL3;
                if (inserirExclusivo(mmu->l3, &vitimaL2, &vitimaL3))
                {
                    mmu->ram->blocos[vitimaL3.endBloco] = vitimaL3;
                }
            }
        }
        return bloco.palavras[offset];
    }
    mmu->l3->misses++;

    // =========================
    // 🔴 4) RAM
    // =========================
    if (tagBloco >= mmu->ram->tamanho)
        return 0;

    LinhaCache bloco = mmu->ram->blocos[tagBloco];

    LinhaCache vitimaL1;
    if (inserirExclusivo(mmu->l1, &bloco, &vitimaL1))
    {
        LinhaCache vitimaL2;
        if (inserirExclusivo(mmu->l2, &vitimaL1, &vitimaL2))
        {
            LinhaCache vitimaL3;
            if (inserirExclusivo(mmu->l3, &vitimaL2, &vitimaL3))
            {
                mmu->ram->blocos[vitimaL3.endBloco] = vitimaL3;
            }
        }
    }

    return bloco.palavras[offset];
}

/**
 * Função de Escrita usando Política WRITE-THROUGH
 */
void MMU_escrever(MMU *mmu, int enderecoFisico, int valor)
{
    int tagBloco = enderecoFisico / TAM_BLOCO;
    int offset = enderecoFisico % TAM_BLOCO;

    // 1. Escreve na RAM
    if (tagBloco < mmu->ram->tamanho)
    {
        mmu->ram->blocos[tagBloco].palavras[offset] = valor;
    }

    // 2. Atualiza Caches (Write Update)
    int idx = buscarEmCache(mmu->l1, tagBloco);
    if (idx != -1)
    {
        mmu->l1->linhas[idx].palavras[offset] = valor;
        atualizaCustos(mmu->l1, idx);
    }

    idx = buscarEmCache(mmu->l2, tagBloco);
    if (idx != -1)
    {
        mmu->l2->linhas[idx].palavras[offset] = valor;
        atualizaCustos(mmu->l2, idx);
    }

    idx = buscarEmCache(mmu->l3, tagBloco);
    if (idx != -1)
    {
        mmu->l3->linhas[idx].palavras[offset] = valor;
        atualizaCustos(mmu->l3, idx);
    }
}

// Libera toda a memória
void MMU_liberar(MMU *mmu)
{
    free(mmu->l1->linhas);
    free(mmu->l1);
    free(mmu->l2->linhas);
    free(mmu->l2);
    free(mmu->l3->linhas);
    free(mmu->l3);
    free(mmu->ram->blocos);
    free(mmu->ram);
    free(mmu);
}