#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "raylib.h"
#include "structs.h"
#include "mapa.h"
#include "menu.h"
#include <math.h>

#define LARGURA 800
#define ALTURA 440
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

void interacao(char objeto, personagem *p, int *pellets){
    int pontos_ganhos = 0;
    switch (objeto){
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
void reset_posicoes(char **matriz, personagem *pacman, inimigo *fantasmas, int num_fantasmas){
    
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

        if (fantasmas[f].estado == 2){
            continue;
        }
        fantasmas[f].posicao_x = fantasmas[f].x_inicial;
        fantasmas[f].posicao_y = fantasmas[f].y_inicial;

        // define corretamente o que tem embaixo
        fantasmas[f].embaixo = matriz[fantasmas[f].posicao_y][fantasmas[f].posicao_x];

        // reposiciona visivelmente
        matriz[fantasmas[f].posicao_y][fantasmas[f].posicao_x] = 'F';
        fantasmas[f].estado = 0;
    }
}

// identificar a colisão
void verificar_colisao(char **matriz, personagem *pacman, inimigo *fantasmas, TELA *tela_ptr, int num_fantasmas, int *pellets){
    if (*tela_ptr == gameover) return;

    for (int f = 0; f < num_fantasmas; f++){
        if (pacman->posicao_x == fantasmas[f].posicao_x && pacman->posicao_y == fantasmas[f].posicao_y){

            // colisão com fantasma não vulneravel
            if (fantasmas[f].estado == 0){
                pacman->vida--;
                if (pacman->vida < 0) pacman->vida = 0;
                pacman->pontuacao -= 200;
                if (pacman->pontuacao < 0){
                    pacman->pontuacao = 0;
                }
                if (pacman->vida > 0){
                    reset_posicoes(matriz, pacman, fantasmas, num_fantasmas);
                }
                else{
                    //Puxar a tela de gameover (pois a vida vai estar zerada)
                    *tela_ptr = gameover;
                }
                return;
            }

            // colisão com o fantasma vuneravel
            else if (fantasmas[f].estado == 1){
                pacman->pontuacao += 100;
                interacao(fantasmas[f].embaixo,pacman,pellets);
                if (!(fantasmas[f].posicao_y == pacman->posicao_y && fantasmas[f].posicao_x == pacman->posicao_x)) {
                    matriz[fantasmas[f].posicao_y][fantasmas[f].posicao_x] = fantasmas[f].embaixo;
                }
                fantasmas[f].estado = 2; // morto
                fantasmas[f].posicao_y = -100; 
                fantasmas[f].posicao_x = -100;
                fantasmas[f].embaixo = '_';
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
    int menor_distancia = -1;
    int melhor_direcao;
    int maior_distancia = -1;
    int pior_direcao;

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

        if (menor_distancia == -1 || menor_distancia > distancia){
            menor_distancia = distancia;
            melhor_direcao = direcao;
        }
        if (maior_distancia == -1 || maior_distancia < distancia){
            maior_distancia = distancia;
            pior_direcao = direcao;
        }
        
        if (p.estado_pac == 1){
            menor_distancia = maior_distancia;
            melhor_direcao = pior_direcao;
        }
    }
    resultado.melhor_direcao = melhor_direcao;
    resultado.menor_distancia = menor_distancia;
    resultado.pior_direcao = pior_direcao;
    resultado.maior_distancia = maior_distancia;
    return resultado;
}

int mov_fant_vermelho(inimigo f, personagem p){
    /*  O fantasma vermelho é mais agressivo e tenta perseguir o jogador. Quando o jogador está perto, o fantasma será mais provável de
    seguir a direção que diminua a distância entre eles. Quando estão distantes, sua ação é mais aleatória.    */
    dist_manha distancia_fantasma = dist_manhattan(f, p);
    int dir_f;

    //Normalizamos a distância de manhattan 
    float dist_normalizada = distancia_fantasma.menor_distancia/58.0f;

    //A probabilidade do fantasma vermelho seguir o jogador é exponencial baseado em quão próximo do jogador ele se encontra
    float probabilidade = expf(-2 * dist_normalizada) * 100;

    if (GetRandomValue(0,100) < probabilidade) dir_f = distancia_fantasma.melhor_direcao;
    else dir_f = f.lista_posicoes[GetRandomValue(0,(f.tamanho_lista-1))];

    return dir_f;        
}

int mov_fant_laranja(inimigo *f, personagem p, Texture2D sprite_medo, Texture2D sprite){
    /*  O fantasma laranja é mais medroso e tenta fugir do jogador. Quando o jogador está perto, o fantasma será mais provável de
    seguir a direção que aumente a distância entre eles. Quando estão distantes, recupera a coragem e é mais provável de perseguir o jogador.    */
    dist_manha distancia_fantasma = dist_manhattan(*f, p);

    int dir_f;

    //Normalizamos a distância de manhattan 
    float dist_normalizada = distancia_fantasma.maior_distancia/58.0f;

    //A probabilidade do fantasma laranja se afastar do jogador é exponencial baseado em quão próximo do jogador ele se encontra
    float probabilidade = expf(-1.8 * dist_normalizada) * 100;

    if (probabilidade > 85) f->sprite = sprite_medo;
    else f->sprite = sprite;

    if (GetRandomValue(0,100) < probabilidade) dir_f = distancia_fantasma.pior_direcao;
    else dir_f = distancia_fantasma.melhor_direcao;

    return dir_f;        
}

int mov_fant_azul(inimigo f, personagem p){
    dist_manha distancia_fantasma = dist_manhattan(f, p);
    int dir_f;

    if (GetRandomValue(0,100)> 75) dir_f = distancia_fantasma.melhor_direcao;
    else dir_f = f.lista_posicoes[GetRandomValue(0,(f.tamanho_lista-1))];

    return dir_f;
}

// int mov_fant_rosa(inimigo f, personagem p, char **matriz){
//     int alvo[2];
//     int pos_x = p.posicao_x;
//     int pos_y = p.posicao_y;
//     char dir = p.direcao;
//     char dir_buf = p.dir_buffer;
//     char dir_inv;
//     int aumento_x = 0;
//     int aumento_y = 0;
//     int var_bandeira = 0;
//     if (dir == 'D') {aumento_x +=1; dir_inv = 'E'};
//     if (dir == 'B') {aumento_y +=1; dir_inv = 'C'};
//     if (dir == 'E') {aumento_x -=1; dir_inv = 'D'};
//     if (dir == 'C') {aumento_y -=1; dir_inv = 'B'};
//     while (matriz[pos_y][pos_x] != '#'){
//         int quantidade = quant_caminhos(matriz, pos_x, pos_y);
//         if (quantidade == 1){
//             var_bandeira = 1;
//             if (dir == 'D') {pos_x = p.posicao_x; aumento_x = -1;}
//             if (dir == 'B') {pos_y = p.posicao_y; aumento_y = -1;}
//             if (dir == 'E') {pos_x = p.posicao_x; aumento_x = 1;}
//             if (dir == 'C') {pos_y = p.posicao_y; aumento_y = 1;}
//         }
//         if (quantidade == 2){
//             if (var_bandeira){
//                 alvo[0] = pos_x;
//                 alvo[1] = pos_y;
//             }
//         }
//         pos_y += aumento_y;
//         pos_x += aumento_x;
//     }
// }

int quant_caminhos(int **matriz, int x, int y){
    int caminhos = 0;
    
    if (matriz[y][x+1] == '.' || (matriz[y][x+1] == '_')) caminhos += 1; //direita
    if (matriz[y+1][x] == '.' || (matriz[y+1][x] == '_')) caminhos += 1; //baixo
    if (matriz[y][x-1] == '.' || (matriz[y][x-1] == '_')) caminhos += 1; // esquerda
    if (matriz[y-1][x] == '.' || (matriz[y-1][x] == '_')) caminhos += 1; // cima

    return caminhos;
}

void verificar_vitoria(int pellets, TELA *tela, personagem *pacman) {
    if (pellets == 0 && pacman->vida > 0) {
        *tela = vitoria;
    }
}

void iniciar_pacman(personagem *pacman, Texture2D sprite){
    /*Inicializa os valores do pacman*/
    pacman->estado_pac = 0;
    pacman->tempo_invu = 0.0;
    pacman->vida = 3;
    pacman->pontuacao = 0;
    pacman->sprite = sprite; 
}

int main(){
    // Inicia o jogo e põe FPS padrão
    iniciar_tela();
    int qnt_pellets;

    TELA tela = jogo;
    float frame_movimento = 0.0f;


    //Tudo relacionado a sprites
    Texture2D sprite_pac1 = LoadTexture("sprites\\homenagem.png"); //sprite_pacman.png
    Texture2D sprite_pac2 = LoadTexture("sprites\\homenagem.png"); //sprite2_pacman.png
    Texture2D sprite_fantasma_v = LoadTexture("sprites\\f_v_pequeno.png"); //f_vermelho
    Texture2D sprite_fantasma_r = LoadTexture("sprites\\f_r_pequeno.png"); //f_rosa
    Texture2D sprite_fantasma_l = LoadTexture("sprites\\f_l_pequeno.png"); //f_laranja
    Texture2D sprite_fantasma_c = LoadTexture("sprites\\f_a_pequeno.png"); //f_ciano
    Texture2D sprite_F_L_medo = LoadTexture("sprites\\f_l_pequeno_correndo.png");
    Texture2D sprite_fantasma_fraco = LoadTexture("sprites/f_fraco.png");
    Texture2D sprite_parede = LoadTexture("sprites\\parede_pequena.png");
    Texture2D sprite_portal = LoadTexture("sprites\\portal.png");
    

    // Cria a struct do pacman, põe suas posições e seu sprite
    personagem pacman;
    iniciar_pacman(&pacman,sprite_pac1);

    // Fantasmas
    inimigo *fantasma;
    fantasma = (inimigo*) malloc(sizeof(inimigo) * 4);
    if (!fantasma) return -1;


    char **matriz = ler_arquivo("mapa3.txt", &qnt_pellets, &pacman, fantasma);
    if (!matriz) return -1;
    qnt_pellets += 4;
   
    Rectangle img = {0, 0, pacman.sprite.width, pacman.sprite.height};
    Rectangle dest = {pacman.posicao_x * 20 + 10, pacman.posicao_y * 20 + 10, 20, 20}; // Pode estar duplicado, mas funciona
    Vector2 centro = {dest.width / 2, dest.height / 2};

    // Struct dos fantasmas + tudo que puder envolvê-los:    
    int num_fantasmas = 4;

    Texture2D sprites_fantasmas[4] = {sprite_fantasma_v, sprite_fantasma_r, sprite_fantasma_l, sprite_fantasma_c};
    for (int f=0; f<4; f++) fantasma[f].sprite = sprites_fantasmas[f];

    float rotacao = 0;
    float tempo_fantasmas = 0.25;

    while (!WindowShouldClose()){

        menu(&tela, matriz, &pacman, fantasma, num_fantasmas, "mapa3.txt", &qnt_pellets);

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
                    img.width = -pacman.sprite.width;
                    rotacao = 0;
                    moveu = 1;

                    // soma a pontuação
                    interacao(matriz[pacman.posicao_y][pacman.posicao_x + 1], &pacman, &qnt_pellets); 
                    // verifica vitoria 
                    verificar_vitoria(qnt_pellets, &tela, &pacman);

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
                    img.width = pacman.sprite.width;
                    rotacao = 0;
                    moveu = 1;

                    // soma a pontuação
                    interacao(matriz[pacman.posicao_y][pacman.posicao_x - 1], &pacman, &qnt_pellets);
                    // verifica vitoria 
                    verificar_vitoria(qnt_pellets, &tela, &pacman);

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
                    img.width = pacman.sprite.width;
                    rotacao = 90;

                    // soma a pontuação
                    interacao(matriz[pacman.posicao_y-1][pacman.posicao_x], &pacman, &qnt_pellets);
                    // verifica vitoria 
                    verificar_vitoria(qnt_pellets, &tela, &pacman);

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
                    img.width = pacman.sprite.width;
                    rotacao = 270;

                    // soma a pontuação
                    interacao(matriz[pacman.posicao_y+1][pacman.posicao_x], &pacman, &qnt_pellets);
                    // verifica vitoria  
                    verificar_vitoria(qnt_pellets, &tela, &pacman);

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
                    if (pacman.sprite.id == sprite_pac1.id) pacman.sprite = sprite_pac2;
                    else pacman.sprite = sprite_pac1;
                }
            }

            // Mecânica dos fantasmas:
            for (int f = 0; f < 4; f++){
                if (fantasma[f].estado == 2) continue;

                fantasma[f].tempo += GetFrameTime();

                if (fantasma[f].tempo >= tempo_fantasmas){
                    //    fantasmas[f].lista_posicoes = realloc(fantasmas[f].lista_posicoes, 0 * sizeof(int));
                    if (fantasma[f].lista_posicoes != NULL) {
                    free(fantasma[f].lista_posicoes); 
                    }
                    fantasma[f].lista_posicoes = NULL; 
                    fantasma[f].tamanho_lista = 0;

                    // Verificação de caminhos possiveis
                    if (verifica_mov_fant('D', matriz, fantasma[f])){
                        adicionar_pos(&fantasma[f].tamanho_lista, &fantasma[f].lista_posicoes, 0);
                    }

                    if (verifica_mov_fant('B', matriz, fantasma[f])){
                        adicionar_pos(&fantasma[f].tamanho_lista, &fantasma[f].lista_posicoes, 1);
                    }

                    if (verifica_mov_fant('E', matriz, fantasma[f])){
                        adicionar_pos(&fantasma[f].tamanho_lista, &fantasma[f].lista_posicoes, 2);
                    }

                    if (verifica_mov_fant('C', matriz, fantasma[f])){
                        adicionar_pos(&fantasma[f].tamanho_lista, &fantasma[f].lista_posicoes, 3);
                    }

                    // Verifica se o fantasma pode voltar
                    if (fantasma[f].tamanho_lista > 1 && pacman.estado_pac == 0){
                        if ((fantasma[f].ultimo_x == fantasma[f].posicao_x+1) && (fantasma[f].ultimo_mov == 0)) nao_voltar(&fantasma[f].tamanho_lista, &fantasma[f].lista_posicoes,0);                     
                        if ((fantasma[f].ultimo_y == fantasma[f].posicao_y+1) && (fantasma[f].ultimo_mov == 1)) nao_voltar(&fantasma[f].tamanho_lista, &fantasma[f].lista_posicoes,1);
                        if ((fantasma[f].ultimo_x == fantasma[f].posicao_x-1) && (fantasma[f].ultimo_mov == 0)) nao_voltar(&fantasma[f].tamanho_lista, &fantasma[f].lista_posicoes,2);
                        if ((fantasma[f].ultimo_y == fantasma[f].posicao_y-1) && (fantasma[f].ultimo_mov == 1)) nao_voltar(&fantasma[f].tamanho_lista, &fantasma[f].lista_posicoes,3);
                    }
                    if (fantasma[f].tamanho_lista == 0) continue;

                    int dir_f;
                    
                    if (fantasma[f].id==0) dir_f = mov_fant_vermelho(fantasma[f],pacman);
                    if (fantasma[f].id==2) dir_f = mov_fant_laranja(&fantasma[f],pacman,sprite_F_L_medo, sprite_fantasma_l);
                    if (fantasma[f].id==3) dir_f = mov_fant_azul(fantasma[f],pacman);
                    if (fantasma[f].id==1) dir_f = fantasma[f].lista_posicoes[GetRandomValue(0,(fantasma[f].tamanho_lista-1))];

                    // DIREITA
                    if (dir_f == 0){
                        matriz[fantasma[f].posicao_y][fantasma[f].posicao_x] = fantasma[f].embaixo;
                        if (matriz[fantasma[f].posicao_y][fantasma[f].posicao_x + 1] == 'T'){
                            fantasma[f].embaixo = matriz[fantasma[f].posicao_y][1];
                            fantasma[f].posicao_x = 1;
                        }
                        fantasma[f].embaixo = matriz[fantasma[f].posicao_y][fantasma[f].posicao_x + 1];
                        matriz[fantasma[f].posicao_y][fantasma[f].posicao_x + 1] = 'F';
                        fantasma[f].ultimo_x = fantasma[f].posicao_x;
                        fantasma[f].posicao_x++;
                        fantasma[f].ultimo_mov = 0;
                    }
                    // BAIXO
                    if (dir_f == 1){
                        matriz[fantasma[f].posicao_y][fantasma[f].posicao_x] = fantasma[f].embaixo;
                        if (matriz[fantasma[f].posicao_y+1][fantasma[f].posicao_x] == 'T'){
                            fantasma[f].embaixo = matriz[fantasma[f].posicao_y][1];
                            fantasma[f].posicao_y = 0;
                        }
                        fantasma[f].embaixo = matriz[fantasma[f].posicao_y + 1][fantasma[f].posicao_x];
                        matriz[fantasma[f].posicao_y + 1][fantasma[f].posicao_x] = 'F';
                        fantasma[f].ultimo_y = fantasma[f].posicao_y;
                        fantasma[f].posicao_y++;
                        fantasma[f].ultimo_mov = 1;
                    }
                    // ESQUERDA
                    if (dir_f == 2){
                        matriz[fantasma[f].posicao_y][fantasma[f].posicao_x] = fantasma[f].embaixo;
                        if (matriz[fantasma[f].posicao_y][fantasma[f].posicao_x - 1] == 'T'){
                            fantasma[f].embaixo = matriz[fantasma[f].posicao_y][38];
                            fantasma[f].posicao_x = 38;
                        }
                        fantasma[f].embaixo = matriz[fantasma[f].posicao_y][fantasma[f].posicao_x - 1];
                        matriz[fantasma[f].posicao_y][fantasma[f].posicao_x - 1] = 'F';
                        fantasma[f].ultimo_x = fantasma[f].posicao_x;
                        fantasma[f].posicao_x--;
                        fantasma[f].ultimo_mov = 0;
                    }
                    // CIMA
                    if (dir_f == 3){
                        matriz[fantasma[f].posicao_y][fantasma[f].posicao_x] = fantasma[f].embaixo;
                        if (matriz[fantasma[f].posicao_y-1][fantasma[f].posicao_x] == 'T'){
                            fantasma[f].embaixo = matriz[fantasma[f].posicao_y][1];
                            fantasma[f].posicao_y = 19;
                        }
                        fantasma[f].embaixo = matriz[fantasma[f].posicao_y - 1][fantasma[f].posicao_x];
                        matriz[fantasma[f].posicao_y - 1][fantasma[f].posicao_x] = 'F';
                        fantasma[f].ultimo_y = fantasma[f].posicao_y;
                        fantasma[f].posicao_y--;
                        fantasma[f].ultimo_mov = 1;
                    }
                    if (pacman.estado_pac == 1){
                        fantasma[f].estado = 1;
                        fantasma[f].sprite = sprite_fantasma_fraco;
                        tempo_fantasmas = 0.33;

                        if (pacman.tempo_invu >= 8.0f){
                            for (int x = 0; x < 4; x++){
                                 if (fantasma[x].estado != 2) {
                                fantasma[x].sprite = sprites_fantasmas[x];
                                fantasma[x].estado = 0;
                            }
                        }
                            pacman.estado_pac = 0;
                            pacman.tempo_invu = 0;
                            tempo_fantasmas = 0.25;
                        }
                    }
                    fantasma[f].tempo = 0.0f;
                }

                verificar_colisao(matriz, &pacman, fantasma,&tela, 4,&qnt_pellets);
                
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
        case vitoria:{
            mostrar_vitoria(&tela, &pacman.pontuacao, &pacman.vida);
        }break; //fim case vitoria
        
        } // fim do switch

        BeginDrawing();
        ClearBackground(BLACK);

        // Desenho do score
        char txt_score[50];
        sprintf(txt_score, "SCORE: %d", pacman.pontuacao);
        DrawText(txt_score, LARGURA - 150, ALTURA - 20, 20, YELLOW);

        //Desenha o pellet
        char pellets[20];
        sprintf(txt_score, "Pellets: %d", qnt_pellets);
        DrawText(txt_score, LARGURA - 500, ALTURA - 20, 20, YELLOW);

        // desenha as vidas
        char txt_vidas[10];
        sprintf(txt_vidas, "VIDAS: %d", pacman.vida);
        DrawText(txt_vidas, 10, ALTURA - 20, 20, WHITE);

        // Insere os sprites certos
        //Pacman:
        dest.x = pacman.posicao_x * 20 + 10;
        dest.y = pacman.posicao_y * 20 + 10;
        DrawTexturePro(pacman.sprite, img, dest, centro, rotacao, WHITE);

        for (int linha = 0; linha < 20; linha++){
            for (int coluna = 0; coluna < 40; coluna++){
                switch (matriz[linha][coluna]){
                case '#': // Parede
                    DrawTexture(sprite_parede, coluna * 20, linha * 20, WHITE);
                    break;
                case '.': // Pellet
                    DrawCircle(coluna * 20 + 10, linha * 20 + 10, 2, WHITE);
                    break;
                case 'T': // Portal
                    DrawTexture(sprite_portal, coluna * 20, linha * 20, WHITE);
                    break;
                case 'o': // Power Pellet
                    DrawCircle(coluna * 20 + 10, linha * 20 + 10, 5, WHITE);
                    break;
                case 'F': // Fantasma
                    for (int j = 0; j < 4; j++){
                        if (fantasma[j].estado != 2 && fantasma[j].posicao_x == coluna && fantasma[j].posicao_y == linha){
                            DrawTexture(fantasma[j].sprite, coluna * 20, linha * 20, WHITE);
                        }
                    }
                    break;
                }
            }
        }

        if (tela == pausa){
            DrawRectangle(0, 0, LARGURA, ALTURA, Fade(BLACK, 0.85f));
            DrawText("MENU", LARGURA / 2 - MeasureText("MENU", 40) / 2, 120, 40, YELLOW);
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
            DrawText("Pressione TAB para Menu", LARGURA/2 - MeasureText("Pressione TAB para Menu", 20)/2, 300, 20, WHITE );
            DrawText("Pressione Q para Sair", LARGURA/2 - MeasureText("Pressione Q para Sair", 20)/2, 340, 20, WHITE );
        }
        if (tela == vitoria){
            DrawText("VOCE VENCEU!", 260, 120, 60, YELLOW);
            DrawText("Todos os pellets foram coletados!", 180, 220, 30, WHITE);
            DrawText(TextFormat("Pontuacao: %d", pacman.pontuacao), 320, 300, 35, GREEN);
            DrawText(TextFormat("Vidas restantes: %d", pacman.vida), 300, 350, 35, RED);
            DrawText("Pressione TAB para voltar ao menu", 180, 450, 25, WHITE);
        }

        // Termina o desenho
        EndDrawing();
    }
    Texture2D sprites[] = {sprite_pac1,sprite_pac2, sprite_fantasma_v, sprite_fantasma_l,
    sprite_fantasma_c, sprite_fantasma_r, sprite_F_L_medo, sprite_fantasma_fraco, sprite_portal, sprite_parede};
    for (int x = 0; x<10; x++) UnloadTexture(sprites[x]);
    
    // Fecha a janela
    CloseWindow();

    for (int fant = 0; fant < 4; fant++)
        free(fantasma[fant].lista_posicoes);
    for (int linha = 0; linha < LinhaMatriz; linha++)
        free(matriz[linha]);
    free(matriz);
    free(fantasma);

    return 0;
}