#include "menu.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "raylib.h"
#include "structs.h"
#include "mapa.h"

#define LinhaMatriz 20
#define ColunaMatriz 40

void novojogo(char **matriz, personagem *pacman, inimigo *fantasmas, int num_fantasmas, const char *mapa_filename, int *pellets) {
    int num_pellets;
    pacman->pontuacao = 0;
    pacman->vida = 3;

    FILE *arq = fopen(mapa_filename, "r");
    if (!arq) {
        printf("novojogo: erro ao abrir %s\n", mapa_filename);
    } else {
        *pellets = ler_mapa(arq, matriz, pacman, fantasmas); 
        fclose(arq);
    }

    for (int y = 0; y < LinhaMatriz; y++) {
        for (int x = 0; x < ColunaMatriz; x++) {
            if (matriz[y][x] == 'P') matriz[y][x] = '_';
        }
    }

    pacman->posicao_x = pacman->x_inicial;
    pacman->posicao_y = pacman->y_inicial;
    pacman->estado_pac = 0;

    if (pacman->posicao_y >= 0 && pacman->posicao_y < LinhaMatriz && pacman->posicao_x >= 0 && pacman->posicao_x < ColunaMatriz) {
        matriz[pacman->posicao_y][pacman->posicao_x] = 'P';
    }

    for (int y = 0; y < LinhaMatriz; y++) {
        for (int x = 0; x < ColunaMatriz; x++) {
            if (matriz[y][x] == 'F') matriz[y][x] = '_';
        }
    }

    for (int f = 0; f < num_fantasmas; f++) {
        fantasmas[f].posicao_x = fantasmas[f].x_inicial;
        fantasmas[f].posicao_y = fantasmas[f].y_inicial;
        if (fantasmas[f].posicao_y >= 0 && fantasmas[f].posicao_y < LinhaMatriz && fantasmas[f].posicao_x >= 0 && fantasmas[f].posicao_x < ColunaMatriz) {
            fantasmas[f].embaixo = '.';
            matriz[fantasmas[f].posicao_y][fantasmas[f].posicao_x] = 'F';
        } else {
            fantasmas[f].embaixo = '_';
        }
        fantasmas[f].estado = 0;
        fantasmas[f].tempo = 0.0f;
        fantasmas[f].ultimo_x = fantasmas[f].posicao_x;
        fantasmas[f].ultimo_y = fantasmas[f].posicao_y;
        fantasmas[f].tamanho_lista = 0;
    }
}

void salvarjogo(char **matriz, personagem *pacman, inimigo *fantasmas, int num_fantasmas, const char *filename, int *pellets) {
    FILE *f = fopen(filename, "wb");
    if (!f) {
        printf("salvarjogo: erro ao abrir %s para escrita\n", filename);
        return;
    }

    fwrite(pacman, sizeof(personagem), 1, f);
    fwrite(&pacman->pontuacao, sizeof(int), 1, f);
    fwrite(&pacman->vida, sizeof(int), 1, f);

    for (int i = 0; i < LinhaMatriz; i++) {
        fwrite(matriz[i], sizeof(char), ColunaMatriz, f);
    }

    fwrite(&num_fantasmas, sizeof(int), 1, f);

    for (int fidx = 0; fidx < num_fantasmas; fidx++) {
        fwrite(&fantasmas[fidx].posicao_x, sizeof(int), 1, f);
        fwrite(&fantasmas[fidx].posicao_y, sizeof(int), 1, f);
        fwrite(&fantasmas[fidx].estado, sizeof(int), 1, f);
        fwrite(&fantasmas[fidx].tempo, sizeof(float), 1, f);
        fwrite(&fantasmas[fidx].embaixo, sizeof(char), 1, f);
        fwrite(&fantasmas[fidx].ultimo_x, sizeof(int), 1, f);
        fwrite(&fantasmas[fidx].ultimo_y, sizeof(int), 1, f);
        fwrite(&fantasmas[fidx].tamanho_lista, sizeof(int), 1, f);
        fwrite(&fantasmas[fidx].ultimo_mov, sizeof(int), 1, f);
        fwrite(&fantasmas[fidx].id, sizeof(int), 1, f);
        if (fantasmas[fidx].tamanho_lista > 0 && fantasmas[fidx].lista_posicoes != NULL) {
            for (int k = 0; k < fantasmas[fidx].tamanho_lista; k++)
                fwrite(&fantasmas[fidx].lista_posicoes[k], sizeof(int), 1, f);
        }
    }
    fwrite(pellets,sizeof(int),1,f);

    fclose(f);
    printf("Jogo salvo em %s\n", filename);
}

void carregarjogo(
    char **matriz, personagem *pacman_ptr, inimigo *fantasmas, int num_fantasmas, const char *filename, int *pellets) {
    FILE *f = fopen(filename, "rb");
    if (!f) {
        printf("carregarjogo: nenhum save encontrado em %s\n", filename);
        return;
    }

    fread(pacman_ptr, sizeof(personagem), 1, f);
    fread(&pacman_ptr->pontuacao, sizeof(int), 1, f);
    fread(&pacman_ptr->vida, sizeof(int), 1, f);

    for (int i = 0; i < LinhaMatriz; i++) {
        fread(matriz[i], sizeof(char), ColunaMatriz, f);
    }

    int saved_num_fantasmas = 0;
    fread(&saved_num_fantasmas, sizeof(int), 1, f);
    int to_read = (saved_num_fantasmas < num_fantasmas) ? saved_num_fantasmas : num_fantasmas;

    for (int fidx = 0; fidx < to_read; fidx++) {
        fread(&fantasmas[fidx].posicao_x, sizeof(int), 1, f);
        fread(&fantasmas[fidx].posicao_y, sizeof(int), 1, f);
        fread(&fantasmas[fidx].estado, sizeof(int), 1, f);
        fread(&fantasmas[fidx].tempo, sizeof(float), 1, f);
        fread(&fantasmas[fidx].embaixo, sizeof(char), 1, f);
        fread(&fantasmas[fidx].ultimo_x, sizeof(int), 1, f);
        fread(&fantasmas[fidx].ultimo_y, sizeof(int), 1, f);
        fread(&fantasmas[fidx].tamanho_lista, sizeof(int), 1, f);
        fread(&fantasmas[fidx].ultimo_mov, sizeof(int), 1, f);
        fread(&fantasmas[fidx].id, sizeof(int), 1, f);

        if (fantasmas[fidx].lista_posicoes) {
            free(fantasmas[fidx].lista_posicoes);
            fantasmas[fidx].lista_posicoes = NULL;
        }
        if (fantasmas[fidx].tamanho_lista > 0) {
            fantasmas[fidx].lista_posicoes = (int*)malloc(sizeof(int) * fantasmas[fidx].tamanho_lista);
            for (int k = 0; k < fantasmas[fidx].tamanho_lista; k++)
                fread(&fantasmas[fidx].lista_posicoes[k], sizeof(int), 1, f);
        } else {
            fantasmas[fidx].lista_posicoes = NULL;
        }

        if (fantasmas[fidx].posicao_y >= 0 && fantasmas[fidx].posicao_y < LinhaMatriz &&
            fantasmas[fidx].posicao_x >= 0 && fantasmas[fidx].posicao_x < ColunaMatriz) {
            matriz[fantasmas[fidx].posicao_y][fantasmas[fidx].posicao_x] = 'F';
        }
    }

    if (saved_num_fantasmas > num_fantasmas) {
        for (int fidx = num_fantasmas; fidx < saved_num_fantasmas; fidx++) {
            int tmpi; float tmpf; char tmpc;
            fread(&tmpi, sizeof(int), 1, f); 
            fread(&tmpi, sizeof(int), 1, f); 
            fread(&tmpi, sizeof(int), 1, f); 
            fread(&tmpf, sizeof(float), 1, f); 
            fread(&tmpc, sizeof(char), 1, f); 
            fread(&tmpi, sizeof(int), 1, f);
            fread(&tmpi, sizeof(int), 1, f); 
            fread(&tmpi, sizeof(int), 1, f); 
            fread(&tmpi, sizeof(int), 1, f); 
        }
    }

    fread(pellets, sizeof(int), 1, f);
    fclose(f);
    printf("Jogo carregado de %s\n", filename);
}

void sairjogo() {
    CloseWindow();
}

void menu(
    TELA *tela_ptr,
    char **matriz,
    personagem *pacman,
    inimigo *fantasmas,
    int num_fantasmas,
    const char *mapa_filename,
    int *pellets
) {
    if (*tela_ptr == jogo && IsKeyPressed(KEY_TAB)) {
        *tela_ptr = pausa;
        return;
    }

    if (*tela_ptr != pausa) return;

    if (IsKeyPressed(KEY_V) || IsKeyPressed(KEY_TAB)) {
        *tela_ptr = jogo;
        return;
    }

    if (IsKeyPressed(KEY_N)) {
        novojogo(matriz, pacman, fantasmas, num_fantasmas, mapa_filename, pellets);
        *pellets += 4;
        *tela_ptr = jogo;
        return;
    }

    if (IsKeyPressed(KEY_C)) {
        carregarjogo(matriz, pacman, fantasmas, num_fantasmas, "save.bin", pellets);
        *tela_ptr = jogo;
        return;
    }

    if (IsKeyPressed(KEY_S)) {
        salvarjogo(matriz, pacman, fantasmas, num_fantasmas, "save.bin", pellets);
        return;
    }

    if (IsKeyPressed(KEY_Q)) {
        sairjogo();
        return;
    }
}

void mostrar_gameover(TELA *tela_ptr, int *pontuacao, int *vidas_ptr){
    if (IsKeyPressed(KEY_TAB)) {
        *tela_ptr = pausa;
    }
    if (IsKeyPressed(KEY_Q)) {
        CloseWindow();
    }
}

void mostrar_vitoria(TELA *tela_ptr, int *pontuacao, int *vidas_ptr){
    if (IsKeyPressed(KEY_TAB)) {
        *tela_ptr = pausa;
    }
}