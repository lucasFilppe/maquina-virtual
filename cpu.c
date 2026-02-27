#include <stdlib.h>
#include <stdio.h>
#include "cpu.h"

// A função de criação continua igual
Cpu* CPU_criar() {
    Cpu *cpu = (Cpu*)malloc(sizeof(Cpu));
    if(cpu == NULL) {
        printf("Erro ao alocar memoria para cpu\n");
        return NULL;
    }
    
    cpu->registrador1 = 0;
    cpu->registrador2 = 0;
    cpu->PC = 0;
    cpu->opcode = 0;
    cpu->programa = NULL;
    cpu->programa_ti = NULL; // Inicializa o TI como NULL
    
    return cpu;
}

void CPU_reset(Cpu* cpu) {
    if(cpu == NULL) {
        printf("Erro: Ponteiro de CPU nulo em CPU_reset\n");
        return;
    }
    cpu->registrador1 = 0;
    cpu->registrador2 = 0;
    cpu->PC = 0;
    cpu->opcode = 0;
}

void CPU_liberar(Cpu* cpu) {
    if(cpu != NULL) {
        free(cpu);
    }
}

void CPU_setPrograma(Cpu* cpu, Instrucao* programaAux, int tamanho) {
    if(cpu != NULL) {
        cpu->programa = programaAux;
        cpu->tamanho_programa = tamanho;
    }
}

// NOVA FUNÇÃO: Define o Tratador de Interrupções
void CPU_setProgramaTI(Cpu* cpu, Instrucao* programaTI, int tamanho) {
    if(cpu != NULL) {
        cpu->programa_ti = programaTI;
        cpu->tamanho_ti = tamanho;
    }
}

// ============================================================================
// EXECUÇÃO DE UMA ÚNICA INSTRUÇÃO (Modularizado)
// ============================================================================
// Extraímos o switch para uma função separada. Assim podemos reaproveitá-la
// tanto para o programa principal quanto para o TI.
void CPU_executar_instrucao(Cpu* cpu, MMU* mmu, Instrucao inst) {
    
    switch(cpu->opcode) {
        case -1: { // halt
            // O halt é tratado no laço principal
            break;
        }
        case 0: { // soma
            cpu->registrador1 = MMU_buscar(mmu, inst.add1);
            cpu->registrador2 = MMU_buscar(mmu, inst.add2);
            cpu->registrador1 += cpu->registrador2;
            MMU_escrever(mmu, inst.add3, cpu->registrador1);
            cpu->PC++;
            break;
        }
        case 1: { // subtrai
            cpu->registrador1 = MMU_buscar(mmu, inst.add1);
            cpu->registrador2 = MMU_buscar(mmu, inst.add2);
            cpu->registrador1 -= cpu->registrador2;
            MMU_escrever(mmu, inst.add3, cpu->registrador1);
            cpu->PC++;
            break;
        }
        case 2: { // registrador -> Memória
            if(inst.add1 == 1) MMU_escrever(mmu, inst.add2, cpu->registrador1);
            else if(inst.add1 == 2) MMU_escrever(mmu, inst.add2, cpu->registrador2);
            cpu->PC++;
            break;
        }
        case 3: { // Memória -> registrador
            if(inst.add1 == 1) cpu->registrador1 = MMU_buscar(mmu, inst.add2);
            else if(inst.add1 == 2) cpu->registrador2 = MMU_buscar(mmu, inst.add2);
            cpu->PC++;
            break;
        }
        case 4: { // Imediato -> Registrador
            if(inst.add1 == 1) cpu->registrador1 = inst.add2;
            else if(inst.add1 == 2) cpu->registrador2 = inst.add2;
            cpu->PC++;
            break;
        }
        case 5: { // Modificação (Output)
            // Aviso: Esta instrução altera o próprio vetor de programa.
            // Para manter a sanidade da simulação, vamos ignorá-la ou apenas
            // imprimir o valor se for o caso. O TP normalmente não a exige.
            cpu->PC++;
            break;
        }
        default: {
            cpu->PC++;
            break;
        }
    }
}

// ============================================================================
// LAÇO PRINCIPAL COM INTERRUPÇÕES (TP3)
// ============================================================================
void CPU_iniciar(Cpu* cpu, MMU* mmu, int probInterrupcao) {
    
    if(cpu == NULL || mmu == NULL || cpu->programa == NULL) {
        printf("Parametros invalidos para CPU_iniciar\n");
        return;
    }
    
    CPU_reset(cpu);
    
    // Executa enquanto não encontrar o opcode de parada (-1)
    while(cpu->opcode != -1) {
        
        // 1. Busca a próxima instrução do programa principal
        Instrucao inst = cpu->programa[cpu->PC];
        cpu->opcode = inst.opcode;
        
        if (cpu->opcode == -1) {
            // Fim do programa principal
            break;
        }
        
        // 2. Executa a instrução
        CPU_executar_instrucao(cpu, mmu, inst);
        
        // =======================================================
        // 3. VERIFICAÇÃO DE INTERRUPÇÃO (O CORAÇÃO DO TP3)
        // =======================================================
        // Sorteia um número de 0 a 99
        int sorteio = rand() % 100;
        
        // Se o número for menor que a probabilidade, a interrupção acontece!
        if (sorteio < probInterrupcao && cpu->programa_ti != NULL) {
            
            // --- SALVAR O CONTEXTO ---
            // Guardamos onde o programa principal parou
            int pc_salvo = cpu->PC;
            int opcode_salvo = cpu->opcode;
            // Opcional: Salvar registradores se o TI for usar os mesmos e corrompê-los.
            // Para o escopo deste TP, apenas o PC é estritamente necessário.
            int reg1_salvo = cpu->registrador1;
            int reg2_salvo = cpu->registrador2;
            
            // --- EXECUTAR O TRATADOR DE INTERRUPÇÃO (TI) ---
            cpu->PC = 0; // O TI começa do início do seu próprio código
            
            // Roda o TI inteiro até acabar (ignorando opcodes de halt do TI se houver)
            for (int i = 0; i < cpu->tamanho_ti; i++) {
                Instrucao inst_ti = cpu->programa_ti[cpu->PC];
                cpu->opcode = inst_ti.opcode;
                
                if (cpu->opcode == -1) break; // Sai se o TI tiver um halt
                
                CPU_executar_instrucao(cpu, mmu, inst_ti);
            }
            
            // --- RESTAURAR O CONTEXTO ---
            // Devolve a CPU ao estado em que estava antes do "raio cair"
            cpu->PC = pc_salvo;
            cpu->opcode = opcode_salvo;
            cpu->registrador1 = reg1_salvo;
            cpu->registrador2 = reg2_salvo;
        }
    }
}