#ifndef STRUCTS_H
#define STRUCTS_H

#include <stdio.h>
#include "raylib.h"

typedef struct 
{
    int posicao_x;
    int posicao_y;
    char direcao;
    char dir_buffer;
    int estado_pac;
    float tempo_invu;
    int pontuacao;
    int vida;
    Texture2D sprite;
    Rectangle img;
    float rotacao;
    int x_inicial;
    int y_inicial;
} personagem;

typedef struct
{
    int posicao_x;
    int posicao_y;
    int estado; //0 - Invulnerável, 1 - Vulnerável
    Texture2D sprite; // Sprite do fantasma
    float tempo; // Tempo para a ação dos fantasmas
    char embaixo; // Verifica o que tem embaixo do fantasma
    int ultimo_x;
    int ultimo_y;
    int *lista_posicoes; // Posições válidas para o fantasma andar
    int tamanho_lista; // Quantidade de posições válidas
    int ultimo_mov; // 0 - X; 1 - Y ; Diz se o último movimento foi para cima ou para baixo
    int x_inicial;
    int y_inicial;
    int id; // 0 - vermelho, 1 - rosa, 2 - laranja, 3 - azul
    int num_fantasmas;
} inimigo;

typedef struct{
    int melhor_direcao;
    int menor_distancia;
    int pior_direcao;
    int maior_distancia;
} dist_manha;

#endif