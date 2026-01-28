#include "ram.h"
#include "instrucao.h"
#include "cpu.h"
#include <stdio.h>

#include <stdio.h>
#include "cpu.h"
#include "ram.h"
#include "instrucao.h"

// Função que realiza a multiplicação A * B na máquina hipotética
// Retorna o resultado inteiro calculado pela CPU
void programaMult( Ram* ram, Cpu* cpu, int multiplicando, int multiplicador) {
    
    // MAPA DE MEMÓRIA DESTA FUNÇÃO:
    // RAM[0] = Multiplicando (Valor de X)
    // RAM[1] = Acumulador / Resultado
    
    // ----------------------------------------------------------------
    // PASSO 1: SETUP (Carregar dados iniciais)
    // ----------------------------------------------------------------
    // Carrega 'multiplicando' na RAM[0] e zera a RAM[1] (Resultado)
    Instrucao setup[] = {
        // RAM[0] = multiplicando
        Instrucao_criar(4, 1, multiplicando, 0), // Reg1 = x
        Instrucao_criar(2, 1, 0, 0),             // RAM[0] = Reg1
        
        // RAM[1] = 0 (Inicializando acumulador)
        Instrucao_criar(4, 1, 0, 0),             // Reg1 = 0
        Instrucao_criar(2, 1, 1, 0),             // RAM[1] = Reg1
        
        Instrucao_criar(-1, 0, 0, 0)             // HALT
    };
    
    CPU_reset(cpu);
    CPU_setPrograma(cpu, setup);
    CPU_iniciar(cpu, ram);

    // ----------------------------------------------------------------
    // PASSO 2: CÁLCULO (Somas Sucessivas)
    // ----------------------------------------------------------------
    // Repete a soma 'multiplicador' vezes
    // RAM[1] = RAM[1] + RAM[0]
    
    for (int i = 0; i < multiplicador; i++) {
        Instrucao soma[] = {
            // Op 0: Soma Acumulador (RAM[1]) + Valor (RAM[0]) -> Salva em Acumulador (RAM[1])
            Instrucao_criar(0, 1, 0, 1), 
            Instrucao_criar(-1, 0, 0, 0)
        };
        
        CPU_reset(cpu); // Reinicia PC para rodar a soma novamente
        CPU_setPrograma(cpu, soma);
        CPU_iniciar(cpu, ram);
    }

    // ----------------------------------------------------------------
    // PASSO 3: EXTRAÇÃO (Output)
    // ----------------------------------------------------------------
    // Pega o valor final de RAM[1] e devolve via instrução
    
    Instrucao output[] = {
        Instrucao_criar(3, 1, 1, 0),   // Op 3: Reg1 = RAM[1] (Resultado)
        Instrucao_criar(5, 1, -1, 0),  // Op 5: Reg1 -> Instrução.add2 (Escreve aqui)
        Instrucao_criar(-1, 0, 0, 0)
    };
    
    CPU_reset(cpu);
    CPU_setPrograma(cpu, output);
    CPU_iniciar(cpu, ram);
    
    // Lê o valor escrito pela CPU na instrução de índice 1
    int resultado = output[1].add2;
    
    printf("resultado %d\n", resultado);
}


void programaFat(Ram* ram, Cpu* cpu, int fat) {
    
    int j = 1;

    // Loop de 1 até fat
    for (int i = 1; i <= fat; i++) {
        
        // 1. Executa Multiplicação (Resultado fica na RAM[1])
        programaMult(ram, cpu, j, i);

        // 2. Extrai o resultado para atualizar 'j'
        Instrucao trecho1[3];

        // --- CORREÇÃO AQUI ---
        // Antes estava (3, 1, 0, 0) -> Lia da RAM[0]
        // Agora deve ser (3, 1, 1, 0) -> Lê da RAM[1] (Onde está o resultado)
        trecho1[0] = Instrucao_criar(3, 1, 1, 0); 

        // Exporta Reg1 para a instrução
        trecho1[1] = Instrucao_criar(5, 1, -1, 0); 
        trecho1[2] = Instrucao_criar(-1, 0, 0, 0);

        CPU_reset(cpu);
        CPU_setPrograma(cpu, trecho1);
        CPU_iniciar(cpu, ram);

        // Atualiza j com o resultado correto
        j = trecho1[1].add2;
        
        // Debug para você ver o valor crescendo
        printf("Fatorial parcial (passo %d): %d\n", i, j);
    }

    // --- Trecho final para exibir o resultado ---
    Instrucao trecho2[3];

    // --- CORREÇÃO AQUI TAMBÉM ---
    // Ler da RAM[1] novamente
    trecho2[0] = Instrucao_criar(3, 1, 1, 0); 
    
    trecho2[1] = Instrucao_criar(5, 1, -1, 0);
    trecho2[2] = Instrucao_criar(-1, 0, 0, 0);

    CPU_reset(cpu);
    CPU_setPrograma(cpu, trecho2);
    CPU_iniciar(cpu, ram);

    printf("O resultado do fatorial eh: %d\n", trecho2[1].add2);
}
/* programa de fatorial usando programaMult
void programaFat(Ram *ram, Cpu *cpu, int n)
{
    // Inicializar RAM[1] = 1 (resultado parcial)
    Instrucao init_result[3] = {
        Instrucao_criar(4, 1, 1, 0), // registrador1 = 1
        Instrucao_criar(2, 1, 1, 0), // RAM[1] = registrador1
        Instrucao_criar(-1, 0, 0, 0)};
    CPU_reset(cpu);
    CPU_setPrograma(cpu, init_result);
    CPU_iniciar(cpu, ram);

    // Inicializar RAM[2] = 1 (contador)
    Instrucao init_counter[3] = {
        Instrucao_criar(4, 1, 1, 0), // registrador1 = 1   <-- corrigido: devemos colocar 1 no registrador antes
        Instrucao_criar(2, 1, 2, 0), // RAM[2] = registrador1
        Instrucao_criar(-1, 0, 0, 0)};
    CPU_reset(cpu);
    CPU_setPrograma(cpu, init_counter);
    CPU_iniciar(cpu, ram);

    // Loop: i = 2 .. n
    for (int i = 2; i <= n; i++)
    {
        // Atualizar contador (RAM[2] = i)
        Instrucao update_counter[3] = {
            Instrucao_criar(4, 1, i, 0), // registrador1 = i
            Instrucao_criar(2, 1, 2, 0), // RAM[2] = registrador1
            Instrucao_criar(-1, 0, 0, 0)};
        CPU_reset(cpu);
        CPU_setPrograma(cpu, update_counter);
        CPU_iniciar(cpu, ram);

        // Ler os operandos atuais do fatorial:
        int resultado = Ram_getDado(ram, 1);     // valor em RAM[1]
        int multiplicador = Ram_getDado(ram, 2); // valor em RAM[2] (que é i)

        // Chama rotina de multiplicação (resultado * multiplicador).
        // programaMult escreve o produto em RAM[3].
        programaMult(ram, cpu, resultado, multiplicador);

        // Agora copiar RAM[3] -> RAM[1] (novo resultado do fatorial)
        Instrucao guardar_resultado[3] = {
            Instrucao_criar(4, 1, Ram_getDado(ram, 3), 0), // registrador1 = RAM[3] (produto)
            Instrucao_criar(2, 1, 1, 0),                   // RAM[1] = registrador1
            Instrucao_criar(-1, 0, 0, 0)};
        CPU_reset(cpu);
        CPU_setPrograma(cpu, guardar_resultado);
        CPU_iniciar(cpu, ram);
    }

    printf("Fatorial de %d = %d\n", n, Ram_getDado(ram, 1));
}

void programaFibonacci(Ram *ram, Cpu *cpu, int n)
{
    // Caso base: Fibonacci(0) = 0
    if (n == 0)
    {
        Instrucao casoZero[3] = {
            Instrucao_criar(4, 1, 0, 0), // registrador1 = 0
            Instrucao_criar(2, 1, 1, 0), // RAM[1] = registrador1
            Instrucao_criar(-1, 0, 0, 0)};
        CPU_reset(cpu);
        CPU_setPrograma(cpu, casoZero);
        CPU_iniciar(cpu, ram);

        printf("Fibonacci de %d = %d\n", n, Ram_getDado(ram, 1));
        return;
    }

    // Inicializar RAM[1] = 0 (F(n-2))
    Instrucao init_f0[3] = {
        Instrucao_criar(4, 1, 0, 0), // registrador1 = 0
        Instrucao_criar(2, 1, 1, 0), // RAM[1] = registrador1
        Instrucao_criar(-1, 0, 0, 0)};
    CPU_reset(cpu);
    CPU_setPrograma(cpu, init_f0);
    CPU_iniciar(cpu, ram);

    // Inicializar RAM[2] = 1 (F(n-1))
    Instrucao init_f1[3] = {
        Instrucao_criar(4, 1, 1, 0), // registrador1 = 1
        Instrucao_criar(2, 1, 2, 0), // RAM[2] = registrador1
        Instrucao_criar(-1, 0, 0, 0)};
    CPU_reset(cpu);
    CPU_setPrograma(cpu, init_f1);
    CPU_iniciar(cpu, ram);

    // Loop de cálculo do Fibonacci
    for (int i = 2; i <= n; i++)
    {
        //
        // RAM[3] = RAM[1] + RAM[2]
        //
        Instrucao soma[2] = {
            Instrucao_criar(0, 1, 2, 3), // RAM[3] = RAM[1] + RAM[2]
            Instrucao_criar(-1, 0, 0, 0)};
        CPU_reset(cpu);
        CPU_setPrograma(cpu, soma);
        CPU_iniciar(cpu, ram);

        //
        // RAM[1] = RAM[2]   (F(n-1) vira F(n-2))
        //
        Instrucao atualiza_f0[3] = {
            Instrucao_criar(3, 1, 2, 0), // registrador1 = RAM[2]
            Instrucao_criar(2, 1, 1, 0), // RAM[1] = registrador1
            Instrucao_criar(-1, 0, 0, 0)};
        CPU_reset(cpu);
        CPU_setPrograma(cpu, atualiza_f0);
        CPU_iniciar(cpu, ram);

        //
        // RAM[2] = RAM[3]   (novo F(n-1))
        //
        Instrucao atualiza_f1[3] = {
            Instrucao_criar(3, 1, 3, 0), // registrador1 = RAM[3]
            Instrucao_criar(2, 1, 2, 0), // RAM[2] = registrador1
            Instrucao_criar(-1, 0, 0, 0)};
        CPU_reset(cpu);
        CPU_setPrograma(cpu, atualiza_f1);
        CPU_iniciar(cpu, ram);
    }

    printf("Fibonacci de %d = %d\n", n, Ram_getDado(ram, 2));
}

void programaSomatorio(Ram *ram, Cpu *cpu, int n)
{
    // Inicializar RAM[0] = 0  (acumulador)
    Instrucao init[3] = {
        Instrucao_criar(4, 1, 0, 0), // registrador1 = 0
        Instrucao_criar(2, 1, 0, 0), // RAM[0] = registrador1
        Instrucao_criar(-1, 0, 0, 0)};
    CPU_reset(cpu);
    CPU_setPrograma(cpu, init);
    CPU_iniciar(cpu, ram);

    // Loop do somatório: soma de 1 até N
    for (int i = 1; i <= n; i++)
    {
        // Coloca i na RAM[1]
        Instrucao guarda_i[3] = {
            Instrucao_criar(4, 1, i, 0), // registrador1 = i
            Instrucao_criar(2, 1, 1, 0), // RAM[1] = registrador1
            Instrucao_criar(-1, 0, 0, 0)};
        CPU_reset(cpu);
        CPU_setPrograma(cpu, guarda_i);
        CPU_iniciar(cpu, ram);

        // RAM[0] = RAM[0] + RAM[1]
        Instrucao soma[2] = {
            Instrucao_criar(0, 0, 1, 0), // RAM[0] = RAM[0] + RAM[1]
            Instrucao_criar(-1, 0, 0, 0)};
        CPU_reset(cpu);
        CPU_setPrograma(cpu, soma);
        CPU_iniciar(cpu, ram);
    }

    // Copia resultado para RAM[2], opcional
    Instrucao copia[3] = {
        Instrucao_criar(3, 1, 0, 0), // registrador1 = RAM[0]
        Instrucao_criar(2, 1, 2, 0), // RAM[2] = registrador1
        Instrucao_criar(-1, 0, 0, 0)};
    CPU_reset(cpu);
    CPU_setPrograma(cpu, copia);
    CPU_iniciar(cpu, ram);

    printf("Resultado de 1 até %d = %d\n", n, Ram_getDado(ram, 2));
}

void programaPotencia(Ram *ram, Cpu *cpu, int base, int expoente)
{
    // Inicializar resultado (RAM[1]) com 1
    Instrucao init_resultado[3] = {
        Instrucao_criar(4, 1, 1, 0), // registrador1 = 1
        Instrucao_criar(2, 1, 1, 0), // RAM[1] = registrador1
        Instrucao_criar(-1, 0, 0, 0)};
    CPU_reset(cpu);
    CPU_setPrograma(cpu, init_resultado);
    CPU_iniciar(cpu, ram);

    for (int i = 0; i < expoente; i++)
    {
        int resultadoAtual = Ram_getDado(ram, 1);

        // Chamar o programa de multiplicação:
        // Multiplica resultadoAtual * base, e guarda o resultado final em RAM[3]
        programaMult(ram, cpu, resultadoAtual, base);

        // Agora, copiar o resultado da multiplicação (RAM[3]) de volta para RAM[1] (resultado)
        Instrucao atualiza_resultado[3] = {
            Instrucao_criar(3, 1, 3, 0), // registrador1 = RAM[3]
            Instrucao_criar(2, 1, 1, 0), // RAM[1] = registrador1
            Instrucao_criar(-1, 0, 0, 0)};
        CPU_reset(cpu);
        CPU_setPrograma(cpu, atualiza_resultado);
        CPU_iniciar(cpu, ram);
    }

    int resultadoFinal = Ram_getDado(ram, 1);
    printf("Resultado de %d elevado a %d = %d\n", base, expoente, resultadoFinal);
}

// Retorna o maior valor entre a e b
int programaMaiorValor(Ram *ram, Cpu *cpu, int a, int b)
{
    // coloca valores na RAM
    Ram_setDado(ram, 0, a); // RAM[0] = a
    Ram_setDado(ram, 1, b); // RAM[1] = b

    // programa que realiza RAM[3] = RAM[0] - RAM[1]
    Instrucao prog_sub[] = {
        Instrucao_criar(1, 0, 1, 3), // opcode 1: sub -> RAM[3] = RAM[0] - RAM[1]
        Instrucao_criar(-1, 0, 0, 0)
    };

    CPU_setPrograma(cpu, prog_sub);
    CPU_iniciar(cpu, ram);

    // ler resultado da subtração e decidir
    int diff = Ram_getDado(ram, 3); // diff = a - b

    if (diff >= 0) {
        // a >= b -> maior é a
        Ram_setDado(ram, 2, a); // RAM[2] = a
    } else {
        // a < b -> maior é b
        Ram_setDado(ram, 2, b); // RAM[2] = b
    }

    return Ram_getDado(ram, 2);
}

// Retorna o menor valor entre a e b
int programaMenorValor(Ram *ram, Cpu *cpu, int a, int b)
{
    // coloca valores na RAM
    Ram_setDado(ram, 0, a); // RAM[0] = a
    Ram_setDado(ram, 1, b); // RAM[1] = b

    // programa que realiza RAM[3] = RAM[0] - RAM[1]
    Instrucao prog_sub[] = {
        Instrucao_criar(1, 0, 1, 3), // opcode 1: sub -> RAM[3] = RAM[0] - RAM[1]
        Instrucao_criar(-1, 0, 0, 0)
    };

    CPU_setPrograma(cpu, prog_sub);
    CPU_iniciar(cpu, ram);

    // ler resultado da subtração e decidir
    int diff = Ram_getDado(ram, 3); // diff = a - b

    if (diff <= 0) {
        // a <= b -> menor é a
        Ram_setDado(ram, 2, a); // RAM[2] = a
    } else {
        // a > b -> menor é b
        Ram_setDado(ram, 2, b); // RAM[2] = b
    }

    return Ram_getDado(ram, 2);
}

// Calcula o Quociente em RAM[2] e o Resto em RAM[0] de A / B
void programaDivisao(Ram *ram, Cpu *cpu, int a, int b)
{
    if (b == 0 || a < 0 || b < 0) {
        printf("Erro/Aviso: Divisão inválida (por zero ou negativo).\n");
        return;
    }

    // 1. Inicializar RAM[0] = A (Dividendo) e RAM[1] = B (Divisor)
    Ram_setDado(ram, 0, a);
    Ram_setDado(ram, 1, b);

    // 2. Inicializar RAM[2] = 0 (Quociente)
    Instrucao init_quociente[3] = {
        Instrucao_criar(4, 1, 0, 0), // registrador1 = 0
        Instrucao_criar(2, 1, 2, 0), // RAM[2] (Quociente) = 0
        Instrucao_criar(-1, 0, 0, 0)};
    CPU_reset(cpu);
    CPU_setPrograma(cpu, init_quociente);
    CPU_iniciar(cpu, ram);

    // 3. Carregar RAM[3] = 1 (Auxiliar para incremento)
    Instrucao carrega_um[3] = {
        Instrucao_criar(4, 1, 1, 0), // registrador1 = 1
        Instrucao_criar(2, 1, 3, 0), // RAM[3] = 1
        Instrucao_criar(-1, 0, 0, 0)};
    CPU_reset(cpu);
    CPU_setPrograma(cpu, carrega_um);
    CPU_iniciar(cpu, ram);

    // Loop de Subtração (A - B)
    while (Ram_getDado(ram, 0) >= b)
    {
        // 4a. Subtrair: RAM[0] = RAM[0] - RAM[1] (Dividendo/Resto = Dividendo/Resto - Divisor)
        Instrucao subtracao[2] = {
            Instrucao_criar(1, 0, 1, 0), // opcode 1 (SUB): RAM[0] = RAM[0] - RAM[1]
            Instrucao_criar(-1, 0, 0, 0)};
        CPU_reset(cpu);
        CPU_setPrograma(cpu, subtracao);
        CPU_iniciar(cpu, ram);

        // 4b. Incrementar Quociente: RAM[2] = RAM[2] + RAM[3] (Quociente = Quociente + 1)
        Instrucao incrementa_quociente[2] = {
            Instrucao_criar(0, 2, 3, 2), // opcode 0 (ADD): RAM[2] = RAM[2] + RAM[3]
            Instrucao_criar(-1, 0, 0, 0)};
        CPU_reset(cpu);
        CPU_setPrograma(cpu, incrementa_quociente);
        CPU_iniciar(cpu, ram);
    }

    printf("Divisão de %d / %d:\n", a, b);
    printf("  Quociente (RAM[2]) = %d\n", Ram_getDado(ram, 2));
    printf("  Resto (RAM[0])     = %d\n", Ram_getDado(ram, 0));
}

void programaMedia(Ram *ram, Cpu *cpu, int *numeros, int count){
    if(count == 0){
        printf("O vetor esta vazio\n");
    }

    //INICALIza ram[0] = 0, acumula a soma
    Instrucao total[3] ={
        Instrucao_criar(4, 1, 0, 0),//registrador1 = 0
        Instrucao_criar(2, 1, 0, 0), //ram[0] = resgistrar1
        Instrucao_criar(-1, 0, 0, 0)
    };
    CPU_reset(cpu);
    CPU_setPrograma(cpu, total);
    CPU_iniciar(cpu, ram);

    // 2. RAM[1] = Count (Número de elementos)
    Ram_setDado(ram, 1, count);
    for (int i = 0; i < count; i++)
    {
        // 3a. Carregar o número atual (numeros[i]) para RAM[2]
        Instrucao carrega_numero[3] = {
            Instrucao_criar(4, 1, numeros[i], 0), // registrador1 = numeros[i]
            Instrucao_criar(2, 1, 2, 0),          // RAM[2] = registrador1
            Instrucao_criar(-1, 0, 0, 0)};
        CPU_reset(cpu);
        CPU_setPrograma(cpu, carrega_numero);
        CPU_iniciar(cpu, ram);

        // 3b. Soma: RAM[0] = RAM[0] + RAM[2] (Total = Total + numero)
        Instrucao soma_total[2] = {
            Instrucao_criar(0, 0, 2, 0), // opcode 0 (ADD): RAM[0] = RAM[0] + RAM[2]
            Instrucao_criar(-1, 0, 0, 0)};
        CPU_reset(cpu);
        CPU_setPrograma(cpu, soma_total);
        CPU_iniciar(cpu, ram);
    }

    // 4. Dividir o Total (RAM[0]) pelo Count (RAM[1])
    // Usamos a função auxiliar programaDivisao com argumentos que refletem seu estado atual na RAM:
    // Dividendo (RAM[0]) = Total; Divisor (RAM[1]) = Count
    // programDivisao vai usar RAM[0] (Dividendo), RAM[1] (Divisor) e colocar o Quociente em RAM[2]
    programaDivisao(ram, cpu, Ram_getDado(ram, 0), Ram_getDado(ram, 1)); 

    // 5. Copiar o Quociente (RAM[2]) para o resultado final RAM[3]
    Instrucao copia_resultado[3] = {
        Instrucao_criar(3, 1, 2, 0), // registrador1 = RAM[2] (Quociente)
        Instrucao_criar(2, 1, 3, 0), // RAM[3] = registrador1
        Instrucao_criar(-1, 0, 0, 0)};
    CPU_reset(cpu);
    CPU_setPrograma(cpu, copia_resultado);
    CPU_iniciar(cpu, ram);

    int media = Ram_getDado(ram, 3); 

    printf("Média (inteira) calculada: %d\n", media);
    
}

// Calcula o Máximo Divisor Comum de A e B (resultado em RAM[2])
void programaMDC(Ram *ram, Cpu *cpu, int a, int b)
{
    // -----------------------------
    // 1. Gravar A e B na RAM
    // RAM[0] = A, RAM[1] = B
    // -----------------------------
    Instrucao initAB[5] = {
        Instrucao_criar(4, 1, a, 0),  // registrador1 = A
        Instrucao_criar(2, 1, 0, 0),  // RAM[0] = registrador1
        Instrucao_criar(4, 1, b, 0),  // registrador1 = B
        Instrucao_criar(2, 1, 1, 0),  // RAM[1] = registrador1
        Instrucao_criar(-1, 0, 0, 0)  // HALT
    };

    CPU_reset(cpu);
    CPU_setPrograma(cpu, initAB);
    CPU_iniciar(cpu, ram);

    printf("Calculando MDC de %d e %d...\n", a, b);

    int tempA, tempB;

    // -----------------------------
    // 2. Loop principal (controlado pelo C)
    // -----------------------------
    while (1)
    {
        // 2a. Ler A e B da RAM
        // Usamos Ram_getDado diretamente para evitar CPU_getRegistrador
        tempA = Ram_getDado(ram, 0); // Ler o valor atual de A (RAM[0])
        tempB = Ram_getDado(ram, 1); // Ler o valor atual de B (RAM[1])

        // 2b. Condição de parada: A == B
        if (tempA == tempB)
            break;

        // -----------------------------
        // 3. Subtrair usando CPU
        // -----------------------------
        if (tempA > tempB)
        {
            // RAM[0] = RAM[0] - RAM[1]
            Instrucao subA[2] = {
                Instrucao_criar(1, 0, 1, 0),   // SUB RAM[0] = RAM[0] - RAM[1]
                Instrucao_criar(-1, 0, 0, 0)
            };
            CPU_reset(cpu);
            CPU_setPrograma(cpu, subA);
            CPU_iniciar(cpu, ram);
        }
        else // tempB > tempA
        {
            // RAM[1] = RAM[1] - RAM[0]
            Instrucao subB[2] = {
                Instrucao_criar(1, 1, 0, 1),   // SUB RAM[1] = RAM[1] - RAM[0]
                Instrucao_criar(-1, 0, 0, 0)
            };
            CPU_reset(cpu);
            CPU_setPrograma(cpu, subB);
            CPU_iniciar(cpu, ram);
        }
    }

    // -----------------------------
    // 4. Copiar resultado (RAM[0] ou RAM[1]) para RAM[2]
    // -----------------------------
    // Já que A == B na saída do loop, o MDC é o valor em RAM[0] (ou RAM[1]).
    // Vamos usar o valor de RAM[0] e copiá-lo para a posição de resultado RAM[2].
    
    Instrucao copiaResultado[4] = {
        Instrucao_criar(3, 1, 0, 0),  // registrador1 = RAM[0] (ou RAM[1] - tanto faz)
        Instrucao_criar(2, 1, 2, 0),  // RAM[2] = registrador1
        Instrucao_criar(-1, 0, 0, 0)
    };

    CPU_reset(cpu);
    CPU_setPrograma(cpu, copiaResultado);
    CPU_iniciar(cpu, ram);

    // -----------------------------
    // 5. Ler e imprimir o resultado final de RAM[2]
    // (Seguindo o padrão de leitura da multiplicação)
    // -----------------------------
    int resultadoMDC = Ram_getDado(ram, 2);
    printf("  MDC = %d\n", resultadoMDC);
}


void programaElevacaoQuadrado(Ram *ram, Cpu *cpu, int n)
{
    printf("Calculando o Quadrado de %d:\n", n);

    // Chama a rotina de multiplicação: n * n
    // O programaMult guarda o resultado em RAM[3].
    //programaMultiplicacao(ram, cpu, n, n); 

    // COPIA PELA CPU: Copiar RAM[3] (Produto) para RAM[4] (Resultado Final)
    Instrucao copia_resultado[3] = {
        Instrucao_criar(3, 1, 3, 0), // Opcode 3: registrador1 = RAM[3]
        Instrucao_criar(2, 1, 4, 0), // Opcode 2: RAM[4] = registrador1
        Instrucao_criar(-1, 0, 0, 0)}; // HALT
        
    CPU_reset(cpu);
    CPU_setPrograma(cpu, copia_resultado);
    CPU_iniciar(cpu, ram);

    int resultado = Ram_getDado(ram, 4);
    printf("Resultado de %d^2 = %d (em RAM[4])\n", n, resultado);
}

void programaRaizQuadrada(Ram *ram, Cpu *cpu, int n)
{
    if (n < 0) {
        printf("Erro: Raiz quadrada de número negativo não suportada.\n");
        return;
    }
    
    // 1. SETUP: Inicializar RAM
    Ram_setDado(ram, 0, n); // RAM[0] = N (Número a ter a raiz calculada)

    // Inicializar RAM[1] = 1 (Primeiro ímpar)
    Instrucao init_impar[3] = {
        Instrucao_criar(4, 1, 1, 0), // registrador1 = 1
        Instrucao_criar(2, 1, 1, 0), // RAM[1] = registrador1
        Instrucao_criar(-1, 0, 0, 0)};
    CPU_reset(cpu); CPU_setPrograma(cpu, init_impar); CPU_iniciar(cpu, ram);

    // Inicializar RAM[2] = 2 (Constante para incremento)
    Instrucao init_dois[3] = {
        Instrucao_criar(4, 1, 2, 0), // registrador1 = 2
        Instrucao_criar(2, 1, 2, 0), // RAM[2] = registrador1
        Instrucao_criar(-1, 0, 0, 0)};
    CPU_reset(cpu); CPU_setPrograma(cpu, init_dois); CPU_iniciar(cpu, ram);
    
    // Inicializar RAM[3] = 0 (Contador da Raiz / Resultado)
    Instrucao init_resultado[3] = {
        Instrucao_criar(4, 1, 0, 0), // registrador1 = 0
        Instrucao_criar(2, 1, 3, 0), // RAM[3] = registrador1
        Instrucao_criar(-1, 0, 0, 0)};
    CPU_reset(cpu); CPU_setPrograma(cpu, init_resultado); CPU_iniciar(cpu, ram);


    printf("Calculando Raiz Quadrada de %d:\n", n);

    // LOOP DE CONTROLE (feito em C)
    // O loop continua enquanto N (RAM[0]) for maior ou igual ao ímpar atual (RAM[1])
    while (Ram_getDado(ram, 0) >= Ram_getDado(ram, 1))
    {
        // 2a. SUBTRAÇÃO PELA CPU: N = N - Ímpar
        // RAM[0] = RAM[0] - RAM[1]
        Instrucao subtracao[2] = {
            Instrucao_criar(1, 0, 1, 0), // Opcode 1 (SUB): RAM[0] = RAM[0] - RAM[1]
            Instrucao_criar(-1, 0, 0, 0)}; // HALT
        CPU_reset(cpu); CPU_setPrograma(cpu, subtracao); CPU_iniciar(cpu, ram);

        // 2b. INCREMENTA O CONTADOR (Raiz): Resultado = Resultado + 1
        // RAM[3] = RAM[3] + RAM[2] (Note: RAM[2] = 2, mas faremos o incremento de 1 por fora)
        
        // Usaremos o registrador para carregar 1 e somar, já que RAM[2] = 2
        Instrucao incrementa_resultado[4] = {
            Instrucao_criar(4, 1, 1, 0), // registrador1 = 1
            Instrucao_criar(3, 2, 3, 0), // registrador2 = RAM[3] (Resultado)
            Instrucao_criar(0, 1, 2, 3), // Opcode 0 (ADD): RAM[3] = registrador1 + registrador2 (Resultado + 1)
            Instrucao_criar(-1, 0, 0, 0)}; // HALT
        CPU_reset(cpu); CPU_setPrograma(cpu, incrementa_resultado); CPU_iniciar(cpu, ram);
        
        // 2c. INCREMENTA O ÍMPAR: Ímpar = Ímpar + 2
        // RAM[1] = RAM[1] + RAM[2]
        Instrucao incrementa_impar[2] = {
            Instrucao_criar(0, 1, 2, 1), // Opcode 0 (ADD): RAM[1] = RAM[1] + RAM[2] (Impar + 2)
            Instrucao_criar(-1, 0, 0, 0)}; // HALT
        CPU_reset(cpu); CPU_setPrograma(cpu, incrementa_impar); CPU_iniciar(cpu, ram);
    }
    
    // O RESULTADO FINAL ESTÁ EM RAM[3]
    int resultado = Ram_getDado(ram, 3);
    printf("Resultado de Raiz Quadrada (inteira) de %d = %d (em RAM[3])\n", n, resultado);
}

// Assume que as funções programaMultiplicacao, programaMDC e programaDivisao
// estão disponíveis e funcionam conforme definido:
// - programaMultiplicacao deixa o produto em RAM[3].
// - programaMDC deixa o MDC em RAM[2].
// - programaDivisao (RAM[0], RAM[1]) deixa o Quociente em RAM[2].

void programaMMC(Ram *ram, Cpu *cpu, int a, int b)
{
    if (a == 0 || b == 0) {
        printf("Erro: MMC não definido para números iguais a zero.\n");
        return;
    }
    
    printf("Calculando o MMC de %d e %d usando a fórmula (A * B) / MDC(A, B):\n", a, b);

    // --- ETAPA 1: CALCULAR O PRODUTO P = A * B ---
    // O programaMultiplicacao armazena o produto P em RAM[3].
    programaMult(ram, cpu, a, b); 
    int produto = Ram_getDado(ram, 3);
    printf("  Produto (A * B) = %d\n", produto);


    // --- ETAPA 2: CALCULAR O DIVISOR D = MDC(A, B) ---
    // O programaMDC armazena o MDC D em RAM[2].
    programaMDC(ram, cpu, a, b);
    int mdc = Ram_getDado(ram, 2);
    printf("  MDC(A, B) = %d\n", mdc);
    
    
    // --- ETAPA 3: PREPARAR PARA A DIVISÃO MMC = P / D ---
    
    // 3a. MOVER o Dividendo P: RAM[3] (Produto) -> RAM[0]
    Instrucao move_produto[3] = {
        Instrucao_criar(3, 1, 3, 0), // Opcode 3: registrador1 = RAM[3] (Produto P)
        Instrucao_criar(2, 1, 0, 0), // Opcode 2: RAM[0] = registrador1 (Novo Dividendo)
        Instrucao_criar(-1, 0, 0, 0)};
    CPU_reset(cpu); CPU_setPrograma(cpu, move_produto); CPU_iniciar(cpu, ram);
    
    // 3b. MOVER o Divisor D: RAM[2] (MDC) -> RAM[1]
    Instrucao move_mdc[3] = {
        Instrucao_criar(3, 1, 2, 0), // Opcode 3: registrador1 = RAM[2] (MDC D)
        Instrucao_criar(2, 1, 1, 0), // Opcode 2: RAM[1] = registrador1 (Novo Divisor)
        Instrucao_criar(-1, 0, 0, 0)};
    CPU_reset(cpu); CPU_setPrograma(cpu, move_mdc); CPU_iniciar(cpu, ram);


    // --- ETAPA 4: DIVISÃO PELA CPU MMC = RAM[0] / RAM[1] ---
    // programaDivisao(Dividendo=RAM[0], Divisor=RAM[1]) deixa o Quociente (MMC) em RAM[2].
    programaDivisao(ram, cpu, Ram_getDado(ram, 0), Ram_getDado(ram, 1)); 

    int mmc_resultado = Ram_getDado(ram, 2);
    printf("\nResultado Final:\n");
    printf("  MMC (RAM[2]) = %d\n", mmc_resultado);
}

/*void carregarVetoresNaRAM(Ram *ram, int *vetorA, int *vetorB, int size)
{
    int capacidade = Ram_getTamanho(ram);

    // Verifica se há espaço suficiente
    if (2 * size > capacidade)
    {
        printf("ERRO: RAM insuficiente. Necessário %d posições, disponível %d\n",
               2 * size, capacidade);
        return;
    }

    // Carregar vetor A
    for (int i = 0; i < size; i++)
        Ram_setDado(ram, i, vetorA[i]);

    // Carregar vetor B
    for (int i = 0; i < size; i++)
        Ram_setDado(ram, size + i, vetorB[i]);

    // Exibir na tela
    printf("Vetor A (RAM[0..%d]): ", size - 1);
    for (int i = 0; i < size; i++)
        printf("%d ", Ram_getDado(ram, i));
    printf("\n");

    printf("Vetor B (RAM[%d..%d]): ", size, size + size - 1);
    for (int i = 0; i < size; i++)
        printf("%d ", Ram_getDado(ram, size + i));
    printf("\n");
}


void programaSomaVetores(Ram *ram, Cpu *cpu, int size)
{
    int capacidade = Ram_getTamanho(ram);

    if (3 * size > capacidade)
    {
        printf("ERRO: RAM insuficiente para armazenar resultado.\n");
        return;
    }

    // Para cada posição i, somar A[i] + B[i]
    for (int i = 0; i < size; i++)
    {
        // Copiar A[i] para RAM[0]
        Instrucao loadA[3] = {
            Instrucao_criar(3, 1, i, 0), // R1 = RAM[i]
            Instrucao_criar(2, 1, 0, 0), // RAM[0] = R1
            Instrucao_criar(-1, 0, 0, 0)
        };
        CPU_reset(cpu);
        CPU_setPrograma(cpu, loadA);
        CPU_iniciar(cpu, ram);

        // Copiar B[i] para RAM[1]
        Instrucao loadB[3] = {
            Instrucao_criar(3, 1, size + i, 0), // R1 = RAM[size+i]
            Instrucao_criar(2, 1, 1, 0),        // RAM[1] = R1
            Instrucao_criar(-1, 0, 0, 0)
        };
        CPU_reset(cpu);
        CPU_setPrograma(cpu, loadB);
        CPU_iniciar(cpu, ram);

        // Somar RAM[0] + RAM[1] → RAM[2]
        Instrucao soma[2] = {
            Instrucao_criar(0, 0, 1, 2), // RAM[2] = RAM[0] + RAM[1]
            Instrucao_criar(-1, 0, 0, 0)
        };
        CPU_reset(cpu);
        CPU_setPrograma(cpu, soma);
        CPU_iniciar(cpu, ram);

        // Copiar resultado para posição final RAM[2*size + i]
        Instrucao gravaResultado[3] = {
            Instrucao_criar(3, 1, 2, 0),         // R1 = RAM[2]
            Instrucao_criar(2, 1, 2 * size + i, 0),
            Instrucao_criar(-1, 0, 0, 0)
        };
        CPU_reset(cpu);
        CPU_setPrograma(cpu, gravaResultado);
        CPU_iniciar(cpu, ram);
    }

    // Imprimir os resultados
    printf("Soma concluída. Resultados em RAM[%d..%d]\n", 2 * size, 3 * size - 1);
    printf("Resultados: ");
    for (int i = 0; i < size; i++)
        printf("%d ", Ram_getDado(ram, 2 * size + i));
    printf("\n");
}
    */
