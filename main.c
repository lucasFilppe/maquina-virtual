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

    // 1. Gerar Instruções com a probabilidade específica
    gerarArquivoInstrucoes(nomeArquivo, qtdInstrucoes, tamRAM, probRepeticao);

    // 2. Carregar Programa
    int qtdLidas;
    Instrucao* programa = lerArquivoInstrucoes(nomeArquivo, &qtdLidas);
    if (!programa) {
        printf("Erro ao ler programa.\n");
        return;
    }

    // 3. Inicializar Hardware com as configurações da Máquina atual (M1, M2...)
    MMU* mmu = MMU_criar(tamRAM, m.l1, m.l2, m.l3);
    Cpu* cpu = CPU_criar();

    // 4. Executar
    CPU_setPrograma(cpu, programa);
    CPU_iniciar(cpu, mmu);

    // 5. Coletar Estatísticas
    int totalL1 = mmu->l1->hits + mmu->l1->misses;
    int totalL2 = mmu->l2->hits + mmu->l2->misses;
    int totalL3 = mmu->l3->hits + mmu->l3->misses;

    float taxaL1 = (totalL1 > 0) ? ((float)mmu->l1->hits * 100.0 / totalL1) : 0.0;
    float taxaL2 = (totalL2 > 0) ? ((float)mmu->l2->hits * 100.0 / totalL2) : 0.0;
    float taxaL3 = (totalL3 > 0) ? ((float)mmu->l3->hits * 100.0 / totalL3) : 0.0;
    
    // Taxa RAM: Porcentagem do total de instruções que acabou indo na RAM
    // (Considerando que tudo passa pela L1, usamos totalL1 como base ou o próprio L3 Misses)
    // Para seguir a tabela do PDF, geralmente "Taxa RAM" é a % de acesso que chegou na RAM.
    float taxaRAM = (totalL1 > 0) ? ((float)mmu->l3->misses * 100.0 / totalL1) : 0.0;

    // Custo Hipotético: L1=1, L2=10, L3=100, RAM=1000
    long custo = (totalL1 * 1) + (totalL2 * 10) + (totalL3 * 100) + (mmu->l3->misses * 1000);

    // 6. Imprimir Linha da Tabela
    // Formato: Maquina | L1 | L2 | L3 | Taxa L1 | Taxa L2 | Taxa L3 | Taxa RAM | Tempo
    printf("| %-3s | %4d | %4d | %4d | %6.2f%% | %6.2f%% | %6.2f%% | %6.2f%% | %8ld |\n", 
           m.nome, m.l1, m.l2, m.l3, taxaL1, taxaL2, taxaL3, taxaRAM, custo);

    // 7. Limpeza
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