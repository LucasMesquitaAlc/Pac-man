#ifndef MAPA_H
#define MAPA_H

#define LinhaMatriz 20
#define ColunaMatriz 40

#include <stdio.h>
#include <stdlib.h>

char **criar_matriz();
void liberar_mapa(char **matriz);
char **ler_arquivo(const char *nome, int *posicao_pacman);
void ler_mapa(FILE *arquivo, char **matriz, int *posicao_pacman);

#endif