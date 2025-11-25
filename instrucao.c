#include "instrucao.h"
#include <stdio.h>

// Função para criar uma nova instrução
Instrucao Instrucao_criar(int opcode, int add1, int add2, int add3) {
    Instrucao inst;
    inst.opcode = opcode;
    inst.add1 = add1;
    inst.add2 = add2;
    inst.add3 = add3;
    return inst;
}