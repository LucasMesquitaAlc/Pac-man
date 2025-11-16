#ifndef STRUCTS_H
#define STRUCTS_H

#include <stdio.h>
#include "raylib.h"

typedef struct 
{
    int posicao_x;
    int posicao_y;
    char direcao;
} personagem;

typedef struct
{
    int posicao_x;
    int posicao_y;
    int estado; //0 - Invulnerável, 1 - Vulnerável
    Texture2D sprite;
    float tempo;
    char embaixo;
    int ultimo_x;
    int ultimo_y;
    int *lista_posicoes;
    int tamanho_lista;
    int ultimo_mov; // 0 - X; 1 - Y
} inimigo;

#endif