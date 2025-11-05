#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "raylib.h"

#define LARGURA 800
#define ALTURA 420

void receber_arquivo(FILE *arquivo, char **matriz) {
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
            coluna++;
        }               
    }        
    fclose(arquivo);
}




int main(){

    //Criação da matriz que será o mapa
    char **matriz;
    matriz = (char**) malloc(20*sizeof(char*));
    for (int i=0; i<20; i++){
        matriz[i] = (char*) malloc(40*sizeof(char));
    }

    //Abre o arquivo do mapa
    FILE *arquivo;
    arquivo = fopen ("mapa1.txt" , "r");
    if (!arquivo) return -1;

    //Passa pra matriz
    receber_arquivo(arquivo, matriz);

    //---------------------------------------------------------

    InitWindow(LARGURA, ALTURA, "Mapa");
    SetTargetFPS(60);

    Texture2D sprite = LoadTexture("sprite_pacman.png");

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(BLACK);

        //Insere os sprites certos
        for (int linha = 0; linha < 20; linha++){
            for (int coluna = 0; coluna < 40; coluna++){
                switch (matriz[linha][coluna])
                {
                case '#': //Parede
                    DrawRectangle(coluna*20,linha*20, 20, 20, BLUE);
                    break;
                case '.': //Pellet
                    DrawCircle(coluna*20+10,linha*20+10,1, WHITE);
                    break;   
                case 'P': //Pacman
                    DrawTexture(sprite,coluna*20,linha*20, YELLOW);
                    break;    
                case 'T': //Portal
                    DrawRectangle(coluna*20,linha*20, 20, 20, PURPLE);
                    break;
                case 'F': //Fantasma
                    DrawCircle(coluna*20+10,linha*20+10,10, RED);
                }
            }
        }
        EndDrawing();
    }
    UnloadTexture(sprite);
    CloseWindow();

    return 0;
}