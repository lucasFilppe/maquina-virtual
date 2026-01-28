#include <stdio.h>
#include "ram.h"
#include "instrucao.h"
#include "cpu.h"
#include "programas.h"

//testando funcionamento da ram
void testaRam(){
    
    printf("TESTA FUNCIONALIDADES DA RAM\n\n");
    Ram ramTesta;
    
    // Cria RAM com 10 posições
    Ram_Criar(&ramTesta, 10);
    Ram_Imprimir(&ramTesta);

    //colocando valores de 1 a 10 de forma aleatoria na ram criada
    printf("CRIANDO RAM COM VALORES ALEATORIOS DE 1 A 10\n");
    gerar_RamAleatoria(&ramTesta, 1, 10);
    Ram_Imprimir(&ramTesta);

    //pegando valor da ram[9]
    int valor = Ram_getDado(&ramTesta, 9);

    //atribui ao valor do edereço 9,  resultado de seu dobro
    int resultado = Ram_setDado(&ramTesta, 9, valor * 2);
    printf("Dobro do valor[%d] = %d\n", valor, resultado);
    Ram_Imprimir(&ramTesta);

    
}

#include <stdio.h>
#include <stdlib.h>
#include "cpu.h"
#include "ram.h"
#include "instrucao.h"

void testaCpu() {
    printf("TESTA FUNCIONALIDADES DA CPU (VIA OPCODES)\n\n");

    // 1. Criar Hardware (RAM vazia e CPU)
    Ram ram;
    Ram_Criar(&ram, 10); // Cria RAM vazia (tudo 0)
    Cpu* cpu = CPU_criar();

    // ----------------------------------------------------------------
    // TESTE 1: SOMA (7 + 3)
    // ----------------------------------------------------------------
    printf("--- Iniciando Teste de Soma ---\n");
    
    Instrucao programaSoma[] = {
        // --- PREPARAÇÃO DOS DADOS (Substitui Ram_setDado) ---
        // Colocar 7 na RAM[0]
        Instrucao_criar(4, 1, 7, 0),   // Op 4: Reg1 = 7 (Imediato)
        Instrucao_criar(2, 1, 0, 0),   // Op 2: RAM[0] = Reg1
        
        // Colocar 3 na RAM[1]
        Instrucao_criar(4, 1, 3, 0),   // Op 4: Reg1 = 3 (Imediato)
        Instrucao_criar(2, 1, 1, 0),   // Op 2: RAM[1] = Reg1

        // --- OPERAÇÃO ---
        // Op 0: RAM[0] + RAM[1] -> RAM[2]
        Instrucao_criar(0, 0, 1, 2),   

        // --- EXTRAÇÃO DO RESULTADO (Substitui Ram_getDado) ---
        // Trazer o resultado da RAM[2] para Reg1
        Instrucao_criar(3, 1, 2, 0),   // Op 3: Reg1 = RAM[2]
        
        // Gravar Reg1 na própria instrução (para o C ler depois)
        Instrucao_criar(5, 1, -1, 0),  // Op 5: Reg1 -> Instrução.add2

        // --- FIM ---
        Instrucao_criar(-1, 0, 0, 0)
    };

    // Executar
    CPU_reset(cpu);
    CPU_setPrograma(cpu, programaSoma);
    CPU_iniciar(cpu, &ram);

    // Verificação
    // O resultado ficou gravado na instrução de índice 6 (onde estava o Opcode 5), no campo add2
    int resultadoSoma = programaSoma[6].add2;
    printf("Resultado da Soma (7 + 3): %d\n\n", resultadoSoma);


    // ----------------------------------------------------------------
    // TESTE 2: SUBTRAÇÃO (7 - 3)
    // ----------------------------------------------------------------
    printf("--- Iniciando Teste de Subtracao ---\n");

    // Nota: Como a RAM persiste, o 7 e o 3 já estão lá. 
    // Mas para ser purista e independente, vamos carregar de novo (ou outros valores).
    
    Instrucao programaSub[] = {
        // Setup (Garanto que 7 e 3 estão lá)
        Instrucao_criar(4, 1, 7, 0),   // Reg1 = 7
        Instrucao_criar(2, 1, 0, 0),   // RAM[0] = 7
        Instrucao_criar(4, 1, 3, 0),   // Reg1 = 3
        Instrucao_criar(2, 1, 1, 0),   // RAM[1] = 3

        // Operação: RAM[0] - RAM[1] -> RAM[2]
        Instrucao_criar(1, 0, 1, 2),   // Op 1: Subtração

        // Extração
        Instrucao_criar(3, 1, 2, 0),   // Op 3: Reg1 = RAM[2]
        Instrucao_criar(5, 1, -1, 0),  // Op 5: Reg1 -> Instrução.add2 (Output)

        Instrucao_criar(-1, 0, 0, 0)
    };

    CPU_reset(cpu);
    CPU_setPrograma(cpu, programaSub);
    CPU_iniciar(cpu, &ram);

    // O Opcode 5 estava no índice 6 do vetor programaSub
    int resultadoSub = programaSub[6].add2;
    printf("Resultado da Subtracao (7 - 3): %d\n", resultadoSub);

    // Opcional: Imprimir a RAM inteira só para conferência final visual
    // Ram_Imprimir(&ram); 

    // Liberar memória
    CPU_liberar(cpu);
    Ram_Liberar(&ram);
}

int main(){

    //testando ram
    //testaRam();

    testaCpu();

    //Criar RAM com tamanho suficiente
    Ram ramProgramas;
    Ram_Criar(&ramProgramas, 5);

    //Criar CPU
    Cpu* cpuProgramas = CPU_criar();

    // Teste 1: 5 × 3
    printf("\n=== Teste 1: MULTIPLICACAO ===\n");
    programaMult(&ramProgramas,cpuProgramas, 10, 10);


    printf("Calculando Fatorial de %d...\n", 5);
    programaFat(&ramProgramas, cpuProgramas, 5);

  return 0;
}