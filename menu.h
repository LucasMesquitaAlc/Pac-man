#ifndef MENU_H
#define MENU_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "raylib.h"
#include "structs.h"
#include "mapa.h"

typedef enum TELA {
    jogo = 0,
    pausa = 1,
    gameover = 2,
    vitoria = 3,
    venceu_final = 4,
    tela_inicial = 5,
    sair = 6
} TELA;

void menu(
    TELA *tela_ptr,
    char **matriz,
    personagem *pacman,
    inimigo *fantasmas_dinamico,
    int *num_fantasmas,
    char *mapa_filename,
    int *pellets,
    Sound musica[], 
    Texture2D sprites[]
); // Menu

// Funções auxiliares exportadas pelo menu
void novojogo(
    char **matriz,
    personagem *pacman,
    inimigo *fantasmas,
    int *num_fantasmas,
    char *mapa_filename,
    int *num_pellets,
    int pontuacao,
    int vida, 
    Texture2D sprites[],
    Sound musica[]
); // Inicia novo jogo

void salvarjogo(
    char **matriz,
    personagem *pacman,
    inimigo *fantasmas,
    int *num_fantasmas,
    char *filename,
    int *num_pellets
); // Salva o jogo em arquivo binário

void carregarjogo(
    char **matriz,
    personagem *pacman_ptr,
    inimigo *fantasmas,
    int *num_fantasmas,
    char *filename,
    int *num_pellets
); // Carrega o jogo salvo no arquivo

void sairjogo(TELA *tela_ptr); // Fecha a janela

void mostrar_gameover(TELA *tela_ptr, int *pontuacao, int *vidas_ptr); // Tela de game over

void mostrar_vitoria(TELA *tela_ptr, 
    int *pontuacao,
    int *vidas_ptr,
    char **matriz,
    personagem *pacman,
    inimigo *fantasmas,
    int *num_fantasmas,
    int *pellets); // Tela de vitória mostrada a cada avanço de fase

#endif