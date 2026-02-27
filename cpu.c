#include <stdlib.h>
#include <stdio.h>
#include "cpu.h"

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
    cpu->programa_ti = NULL; 
    
    return cpu;
}

void CPU_reset(Cpu* cpu) {
    if(cpu == NULL) return;
    cpu->registrador1 = 0;
    cpu->registrador2 = 0;
    cpu->PC = 0;
    cpu->opcode = 0;
}

void CPU_liberar(Cpu* cpu) {
    if(cpu != NULL) free(cpu);
}

void CPU_setPrograma(Cpu* cpu, Instrucao* programaAux, int tamanho) {
    if(cpu != NULL) {
        cpu->programa = programaAux;
        cpu->tamanho_programa = tamanho;
    }
}

// 🗣️ FALA NA APRESENTAÇÃO: "Para o TP3, criei esta função para injetar o 
// Tratador de Interrupção (TI) na CPU, separando-o do programa principal."
void CPU_setProgramaTI(Cpu* cpu, Instrucao* programaTI, int tamanho) {
    if(cpu != NULL) {
        cpu->programa_ti = programaTI;
        cpu->tamanho_ti = tamanho;
    }
}

// ============================================================================
// EXECUÇÃO DE UMA ÚNICA INSTRUÇÃO
// ============================================================================
// 🗣️ FALA NA APRESENTAÇÃO: "Modularizei o switch de opcodes nesta função. 
// Isso evitou duplicação de código, pois agora a CPU usa exatamente a mesma 
// lógica para rodar instruções do programa principal e do TI."
void CPU_executar_instrucao(Cpu* cpu, MMU* mmu, Instrucao inst) {
    
    switch(cpu->opcode) {
        case -1: break; // halt
        case 0: // soma
            cpu->registrador1 = MMU_buscar(mmu, inst.add1);
            cpu->registrador2 = MMU_buscar(mmu, inst.add2);
            cpu->registrador1 += cpu->registrador2;
            MMU_escrever(mmu, inst.add3, cpu->registrador1);
            cpu->PC++;
            break;
        case 1: // subtrai
            cpu->registrador1 = MMU_buscar(mmu, inst.add1);
            cpu->registrador2 = MMU_buscar(mmu, inst.add2);
            cpu->registrador1 -= cpu->registrador2;
            MMU_escrever(mmu, inst.add3, cpu->registrador1);
            cpu->PC++;
            break;
        case 2: // registrador -> Memória
            if(inst.add1 == 1) MMU_escrever(mmu, inst.add2, cpu->registrador1);
            else if(inst.add1 == 2) MMU_escrever(mmu, inst.add2, cpu->registrador2);
            cpu->PC++;
            break;
        case 3: // Memória -> registrador
            if(inst.add1 == 1) cpu->registrador1 = MMU_buscar(mmu, inst.add2);
            else if(inst.add1 == 2) cpu->registrador2 = MMU_buscar(mmu, inst.add2);
            cpu->PC++;
            break;
        case 4: // Imediato -> Registrador
            if(inst.add1 == 1) cpu->registrador1 = inst.add2;
            else if(inst.add1 == 2) cpu->registrador2 = inst.add2;
            cpu->PC++;
            break;
        default:
            cpu->PC++;
            break;
    }
}

// ============================================================================
// LAÇO PRINCIPAL COM INTERRUPÇÕES (O CORAÇÃO DO TP3)
// ============================================================================
void CPU_iniciar(Cpu* cpu, MMU* mmu, int probInterrupcao) {
    
    if(cpu == NULL || mmu == NULL || cpu->programa == NULL) return;
    
    CPU_reset(cpu);
    
    // 🗣️ FALA NA APRESENTAÇÃO: "Este é o ciclo de instrução (Fetch-Decode-Execute)."
    while(cpu->opcode != -1) {
        
        // 1. Busca a instrução (Fetch)
        Instrucao inst = cpu->programa[cpu->PC];
        cpu->opcode = inst.opcode;
        
        if (cpu->opcode == -1) break; // Fim do programa
        
        // 2. Executa a instrução (Execute)
        CPU_executar_instrucao(cpu, mmu, inst);
        
        // =======================================================
        // 3. VERIFICAÇÃO DE INTERRUPÇÃO
        // =======================================================
        // 🗣️ FALA NA APRESENTAÇÃO: "Ao final de cada instrução, o hardware 
        // verifica se houve interrupção rolando um dado (probabilidade)."
        int sorteio = rand() % 100;
        
        // Se o número for menor que a probabilidade, a interrupção acontece!
        if (sorteio < probInterrupcao && cpu->programa_ti != NULL) {
            
            // --- PASSO A: SALVAR O CONTEXTO ---
            // 🗣️ FALA NA APRESENTAÇÃO: "Aqui ocorre o Salvamento de Contexto. 
            // Guardo o PC e os registradores atuais em variáveis seguras para 
            // não perdê-los quando o SO assumir o controle."
            int pc_salvo = cpu->PC;
            int opcode_salvo = cpu->opcode;
            int reg1_salvo = cpu->registrador1;
            int reg2_salvo = cpu->registrador2;
            
            // --- PASSO B: EXECUTAR O TRATADOR DE INTERRUPÇÃO (TI) ---
            // 🗣️ FALA NA APRESENTAÇÃO: "Zero o PC e desvio o fluxo para rodar 
            // a rotina do Sistema Operacional até o fim."
            cpu->PC = 0; 
            
            for (int i = 0; i < cpu->tamanho_ti; i++) {
                Instrucao inst_ti = cpu->programa_ti[cpu->PC];
                cpu->opcode = inst_ti.opcode;
                if (cpu->opcode == -1) break; 
                
                CPU_executar_instrucao(cpu, mmu, inst_ti);
            }
            
            // --- PASSO C: RESTAURAR O CONTEXTO ---
            // 🗣️ FALA NA APRESENTAÇÃO: "O TI terminou. Devolvo os valores 
            // antigos para os registradores. O programa principal volta a 
            // executar sem nem perceber que foi pausado."
            cpu->PC = pc_salvo;
            cpu->opcode = opcode_salvo;
            cpu->registrador1 = reg1_salvo;
            cpu->registrador2 = reg2_salvo;
        }
    }
}