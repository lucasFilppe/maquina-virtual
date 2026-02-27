#include "mmu.h"

// ============================================================================
// CONFIGURAÇÃO DA POLÍTICA DE SUBSTITUIÇÃO
// ============================================================================
#define POLITICA 0 // 0 = LRU, 1 = LFU

Cache *criarCache(int tamanho) {
    Cache *c = (Cache *)malloc(sizeof(Cache));
    c->linhas = (LinhaCache *)malloc(sizeof(LinhaCache) * tamanho);
    c->tamanho = tamanho;
    c->hits = 0;   
    c->misses = 0; 

    for (int i = 0; i < tamanho; i++) {
        c->linhas[i].valido = false;     
        c->linhas[i].custo = 0;          
        c->linhas[i].atualizado = false; 
    }
    return c;
}

// ============================================================================
// INICIALIZAÇÃO E ALOCAÇÃO DE RECURSOS FÍSICOS
// ============================================================================
MMU *MMU_criar(int sizeRAM, int sizeL1, int sizeL2, int sizeL3) {
    MMU *mmu = (MMU *)malloc(sizeof(MMU));

    // 🗣️ FALA NA APRESENTAÇÃO: "A primeira grande mudança do TP3 está aqui. 
    // A RAM deixou de ser um vetor infinito. Eu a estruturei exatamente como 
    // uma Cache, com tamanho limite fixado e bits de validade. Se ela encher, 
    // terá que expulsar dados para o disco."
    mmu->ram = (RamBloco *)malloc(sizeof(RamBloco));
    mmu->ram->tamanho = sizeRAM;
    mmu->ram->hits = 0;
    mmu->ram->misses = 0;
    mmu->ram->blocos = (LinhaCache *)malloc(sizeof(LinhaCache) * sizeRAM);

    for (int i = 0; i < sizeRAM; i++) {
        mmu->ram->blocos[i].valido = false; 
        mmu->ram->blocos[i].custo = 0;
        mmu->ram->blocos[i].atualizado = false;
    }

    mmu->l1 = criarCache(sizeL1); 
    mmu->l2 = criarCache(sizeL2); 
    mmu->l3 = criarCache(sizeL3); 

    // 🗣️ FALA NA APRESENTAÇÃO: "Para simular o HD, utilizo um arquivo binário 
    // aberto com 'wb+'. O 'b' de binário é crucial porque permite gravar e ler 
    // a struct inteira em bytes crus, garantindo um acesso aleatório O(1) com fseek."
    mmu->hd = fopen("hd.bin", "wb+"); 
    if (mmu->hd == NULL) {
        printf("ERRO CRÍTICO: Falha ao iniciar o Disco Rígido!\n");
        exit(1);
    }
    mmu->acessos_hd = 0;

    if (POLITICA == 0) printf("Politica Ativa: LRU (Least Recently Used)\n");
    else printf("Politica Ativa: LFU (Least Frequently Used)\n");

    return mmu;
}

// ============================================================================
// POLÍTICA DE SUBSTITUIÇÃO (LRU / LFU)
// ============================================================================
// 🗣️ FALA NA APRESENTAÇÃO: "Como eu unifiquei a estrutura da LinhaCache para 
// as Caches e para a RAM, a lógica de substituição é a mesma. No LRU, a cada 
// acesso, eu zero a 'idade' do bloco acessado e envelheço (custo++) todos os 
// outros blocos válidos."
void atualizaCustos(Cache *c, int indiceAcessado) {
    if (POLITICA == 0) { 
        for (int i = 0; i < c->tamanho; i++) {
            if (i == indiceAcessado) c->linhas[i].custo = 0;
            else if (c->linhas[i].valido) c->linhas[i].custo++;
        }
    } else { 
        c->linhas[indiceAcessado].custo++;
    }
}

int obterIndiceParaSubstituir(Cache *c) {
    for (int i = 0; i < c->tamanho; i++) {
        if (!c->linhas[i].valido) return i;
    }

    int indiceEscolhido = 0;
    if (POLITICA == 0) { 
        int maiorCusto = -1;
        for (int i = 0; i < c->tamanho; i++) {
            if (c->linhas[i].custo > maiorCusto) {
                maiorCusto = c->linhas[i].custo;
                indiceEscolhido = i;
            }
        }
    } else { 
        int menorFrequencia = c->linhas[0].custo;
        for (int i = 1; i < c->tamanho; i++) {
            if (c->linhas[i].custo < menorFrequencia) {
                menorFrequencia = c->linhas[i].custo;
                indiceEscolhido = i;
            }
        }
    }
    return indiceEscolhido;
}

bool inserirExclusivo(Cache *c, LinhaCache *bloco, LinhaCache *vitima) {
    int idx = obterIndiceParaSubstituir(c);
    bool houveVitima = c->linhas[idx].valido;

    if (houveVitima) *vitima = c->linhas[idx];

    c->linhas[idx] = *bloco;
    c->linhas[idx].valido = true;

    if (POLITICA == 0) atualizaCustos(c, idx);
    else c->linhas[idx].custo = 1;

    return houveVitima;
}

int buscarEmCache(Cache *c, int tagBloco) {
    for (int i = 0; i < c->tamanho; i++) {
        if (c->linhas[i].valido && c->linhas[i].endBloco == tagBloco) {
            atualizaCustos(c, i); 
            return i;             
        }
    }
    return -1; 
}

// Funções idênticas para a RAM (omitidas nos comentários da fala para economizar tempo)
void atualizaCustosRAM(RamBloco *ram, int indiceAcessado) {
    if (POLITICA == 0) {
        for (int i = 0; i < ram->tamanho; i++) {
            if (i == indiceAcessado) ram->blocos[i].custo = 0;
            else if (ram->blocos[i].valido) ram->blocos[i].custo++;
        }
    } else {
        ram->blocos[indiceAcessado].custo++;
    }
}

int obterIndiceParaSubstituirRAM(RamBloco *ram) {
    for (int i = 0; i < ram->tamanho; i++) {
        if (!ram->blocos[i].valido) return i;
    }
    int indiceEscolhido = 0;
    if (POLITICA == 0) {
        int maiorCusto = -1;
        for (int i = 0; i < ram->tamanho; i++) {
            if (ram->blocos[i].custo > maiorCusto) {
                maiorCusto = ram->blocos[i].custo;
                indiceEscolhido = i;
            }
        }
    } else {
        int menorFrequencia = ram->blocos[0].custo;
        for (int i = 1; i < ram->tamanho; i++) {
            if (ram->blocos[i].custo < menorFrequencia) {
                menorFrequencia = ram->blocos[i].custo;
                indiceEscolhido = i;
            }
        }
    }
    return indiceEscolhido;
}

bool inserirExclusivoRAM(RamBloco *ram, LinhaCache *bloco, LinhaCache *vitima) {
    int idx = obterIndiceParaSubstituirRAM(ram);
    bool houveVitima = ram->blocos[idx].valido;

    if (houveVitima) *vitima = ram->blocos[idx];

    ram->blocos[idx] = *bloco;
    ram->blocos[idx].valido = true;

    if (POLITICA == 0) atualizaCustosRAM(ram, idx);
    else ram->blocos[idx].custo = 1;

    return houveVitima;
}

int buscarEmRam(RamBloco *ram, int tagBloco) {
    for (int i = 0; i < ram->tamanho; i++) {
        if (ram->blocos[i].valido && ram->blocos[i].endBloco == tagBloco) {
            atualizaCustosRAM(ram, i);
            return i;
        }
    }
    return -1;
}

// ============================================================================
// O CASCATEAMENTO EXCLUSIVO DA HIERARQUIA (A MÁGICA DO TP2 E TP3)
// ============================================================================

// 🗣️ FALA NA APRESENTAÇÃO: "Esta é a função mais importante da arquitetura. 
// Ela garante a Exclusividade das caches. Se a L1 estiver cheia, o bloco 
// antigo é empurrado em cascata para a L2, L3, e RAM. Se a RAM também encher..."
void inserirNaHierarquia(MMU* mmu, LinhaCache* bloco) {
    LinhaCache vitimaL1;
    if (inserirExclusivo(mmu->l1, bloco, &vitimaL1)) {
        LinhaCache vitimaL2;
        if (inserirExclusivo(mmu->l2, &vitimaL1, &vitimaL2)) {
            LinhaCache vitimaL3;
            if (inserirExclusivo(mmu->l3, &vitimaL2, &vitimaL3)) {
                LinhaCache vitimaRAM;
                if (inserirExclusivoRAM(mmu->ram, &vitimaL3, &vitimaRAM)) {
                    
                    // 🗣️ FALA NA APRESENTAÇÃO: "...Nós chegamos ao fundo do poço! 
                    // A RAM devolve uma vítima, e eu sou obrigado a salvá-la no 
                    // arquivo do disco rígido usando fwrite (Swap Out), abrindo 
                    // espaço físico na memória principal."
                    fseek(mmu->hd, vitimaRAM.endBloco * sizeof(LinhaCache), SEEK_SET);
                    fwrite(&vitimaRAM, sizeof(LinhaCache), 1, mmu->hd);
                }
            }
        }
    }
}

// ============================================================================
// BUSCA HIERÁRQUICA E RESOLUÇÃO DE MISSES
// ============================================================================
int MMU_buscar(MMU *mmu, int enderecoFisico) {
    int tagBloco = enderecoFisico / TAM_BLOCO;
    int offset = enderecoFisico % TAM_BLOCO;
    int idx;

    // 🔵 1) L1
    idx = buscarEmCache(mmu->l1, tagBloco);
    if (idx != -1) { mmu->l1->hits++; return mmu->l1->linhas[idx].palavras[offset]; }
    mmu->l1->misses++;

    // 🟡 2) L2
    idx = buscarEmCache(mmu->l2, tagBloco);
    if (idx != -1) {
        mmu->l2->hits++;
        LinhaCache bloco = mmu->l2->linhas[idx];
        mmu->l2->linhas[idx].valido = false; // Mantém exclusividade
        inserirNaHierarquia(mmu, &bloco);
        return bloco.palavras[offset];
    }
    mmu->l2->misses++;

    // 🟠 3) L3
    idx = buscarEmCache(mmu->l3, tagBloco);
    if (idx != -1) {
        mmu->l3->hits++;
        LinhaCache bloco = mmu->l3->linhas[idx];
        mmu->l3->linhas[idx].valido = false; 
        inserirNaHierarquia(mmu, &bloco);
        return bloco.palavras[offset];
    }
    mmu->l3->misses++;

    // 🔴 4) RAM
    idx = buscarEmRam(mmu->ram, tagBloco);
    if (idx != -1) {
        mmu->ram->hits++;
        LinhaCache bloco = mmu->ram->blocos[idx];
        mmu->ram->blocos[idx].valido = false; 
        inserirNaHierarquia(mmu, &bloco);
        return bloco.palavras[offset];
    }
    mmu->ram->misses++;

    // 💽 5) DISCO RÍGIDO (HD) - O PIOR CASO
    // 🗣️ FALA NA APRESENTAÇÃO: "Se deu Miss na RAM, temos uma Faltas de Página 
    // (Page Fault). A MMU incrementa os acessos de I/O, calcula o endereço físico 
    // real do bloco no arquivo com fseek, e puxa os bytes com fread."
    mmu->acessos_hd++;
    
    LinhaCache blocoNovo;
    blocoNovo.endBloco = tagBloco;
    blocoNovo.valido = true;
    blocoNovo.atualizado = false;
    if (POLITICA == 0) blocoNovo.custo = 0; else blocoNovo.custo = 1;

    fseek(mmu->hd, tagBloco * sizeof(LinhaCache), SEEK_SET);
    size_t lidos = fread(&blocoNovo, sizeof(LinhaCache), 1, mmu->hd);
    
    // 🗣️ FALA NA APRESENTAÇÃO: "Um detalhe importante: se o fread retornar 0, 
    // quer dizer que a CPU pediu um endereço que nunca foi gravado no disco. 
    // Nesse caso, eu simulo a alocação preenchendo o bloco com zeros."
    if (lidos == 0) {
        for (int i = 0; i < TAM_BLOCO; i++) blocoNovo.palavras[i] = 0;
    }

    inserirNaHierarquia(mmu, &blocoNovo);
    return blocoNovo.palavras[offset];
}

// ============================================================================
// ESCRITA NA MEMÓRIA (WRITE-THROUGH)
// ============================================================================
void MMU_escrever(MMU *mmu, int enderecoFisico, int valor) {
    int tagBloco = enderecoFisico / TAM_BLOCO;
    int offset = enderecoFisico % TAM_BLOCO;

    // 🗣️ FALA NA APRESENTAÇÃO: "A política de escrita escolhida foi o Write-Through 
    // por questões de confiabilidade. Logo, o Disco Rígido é a única verdade 
    // absoluta. Eu sempre gravo fisicamente no arquivo binário primeiro."
    LinhaCache blocoHD;
    fseek(mmu->hd, tagBloco * sizeof(LinhaCache), SEEK_SET);
    size_t lidos = fread(&blocoHD, sizeof(LinhaCache), 1, mmu->hd);
    
    if (lidos == 0) { 
        blocoHD.endBloco = tagBloco;
        blocoHD.valido = true;
        for(int i=0; i<TAM_BLOCO; i++) blocoHD.palavras[i] = 0;
    }
    
    blocoHD.palavras[offset] = valor;
    
    fseek(mmu->hd, tagBloco * sizeof(LinhaCache), SEEK_SET);
    fwrite(&blocoHD, sizeof(LinhaCache), 1, mmu->hd); 

    // 🗣️ FALA NA APRESENTAÇÃO: "Em seguida, aplico o Write-Update: eu varro 
    // a L1, L2, L3 e RAM. Se o bloco estiver carregado em alguma delas, eu 
    // atualizo o valor lá também, garantindo a Coerência de Cache."
    int idx;
    idx = buscarEmCache(mmu->l1, tagBloco);
    if (idx != -1) { mmu->l1->linhas[idx].palavras[offset] = valor; atualizaCustos(mmu->l1, idx); }

    idx = buscarEmCache(mmu->l2, tagBloco);
    if (idx != -1) { mmu->l2->linhas[idx].palavras[offset] = valor; atualizaCustos(mmu->l2, idx); }

    idx = buscarEmCache(mmu->l3, tagBloco);
    if (idx != -1) { mmu->l3->linhas[idx].palavras[offset] = valor; atualizaCustos(mmu->l3, idx); }

    idx = buscarEmRam(mmu->ram, tagBloco);
    if (idx != -1) { mmu->ram->blocos[idx].palavras[offset] = valor; atualizaCustosRAM(mmu->ram, idx); }
}

void MMU_liberar(MMU *mmu) {
    free(mmu->l1->linhas); free(mmu->l1);
    free(mmu->l2->linhas); free(mmu->l2);
    free(mmu->l3->linhas); free(mmu->l3);
    free(mmu->ram->blocos); free(mmu->ram);
    
    // 🗣️ FALA NA APRESENTAÇÃO: "Por fim, como bons engenheiros de software, 
    // fechamos o ponteiro do arquivo e deletamos o hd.bin para não deixar 
    // lixo no computador."
    fclose(mmu->hd);  
    remove("hd.bin"); 
    
    free(mmu);
}