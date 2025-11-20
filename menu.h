#ifndef MENU_H
#define MENU_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "raylib.h"
#include "structs.h"
#include "mapa.h"


typedef enum TELA {
    jogo = 0,         // O jogo está rodando
    pausa = 1    // O menu está aberto (pausado)
} TELA;

void menu(TELA *tela_ptr, char **matriz, personagem *pacman_ptr, int *pontuacao_ptr, int *vidas_ptr);

void novojogo(char **matriz, personagem *pacman_ptr, int *pontuacao_ptr, int *vidas_ptr);
void carregarjogo(char **matriz, personagem *pacman_ptr, int *pontuacao_ptr, int *vidas_ptr);
void salvarjogo(char **matriz, personagem pacman, int pontuacao, int vidas);
void sairjogo();

#endif