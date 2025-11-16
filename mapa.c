#include <stdio.h>
#include <stdlib.h>
#include "mapa.h"
#include "raylib.h"
#include "pacman.h"
#include "fantasmas.h"


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

char **ler_arquivo(const char *nome, int *posicao_pacman){
    // Pega o mapa
    FILE *arquivo = fopen(nome, "r");
    if (!arquivo) return NULL;

    // Passa pra matriz
    char **matriz = criar_matriz();
    ler_mapa(arquivo, matriz, posicao_pacman);
    fclose(arquivo);
    return matriz;
}

// void desenhar_mapa(char **matriz){
//     // NÃO FUNCIONA AINDA
//     for (int linha = 0; linha < 20; linha++){
//             for (int coluna = 0; coluna < 40; coluna++){
//                 switch (matriz[linha][coluna])
//                 {
//                 case '#': // Parede
//                     DrawTexture(sprite_parede,coluna*20,linha*20,WHITE);
//                     break;
//                 case '.': // Pellet
//                     DrawCircle(coluna*20+10,linha*20+10,2, WHITE);
//                     break;   
//                 case 'P': // Pacman
//                     dest.x = coluna * 20 + 10;
//                     dest.y = linha * 20 + 10;
//                     DrawTexturePro(sprite, img, dest, centro, rotacao, WHITE);
//                     break;    
//                 case 'T': // Portal
//                     DrawTexture(sprite_portal,coluna*20,linha*20,WHITE);
//                     break;
//                 case 'o': // Power Pellet
//                     DrawCircle(coluna*20+10,linha*20+10,5, WHITE);
//                     break;
//                 case 'F': // Fantasma
//                     for (int j = 0; j < 4; j++) {
//                         if (fantasmas[j].posicao_x == coluna && fantasmas[j].posicao_y == linha) {
//                             DrawTexture(fantasmas[j].sprite, coluna*20, linha*20, WHITE);
//                         }
//                     }
//                 break;
//                 }
//             }
//         }
// }

void ler_mapa(FILE *arquivo, char **matriz, int *posicao_pacman) {
    char caractere;
    int linha = 0;
    int coluna = 0;
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
            // if (caractere == 'F'){

            // }
        coluna++;               
        }         
    }
}
