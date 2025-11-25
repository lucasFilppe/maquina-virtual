#include "ram.h"
#include "instrucao.h"
#include "cpu.h"
#include <stdio.h>

// programa de multiplicação
void programaMult(Ram *ram, Cpu *cpu, int x, int y)
{
    // Colocar o multiplicando (x) na RAM[1] com 3 instruçoes
    Instrucao carregar_x[3] = {
        Instrucao_criar(4, 1, x, 0), // registrador1 = x
        Instrucao_criar(2, 1, 1, 0), // RAM[1] = registrador1
        Instrucao_criar(-1, 0, 0, 0)};
    CPU_reset(cpu);
    CPU_setPrograma(cpu, carregar_x);
    CPU_iniciar(cpu, ram);

    // Colocar o multiplicador (y) na RAM[2]
    Instrucao carregar_y[3] = {
        Instrucao_criar(4, 1, y, 0), // registrador1 = y
        Instrucao_criar(2, 1, 2, 0), // RAM[2] = registrador1
        Instrucao_criar(-1, 0, 0, 0)};
    CPU_reset(cpu);
    CPU_setPrograma(cpu, carregar_y);
    CPU_iniciar(cpu, ram);

    // Inicializar acumulador (RAM[3]) = 0
    Instrucao init_resultado[3] = {
        Instrucao_criar(4, 1, 0, 0), // registrador1 = 0
        Instrucao_criar(2, 1, 3, 0), // RAM[3] = registrador1
        Instrucao_criar(-1, 0, 0, 0)};
    CPU_reset(cpu);
    CPU_setPrograma(cpu, init_resultado);
    CPU_iniciar(cpu, ram);

    // Loop: repetir y vezes → resultado += x
    for (int i = 0; i < y; i++)
    {
        // Somar RAM[3] + RAM[1] → novo resultado na RAM[3]
        Instrucao soma[2] = {
            Instrucao_criar(0, 3, 1, 3), // RAM[3] = RAM[3] + RAM[1]
            Instrucao_criar(-1, 0, 0, 0)};
        CPU_reset(cpu);
        CPU_setPrograma(cpu, soma);
        CPU_iniciar(cpu, ram);
    }

    int resultado = Ram_getDado(ram, 3);
    printf("Resultado de %d x %d = %d\n", x, y, resultado);
}

// programa de fatorial usando programaMult
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
    // 1. Inicializar RAM[0] = A e RAM[1] = B
    Ram_setDado(ram, 0, a);
    Ram_setDado(ram, 1, b);
    
    int tempA = a;
    int tempB = b;

    printf("Calculando MDC de %d e %d:\n", a, b);

    // Loop: continua enquanto A != B
    while (tempA != tempB)
    {
        // operação de subtração

        if (tempA > tempB)
        {
            // A > B: RAM[0] = RAM[0] - RAM[1]
            Instrucao subtracao_a[2] = {
                Instrucao_criar(1, 0, 1, 0), // opcode 1 (SUB): RAM[0] = RAM[0] - RAM[1]
                Instrucao_criar(-1, 0, 0, 0)};
            CPU_reset(cpu);
            CPU_setPrograma(cpu, subtracao_a);
            CPU_iniciar(cpu, ram);
        }
        else // B > A
        {
            // B > A: RAM[1] = RAM[1] - RAM[0]
            Instrucao subtracao_b[2] = {
                Instrucao_criar(1, 1, 0, 1), // opcode 1 (SUB): RAM[1] = RAM[1] - RAM[0]
                Instrucao_criar(-1, 0, 0, 0)};
            CPU_reset(cpu);
            CPU_setPrograma(cpu, subtracao_b);
            CPU_iniciar(cpu, ram);
        }

        // Atualiza as variáveis de controle do loop lendo da RAM
        tempA = Ram_getDado(ram, 0);
        tempB = Ram_getDado(ram, 1);
        // printf("  Passo: A=%d, B=%d\n", tempA, tempB); // Opcional para debug
    }

    // 2. Copiar o resultado (RAM[0] ou RAM[1]) para RAM[2]
    Instrucao copia_resultado[3] = {
        Instrucao_criar(3, 1, 0, 0), // registrador1 = RAM[0]
        Instrucao_criar(2, 1, 2, 0), // RAM[2] = registrador1
        Instrucao_criar(-1, 0, 0, 0)};
    CPU_reset(cpu);
    CPU_setPrograma(cpu, copia_resultado);
    CPU_iniciar(cpu, ram);

    printf("  MDC (RAM[2]) = %d\n", Ram_getDado(ram, 2));
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

