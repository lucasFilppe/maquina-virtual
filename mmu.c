#include "mmu.h"

// ============================================================================
// CONFIGURAÇÃO DA POLÍTICA DE SUBSTITUIÇÃO
// ============================================================================
// Define qual algoritmo será usado quando a cache estiver cheia e precisarmos 
// "expulsar" um bloco para dar lugar a um novo.
// 0 = LRU (Least Recently Used): Remove o dado usado há mais tempo (o mais "velho").
// 1 = LFU (Least Frequently Used): Remove o dado menos acessado (o menos "popular").
#define POLITICA 0  

/**
 * Função auxiliar para criar e inicializar uma Cache.
 * * @param tamanho: Número de linhas (slots) que a cache terá.
 * @return: Um ponteiro para a estrutura Cache alocada.
 */
Cache* criarCache(int tamanho) {
    // Aloca memória para a estrutura de controle da Cache
    Cache* c = (Cache*)malloc(sizeof(Cache));
    
    // Aloca o vetor de linhas (onde os dados realmente ficam)
    c->linhas = (LinhaCache*)malloc(sizeof(LinhaCache) * tamanho);
    
    c->tamanho = tamanho;
    c->hits = 0;   // Contador de acertos
    c->misses = 0; // Contador de erros

    // Inicialização segura: Percorre todas as linhas para garantir que 
    // não haja "lixo de memória".
    for(int i=0; i<tamanho; i++) {
        c->linhas[i].valido = false;    // Indica que a linha está vazia
        c->linhas[i].custo = 0;         // Variável dupla: serve para idade (LRU) ou frequência (LFU)
        c->linhas[i].atualizado = false;// Flag usada em políticas Write-Back (não usada aqui no Write-Through)
    }
    return c;
}

/**
 * Função Principal de Inicialização da MMU (Memory Management Unit).
 * Cria a RAM e os 3 níveis de Cache.
 */
MMU* MMU_criar(int sizeRAM, int sizeL1, int sizeL2, int sizeL3) {
    MMU* mmu = (MMU*)malloc(sizeof(MMU));
    
    // --- Configuração da RAM (Memória Principal) ---
    // A RAM é simulada como um vetor gigante de Blocos.
    mmu->ram = (RamBloco*)malloc(sizeof(RamBloco));
    mmu->ram->tamanho = sizeRAM;
    mmu->ram->blocos = (LinhaCache*)malloc(sizeof(LinhaCache) * sizeRAM);
    
    // Inicializa a RAM com zeros
    for(int i=0; i<sizeRAM; i++) {
        mmu->ram->blocos[i].endBloco = i; // O endereço do bloco é o próprio índice
        for(int j=0; j<TAM_BLOCO; j++) {
            mmu->ram->blocos[i].palavras[j] = 0;
        }
    }

    // --- Configuração das Caches ---
    mmu->l1 = criarCache(sizeL1); // Cache pequena e rápida
    mmu->l2 = criarCache(sizeL2); // Cache média
    mmu->l3 = criarCache(sizeL3); // Cache grande e lenta
    
    // Feedback visual para saber qual política está rodando
    if (POLITICA == 0) printf("Politica Ativa: LRU (Least Recently Used)\n");
    else printf("Politica Ativa: LFU (Least Frequently Used)\n");
    
    return mmu;
}

// ============================================================================
// LÓGICA DE GERENCIAMENTO DE SUBSTITUIÇÃO (LRU / LFU)
// ============================================================================

/**
 * Atualiza o campo 'custo' das linhas da cache após um acesso.
 * Esta função é o coração da política de substituição.
 */
void atualizaCustos(Cache* c, int indiceAcessado) {
    if (POLITICA == 0) {
        // === LÓGICA LRU (Menos Recentemente Usado) ===
        // Objetivo: Manter o registro de "idade" do acesso.
        // 1. O índice acessado vira 0 (ele é o "bebê", o mais novo).
        // 2. Todos os outros válidos ganham +1 (envelhecem).
        for(int i=0; i<c->tamanho; i++) {
            if (i == indiceAcessado) {
                c->linhas[i].custo = 0; 
            } else if (c->linhas[i].valido) {
                c->linhas[i].custo++;
            }
        }
    } else {
        // === LÓGICA LFU (Menos Frequentemente Usado) ===
        // Objetivo: Contar popularidade.
        // Apenas incrementamos o contador de quem foi acessado.
        c->linhas[indiceAcessado].custo++;
    }
}

/**
 * Decide quem será "expulso" da cache quando ela estiver cheia.
 * @return: O índice da linha que será sobrescrita.
 */
int obterIndiceParaSubstituir(Cache* c) {
    // 1. Prioridade Absoluta: Espaço Vazio
    // Se houver alguma linha inválida (vazia), usamos ela antes de expulsar alguém.
    for(int i=0; i<c->tamanho; i++) {
        if (!c->linhas[i].valido) return i;
    }
    
    int indiceEscolhido = 0;
    
    if (POLITICA == 0) {
        // === LRU: Expulsa o MAIS VELHO ===
        // Procuramos quem tem o MAIOR custo (maior tempo sem uso).
        int maiorCusto = -1;
        for(int i=0; i<c->tamanho; i++) {
            if (c->linhas[i].custo > maiorCusto) {
                maiorCusto = c->linhas[i].custo;
                indiceEscolhido = i;
            }
        }
    } else {
        // === LFU: Expulsa o MENOS POPULAR ===
        // Procuramos quem tem o MENOR custo (menor frequência de uso).
        // Nota: Isso pode sofrer de "memória de elefante" (dados antigos muito usados não saem).
        int menorFrequencia = c->linhas[0].custo;
        for(int i=1; i<c->tamanho; i++) {
            if (c->linhas[i].custo < menorFrequencia) {
                menorFrequencia = c->linhas[i].custo;
                indiceEscolhido = i;
            }
        }
    }
    return indiceEscolhido;
}

/**
 * Função responsável por trazer um bloco de um nível inferior (RAM/L3/L2)
 * para um nível superior. Realiza a substituição se necessário.
 */
void copiarBlocoParaCache(Cache* dest, LinhaCache* blocoOrigem) {
    // 1. Escolhe onde guardar (vazio ou vítima da substituição)
    int idx = obterIndiceParaSubstituir(dest);
    
    // 2. Copia os dados (as 4 palavras do bloco)
    for(int i=0; i<TAM_BLOCO; i++) {
        dest->linhas[idx].palavras[i] = blocoOrigem->palavras[i];
    }
    
    // 3. Atualiza Metadados (Tag e Valid Bit)
    dest->linhas[idx].endBloco = blocoOrigem->endBloco; 
    dest->linhas[idx].valido = true;
    dest->linhas[idx].atualizado = false;
    
    // 4. Inicializa o custo do novo morador
    if (POLITICA == 0) {
        dest->linhas[idx].custo = 0; // LRU: Nasce com idade 0 (novo)
    } else {
        dest->linhas[idx].custo = 1; // LFU: Nasce com 1 acesso
    }
    
    // No caso do LRU, precisamos envelhecer os vizinhos
    if (POLITICA == 0) {
        atualizaCustos(dest, idx); 
    }
}

// ============================================================================
// BUSCA E MAPEAMENTO
// ============================================================================

/**
 * Realiza uma busca na Cache usando Mapeamento Totalmente Associativo.
 * Associativo Total = O bloco pode estar em QUALQUER linha.
 * Por isso, precisamos percorrer a cache inteira com um 'for'.
 */
int buscarEmCache(Cache* c, int tagBloco) {
    for(int i=0; i<c->tamanho; i++) {
        // Verifica se a linha tem dados válidos E se a Tag (endereço do bloco) bate
        if(c->linhas[i].valido && c->linhas[i].endBloco == tagBloco) {
            atualizaCustos(c, i); // Hit! Atualiza a idade (LRU) ou frequência (LFU)
            return i; // Retorna o índice onde achou
        }
    }
    return -1; // Retorna -1 se deu MISS (não achou)
}

/**
 * Função Principal de Leitura (UCM - Unidade de Controle de Memória).
 * Simula a hierarquia de memória em cascata: L1 -> L2 -> L3 -> RAM.
 */
int MMU_buscar(MMU* mmu, int enderecoFisico) {
    // Calcula qual é o bloco e qual a palavra dentro do bloco (Offset)
    int tagBloco = enderecoFisico / TAM_BLOCO;
    int offset = enderecoFisico % TAM_BLOCO;
    
    // --- Nível 1: Cache L1 (Mais rápida) ---
    int idx = buscarEmCache(mmu->l1, tagBloco);
    if (idx != -1) {
        mmu->l1->hits++; // Estatística
        return mmu->l1->linhas[idx].palavras[offset];
    }
    mmu->l1->misses++; // Não achou na L1

    // --- Nível 2: Cache L2 ---
    idx = buscarEmCache(mmu->l2, tagBloco);
    if (idx != -1) {
        mmu->l2->hits++;
        // Propriedade de Inclusão: Se achou na L2, copia para a L1
        copiarBlocoParaCache(mmu->l1, &mmu->l2->linhas[idx]);
        return mmu->l2->linhas[idx].palavras[offset];
    }
    mmu->l2->misses++;

    // --- Nível 3: Cache L3 ---
    idx = buscarEmCache(mmu->l3, tagBloco);
    if (idx != -1) {
        mmu->l3->hits++;
        // Propriedade de Inclusão: Sobe para L2 e L1
        copiarBlocoParaCache(mmu->l2, &mmu->l3->linhas[idx]);
        copiarBlocoParaCache(mmu->l1, &mmu->l3->linhas[idx]);
        return mmu->l3->linhas[idx].palavras[offset];
    }
    mmu->l3->misses++;

    // --- Nível 4: RAM (Memória Principal) ---
    // Se chegou aqui, é um Miss total. Busca na fonte original.
    if (tagBloco >= mmu->ram->tamanho) return 0; // Proteção de limites
    
    LinhaCache* blocoRam = &mmu->ram->blocos[tagBloco];
    
    // Traz o dado da RAM preenchendo todas as caches (Princípio da Localidade)
    copiarBlocoParaCache(mmu->l3, blocoRam);
    copiarBlocoParaCache(mmu->l2, blocoRam);
    copiarBlocoParaCache(mmu->l1, blocoRam);
    
    return blocoRam->palavras[offset];
}

/**
 * Função de Escrita usando Política WRITE-THROUGH.
 * Write-Through: Escreve na RAM imediatamente (segurança) e atualiza
 * as caches apenas se o dado já estiver lá (coerência).
 */
void MMU_escrever(MMU* mmu, int enderecoFisico, int valor) {
    int tagBloco = enderecoFisico / TAM_BLOCO;
    int offset = enderecoFisico % TAM_BLOCO;

    // 1. Escreve diretamente na RAM (Garante que o dado está salvo)
    if (tagBloco < mmu->ram->tamanho) {
        mmu->ram->blocos[tagBloco].palavras[offset] = valor;
    }

    // 2. Atualiza as caches se o bloco estiver carregado (Write Update)
    // Isso mantém a coerência: se a CPU ler da cache depois, pegará o valor novo.
    
    int idx = buscarEmCache(mmu->l1, tagBloco);
    if (idx != -1) {
        mmu->l1->linhas[idx].palavras[offset] = valor;
        atualizaCustos(mmu->l1, idx); // Escrita também conta como uso!
    }

    idx = buscarEmCache(mmu->l2, tagBloco);
    if (idx != -1) {
        mmu->l2->linhas[idx].palavras[offset] = valor;
        atualizaCustos(mmu->l2, idx);
    }
    
    idx = buscarEmCache(mmu->l3, tagBloco);
    if (idx != -1) {
        mmu->l3->linhas[idx].palavras[offset] = valor;
        atualizaCustos(mmu->l3, idx);
    }
}

// Libera toda a memória alocada pelo simulador
void MMU_liberar(MMU* mmu) {
    free(mmu->l1->linhas); free(mmu->l1);
    free(mmu->l2->linhas); free(mmu->l2);
    free(mmu->l3->linhas); free(mmu->l3);
    free(mmu->ram->blocos); free(mmu->ram);
    free(mmu);
}