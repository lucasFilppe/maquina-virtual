#include <stdlib.h>
#include <stdio.h>
#include "cpu.h"
#include "instrucao.h"
#include "mmu.h" // <--- MUDANÇA: Agora incluímos a MMU

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

void CPU_setPrograma(Cpu* cpu, Instrucao* programaAux) {
    if(cpu != NULL) {
        cpu->programa = programaAux;
    }
}

// --- MUDANÇA CRÍTICA AQUI ---
// A CPU agora conversa com a MMU, não com a RAM direta
void CPU_iniciar(Cpu* cpu, MMU* mmu) {
    
    if(cpu == NULL || mmu == NULL || cpu->programa == NULL) {
        printf("Parametros invalidos para CPU_iniciar\n");
        return;
    }
    
    CPU_reset(cpu);
    
    while(cpu->opcode != -1) {
        Instrucao inst = cpu->programa[cpu->PC];
        cpu->opcode = inst.opcode;
        
        switch(cpu->opcode) {
            case -1: { // halt
                printf("Programa terminou!!\n");
                break;
            }
            case 0: { // soma
                // ANTES: Ram_getDado(ram, inst.add1);
                // AGORA: A MMU procura nas Caches L1->L2->L3->RAM
                cpu->registrador1 = MMU_buscar(mmu, inst.add1);
                cpu->registrador2 = MMU_buscar(mmu, inst.add2);
                
                cpu->registrador1 += cpu->registrador2;
                
                // ANTES: Ram_setDado(ram, inst.add3, ...);
                // AGORA: A MMU decide onde escrever (Write-Back ou Write-Through)
                MMU_escrever(mmu, inst.add3, cpu->registrador1);
                
                //printf("Inst sum -> MMU escreveu na posicao %d o valor %d\n", inst.add3, cpu->registrador1);
                cpu->PC++;
                break;
            }
            case 1: { // subtrai
                // Solicita dados à MMU
                cpu->registrador1 = MMU_buscar(mmu, inst.add1);
                cpu->registrador2 = MMU_buscar(mmu, inst.add2);
                
                cpu->registrador1 -= cpu->registrador2;
                
                // Salva via MMU
                MMU_escrever(mmu, inst.add3, cpu->registrador1);
                
                //printf("Inst sub -> MMU escreveu na posicao %d o valor %d\n", inst.add3, cpu->registrador1);
                cpu->PC++;
                break;
            }
            case 2: { // registrador -> Memória
                if(inst.add1 == 1) {
                    MMU_escrever(mmu, inst.add2, cpu->registrador1);
                    //printf("Inst copy_reg_mem -> Posicao %d recebe Reg1 (%d)\n", inst.add2, cpu->registrador1);
                } else if(inst.add1 == 2) {
                    MMU_escrever(mmu, inst.add2, cpu->registrador2);
                    //printf("Inst copy_reg_mem -> Posicao %d recebe Reg2 (%d)\n", inst.add2, cpu->registrador2);
                }
                cpu->PC++;
                break;
            }
            case 3: { // Memória -> registrador
                if(inst.add1 == 1) {
                    cpu->registrador1 = MMU_buscar(mmu, inst.add2);
                    //printf("Inst copy_mem_reg -> Reg1 recebe conteudo %d\n", cpu->registrador1);
                } else if(inst.add1 == 2) {
                    cpu->registrador2 = MMU_buscar(mmu, inst.add2);
                    //printf("Inst copy_mem_reg -> Reg2 recebe conteudo %d\n", cpu->registrador2);
                }
                cpu->PC++;
                break;
            }
            case 4: { // Imediato -> Registrador
                // (Não muda nada, pois não acessa memória)
                if(inst.add1 == 1) {
                    cpu->registrador1 = inst.add2;
                    //printf("Inst copy_ext_reg -> Registrador1 com conteudo %d\n", cpu->registrador1);
                } else if(inst.add1 == 2) {
                    cpu->registrador2 = inst.add2;
                    //printf("Inst copy_ext_reg -> Registrador2 com conteudo %d\n", cpu->registrador2);
                }
                cpu->PC++;
                break;
            }
            case 5: { // Modificação de Instrução (Output)
                // (Não muda nada, altera o próprio vetor de programa)
                if(inst.add1 == 1) {
                    cpu->programa[cpu->PC].add2 = cpu->registrador1;
                    //printf("Inst obtain_reg -> Registrador1 com conteudo %d\n", cpu->registrador1);
                } else if(inst.add1 == 2) {
                    cpu->programa[cpu->PC].add2 = cpu->registrador2;
                    //printf("Inst obtain_reg -> Registrador2 com conteudo %d\n", cpu->registrador2);
                }
                cpu->PC++;
                break;
            }
            default: {
                //printf("Instrucao desconhecida: %d\n", cpu->opcode);
                cpu->PC++;
                break;
            }
        }
    }
}