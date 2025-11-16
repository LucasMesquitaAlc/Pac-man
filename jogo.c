#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "raylib.h"
#include "structs.h"
#include "mapa.h" 

#define LARGURA 800
#define ALTURA 420
#define LinhaMatriz 20
#define ColunaMatriz 40



void iniciar_tela(){
    InitWindow(LARGURA, ALTURA, "PAC-MAN by BacharelBCT");
    SetTargetFPS(60);
    Image logo = LoadImage("sprites\\pacman_original.png");
    SetWindowIcon(logo);
}

void adicionar_pos(int *tamanho_lista, int **lista ,int valor){
    *tamanho_lista += 1;
    *lista = realloc(*lista, (*tamanho_lista) * sizeof(int));
    (*lista)[*tamanho_lista-1] = valor;
}

void nao_voltar(int *tamanho_lista, int **lista, int Direcao){
    int tam_nova_lista = *tamanho_lista;

    for (int i = 0; i<*tamanho_lista; i++){
        if ((*lista)[i] == Direcao){
            tam_nova_lista--;
            (*lista)[i] = -1;     
        }
    }
    int nova_lista[tam_nova_lista];
    int indice = 0;
    for (int i = 0; i<*tamanho_lista; i++){
        if ((*lista)[i] != -1){
            nova_lista[indice] = (*lista)[i];
            indice++;
        }
    }
    *lista = realloc(*lista, tam_nova_lista * sizeof(int));
    for (int i = 0; i<tam_nova_lista; i++) (*lista)[i] = nova_lista[i];
    *(tamanho_lista) = tam_nova_lista;
}


int main() {
    // Inicia o jogo e põe FPS padrão
    iniciar_tela();
    Texture2D sprite = LoadTexture("sprites\\sprite_pacman.png");

    float frame_movimento = 0.0f;

    int posicoes[2];
    char **matriz = ler_arquivo("mapa1.txt", posicoes);
    if (!matriz) return -1;

    // Cria a struct do pacman, põe suas posições e seu sprite
    personagem pacman;
    pacman.posicao_x = posicoes[1];
    pacman.posicao_y = posicoes[0];

    
    Rectangle img = {0, 0, sprite.width, sprite.height};
    Rectangle dest = {pacman.posicao_x*20 + 10, pacman.posicao_y*20 + 10, 20, 20}; //Pode estar duplicado, mas funciona
    Vector2 centro = {dest.width/2, dest.height/2};
    
    // Struct dos fantasmas + tudo que puder envolvê-los:
    inimigo fantasmas[4]; // talvez seja melhor por esse vetor alocado dinamicamente

    Texture2D sprite_fantasma_v = LoadTexture("sprites\\f_vermelho.png");
    Texture2D sprite_fantasma_r = LoadTexture("sprites\\f_rosa.png");
    Texture2D sprite_fantasma_l = LoadTexture("sprites\\f_laranja.png");
    Texture2D sprite_fantasma_c = LoadTexture("sprites\\f_ciano.png");
    Texture2D sprites_fantasmas[4] = {sprite_fantasma_v, sprite_fantasma_r, sprite_fantasma_l, sprite_fantasma_c};

    //Definicao dos fantasmas
    int i = 0;
    for (int linha = 0; linha < 20; linha++) {
        for (int coluna = 0; coluna < 40; coluna++) {
            if (matriz[linha][coluna] == 'F' && i < 4) {
                fantasmas[i].posicao_y = linha;
                fantasmas[i].posicao_x = coluna;
                fantasmas[i].sprite = sprites_fantasmas[i];
                fantasmas[i].embaixo = '.';
                fantasmas[i].estado = 0;
                fantasmas[i].tempo = 0.0f;
                fantasmas[i].ultimo_x = coluna;
                fantasmas[i].ultimo_y = linha;
                fantasmas[i].tamanho_lista = 0;
                fantasmas[i].lista_posicoes = NULL;
                i++;
            }
        }
    }
    

    // Sprites de construções
    Texture2D sprite_parede = LoadTexture("sprites\\parede_nova.png");
    Texture2D sprite_portal = LoadTexture("sprites\\portal.png");


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

        // Interações do Pac-Man com o cenário:
        if (frame_movimento >= 0.25f) {
            if(pacman.direcao == 'D' && (matriz[pacman.posicao_y][pacman.posicao_x+1] != '#')){
                img.width = -sprite.width;
                rotacao = 0;
                moveu = 1;
                matriz[pacman.posicao_y][pacman.posicao_x] = '_';
                if (matriz[pacman.posicao_y][pacman.posicao_x+1] == 'T'){ //Funcionalidade do Pac-Man com o Portal
                    pacman.posicao_x = 1;
                    continue;}
                matriz[pacman.posicao_y][pacman.posicao_x+1] = 'P';
                matriz[pacman.posicao_y][pacman.posicao_x++] = '_';
                
            }
            if(pacman.direcao == 'E' && (matriz[pacman.posicao_y][pacman.posicao_x-1] != '#')){            
                img.width = sprite.width;
                rotacao = 0;
                moveu = 1;
                matriz[pacman.posicao_y][pacman.posicao_x] = '_';
                if (matriz[pacman.posicao_y][pacman.posicao_x-1] == 'T'){
                    pacman.posicao_x = 38;
                    continue;}
                matriz[pacman.posicao_y][pacman.posicao_x-1] = 'P';
                matriz[pacman.posicao_y][pacman.posicao_x--] = '_';
                
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

        // Mecânica dos fantasmas:
        for(int f = 0; f < 4; f++){
        fantasmas[f].tempo += GetFrameTime();


        if (fantasmas[f].tempo >= 0.25f) {
            fantasmas[f].lista_posicoes = realloc(fantasmas[f].lista_posicoes, 0 * sizeof(int));
            fantasmas[f].tamanho_lista = 0;


            //Verificação de caminhos possiveis                
            if (matriz[fantasmas[f].posicao_y][fantasmas[f].posicao_x+1] != '#'){
                adicionar_pos(&fantasmas[f].tamanho_lista, &fantasmas[f].lista_posicoes, 0);
            } 
                
            if (matriz[fantasmas[f].posicao_y+1][fantasmas[f].posicao_x] != '#'){
                adicionar_pos(&fantasmas[f].tamanho_lista, &fantasmas[f].lista_posicoes, 1);
            }

            if (matriz[fantasmas[f].posicao_y][fantasmas[f].posicao_x-1] != '#') {
                adicionar_pos(&fantasmas[f].tamanho_lista, &fantasmas[f].lista_posicoes, 2);
            }

            if(matriz[fantasmas[f].posicao_y-1][fantasmas[f].posicao_x] !='#' ){
                adicionar_pos(&fantasmas[f].tamanho_lista, &fantasmas[f].lista_posicoes, 3);
            }

            // Verifica se o fantasma pode voltar      

            if (fantasmas[f].tamanho_lista > 1){
                if ((fantasmas[f].ultimo_x == fantasmas[f].posicao_x+1) && (fantasmas[f].ultimo_mov == 0)) nao_voltar(&fantasmas[f].tamanho_lista, &fantasmas[f].lista_posicoes,0);                     
                if ((fantasmas[f].ultimo_y == fantasmas[f].posicao_y+1) && (fantasmas[f].ultimo_mov == 1)) nao_voltar(&fantasmas[f].tamanho_lista, &fantasmas[f].lista_posicoes,1);
                if ((fantasmas[f].ultimo_x == fantasmas[f].posicao_x-1) && (fantasmas[f].ultimo_mov == 0)) nao_voltar(&fantasmas[f].tamanho_lista, &fantasmas[f].lista_posicoes,2);
                if ((fantasmas[f].ultimo_y == fantasmas[f].posicao_y-1) && (fantasmas[f].ultimo_mov == 1)) nao_voltar(&fantasmas[f].tamanho_lista, &fantasmas[f].lista_posicoes,3);
            }
            if (fantasmas[f].tamanho_lista == 0) continue;

            int dir_f = fantasmas[f].lista_posicoes[GetRandomValue(0,(fantasmas[f].tamanho_lista-1))];  
            
            // DIREITA
            if(dir_f == 0){
                matriz[fantasmas[f].posicao_y][fantasmas[f].posicao_x] = fantasmas[f].embaixo;
                if(matriz[fantasmas[f].posicao_y][fantasmas[f].posicao_x + 1] == 'T'){
                    fantasmas[f].embaixo = matriz[fantasmas[f].posicao_y][1];
                    fantasmas[f].posicao_x = 1;
                }
                fantasmas[f].embaixo = matriz[fantasmas[f].posicao_y][fantasmas[f].posicao_x + 1];
                matriz[fantasmas[f].posicao_y][fantasmas[f].posicao_x + 1] = 'F';               
                fantasmas[f].ultimo_x = fantasmas[f].posicao_x;
                fantasmas[f].posicao_x++;
                fantasmas[f].ultimo_mov = 0;
            }
            // BAIXO
            if(dir_f == 1){
                matriz[fantasmas[f].posicao_y][fantasmas[f].posicao_x] = fantasmas[f].embaixo;
                fantasmas[f].embaixo = matriz[fantasmas[f].posicao_y + 1][fantasmas[f].posicao_x];
                matriz[fantasmas[f].posicao_y + 1][fantasmas[f].posicao_x] = 'F';
                fantasmas[f].ultimo_y = fantasmas[f].posicao_y;
                fantasmas[f].posicao_y++;
                fantasmas[f].ultimo_mov = 1;
            }
            // ESQUERDA
            if(dir_f == 2){
                matriz[fantasmas[f].posicao_y][fantasmas[f].posicao_x] = fantasmas[f].embaixo;
                if(matriz[fantasmas[f].posicao_y][fantasmas[f].posicao_x - 1] == 'T'){
                    fantasmas[f].embaixo = matriz[fantasmas[f].posicao_y][38];
                    fantasmas[f].posicao_x = 38;            
                }
                fantasmas[f].embaixo = matriz[fantasmas[f].posicao_y][fantasmas[f].posicao_x - 1];
                matriz[fantasmas[f].posicao_y][fantasmas[f].posicao_x - 1] = 'F';
                fantasmas[f].ultimo_x = fantasmas[f].posicao_x;
                fantasmas[f].posicao_x--;
                fantasmas[f].ultimo_mov = 0;
            }
            // CIMA
            if(dir_f == 3){
                matriz[fantasmas[f].posicao_y][fantasmas[f].posicao_x] = fantasmas[f].embaixo;
                fantasmas[f].embaixo = matriz[fantasmas[f].posicao_y - 1][fantasmas[f].posicao_x];
                matriz[fantasmas[f].posicao_y - 1][fantasmas[f].posicao_x] = 'F';
                fantasmas[f].ultimo_y = fantasmas[f].posicao_y;
                fantasmas[f].posicao_y--;
                fantasmas[f].ultimo_mov = 1;             
            }
        
                
        fantasmas[f].tempo = 0.0f;}
        }
        
        BeginDrawing();
        ClearBackground(BLACK);

        // Insere os sprites certos
        for (int linha = 0; linha < 20; linha++){
            for (int coluna = 0; coluna < 40; coluna++){
                switch (matriz[linha][coluna])
                {
                case '#': // Parede
                    DrawTexture(sprite_parede,coluna*20,linha*20,WHITE);
                    break;
                case '.': // Pellet
                    DrawCircle(coluna*20+10,linha*20+10,2, WHITE);
                    break;   
                case 'P': // Pacman
                    dest.x = coluna * 20 + 10;
                    dest.y = linha * 20 + 10;
                    DrawTexturePro(sprite, img, dest, centro, rotacao, WHITE);
                    break;    
                case 'T': // Portal
                    DrawTexture(sprite_portal,coluna*20,linha*20,WHITE);
                    break;
                case 'o': // Power Pellet
                    DrawCircle(coluna*20+10,linha*20+10,5, WHITE);
                    break;
                case 'F': // Fantasma
                    for (int j = 0; j < 4; j++) {
                        if (fantasmas[j].posicao_x == coluna && fantasmas[j].posicao_y == linha) {
                            DrawTexture(fantasmas[j].sprite, coluna*20, linha*20, WHITE);
                        }
                    }
                break;
                }
            }
        }
        
        
        if (moveu) frame_movimento = 0;
        // Termina o desenho
        EndDrawing();
    }

    // Fecha a janela
    CloseWindow();

    for (int fant = 0; fant < 4; fant++) free(fantasmas[fant].lista_posicoes);
    for (int linha = 0; linha < LinhaMatriz; linha++) free(matriz[linha]);
    free(matriz);
    
    return 0;
}