#include <stdio.h>
#include <stdlib.h>
#include "mapa.h"
#include "raylib.h"
// #include "pacman.h"
// #include "fantasmas.h"


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

char **ler_arquivo(const char *nome, int *posicao_pacman, int *pellets){
    // Pega o mapa
    FILE *arquivo = fopen(nome, "r");
    if (!arquivo) return NULL;

    // Passa pra matriz
    char **matriz = criar_matriz();
    *pellets = ler_mapa(arquivo, matriz, posicao_pacman);
    fclose(arquivo);
    return matriz;
}

int ler_mapa(FILE *arquivo, char **matriz, int *posicao_pacman) {
    char caractere;
    int linha = 0;
    int coluna = 0;
    int num_pellets;
    while (fread(&caractere, sizeof(char), 1, arquivo) == 1){

        if (caractere == '\n'){
            linha++;
            coluna = 0;
        }
        else{
            matriz[linha][coluna] = caractere;
            if (caractere == 'P'){
                posicao_pacman[0] = linha;
                posicao_pacman[1] = coluna;  
            }
            if (caractere == '.' || caractere == 'o' ){
                num_pellets += 1;
            }
        coluna++;               
        }         
    }

    return num_pellets;
}
