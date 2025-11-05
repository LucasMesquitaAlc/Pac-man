#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "raylib.h"

#define LARGURA 800
#define ALTURA 420
#define LinhaMatriz 20
#define ColunaMatriz 40

typedef struct 
{
    int posicao_x;
    int posicao_y;
    char direcao;
} personagem;


void receber_arquivo(FILE *arquivo, char **matriz, int *posicao_pacman) {
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
                posicao_pacman[0] =linha;
                posicao_pacman[1] = coluna;  
        }
        coluna++;               
    }        
 

}}


int main() {
    //Inicia o jogo e põe FPS padrão
    InitWindow(LARGURA, ALTURA, "Jogo");
    SetTargetFPS(60);


    float frame_movimento = 0.0f;

    //Cria a matriz
    char **matriz;
    matriz = (char**) malloc(20*sizeof(char*));
    for (int i=0; i<20; i++){
        matriz[i] = (char*) malloc(40*sizeof(char));
    }

    //Pega o mapa
    FILE *arquivo;
    arquivo = fopen ("mapa1.txt" , "r");
    if (!arquivo) return -1;

    //Passa pra matriz
    int posicoes[2];
    receber_arquivo(arquivo, matriz,posicoes);
    fclose(arquivo);

    //Cria a struct do pacman, põe suas posições e seu sprite
    personagem pacman;

    pacman.posicao_x = posicoes[1];
    pacman.posicao_y = posicoes[0];

    //Carrega os sprites do pacman
    Texture2D sprite = LoadTexture("sprite_pacman.png");
    Rectangle img = {0, 0, sprite.width, sprite.height};
    Rectangle dest = {pacman.posicao_x*20 + 10, pacman.posicao_y*20 + 10, 20, 20};
    Vector2 centro = {dest.width/2, dest.height/2};
    float rotacao = 0;


    while (!WindowShouldClose())
    {

        int moveu = 0;
        float tempo = GetFrameTime();
        frame_movimento += tempo;

        if (IsKeyPressed(KEY_RIGHT)) pacman.direcao = 'D';
        if (IsKeyPressed(KEY_LEFT)) pacman.direcao = 'E';
        if (IsKeyPressed(KEY_UP)) pacman.direcao = 'C';
        if (IsKeyPressed(KEY_DOWN)) pacman.direcao = 'B';

        if (frame_movimento >= 0.1f) {
            if(pacman.direcao == 'D' && (matriz[pacman.posicao_y][pacman.posicao_x+1] != '#')){
                img.width = -sprite.width;
                rotacao = 0;
                matriz[pacman.posicao_y][pacman.posicao_x+1] = 'P';
                matriz[pacman.posicao_y][pacman.posicao_x++] = '_';
                moveu = 1;
            }
            if(pacman.direcao == 'E' && (matriz[pacman.posicao_y][pacman.posicao_x-1] != '#')){
                img.width = sprite.width;
                rotacao = 0;
                matriz[pacman.posicao_y][pacman.posicao_x-1] = 'P';
                matriz[pacman.posicao_y][pacman.posicao_x--] = '_';
                moveu = 1;
            }
            if(pacman.direcao == 'C' && (matriz[pacman.posicao_y-1][pacman.posicao_x] != '#')){
                img.width = sprite.width;
                rotacao = 90;
                matriz[pacman.posicao_y-1][pacman.posicao_x] = 'P';
                matriz[pacman.posicao_y--][pacman.posicao_x] = '_';
                moveu = 1;
            }
            if(pacman.direcao == 'B' && (matriz[pacman.posicao_y+1][pacman.posicao_x] != '#')){
                img.width = sprite.width;                
                rotacao = 270;
                matriz[pacman.posicao_y+1][pacman.posicao_x] = 'P';
                matriz[pacman.posicao_y++][pacman.posicao_x] = '_';
                moveu = 1;
            }
        }
        

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
                    dest.x = coluna * 20 + 10;
                    dest.y = linha * 20 + 10;
                    DrawTexturePro(sprite, img, dest, centro, rotacao, WHITE);
                    break;    
                case 'T': //Portal
                    DrawRectangle(coluna*20,linha*20, 20, 20, PURPLE);
                    break;
                case 'F': //Fantasma
                    DrawCircle(coluna*20+10,linha*20+10,10, RED);
                }
            }
        }
        
        
        if (moveu) frame_movimento = 0;
        // Termina o desenho
        EndDrawing();
    }

    // Fecha a janela
    CloseWindow();

    return 0;
}
