#ifndef programas_h
#define programas_h

#include"ram.h"
#include"instrucao.h"
#include"cpu.h"

void programaMult(Ram* ram, Cpu* cpu,  int multiplicando, int multiplicador);
void programaFat(Ram* ram, Cpu* cpu, int fat);/*
void programaPotencia(Ram* ram, Cpu* cpu, int base, int expoente);
void programaFibonacci(Ram* ram, Cpu* cpu, int n);
void programaTaylor(Ram* ram, Cpu* cpu, int x, int n);
void programaSomatorio(Ram* ram, Cpu* cpu, int n);
int programaMaiorValor(Ram *ram, Cpu *cpu, int a, int b);
int programaMenorValor(Ram *ram, Cpu *cpu, int a, int b);
void programaDivisao(Ram *ram, Cpu *cpu, int a, int b);
void programaMedia(Ram *ram, Cpu *cpu, int *numeros, int count);
void programaMDC(Ram *ram, Cpu *cpu, int a, int b);






void programaDiv(Ram* ram, Cpu* cpu, int dividendo, int divisor);
void carregarVetoresNaRAM(Ram* ram, int* vetorA, int* vetorB, int size);
void programaSomaVetores(Ram* ram, Cpu* cpu, int size);*/

#endif