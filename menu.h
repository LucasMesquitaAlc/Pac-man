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
    nivel = 3,
    vitoria = 4,
    venceu_final = 5,
    tela_inicial = 6
} TELA;

/*
  Nota: nova assinatura do menu (sem variaveis globais).
  - matriz: matriz atual do mapa (20x40) (é sobrescrita por novojogo/carregar)
  - pacman: ponteiro para personagem
  - pontuacao, vidas_ptr: ponteiros
  - fantasmas: vetor de inimigo (tamanho num_fantasmas)
  - num_fantasmas: numero de fantasmas (ex: 4)
  - inicial_pacman_x/y: posições iniciais guardadas na main
  - inicial_fantasma_x/y: arrays com posições iniciais (main)
  - mapa_filename: nome do arquivo do mapa (ex: "mapa1.txt")
*/
void menu(
    TELA *tela_ptr,
    char **matriz,
    personagem *pacman,
    inimigo *fantasmas_dinamico,
    int *num_fantasmas,
    char *mapa_filename,
    int *pellets,
    Sound musica
);

// Funções auxiliares exportadas pelo menu
void novojogo(
    char **matriz,
    personagem *pacman,
    inimigo *fantasmas,
    int *num_fantasmas,
    char *mapa_filename,
    int *num_pellets,
    int pontuacao,
    int vida
);

void salvarjogo(
    char **matriz,
    personagem *pacman,
    inimigo *fantasmas,
    int *num_fantasmas,
    char *filename,
    int *num_pellets
);

void carregarjogo(
    char **matriz,
    personagem *pacman_ptr,
    inimigo *fantasmas,
    int *num_fantasmas,
    char *filename,
    int *num_pellets
);

void sairjogo();

void mostrar_gameover(TELA *tela_ptr, int *pontuacao, int *vidas_ptr);

void mostrar_vitoria(TELA *tela_ptr, int *pontuacao, int *vidas_ptr);

#endif