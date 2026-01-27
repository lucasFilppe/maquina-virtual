#include <stdlib.h>
#include <stdio.h>
#include "cpu.h"
#include "instrucao.h"
#include "ram.h"

//funçaõ que para cria uma CPu
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
    
    // Reseta registradores
    cpu->registrador1 = 0;
    cpu->registrador2 = 0;
    
    // Reseta contador de programa
    cpu->PC = 0;
    
    // Reseta estado de execução
    cpu->opcode = 0;
    
    // Não resetamos o ponteiro de programa pois ele é gerenciado externamente
    // cpu->programa = NULL; // Não deve ser feito aqui!
}

//funão para liberar memoria da cpu
void CPU_liberar(Cpu* cpu) {
    if(cpu != NULL) {
        free(cpu);
    }
}

/* @brief Define o ponteiro para o programa (conjunto de instruções) a ser executado pela CPU.
 * * Esta função atribui um novo array de instruções à estrutura da CPU.
 * * @param cpu Um ponteiro para a estrutura Cpu a ser configurada.
 * @param programaAux Um ponteiro para o array de estruturas Instrucao que representa o programa.
 */
void CPU_setPrograma(Cpu* cpu, Instrucao* programaAux) {
    if(cpu != NULL) {
        cpu->programa = programaAux;
    }
}


void CPU_iniciar(Cpu* cpu, Ram* ram) {
    if(cpu == NULL || ram == NULL || cpu->programa == NULL) {
        printf("Parametros invalidos para CPU_iniciar\n");
        return;
    }
    
     // Substituir reset manual por chamada à CPU_reset
    CPU_reset(cpu);  // Usa a nova função de reset
    
    while(cpu->opcode != -1) {
        Instrucao inst = cpu->programa[cpu->PC];
        cpu->opcode = inst.opcode;//atrubui os valor da instruçao opcode a cpu
        
        switch(cpu->opcode) {
            case -1: { // halt
                printf("Programa terminou!!\n");
                //Ram_Imprimir(ram);
                break;
            }
            case 0: { // soma
                cpu->registrador1 = Ram_getDado(ram, inst.add1);
                cpu->registrador2 = Ram_getDado(ram, inst.add2);
                cpu->registrador1 += cpu->registrador2;
                Ram_setDado(ram, inst.add3, cpu->registrador1);
                printf("Inst sum -> RAM posicao %d com conteudo %d\n", inst.add3, cpu->registrador1);
                cpu->PC++;
                break;
            }
            case 1: { // subtrai
                cpu->registrador1 = Ram_getDado(ram, inst.add1);
                cpu->registrador2 = Ram_getDado(ram, inst.add2);
                cpu->registrador1 -= cpu->registrador2;
                Ram_setDado(ram, inst.add3, cpu->registrador1);
                printf("Inst sub -> RAM posicao %d com conteudo %d\n", inst.add3, cpu->registrador1);
                cpu->PC++;
                break;
            }
            case 2: { // registrador -> RAM
                if(inst.add1 == 1) {
                    Ram_setDado(ram, inst.add2, cpu->registrador1);
                    printf("Inst copy_reg_ram -> RAM posicao %d com conteudo %d\n", inst.add2, cpu->registrador1);
                } else if(inst.add1 == 2) {
                    Ram_setDado(ram, inst.add2, cpu->registrador2);
                    printf("Inst copy_reg_ram -> RAM posicao %d com conteudo %d\n", inst.add2, cpu->registrador2);
                }
                cpu->PC++;
                break;
            }
            case 3: { // RAM -> registrador
                if(inst.add1 == 1) {
                    cpu->registrador1 = Ram_getDado(ram, inst.add2);
                    printf("Inst copy_ram_reg -> Registrador1 com conteudo %d\n", cpu->registrador1);
                } else if(inst.add1 == 2) {
                    cpu->registrador2 = Ram_getDado(ram, inst.add2);
                    printf("Inst copy_ram_reg -> Registrador2 com conteudo %d\n", cpu->registrador2);
                }
                cpu->PC++;
                break;
            }
            case 4: { // valor imediato -> registrador
                if(inst.add1 == 1) {
                    cpu->registrador1 = inst.add2;
                    printf("Inst copy_ext_reg -> Registrador1 com conteudo %d\n", cpu->registrador1);
                } else if(inst.add1 == 2) {
                    cpu->registrador2 = inst.add2;
                    printf("Inst copy_ext_reg -> Registrador2 com conteudo %d\n", cpu->registrador2);
                }
                cpu->PC++;
                break;
            }
            case 5: { // registrador -> instrução (modifica programa)
                if(inst.add1 == 1) {
                    cpu->programa[cpu->PC].add2 = cpu->registrador1;
                    printf("Inst obtain_reg -> Registrador1 com conteudo %d\n", cpu->registrador1);
                } else if(inst.add1 == 2) {
                    cpu->programa[cpu->PC].add2 = cpu->registrador2;
                    printf("Inst obtain_reg -> Registrador2 com conteudo %d\n", cpu->registrador2);
                }
                cpu->PC++;
                break;
            }
            default: {
                printf("Instrucao desconhecida: %d\n", cpu->opcode);
                cpu->PC++;
                break;
            }
        }
    }
}