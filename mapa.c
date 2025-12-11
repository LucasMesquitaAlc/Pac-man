#include <stdio.h>
#include <stdlib.h>
#include "mapa.h"
#include "raylib.h"
#include "structs.h"


char **criar_matriz(){
    char **matriz = (char**) malloc(LinhaMatriz*sizeof(char*));
    for (int i=0; i<20; i++){
        matriz[i] = (char*) malloc(ColunaMatriz*sizeof(char));
    }
    return matriz;
}

void liberar_mapa(char **matriz){
     for (int i = 0; i < LinhaMatriz; i++)
        free(matriz[i]);
    free(matriz);
}

char **ler_arquivo(const char *nome, int *pellets, personagem *pacman, inimigo *fantasmas, int *num_fantasmas){
    // Pega o mapa
    FILE *arquivo = fopen(nome, "r");
    if (!arquivo) return NULL;

    // Passa pra matriz
    char **matriz = criar_matriz();
    *pellets = ler_mapa(arquivo, matriz, pacman, fantasmas, num_fantasmas);
    fclose(arquivo);
    return matriz;
}

int ler_mapa(FILE *arquivo, char **matriz, personagem *pacman, inimigo *fantasmas, int *num_fantasmas) {
    char caractere;
    int linha = 0;
    int coluna = 0;
    int num_pellets =0;
    int f = 0;
    while (fread(&caractere, sizeof(char), 1, arquivo) == 1){

        if (caractere == '\n'){
            linha++;
            coluna = 0;
        }
        else{
            matriz[linha][coluna] = caractere;
            if (caractere == 'P'){
                pacman->posicao_x = coluna;
                pacman->posicao_y = linha;
                pacman->x_inicial = coluna;
                pacman->y_inicial = linha; 
            }
            if (caractere == '.' || caractere == 'o' ){
                num_pellets += 1;
            }
            if (caractere == 'F'){
                fantasmas[f].posicao_y = linha;
                fantasmas[f].posicao_x = coluna;
                fantasmas[f].x_inicial = coluna;
                fantasmas[f].y_inicial = linha;
                fantasmas[f].ultimo_x = coluna;
                fantasmas[f].ultimo_y = linha;
                fantasmas[f].embaixo = '.';
                fantasmas[f].estado = 0;
                fantasmas[f].tempo = 0.0f;
                fantasmas[f].tamanho_lista = 0;
                fantasmas[f].lista_posicoes = NULL;
                fantasmas[f].id = f;
                *num_fantasmas += 1;
                f++;
            }
        coluna++;               
        }         
    }
    return num_pellets;
}

void passar_mapa(char mapa[], char arquivo[], int *fase){
    char nome_arquivo[25];
    *fase+=1;

    sprintf(mapa, "mapa%d.txt", *fase);
    sprintf(arquivo, "mapas\\%s",mapa);
}

int descobrir_ultima_fase(){
    char mapa[15] = "mapa1.txt";
    char nome_arquivo[25];

    for (int x = 1; x<=20; x++){ //Assume que as fases só vão só até 20
        sprintf(&mapa[4], "%d.txt", x);
        sprintf(nome_arquivo, "mapas\\%s",mapa);

        FILE *f = fopen(nome_arquivo,"r");
        if (!f) return x-1;
    }
}