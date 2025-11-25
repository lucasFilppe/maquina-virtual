#ifndef ram_h
#define ram_h

typedef struct ram {
    int *memoria;
    int tamanho;
} Ram;

void Ram_Criar(Ram *ram, int tamanho); 
void Ram_Liberar(Ram *ram); 
void Ram_Imprimir(Ram *ram);
int Ram_getDado(Ram *ram, int endereco);
int Ram_setDado(Ram *ram, int endereco, int valor);
void gerar_RamAleatoria(Ram *ram, int valor_min, int valor_max);

#endif