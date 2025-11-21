#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "raylib.h"
#include "structs.h"
#include "mapa.h"
#include "menu.h"
#include <math.h>

#define LARGURA 1600
#define ALTURA 840
#define LinhaMatriz 20
#define ColunaMatriz 40

void iniciar_tela(){
    InitWindow(LARGURA, ALTURA, "PAC-MAN by BacharelBCT");
    SetTargetFPS(60);
    Image logo = LoadImage("sprites\\pacman_original.png");
    SetWindowIcon(logo);
}

void adicionar_pos(int *tamanho_lista, int **lista, int valor){
    *tamanho_lista += 1;
    *lista = realloc(*lista, (*tamanho_lista) * sizeof(int));
    (*lista)[*tamanho_lista - 1] = valor;
}

void nao_voltar(int *tamanho_lista, int **lista, int Direcao){
    int tam_nova_lista = *tamanho_lista;

    for (int i = 0; i < *tamanho_lista; i++){
        if ((*lista)[i] == Direcao){
            tam_nova_lista--;
            (*lista)[i] = -1;
        }
    }
    int nova_lista[tam_nova_lista];
    int indice = 0;
    for (int i = 0; i < *tamanho_lista; i++){
        if ((*lista)[i] != -1){
            nova_lista[indice] = (*lista)[i];
            indice++;
        }
    }
    *lista = realloc(*lista, tam_nova_lista * sizeof(int));
    for (int i = 0; i < tam_nova_lista; i++)
        (*lista)[i] = nova_lista[i];
    *(tamanho_lista) = tam_nova_lista;
}

void interacao(char **matriz, int linha, int coluna, personagem *p, int *pellets){
    int pontos_ganhos = 0;
    switch (matriz[linha][coluna]){
    case '.': // Pellet
        pontos_ganhos = 10;
        *pellets -= 1;
        break;
    case 'o': // Power Pellet
        p->tempo_invu = 0;
        pontos_ganhos = 50;
        p->estado_pac = 1;
        *pellets -= 1;
        break;
    }
    p->pontuacao += pontos_ganhos;
}

// função para resetar a posição
void reset_posicoes(char **matriz, personagem *pacman, inimigo *fantasmas, int num_fantasmas, int *inicial_fantasma_y, int *inicial_fantasma_x){
    
    matriz[pacman->posicao_y][pacman->posicao_x] = '_';

    // volta o pacman pra posição inicial
    pacman->posicao_x = pacman->x_inicial;
    pacman->posicao_y = pacman->y_inicial;
    matriz[pacman->posicao_y][pacman->posicao_x] = 'P';

    // Limpa qualquer fantasma no mapa
    for (int y = 0; y < 20; y++){
        for (int x = 0; x < 40; x++)
        {
            if (matriz[y][x] == 'F')
                matriz[y][x] = '_';
        }
    }

    for (int f = 0; f < num_fantasmas; f++){
        fantasmas[f].posicao_x = inicial_fantasma_x[f];
        fantasmas[f].posicao_y = inicial_fantasma_y[f];

        // define corretamente o que tem embaixo
        fantasmas[f].embaixo = matriz[fantasmas[f].posicao_y][fantasmas[f].posicao_x];

        // reposiciona visivelmente
        matriz[fantasmas[f].posicao_y][fantasmas[f].posicao_x] = 'F';
        fantasmas[f].estado = 0;
    }
}

// identificar a colisão
void verificar_colisao(char **matriz, personagem *pacman, inimigo *fantasmas, TELA *tela_ptr, int num_fantasmas, int *inicial_fantasma_y, int *inicial_fantasma_x){
    if (*tela_ptr == gameover) return;

    for (int f = 0; f < num_fantasmas; f++){
        if (pacman->posicao_x == fantasmas[f].posicao_x && pacman->posicao_y == fantasmas[f].posicao_y){

            // colisão com fantasma não vuneravel
            if (fantasmas[f].estado == 0){
                pacman->vida--;
                if (pacman->vida < 0) pacman->vida = 0;
                pacman->pontuacao -= 200;
                if (pacman->pontuacao < 0){
                    pacman->pontuacao = 0;
                }
                if (pacman->vida > 0){
                    reset_posicoes(matriz, pacman, fantasmas, num_fantasmas, inicial_fantasma_y, inicial_fantasma_x);
                }
                else{
                    //Puxar a tela de gameover (pois a vida vai estar zerada)
                    *tela_ptr = gameover;
                }
                return;
            }

            // colisão com o fantasma vuneravel
            else if (fantasmas[f].estado == 1){
                int pontos_fantasma = 200;
                pacman->pontuacao += pontos_fantasma;
                matriz[fantasmas[f].posicao_y][fantasmas[f].posicao_x] = fantasmas[f].embaixo;

                fantasmas[f].posicao_y = -1; // Posição fora do mapa
                fantasmas[f].posicao_x = -1; // Posição fora do mapa
                fantasmas[f].estado = 0;
            }
        }
    }
}

int verifica_mov_pac(char direc, char **matriz, personagem *p){
    //Verifica se o pacman pode andar em certa direção
    switch (direc){
        case 'D':
            return (matriz[p->posicao_y][p->posicao_x+1] != '#'); 
        case 'E':
            return (matriz[p->posicao_y][p->posicao_x-1] != '#'); 
        case 'C':
            return (matriz[p->posicao_y-1][p->posicao_x] != '#');
        case 'B':
            return (matriz[p->posicao_y+1][p->posicao_x] != '#'); 
    }
}

int verifica_mov_fant(char direc, char **matriz, inimigo f){
    //Verifica se o fantasma f pode andar em certa direção
    switch (direc){
    case 'D':
        return ((matriz[f.posicao_y][f.posicao_x+1] != '#') && (matriz[f.posicao_y][f.posicao_x+1] != 'F'));
    case 'E':
        return ((matriz[f.posicao_y][f.posicao_x-1] != '#') && (matriz[f.posicao_y][f.posicao_x-1] != 'F'));
    case 'C':
        return ((matriz[f.posicao_y-1][f.posicao_x] != '#') && (matriz[f.posicao_y-1][f.posicao_x] != 'F'));
    case 'B':
        return ((matriz[f.posicao_y+1][f.posicao_x] != '#') && (matriz[f.posicao_y+1][f.posicao_x] != 'F'));
    }
}

dist_manha dist_manhattan(inimigo f, personagem p){
    // Calcula a menor distância de manhattan do fantasma até o pacman
    int melhor_distancia = -1.0;
    int melhor_direcao;

    dist_manha resultado;

    for (int i = 0; i < f.tamanho_lista; i++){

        int direcao = f.lista_posicoes[i];
        float distancia;

        if (direcao == 0){
        distancia = abs(p.posicao_x - (f.posicao_x+1)) + abs(p.posicao_y - f.posicao_y);
        }
        else if (direcao == 1){
        distancia = abs(p.posicao_x - f.posicao_x) + abs(p.posicao_y - (f.posicao_y+1));
        }
        else if (direcao == 2){
        distancia = abs(p.posicao_x - (f.posicao_x-1)) + abs(p.posicao_y - f.posicao_y);
        }
        else if (direcao == 3){
        distancia = abs(p.posicao_x - f.posicao_x) + abs(p.posicao_y - (f.posicao_y-1));
        }

        if (p.estado_pac == 0){
            if (melhor_distancia == -1 || melhor_distancia > distancia){
                melhor_distancia = distancia;
                melhor_direcao = direcao;
            }
        }
        else{
            if (melhor_distancia == -1 || melhor_distancia < distancia){
                melhor_distancia = distancia;
                melhor_direcao = direcao;
            }
        }
    }
    resultado.melhor_direcao = melhor_direcao;
    resultado.melhor_distancia = melhor_distancia;
    return resultado;
}

int main(){
    // Inicia o jogo e põe FPS padrão
    iniciar_tela();
    int qnt_pellets;
    Texture2D sprite = LoadTexture("sprites\\sprite_pacman.png");

    TELA tela = jogo;

    float frame_movimento = 0.0f;

    int posicoes[2];
    char **matriz = ler_arquivo("mapa2.txt", posicoes, &qnt_pellets);
    if (!matriz)
        return -1;

    // Cria a struct do pacman, põe suas posições e seu sprite
    personagem pacman;
    pacman.posicao_x = posicoes[1];
    pacman.posicao_y = posicoes[0];
    pacman.x_inicial = posicoes[1];
    pacman.y_inicial = posicoes[0];
    pacman.estado_pac = 0;
    pacman.tempo_invu = 0.0;
    pacman.vida = 3;
    pacman.pontuacao = 0; 

    Texture2D sprite1 = LoadTexture("sprites\\sprite_pacman.png");
    Texture2D sprite2 = LoadTexture("sprites\\sprite2_pacman.png");
    pacman.sprite = sprite1;

    // Posição inicial do pacman
    int inicial_pacman_x = posicoes[1];
    int inicial_pacman_y = posicoes[0];

    Rectangle img = {0, 0, pacman.sprite.width, pacman.sprite.height};
    Rectangle dest = {pacman.posicao_x * 40 + 20, pacman.posicao_y * 40 + 20, 40, 40}; // Pode estar duplicado, mas funciona
    Vector2 centro = {dest.width / 2, dest.height / 2};

    // Struct dos fantasmas + tudo que puder envolvê-los:
    inimigo fantasmas[4]; // talvez seja melhor por esse vetor alocado dinamicamente
    int num_fantasmas = 4;

    Texture2D sprite_fantasma_v = LoadTexture("sprites\\f_vermelho.png");
    Texture2D sprite_fantasma_r = LoadTexture("sprites\\f_rosa.png");
    Texture2D sprite_fantasma_l = LoadTexture("sprites\\f_laranja.png");
    Texture2D sprite_fantasma_c = LoadTexture("sprites\\f_ciano.png");
    Texture2D sprite_fantasma_fraco = LoadTexture("sprites/f_fraco.png");
    Texture2D sprites_fantasmas[4] = {sprite_fantasma_v, sprite_fantasma_r, sprite_fantasma_l, sprite_fantasma_c};


    // Definicao dos fantasmas
    int inicial_fantasma_y[4];
    int inicial_fantasma_x[4];

    int i = 0;
    for (int linha = 0; linha < 20; linha++){  
        for (int coluna = 0; coluna < 40; coluna++){
            if (matriz[linha][coluna] == 'F' && i < 4){
                // verificar_colisao inicial para o reset
                inicial_fantasma_y[i] = linha;
                inicial_fantasma_x[i] = coluna;

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
                fantasmas[i].x_inicial = coluna;
                fantasmas[i].y_inicial = linha;
                i++;
            }
        }
    }

    // Sprites de construções
    Texture2D sprite_parede = LoadTexture("sprites\\parede_nova.png");
    Texture2D sprite_portal = LoadTexture("sprites\\portal.png");

    float rotacao = 0;
    float tempo_fantasmas = 0.25;

    while (!WindowShouldClose()){

        menu(&tela, matriz, &pacman, fantasmas, num_fantasmas, inicial_fantasma_x, inicial_fantasma_y, "mapa1.txt");

        switch (tela){

        case jogo:{

            int moveu = 0;
            float tempo = GetFrameTime();
            frame_movimento += tempo;
            if (pacman.estado_pac == 1) pacman.tempo_invu += tempo;

            if (IsKeyPressed(KEY_RIGHT)) pacman.dir_buffer = 'D';
            if (IsKeyPressed(KEY_LEFT)) pacman.dir_buffer = 'E';
            if (IsKeyPressed(KEY_UP)) pacman.dir_buffer = 'C';
            if (IsKeyPressed(KEY_DOWN)) pacman.dir_buffer = 'B';

            // Interações do Pac-Man com o cenário:
            if (frame_movimento >= 0.25f){

                if(pacman.dir_buffer == 'D' && verifica_mov_pac('D', matriz, &pacman)) pacman.direcao = 'D';
                if(pacman.dir_buffer == 'E' && verifica_mov_pac('E', matriz, &pacman)) pacman.direcao = 'E';
                if(pacman.dir_buffer == 'C' && verifica_mov_pac('C', matriz, &pacman)) pacman.direcao = 'C';
                if(pacman.dir_buffer == 'B' && verifica_mov_pac('B', matriz, &pacman)) pacman.direcao = 'B';

                if (pacman.direcao == 'D' && verifica_mov_pac('D', matriz, &pacman)){
                    img.width = -sprite.width;
                    rotacao = 0;
                    moveu = 1;

                    // soma a pontuação
                    interacao(matriz, pacman.posicao_y, pacman.posicao_x + 1, &pacman, &qnt_pellets);

                    matriz[pacman.posicao_y][pacman.posicao_x] = '_';
                    if (matriz[pacman.posicao_y][pacman.posicao_x + 1] == 'T'){
                        matriz[pacman.posicao_y][pacman.posicao_x] = '_';
                        pacman.posicao_x = 1;
                        matriz[pacman.posicao_y][pacman.posicao_x] = 'P';
                        continue;
                    }
                    matriz[pacman.posicao_y][pacman.posicao_x] = '_';
                    pacman.posicao_x++;
                    matriz[pacman.posicao_y][pacman.posicao_x] = 'P';
                }
                if (pacman.direcao == 'E' && verifica_mov_pac('E', matriz, &pacman)){           
                    img.width = sprite.width;
                    rotacao = 0;
                    moveu = 1;

                    // soma a pontuação
                    interacao(matriz, pacman.posicao_y, pacman.posicao_x - 1, &pacman, &qnt_pellets);

                    matriz[pacman.posicao_y][pacman.posicao_x] = '_';
                    if (matriz[pacman.posicao_y][pacman.posicao_x - 1] == 'T'){
                        matriz[pacman.posicao_y][pacman.posicao_x] = '_';
                        pacman.posicao_x = 38;
                        matriz[pacman.posicao_y][pacman.posicao_x] = 'P';
                        continue;
                    }

                    matriz[pacman.posicao_y][pacman.posicao_x] = '_';
                    pacman.posicao_x--;
                    matriz[pacman.posicao_y][pacman.posicao_x] = 'P';
                }
                if (pacman.direcao == 'C' && verifica_mov_pac('C', matriz, &pacman)){
                    img.width = sprite.width;
                    rotacao = 90;

                    // soma a pontuação
                    interacao(matriz, pacman.posicao_y - 1, pacman.posicao_x, &pacman, &qnt_pellets);
                    if (matriz[pacman.posicao_y-1][pacman.posicao_x] == 'T'){
                        matriz[pacman.posicao_y][pacman.posicao_x] = '_';
                        pacman.posicao_y = 18;
                        matriz[pacman.posicao_y][pacman.posicao_x] = 'P';
                        continue;
                    }

                    matriz[pacman.posicao_y][pacman.posicao_x] = '_';
                    pacman.posicao_y--;
                    matriz[pacman.posicao_y][pacman.posicao_x] = 'P';
                    moveu = 1;
                }
                if (pacman.direcao == 'B' && verifica_mov_pac('B', matriz, &pacman)){
                    img.width = sprite.width;
                    rotacao = 270;

                    // soma a pontuação
                    interacao(matriz, pacman.posicao_y + 1, pacman.posicao_x, &pacman, &qnt_pellets);
                    if (matriz[pacman.posicao_y+1][pacman.posicao_x] == 'T'){
                        matriz[pacman.posicao_y][pacman.posicao_x] = '_';
                        pacman.posicao_y = 1;
                        matriz[pacman.posicao_y][pacman.posicao_x] = 'P';
                        continue;
                    }
                    matriz[pacman.posicao_y][pacman.posicao_x] = '_';
                    pacman.posicao_y++;
                    matriz[pacman.posicao_y][pacman.posicao_x] = 'P';
                    moveu = 1;
                }
                if (moveu){
                    if (pacman.sprite.id == sprite1.id) pacman.sprite = sprite2;
                    else pacman.sprite = sprite1;
                }
            }

            // Mecânica dos fantasmas:
            for (int f = 0; f < 4; f++){
                fantasmas[f].tempo += GetFrameTime();

                if (fantasmas[f].tempo >= tempo_fantasmas){
                    fantasmas[f].lista_posicoes = realloc(fantasmas[f].lista_posicoes, 0 * sizeof(int));
                    fantasmas[f].tamanho_lista = 0;

                    // Verificação de caminhos possiveis
                    if (verifica_mov_fant('D', matriz, fantasmas[f])){
                        adicionar_pos(&fantasmas[f].tamanho_lista, &fantasmas[f].lista_posicoes, 0);
                    }

                    if (verifica_mov_fant('B', matriz, fantasmas[f])){
                        adicionar_pos(&fantasmas[f].tamanho_lista, &fantasmas[f].lista_posicoes, 1);
                    }

                    if (verifica_mov_fant('E', matriz, fantasmas[f])){
                        adicionar_pos(&fantasmas[f].tamanho_lista, &fantasmas[f].lista_posicoes, 2);
                    }

                    if (verifica_mov_fant('C', matriz, fantasmas[f])){
                        adicionar_pos(&fantasmas[f].tamanho_lista, &fantasmas[f].lista_posicoes, 3);
                    }

                    // Verifica se o fantasma pode voltar
                    if (fantasmas[f].tamanho_lista > 1 && pacman.estado_pac == 0){
                        if ((fantasmas[f].ultimo_x == fantasmas[f].posicao_x+1) && (fantasmas[f].ultimo_mov == 0)) nao_voltar(&fantasmas[f].tamanho_lista, &fantasmas[f].lista_posicoes,0);                     
                        if ((fantasmas[f].ultimo_y == fantasmas[f].posicao_y+1) && (fantasmas[f].ultimo_mov == 1)) nao_voltar(&fantasmas[f].tamanho_lista, &fantasmas[f].lista_posicoes,1);
                        if ((fantasmas[f].ultimo_x == fantasmas[f].posicao_x-1) && (fantasmas[f].ultimo_mov == 0)) nao_voltar(&fantasmas[f].tamanho_lista, &fantasmas[f].lista_posicoes,2);
                        if ((fantasmas[f].ultimo_y == fantasmas[f].posicao_y-1) && (fantasmas[f].ultimo_mov == 1)) nao_voltar(&fantasmas[f].tamanho_lista, &fantasmas[f].lista_posicoes,3);
                    }
                    if (fantasmas[f].tamanho_lista == 0) continue;

                    int dir_f;
                    float probabilidade;

                    if (f==0){
                        dist_manha dist_fantasma = dist_manhattan(fantasmas[f], pacman);
                        float dist_normalizada = dist_fantasma.melhor_distancia /58.0f;

                        //A probabilidade do fantasma vermelho seguir o jogador é exponencial baseado em quão próximo do jogador ele se encontra
                        probabilidade = expf(-2 * dist_normalizada) * 100;

                        // printf("%f \n", probabilidade);

                        if (GetRandomValue(0,100) < probabilidade) dir_f = dist_fantasma.melhor_direcao;
                        else dir_f = fantasmas[f].lista_posicoes[GetRandomValue(0,(fantasmas[f].tamanho_lista-1))];
                    }
                    else{
                        dir_f = fantasmas[f].lista_posicoes[GetRandomValue(0,(fantasmas[f].tamanho_lista-1))];
                    }

                    // DIREITA
                    if (dir_f == 0){
                        matriz[fantasmas[f].posicao_y][fantasmas[f].posicao_x] = fantasmas[f].embaixo;
                        if (matriz[fantasmas[f].posicao_y][fantasmas[f].posicao_x + 1] == 'T'){
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
                    if (dir_f == 1){
                        matriz[fantasmas[f].posicao_y][fantasmas[f].posicao_x] = fantasmas[f].embaixo;
                        if (matriz[fantasmas[f].posicao_y+1][fantasmas[f].posicao_x] == 'T'){
                            fantasmas[f].embaixo = matriz[fantasmas[f].posicao_y][1];
                            fantasmas[f].posicao_y = 0;
                        }
                        fantasmas[f].embaixo = matriz[fantasmas[f].posicao_y + 1][fantasmas[f].posicao_x];
                        matriz[fantasmas[f].posicao_y + 1][fantasmas[f].posicao_x] = 'F';
                        fantasmas[f].ultimo_y = fantasmas[f].posicao_y;
                        fantasmas[f].posicao_y++;
                        fantasmas[f].ultimo_mov = 1;
                    }
                    // ESQUERDA
                    if (dir_f == 2){
                        matriz[fantasmas[f].posicao_y][fantasmas[f].posicao_x] = fantasmas[f].embaixo;
                        if (matriz[fantasmas[f].posicao_y][fantasmas[f].posicao_x - 1] == 'T'){
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
                    if (dir_f == 3){
                        matriz[fantasmas[f].posicao_y][fantasmas[f].posicao_x] = fantasmas[f].embaixo;
                        if (matriz[fantasmas[f].posicao_y-1][fantasmas[f].posicao_x] == 'T'){
                            fantasmas[f].embaixo = matriz[fantasmas[f].posicao_y][1];
                            fantasmas[f].posicao_y = 19;
                        }
                        fantasmas[f].embaixo = matriz[fantasmas[f].posicao_y - 1][fantasmas[f].posicao_x];
                        matriz[fantasmas[f].posicao_y - 1][fantasmas[f].posicao_x] = 'F';
                        fantasmas[f].ultimo_y = fantasmas[f].posicao_y;
                        fantasmas[f].posicao_y--;
                        fantasmas[f].ultimo_mov = 1;
                    }
                    if (pacman.estado_pac == 1){
                        fantasmas[f].estado = 1;
                        fantasmas[f].sprite = sprite_fantasma_fraco;
                        tempo_fantasmas = 0.33;

                        if (pacman.tempo_invu >= 8.0f){
                            for (int x = 0; x < 4; x++){
                                fantasmas[x].sprite = sprites_fantasmas[x];
                                fantasmas[x].estado = 0;
                            }
                            pacman.estado_pac = 0;
                            pacman.tempo_invu = 0;
                            tempo_fantasmas = 0.25;
                        }
                    }
                    fantasmas[f].tempo = 0.0f;
                }

                verificar_colisao(matriz, &pacman, fantasmas,&tela, 4, inicial_fantasma_y, inicial_fantasma_x);
                
                if (pacman.vida <= 0) {
                    tela = gameover;
                }

                if (moveu)
                    frame_movimento = 0;
            }
        } break; //fim case jogo 
        case pausa:{

        }break; //fim case pausa 
        case gameover: {    
            mostrar_gameover(&tela, &pacman.pontuacao, &pacman.vida);
        } break;
        
        } // fim do switch

        BeginDrawing();
        ClearBackground(BLACK);

        // Desenho do score
        char txt_score[50];
        sprintf(txt_score, "SCORE: %d", pacman.pontuacao);
        DrawText(txt_score, LARGURA - 150, ALTURA - 20, 20, YELLOW);

        char pellets[20];
        sprintf(txt_score, "Pellets: %d", qnt_pellets);
        DrawText(txt_score, LARGURA - 500, ALTURA - 20, 20, YELLOW);

        // desenha as vidas
        char txt_vidas[10];
        sprintf(txt_vidas, "VIDAS: %d", pacman.vida);
        DrawText(txt_vidas, 10, ALTURA - 20, 20, WHITE);

        // Insere os sprites certos
        for (int linha = 0; linha < 20; linha++){
            for (int coluna = 0; coluna < 40; coluna++){
                switch (matriz[linha][coluna]){
                case '#': // Parede
                    DrawTexture(sprite_parede, coluna * 40, linha * 40, WHITE);
                    break;
                case '.': // Pellet
                    DrawCircle(coluna * 40 + 20, linha * 40 + 20, 4, WHITE);
                    break;
                case 'P': // Pacman
                    dest.x = coluna * 40 + 20;
                    dest.y = linha * 40 + 20;
                    DrawTexturePro(pacman.sprite, img, dest, centro, rotacao, WHITE);
                    break;
                case 'T': // Portal
                    DrawTexture(sprite_portal, coluna * 40, linha * 40, WHITE);
                    break;
                case 'o': // Power Pellet
                    DrawCircle(coluna * 40 + 20, linha * 40 + 20, 10, WHITE);
                    break;
                case 'F': // Fantasma
                    for (int j = 0; j < 4; j++){
                        if (fantasmas[j].posicao_x == coluna && fantasmas[j].posicao_y == linha){
                            DrawTexture(fantasmas[j].sprite, coluna * 40, linha * 40, WHITE);
                        }
                    }
                    break;
                }
            }
        }

        if (tela == pausa){
            // Desenha a caixa de menu e as opções de texto por cima do jogo pausado
            DrawRectangle(0, 0, LARGURA, ALTURA, Fade(BLACK, 0.85f));
            DrawText("PAUSADO", LARGURA / 2 - MeasureText("PAUSADO", 40) / 2, 120, 40, YELLOW);
            DrawText("Novo Jogo (N)", LARGURA / 2 - 100, 200, 20, WHITE);
            DrawText("Carregar Jogo (C)", LARGURA / 2 - 100, 230, 20, WHITE);
            DrawText("Salvar Jogo (S)", LARGURA / 2 - 100, 260, 20, WHITE);
            DrawText("Voltar ao Jogo (V/TAB)", LARGURA / 2 - 100, 290, 20, WHITE);
            DrawText("Sair (Q)", LARGURA / 2 - 100, 320, 20, WHITE);
        }

        if (tela == gameover) {
            DrawRectangle(0, 0, LARGURA, ALTURA, Fade(BLACK, 0.85f));
            DrawText("GAME OVER", LARGURA/2 - MeasureText("GAME OVER", 60)/2, 150, 60, RED);
            char textoScore[50];
            sprintf(textoScore, "Pontuacao: %d", pacman.pontuacao);
            DrawText(textoScore, LARGURA/2 - MeasureText(textoScore, 30)/2, 240, 30, WHITE);
            DrawText("Pressione Q para Sair", LARGURA/2 - MeasureText("Pressione Q para Sair", 20)/2, 300, 20, WHITE );
}
        // Termina o desenho
        EndDrawing();
    }

    // Fecha a janela
    CloseWindow();

    for (int fant = 0; fant < 4; fant++)
        free(fantasmas[fant].lista_posicoes);
    for (int linha = 0; linha < LinhaMatriz; linha++)
        free(matriz[linha]);
    free(matriz);

    return 0;
}