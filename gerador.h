#ifndef GERADOR_H
#define GERADOR_H

#include "instrucao.h"

// Gera o arquivo de instruções
// ATUALIZADO: Agora inclui o 4º parâmetro 'probabilidadeRepeticao'
void gerarArquivoInstrucoes(const char* nomeArquivo, int qtdInstrucoes, int tamMemoria, int probabilidadeRepeticao);

// Lê o arquivo para a memória
Instrucao* lerArquivoInstrucoes(const char* nomeArquivo, int* qtdLidas);

#endif