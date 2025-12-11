#ifndef MAPA_H
#define MAPA_H

#define LinhaMatriz 20
#define ColunaMatriz 40

#include <stdio.h>
#include <stdlib.h>
#include "structs.h"

char **criar_matriz();
void liberar_mapa(char **matriz);
char **ler_arquivo(const char *nome, int *pellets, personagem *pacman, inimigo *fantasmas, int *num_fantasmas);
int ler_mapa(FILE *arquivo, char **matriz, personagem *pacman, inimigo *fantasmas, int *num_fantasmas);
void passar_mapa(char mapa[], char arquivo[], int *fase);
int descobrir_ultima_fase();

#endif