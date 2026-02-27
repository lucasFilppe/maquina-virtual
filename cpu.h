#ifndef CPU_H
#define CPU_H

#include "instrucao.h"
#include "mmu.h" // A CPU conhece a MMU

typedef struct {
    int registrador1;
    int registrador2;
    int PC;
    int opcode;
    
    // Programa Principal
    Instrucao *programa;
    int tamanho_programa; // Útil saber o tamanho se precisarmos iterar com for
    
    // Tratador de Interrupção (TI) - NOVO PARA O TP3
    Instrucao *programa_ti;
    int tamanho_ti;
    
} Cpu;

// Funções
Cpu* CPU_criar();
void CPU_reset(Cpu* cpu);
void CPU_liberar(Cpu* cpu);

void CPU_setPrograma(Cpu* cpu, Instrucao* programaAux, int tamanho);
void CPU_setProgramaTI(Cpu* cpu, Instrucao* programaTI, int tamanho); // NOVA

// A inicialização agora recebe a probabilidade de ocorrer uma interrupção (0 a 100)
void CPU_iniciar(Cpu* cpu, MMU* mmu, int probInterrupcao);

#endif