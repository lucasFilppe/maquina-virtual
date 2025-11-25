#ifndef instrucao_h
#define instrucao_h

typedef struct instrucao {
    int add1;
    int add2;
    int add3;
    int opcode;
} Instrucao;

Instrucao Instrucao_criar(int opcode, int add1, int add2, int add3);
#endif