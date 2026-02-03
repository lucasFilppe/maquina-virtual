#ifndef cpu_h
#define cpu_h

#include"ram.h"
#include"instrucao.h"
#include"cpu.h"
#include "mmu.h"

typedef struct cpu{

  int registrador1;
  int registrador2;
  int PC;
  Instrucao* programa;
  int opcode;
}Cpu;

Cpu* CPU_criar();
void CPU_liberar(Cpu* cpu);
void CPU_setPrograma(Cpu* cpu, Instrucao* programaAux);
void CPU_iniciar(Cpu* cpu, MMU* mmu);
void CPU_reset(Cpu* cpu);
int CPU_getRegistrador(Cpu *cpu, int indice);


#endif