#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "gerador.h"

// Configurações internas
#define TAM_FOR 10      // Tamanho do bloco que se repete
#define NUM_OPCODES 4   // Quantidade de opcodes da CPU

// MUDANÇA: Agora recebe 'probabilidadeRepeticao' (0 a 100) como parâmetro
void gerarArquivoInstrucoes(const char* nomeArquivo, int qtdInstrucoes, int tamMemoria, int probabilidadeRepeticao) {
    FILE* arquivo = fopen(nomeArquivo, "w");
    if (!arquivo) {
        printf("Erro ao criar arquivo %s\n", nomeArquivo);
        exit(1);
    }

    srand(time(NULL));

    // 1. Criar o "Loop" (A sequência que vai se repetir)
    int loopOpcode[TAM_FOR];
    int loopAdd1[TAM_FOR];
    int loopAdd2[TAM_FOR];
    int loopAdd3[TAM_FOR];

    for (int i = 0; i < TAM_FOR; i++) {
        loopOpcode[i] = rand() % NUM_OPCODES;
        loopAdd1[i] = rand() % tamMemoria;
        loopAdd2[i] = rand() % tamMemoria;
        loopAdd3[i] = rand() % tamMemoria;
    }

    // 2. Gerar as instruções
    int i = 0;
    while (i < qtdInstrucoes) {
        int dado = rand() % 100; 

        // Usa a probabilidade passada por parâmetro (ex: 50, 75, 90)
        if (dado < probabilidadeRepeticao && (i + TAM_FOR) <= qtdInstrucoes) {
            for (int j = 0; j < TAM_FOR; j++) {
                fprintf(arquivo, "%d:%d:%d:%d\n", 
                        loopOpcode[j], loopAdd1[j], loopAdd2[j], loopAdd3[j]);
            }
            i += TAM_FOR;
        } 
        else {
            int op = rand() % NUM_OPCODES;
            int a1 = rand() % tamMemoria;
            int a2 = rand() % tamMemoria;
            int a3 = rand() % tamMemoria;
            fprintf(arquivo, "%d:%d:%d:%d\n", op, a1, a2, a3);
            i++;
        }
    }
    
    fprintf(arquivo, "-1:0:0:0\n"); // HALT
    fclose(arquivo);
    // printf("Arquivo '%s' gerado com sucesso (Prob: %d%%)!\n", nomeArquivo, probabilidadeRepeticao);
}

Instrucao* lerArquivoInstrucoes(const char* nomeArquivo, int* qtdLidas) {
    FILE* arquivo = fopen(nomeArquivo, "r");
    if (!arquivo) {
        printf("Erro ao abrir arquivo %s\n", nomeArquivo);
        return NULL;
    }

    int capacidade = 20000; 
    Instrucao* programa = (Instrucao*)malloc(capacidade * sizeof(Instrucao));
    
    int i = 0;
    int op, a1, a2, a3;

    while (fscanf(arquivo, "%d:%d:%d:%d", &op, &a1, &a2, &a3) == 4) {
        if (i >= capacidade) break;
        programa[i].opcode = op;
        programa[i].add1 = a1;
        programa[i].add2 = a2;
        programa[i].add3 = a3;
        i++;
    }

    fclose(arquivo);
    *qtdLidas = i;
    return programa;
}