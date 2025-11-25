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

//testando cpu
void testaCpu(){

    printf("TESTA FUNCIONALIDADESD DA CPU\n\n");
    // Criar RAM com 3 posições
    Ram ram;
    Ram_Criar(&ram, 10);

    // Inicializar a RAM com valores fixos
    Ram_setDado(&ram, 0, 7);   // RAM[0] = 7
    Ram_setDado(&ram, 1, 3);   // RAM[1] = 3
    Ram_setDado(&ram, 2, 0);   // RAM[2] será o resultado da soma

    // Criar CPU
    Cpu* cpu = CPU_criar();

    // Cria um pequeno programa de soma
    // Instruções:
    // 1) SOMA RAM[0] + RAM[1] -> RAM[2]
    // 2) HALT
    Instrucao programaSoma[] = {
        Instrucao_criar(0, 0, 1, 2),   // soma RAM[0] + RAM[1] → RAM[2]
        Instrucao_criar(-1, 0, 0, 0)   // halt
    };

    // Carregar programa na CPU
    CPU_setPrograma(cpu, programaSoma);
    // Executar programa
    CPU_iniciar(cpu, &ram);
    // Imprimir ram
    Ram_Imprimir(&ram);

    //criando pequeno programa de subtração com os endereços 0,1 e 2
    Instrucao programaSubtracao[] = {
        Instrucao_criar(1, 0, 1, 2),
        Instrucao_criar(-1,0,0,0)
    };

    // Carregar programa na CPU
    CPU_setPrograma(cpu, programaSubtracao);
    // Executar programa
    CPU_iniciar(cpu, &ram);
    // Imprimir ram
    Ram_Imprimir(&ram);
    
    // Liberar memória
    CPU_liberar(cpu);
    Ram_Liberar(&ram);
}

int main(){

    //testando ram
    testaRam();

    testaCpu();

    // Criar RAM com tamanho suficiente
    Ram ramProgramas;
    Ram_Criar(&ramProgramas, 20);

    // Criar CPU
    Cpu* cpuProgramas = CPU_criar();

    // Teste 1: 5 × 3
    printf("\n=== Teste 1: 5 × 3 ===\n");
    programaMult(&ramProgramas, cpuProgramas, 5, 3);
    //Ram_Imprimir(&ramProgramas);    

    printf("\n=== Teste 2: FATORIAL ===\n");
    programaFat(&ramProgramas, cpuProgramas, 5);
    //Ram_Imprimir(&ramProgramas);
    
    printf("\n=== Teste 3: Fibonnacci ===\n");
    programaFibonacci(&ramProgramas, cpuProgramas, 10);
    //Ram_Imprimir(&ramProgramas); 

    printf("\n=== Teste 4: SOMATORIO ===\n");
    programaSomatorio(&ramProgramas, cpuProgramas, 5);

    printf("\n=== Teste 5: SOMATORIO ===\n");
    programaPotencia(&ramProgramas, cpuProgramas, 2, 3);
    
    printf("\n=== Teste 6: MAIOR NUMERO ===\n");
     int maior = programaMaiorValor(&ramProgramas, cpuProgramas, 25, 10);
    printf("Maior valor encontrado = %d\n", maior);

   printf("\n=== Teste 7: MENOR NUMERO ===\n");
    int menor = programaMenorValor(&ramProgramas, cpuProgramas, 50, 40);
    printf("Menor valor encontrado = %d\n", menor);

    printf("\n=== Teste 8: DIVISAO ===\n");
    programaDivisao(&ramProgramas, cpuProgramas, 50, 5);

    printf("\n=== Teste 9: media ===\n");

    int dados_para_media[] = {15, 25, 30, 10}; // Exemplo: 15 + 25 + 30 + 10 = 80
    int numero_de_elementos = 4;              // Total de elementos
    programaMedia(&ramProgramas, cpuProgramas, dados_para_media, numero_de_elementos);

     printf("\n=== Teste 10: MDC ===\n");
     programaMDC(&ramProgramas, cpuProgramas, 48, 18);


  return 0;
}