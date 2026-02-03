#include "ram.h"

// Cria a RAM alocando blocos em vez de inteiros
void Ram_Criar(Ram *ram, int qtdBlocos) {
    // Aloca vetor de estruturas BlocoMemoria
    ram->blocos = (BlocoMemoria*)malloc(qtdBlocos * sizeof(BlocoMemoria));
    
    if(ram->blocos == NULL){
        printf("Erro: Memoria insuficiente para criar RAM com %d blocos\n", qtdBlocos);
        exit(1);
    }
    
    ram->tamanho = qtdBlocos;

    // Inicializa todos os blocos com zeros
    for(int i = 0; i < qtdBlocos; i++) {
        ram->blocos[i].endBloco = i; // O endereço do bloco é seu próprio índice
        ram->blocos[i].valido = 0;
        ram->blocos[i].atualizado = 0;
        ram->blocos[i].custo = 0;
        
        for(int j = 0; j < TAM_BLOCO; j++) {
            ram->blocos[i].palavras[j] = 0;
        }
    }
}

void Ram_Liberar(Ram *ram) {
    if(ram != NULL && ram->blocos != NULL) {
        free(ram->blocos);
        ram->blocos = NULL;
        ram->tamanho = 0;
    }
}

// Imprimir agora mostra os blocos agrupados
void Ram_Imprimir(Ram *ram) {
    if (ram == NULL || ram->blocos == NULL) {
        printf("RAM nao inicializada!\n");
        return;
    }

    printf("=== CONTEUDO DA RAM (Por Blocos) ===\n");
    printf("Tamanho: %d Blocos (%d Palavras totais)\n", 
            ram->tamanho, ram->tamanho * TAM_BLOCO);
    printf("----------------------------------------------------\n");
    printf("| Bloco ID | Palavra 0 | Palavra 1 | Palavra 2 | Palavra 3 |\n");
    printf("----------------------------------------------------\n");
    
    for (int i = 0; i < ram->tamanho; i++) {
        // Imprime apenas se tiver algum dado diferente de zero (opcional, para não poluir)
        // Se quiser ver tudo, remova o 'if' abaixo ou a lógica de verificação
        int temDado = 0;
        for(int k=0; k<TAM_BLOCO; k++) if(ram->blocos[i].palavras[k] != 0) temDado = 1;
        
        // Vamos imprimir tudo para validar, ou descomente a linha acima para filtrar
        printf("| %04d     | %9d | %9d | %9d | %9d |\n", 
               i, 
               ram->blocos[i].palavras[0], 
               ram->blocos[i].palavras[1], 
               ram->blocos[i].palavras[2], 
               ram->blocos[i].palavras[3]);
    }
    printf("----------------------------------------------------\n\n");
}

// Função auxiliar para preencher RAM com valores aleatórios (adaptada)
void gerar_RamAleatoria(Ram *ram, int valor_min, int valor_max) {
    srand(time(NULL));
    for(int i = 0; i < ram->tamanho; i++) {
        for(int j = 0; j < TAM_BLOCO; j++) {
            ram->blocos[i].palavras[j] = valor_min + (rand() % (valor_max - valor_min + 1));
        }
    }
}