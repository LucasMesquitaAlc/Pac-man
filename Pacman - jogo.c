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
    InitAudioDevice();
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

void interacao(char objeto, personagem *p, int *pellets, Sound som[]){
   
    SetSoundVolume(som[2], 0.2f);
    SetSoundVolume(som[3], 0.8f);  
    if (!IsSoundPlaying(som[2]) && !IsSoundPlaying(som[4])) PlaySound(som[2]);     // toca
    
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
        PlaySound(som[4]);
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

    pacman->rotacao = 0;
}

// identificar a colisão
void verificar_colisao(char **matriz, personagem *pacman, inimigo *fantasmas, TELA *tela_ptr, int num_fantasmas, int *pellets, Sound som[]){
    SetSoundVolume(som[0], 0.3f);
    SetSoundVolume(som[1], 0.5f);
    if (*tela_ptr == gameover) return;

    for (int f = 0; f < num_fantasmas; f++){
        if (pacman->posicao_x == fantasmas[f].posicao_x && pacman->posicao_y == fantasmas[f].posicao_y){

            // colisão com fantasma não vulneravel
            if (fantasmas[f].estado == 0){
                pacman->vida--;
                PlaySound(som[0]);
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
                PlaySound(som[1]);
                interacao(fantasmas[f].embaixo,pacman,pellets, som);
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

dist_manha dist_manhattan(inimigo f, int x, int y, personagem p){
    // Calcula a menor distância de manhattan do fantasma até o pacman
    int menor_distancia = -1;
    int melhor_direcao = -1;
    int maior_distancia = -1;
    int pior_direcao = -1;

    dist_manha resultado;

    for (int i = 0; i < f.tamanho_lista; i++){

        int direcao = f.lista_posicoes[i];
        float distancia;

        if (direcao == 0){
        distancia = abs(x - (f.posicao_x+1)) + abs(y - f.posicao_y);
        }
        else if (direcao == 1){
        distancia = abs(x - f.posicao_x) + abs(y - (f.posicao_y+1));
        }
        else if (direcao == 2){
        distancia = abs(x - (f.posicao_x-1)) + abs(y - f.posicao_y);
        }
        else if (direcao == 3){
        distancia = abs(x - f.posicao_x) + abs(y - (f.posicao_y-1));
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
    dist_manha distancia_fantasma = dist_manhattan(f, p.posicao_x, p.posicao_y, p);
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
    dist_manha distancia_fantasma = dist_manhattan(*f, p.posicao_x, p.posicao_y, p);

    int dir_f;

    //Normalizamos a distância de manhattan 
    float dist_normalizada = distancia_fantasma.maior_distancia/58.0f;

    //A probabilidade do fantasma laranja se afastar do jogador é exponencial baseado em quão próximo do jogador ele se encontra
    float probabilidade = expf(-2.3 * dist_normalizada) * 100;

    if (probabilidade > 90) f->sprite = sprite_medo;
    else f->sprite = sprite;

    if (GetRandomValue(0,100) < probabilidade) dir_f = distancia_fantasma.pior_direcao;
    else dir_f = distancia_fantasma.melhor_direcao;

    return dir_f;        
}

int mov_fant_azul(inimigo f, personagem p){
    dist_manha distancia_fantasma = dist_manhattan(f, p.posicao_x, p.posicao_y, p);
    int dir_f;

    if (GetRandomValue(0,100)> 66) dir_f = distancia_fantasma.melhor_direcao;
    else dir_f = f.lista_posicoes[GetRandomValue(0,(f.tamanho_lista-1))];

    return dir_f;
}

int mov_fant_rosa(inimigo f, personagem p, char **matriz){
    dist_manha distancia_fantasma = dist_manhattan(f, p.posicao_x, p.posicao_y, p);
    int dir_f;

    if (distancia_fantasma.menor_distancia < 6 || p.direcao == -1) return distancia_fantasma.melhor_direcao;
    else{
        for (int i= 1; i <= 50; i++){
            if (p.dir_buffer == 'D'){
                if (matriz[p.posicao_y][p.posicao_x+i] != '#' && matriz[p.posicao_y][p.posicao_x+i] != 'T') {}
                else return dist_manhattan(f,p.posicao_x+i-1,p.posicao_y,p).melhor_direcao;
            }
            if (p.dir_buffer == 'B'){
                if (matriz[p.posicao_y+i][p.posicao_x] != '#' && matriz[p.posicao_y+i][p.posicao_x] != 'T') {}
                else return dist_manhattan(f,p.posicao_x,p.posicao_y+i-1,p).melhor_direcao;
            }
            if (p.dir_buffer == 'E'){
                if (matriz[p.posicao_y][p.posicao_x-i] != '#' && matriz[p.posicao_y][p.posicao_x-i] != 'T') {}
                else return dist_manhattan(f,p.posicao_x-i+1,p.posicao_y,p).melhor_direcao;
            }
            if (p.dir_buffer == 'C'){
                if (matriz[p.posicao_y-i][p.posicao_x] != '#' && matriz[p.posicao_y-i][p.posicao_x] != 'T') {}
                else return dist_manhattan(f,p.posicao_x,p.posicao_y-i+1,p).melhor_direcao;
            } 
        }
    }
}

int quant_caminhos(char **matriz, int x, int y){
    int caminhos = 0;
    
    if (matriz[y][x+1] == '.' || (matriz[y][x+1] == '_')) caminhos += 1; //direita
    if (matriz[y+1][x] == '.' || (matriz[y+1][x] == '_')) caminhos += 1; //baixo
    if (matriz[y][x-1] == '.' || (matriz[y][x-1] == '_')) caminhos += 1; // esquerda
    if (matriz[y-1][x] == '.' || (matriz[y-1][x] == '_')) caminhos += 1; // cima

    return caminhos;
}

bool verificar_vitoria(int pellets, TELA *tela, personagem *pacman, int fase_atual, int fase_final) {
    if (pellets == 0 && pacman->vida > 0) {
        if (fase_atual == fase_final) *tela = venceu_final;
        else *tela = vitoria;

        return true;
    }
    else return false;
}

void iniciar_pacman(personagem *pacman, Texture2D sprite){
    /*Inicializa os valores do pacman*/
    pacman->estado_pac = 0;
    pacman->tempo_invu = 0.0;
    pacman->vida = 3;
    pacman->pontuacao = 0;
    pacman->sprite = sprite; 
    pacman->img.x = 0;
    pacman->img.y = 0;
    pacman->img.width  = pacman->sprite.width;
    pacman->img.height = pacman->sprite.height;
    pacman->rotacao = 0;
    pacman->direcao = -1;
}

int main(){
    // Inicia o jogo e põe FPS padrão
    iniciar_tela();
    SetExitKey(KEY_NULL);
    int qnt_pellets = 0;

    TELA tela = tela_inicial;
    float frame_movimento = 0.0f;

    // Tudo relacionado a sprites
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

    //Musicas 
    Sound SomMorte = LoadSound("sons\\pacman_death.wav");
    Sound ComeFantasma = LoadSound("sons\\pacman_eatghost.wav");
    Sound ComePellet = LoadSound("sons\\pacman_chomp.wav");
    Sound Iniciar_jogo = LoadSound("sons\\pacman_beginning.wav");
    Sound Power_pellet = LoadSound("sons\\pacman_power_pellet.mp3");
    Sound sons[] = {SomMorte,ComeFantasma,ComePellet, Iniciar_jogo, Power_pellet};

    // Cria a struct do pacman, põe suas posições e seu sprite
    personagem pacman;
    iniciar_pacman(&pacman,sprite_pac1);

    // Fantasmas
    inimigo *fantasma;
    fantasma = (inimigo*) malloc(sizeof(inimigo) * 4);
    if (!fantasma) return -1;


    char fase[15] = "mapa1.txt";
    char nome_arquivo[25] = "mapas\\mapa1.txt";
    int fase_atual = 1;
    int ultima_fase = descobrir_ultima_fase();


    int num_fantasmas = 0;
    char **matriz = ler_arquivo(nome_arquivo, &qnt_pellets, &pacman, fantasma, &num_fantasmas);
    if (!matriz) return -1;
    qnt_pellets += num_fantasmas;
   
    Rectangle dest = {pacman.posicao_x * 20 + 10, pacman.posicao_y * 20 + 10, 20, 20}; // Pode estar duplicado, mas funciona
    Vector2 centro = {dest.width / 2, dest.height / 2};

    // Struct dos fantasmas + tudo que puder envolvê-los:    

    Texture2D sprites_fantasmas[4] = {sprite_fantasma_v, sprite_fantasma_r, sprite_fantasma_l, sprite_fantasma_c};
    for (int f=0; f<4; f++) fantasma[f].sprite = sprites_fantasmas[f];

    float tempo_fantasmas = 0.25f;
    while (!WindowShouldClose()){
        menu(&tela, matriz, &pacman, fantasma, &num_fantasmas, nome_arquivo, &qnt_pellets, sons[3]);
        
        switch (tela){

        case tela_inicial: {} break;

        case jogo:{
            
            if (IsSoundPlaying(sons[3]) || IsSoundPlaying(sons[0])){} 
            else{

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
                        pacman.img.width = -pacman.sprite.width;
                        pacman.rotacao = 0;
                        moveu = 1;

                        
                        // soma a pontuação
                        interacao(matriz[pacman.posicao_y][pacman.posicao_x + 1], &pacman, &qnt_pellets, sons); 
                        // verifica vitoria 
                        verificar_vitoria(qnt_pellets, &tela, &pacman,fase_atual, ultima_fase);
                        matriz[pacman.posicao_y][pacman.posicao_x] = '_';
                        if (matriz[pacman.posicao_y][pacman.posicao_x + 1] == 'T'){
                            matriz[pacman.posicao_y][pacman.posicao_x] = '_';
                            interacao(matriz[pacman.posicao_y][1], &pacman, &qnt_pellets, sons); 
                            pacman.posicao_x = 1;
                            verificar_vitoria(qnt_pellets, &tela, &pacman,fase_atual, ultima_fase);
                            matriz[pacman.posicao_y][pacman.posicao_x] = 'P';
                        }
                        else{
                        matriz[pacman.posicao_y][pacman.posicao_x] = '_';
                        pacman.posicao_x++;
                        matriz[pacman.posicao_y][pacman.posicao_x] = 'P';
                        }
                    }
                    if (pacman.direcao == 'E' && verifica_mov_pac('E', matriz, &pacman)){           
                        pacman.img.width = pacman.sprite.width;
                        pacman.rotacao = 0;
                        moveu = 1;

                        // soma a pontuação
                        interacao(matriz[pacman.posicao_y][pacman.posicao_x - 1], &pacman, &qnt_pellets, sons);
                        // verifica vitoria 
                        verificar_vitoria(qnt_pellets, &tela, &pacman,fase_atual, ultima_fase);

                        matriz[pacman.posicao_y][pacman.posicao_x] = '_';
                        if (matriz[pacman.posicao_y][pacman.posicao_x - 1] == 'T'){
                            matriz[pacman.posicao_y][pacman.posicao_x] = '_';
                            interacao(matriz[pacman.posicao_y][38], &pacman, &qnt_pellets, sons); 
                            pacman.posicao_x = 38;
                            verificar_vitoria(qnt_pellets, &tela, &pacman,fase_atual, ultima_fase);
                            matriz[pacman.posicao_y][pacman.posicao_x] = 'P';
                        }
                        else{
                        matriz[pacman.posicao_y][pacman.posicao_x] = '_';
                        pacman.posicao_x--;
                        matriz[pacman.posicao_y][pacman.posicao_x] = 'P';
                        }
                    }
                    if (pacman.direcao == 'C' && verifica_mov_pac('C', matriz, &pacman)){
                        pacman.img.width = pacman.sprite.width;
                        pacman.rotacao = 90;

                        // soma a pontuação
                        interacao(matriz[pacman.posicao_y-1][pacman.posicao_x], &pacman, &qnt_pellets, sons);
                        // verifica vitoria 
                        verificar_vitoria(qnt_pellets, &tela, &pacman,fase_atual, ultima_fase);

                        if (matriz[pacman.posicao_y-1][pacman.posicao_x] == 'T'){
                            matriz[pacman.posicao_y][pacman.posicao_x] = '_';
                            interacao(matriz[18][pacman.posicao_x], &pacman, &qnt_pellets, sons); 
                            pacman.posicao_y = 18;
                            verificar_vitoria(qnt_pellets, &tela, &pacman,fase_atual, ultima_fase);
                            matriz[pacman.posicao_y][pacman.posicao_x] = 'P';
                        }
                        else{
                        matriz[pacman.posicao_y][pacman.posicao_x] = '_';
                        pacman.posicao_y--;
                        matriz[pacman.posicao_y][pacman.posicao_x] = 'P';
                        moveu = 1;
                        }
                    }
                    if (pacman.direcao == 'B' && verifica_mov_pac('B', matriz, &pacman)){
                        pacman.img.width = pacman.sprite.width;
                        pacman.rotacao = 270;

                        // soma a pontuação
                        interacao(matriz[pacman.posicao_y+1][pacman.posicao_x], &pacman, &qnt_pellets, sons);
                        // verifica vitoria  
                        verificar_vitoria(qnt_pellets, &tela, &pacman,fase_atual, ultima_fase);

                        if (matriz[pacman.posicao_y+1][pacman.posicao_x] == 'T'){
                            matriz[pacman.posicao_y][pacman.posicao_x] = '_';
                            interacao(matriz[1][pacman.posicao_x], &pacman, &qnt_pellets, sons);
                            pacman.posicao_y = 1;
                            verificar_vitoria(qnt_pellets, &tela, &pacman,fase_atual, ultima_fase); 
                            matriz[pacman.posicao_y][pacman.posicao_x] = 'P';
                        }
                        else{
                        matriz[pacman.posicao_y][pacman.posicao_x] = '_';
                        pacman.posicao_y++;
                        matriz[pacman.posicao_y][pacman.posicao_x] = 'P';
                        moveu = 1;
                        }
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

                        int dir_f = -10;
                        
                        if (fantasma[f].id==0) dir_f = mov_fant_vermelho(fantasma[f],pacman);
                        if (fantasma[f].id==2) dir_f = mov_fant_laranja(&fantasma[f],pacman,sprite_F_L_medo, sprite_fantasma_l);
                        if (fantasma[f].id==3) dir_f = mov_fant_azul(fantasma[f],pacman);
                        if (fantasma[f].id==1) dir_f = mov_fant_rosa(fantasma[f],pacman,matriz);

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

                    verificar_colisao(matriz, &pacman, fantasma,&tela, 4,&qnt_pellets, sons);
                    
                    if (pacman.vida <= 0) tela = gameover;              
                }
                if (moveu) frame_movimento = 0;
            }
        } break; //fim case jogo 
        case pausa:{

        }break; //fim case pausa 
        case gameover: {    
            mostrar_gameover(&tela, &pacman.pontuacao, &pacman.vida);
        } break;
        case vitoria:{
            mostrar_vitoria(&tela, &pacman.pontuacao, &pacman.vida);
            if(verificar_vitoria(qnt_pellets, &tela, &pacman,fase_atual, ultima_fase) == true){
                passar_mapa(fase,nome_arquivo,&fase_atual);
                if(fase_atual <= ultima_fase) novojogo(matriz, &pacman, fantasma, &num_fantasmas, nome_arquivo, &qnt_pellets, pacman.pontuacao, pacman.vida);
                qnt_pellets += num_fantasmas;
            }
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
        DrawTexturePro(pacman.sprite, pacman.img, dest, centro, pacman.rotacao, WHITE);

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
        if (tela == tela_inicial) {
            DrawRectangle(0, 0, LARGURA, ALTURA, Fade(BLACK, 0.85f));
            DrawText("BEM VINDO AO PACMAN", LARGURA / 2 - MeasureText("BEM VINDO AO PACMAN", 40) / 2, 120, 40, YELLOW);
            DrawText("Novo Jogo (N)", LARGURA / 2 - 100, 220, 20, WHITE);
            DrawText("Carregar Jogo (C)", LARGURA / 2 - 100, 250, 20, WHITE);
            DrawText("Sair (Q)", LARGURA / 2 - 100, 280, 20, WHITE);
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
            StopSound(sons[4]);
            DrawRectangle(0, 0, LARGURA, ALTURA, Fade(BLACK, 0.85f));
            DrawText("VOCE VENCEU O NIVEL!", LARGURA / 2 - MeasureText("VOCE VENCEU O NIVEL!", 40) / 2, 120, 40, YELLOW);
            DrawText(TextFormat("Pontuacao: %d", pacman.pontuacao), LARGURA / 2 - 100, 200, 20, GREEN);
            DrawText(TextFormat("Vidas restantes: %d", pacman.vida), LARGURA / 2 - 100, 230, 20, RED);
            DrawText("Avancar de Nivel (ENTER)", LARGURA / 2 - 100, 280, 20, WHITE);
            DrawText("Salvar Jogo (S)", LARGURA / 2 - 100, 310, 20, WHITE);
            DrawText("Sair sem salvar (Q)", LARGURA / 2 - 100, 340, 20, WHITE);
            DrawText("Menu de Pausa (TAB)", LARGURA / 2 - 100, 370, 20, WHITE);

            if (IsKeyPressed(KEY_ENTER)){
                tela = jogo;
                PlaySound(sons[3]);}
        }

        if (tela == venceu_final) { 
            StopSound(sons[4]);
            DrawRectangle(0, 0, LARGURA, ALTURA, Fade(BLACK, 0.85f));
            DrawText("VOCE VENCEU O JOGO!", LARGURA / 2 - MeasureText("VOCE VENCEU O JOGO!", 40) / 2, 120,40, YELLOW);
            DrawText(TextFormat("Pontuacao Total: %d", pacman.pontuacao), LARGURA / 2 - 100, 200, 20, GREEN);
            DrawText("Novo jogo (N)", LARGURA / 2 - 100, 260, 20, WHITE);
            DrawText("Voltar ao Menu Inicial (TAB)", LARGURA / 2 - 100, 290, 20, WHITE);
            DrawText("Sair (Q)", LARGURA / 2 - 100, 320, 20, WHITE);
        }

        // Termina o desenho
        EndDrawing();
    }
    Texture2D sprites[] = {sprite_pac1,sprite_pac2, sprite_fantasma_v, sprite_fantasma_l,
    sprite_fantasma_c, sprite_fantasma_r, sprite_F_L_medo, sprite_fantasma_fraco, sprite_portal, sprite_parede};
    for (int x = 0; x<10; x++) UnloadTexture(sprites[x]);
    
    // Fecha a janela
    CloseWindow();

    UnloadSound(SomMorte);
    UnloadSound(ComeFantasma);
    UnloadSound(ComePellet);
    CloseAudioDevice();

    for (int fant = 0; fant < 4; fant++)
        free(fantasma[fant].lista_posicoes);
    for (int linha = 0; linha < LinhaMatriz; linha++)
        free(matriz[linha]);
    free(matriz);
    free(fantasma);

    return 0;
}