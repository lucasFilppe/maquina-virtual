#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "cpu.h"
#include "mmu.h"
#include "gerador.h"

// Estrutura para configurar as máquinas
typedef struct {
    char* nome;
    int l1;
    int l2;
    int l3;
} Maquina;

void rodarSimulacao(Maquina m, int probRepeticao, int tamRAM, int qtdInstrucoes) {
    const char* nomeProg = "programa_temp.txt";
    const char* nomeTI = "ti_temp.txt";
    int qtdInstrucoesTI = 10; // O TI é um programa pequeno
    int probInterrupcao = 1;   // 1% de chance de ocorrer interrupção a cada ciclo

    // 1. Gera e Lê o Programa Principal
    gerarArquivoInstrucoes(nomeProg, qtdInstrucoes, tamRAM, probRepeticao);
    int tamanhoProg;
    Instrucao* programa = lerArquivoInstrucoes(nomeProg, &tamanhoProg);

    // 2. Gera e Lê o Tratador de Interrupção (TI)
    // Usamos 0% de repetição para simular um código de sistema operacional mais aleatório
    gerarArquivoInstrucoes(nomeTI, qtdInstrucoesTI, tamRAM, 0);
    int tamanhoTI;
    Instrucao* programaTI = lerArquivoInstrucoes(nomeTI, &tamanhoTI);

    if (!programa || !programaTI) {
        printf("Erro ao carregar programas na memoria.\n");
        return;
    }

    // 3. Inicializa o Hardware
    MMU* mmu = MMU_criar(tamRAM, m.l1, m.l2, m.l3);
    Cpu* cpu = CPU_criar();

    CPU_setPrograma(cpu, programa, tamanhoProg);
    CPU_setProgramaTI(cpu, programaTI, tamanhoTI); // ⚡ NOVO: Instala o TI na CPU

    // ==========================================
    // ⏱️ EXECUÇÃO COM MEDIÇÃO DE TEMPO REAL
    // ==========================================
    clock_t tempo_inicio = clock();

    // Roda a simulação inteira (Programa + Interrupções + I/O no HD)
    CPU_iniciar(cpu, mmu, probInterrupcao);

    clock_t tempo_fim = clock();
    double tempo_real_segundos = (double)(tempo_fim - tempo_inicio) / CLOCKS_PER_SEC;

    // ==========================================
    // ESTATÍSTICAS E TABELA
    // ==========================================
    int acessosL1 = mmu->l1->hits + mmu->l1->misses;
    int acessosL2 = mmu->l1->misses;
    int acessosL3 = mmu->l2->misses;
    int acessosRAM = mmu->l3->misses;

    float taxaL1 = (acessosL1 > 0) ? ((float)mmu->l1->hits * 100.0 / acessosL1) : 0.0;
    float taxaL2 = (acessosL2 > 0) ? ((float)mmu->l2->hits * 100.0 / acessosL2) : 0.0;
    float taxaL3 = (acessosL3 > 0) ? ((float)mmu->l3->hits * 100.0 / acessosL3) : 0.0;
    float taxaRAM = (acessosRAM > 0) ? ((float)mmu->ram->hits * 100.0 / acessosRAM) : 0.0;

    // O acesso ao disco é o total de leituras/escritas físicas no arquivo hd.bin
    long acessosHD = mmu->acessos_hd;

    // Print formatado para a tabela
    printf("| %-3s | %4d | %4d | %4d | %6.2f%% | %6.2f%% | %6.2f%% | %7.2f%% | %6ld | %7.4f s |\n",
           m.nome, m.l1, m.l2, m.l3,
           taxaL1, taxaL2, taxaL3, taxaRAM, acessosHD, tempo_real_segundos);

    // 4. Limpeza de Memória e Arquivos Temporários
    CPU_liberar(cpu);
    MMU_liberar(mmu);
    free(programa);
    free(programaTI);
    remove(nomeProg);
    remove(nomeTI);
}

int main() {
    int tamRAM = 1000;       // Tamanho máximo da RAM (Blocos)
    int qtdInstrucoes = 10000; // Tamanho do programa principal

    Maquina maquinas[] = {
        {"M1", 8,  16, 32},
        {"M2", 32, 64, 128},
        {"M3", 16, 64, 256},
        {"M4", 8,  32, 128},
        {"M5", 16, 32, 64}
    };
    int numMaquinas = 5;
    int probabilidades[] = {50, 75, 90};
    int numProbs = 3;

    printf("=== SIMULADOR DE SISTEMA COMPLETO (TP3) ===\n\n");

    for (int p = 0; p < numProbs; p++) {
        int prob = probabilidades[p];
        printf("\n>>> CENARIO: %d%% de Repeticao (Carga da CPU) <<<\n", prob);
        printf("+-----+------+------+------+---------+---------+---------+----------+--------+------------+\n");
        printf("| Maq |   L1 |   L2 |   L3 | Hit L1  | Hit L2  | Hit L3  | Hit RAM  | I/O HD | Tempo Real |\n");
        printf("+-----+------+------+------+---------+---------+---------+----------+--------+------------+\n");

        for (int m = 0; m < numMaquinas; m++) {
            rodarSimulacao(maquinas[m], prob, tamRAM, qtdInstrucoes);
        }
        printf("+-----+------+------+------+---------+---------+---------+----------+--------+------------+\n");
    }

    return 0;
}