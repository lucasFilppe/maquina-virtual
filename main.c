#include <stdio.h>
#include <stdlib.h>
#include "cpu.h"
#include "mmu.h"
#include "gerador.h"
#include "instrucao.h"

// Estrutura para facilitar a configuração das máquinas
typedef struct {
    char* nome;
    int l1;
    int l2;
    int l3;
} Maquina;

void rodarSimulacao(Maquina m, int probRepeticao, int tamRAM, int qtdInstrucoes) {
    const char* nomeArquivo = "programa_temp.txt";

    gerarArquivoInstrucoes(nomeArquivo, qtdInstrucoes, tamRAM, probRepeticao);

    int qtdLidas;
    Instrucao* programa = lerArquivoInstrucoes(nomeArquivo, &qtdLidas);
    if (!programa) {
        printf("Erro ao ler programa.\n");
        return;
    }

    MMU* mmu = MMU_criar(tamRAM, m.l1, m.l2, m.l3);
    Cpu* cpu = CPU_criar();

    CPU_setPrograma(cpu, programa);
    CPU_iniciar(cpu, mmu);

    // =============================
    // ACESSOS REAIS
    // =============================

    int totalAcessos = mmu->l1->hits + mmu->l1->misses;

    int acessosL2 = mmu->l1->misses;
    int acessosL3 = mmu->l2->misses;
    int acessosRAM = mmu->l3->misses;

    // =============================
    // TAXAS CORRETAS
    // =============================

    float taxaL1 = (totalAcessos > 0)
        ? ((float)mmu->l1->hits * 100.0 / totalAcessos)
        : 0.0;

    float taxaL2 = (acessosL2 > 0)
        ? ((float)mmu->l2->hits * 100.0 / acessosL2)
        : 0.0;

    float taxaL3 = (acessosL3 > 0)
        ? ((float)mmu->l3->hits * 100.0 / acessosL3)
        : 0.0;

    float taxaRAM = (totalAcessos > 0)
        ? ((float)acessosRAM * 100.0 / totalAcessos)
        : 0.0;

    // =============================
    // CUSTO CORRETO
    // =============================

    long custo = 0;

    custo += totalAcessos * 1;          // Todo acesso passa na L1
    custo += acessosL2 * 10;            // Só quem erra L1 vai pra L2
    custo += acessosL3 * 100;           // Só quem erra L2 vai pra L3
    custo += acessosRAM * 1000;         // Só quem erra L3 vai pra RAM

    // =============================
    // PRINT
    // =============================

    printf("| %-3s | %4d | %4d | %4d | %6.2f%% | %6.2f%% | %6.2f%% | %6.2f%% | %8ld |\n",
           m.nome, m.l1, m.l2, m.l3,
           taxaL1, taxaL2, taxaL3, taxaRAM, custo);

    CPU_liberar(cpu);
    MMU_liberar(mmu);
    free(programa);
}

int main() {
    // Configurações Fixas
    int tamRAM = 1000;
    int qtdInstrucoes = 10000;

    // Definição das 5 Máquinas conforme PDF
    Maquina maquinas[] = {
        {"M1", 8,  16, 32},
        {"M2", 32, 64, 128},
        {"M3", 16, 64, 256},
        {"M4", 8,  32, 128},
        {"M5", 16, 32, 64}
    };
    int numMaquinas = 5;

    // Definição das Probabilidades conforme PDF
    int probabilidades[] = {50, 75, 90};
    int numProbs = 3;

    printf("=== SIMULADOR DE CACHES - RESULTADOS TABULARES ===\n\n");

    // Loop para cada probabilidade (Gera uma tabela para cada cenário)
    for (int p = 0; p < numProbs; p++) {
        int prob = probabilidades[p];
        printf("\n>>> CENARIO: %d%% de Repeticao <<<\n", prob);
        printf("+-----+------+------+------+---------+---------+---------+---------+----------+\n");
        printf("| Maq |   L1 |   L2 |   L3 | Hit L1  | Hit L2  | Hit L3  | TaxaRAM | Tempo    |\n");
        printf("+-----+------+------+------+---------+---------+---------+---------+----------+\n");

        for (int m = 0; m < numMaquinas; m++) {
            rodarSimulacao(maquinas[m], prob, tamRAM, qtdInstrucoes);
        }
        printf("+-----+------+------+------+---------+---------+---------+---------+----------+\n");
    }

    return 0;
}