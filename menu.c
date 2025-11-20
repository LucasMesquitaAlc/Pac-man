#include "menu.h" 
#include <stdio.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "raylib.h"
#include "structs.h"
#include "mapa.h"

void novojogo(char **matriz, personagem *pacman_ptr, int *pontuacao_ptr, int *vidas_ptr) {

    *pontuacao_ptr = 0;
    *vidas_ptr = 3; 
    
    //Reseta superficial (tem que botar a posição certa dele)
    pacman_ptr->posicao_x = 1;
    pacman_ptr->posicao_y = 1;

}

#define LinhaMatriz 20
#define ColunaMatriz 40

void salvarjogo(char **matriz, personagem pacman, int pontuacao, int vidas) {
    FILE *f = fopen("save.bin", "wb");
    if (f == NULL) return; 

    fwrite(&pacman, sizeof(personagem), 1, f);
    fwrite(&pontuacao, sizeof(int), 1, f);
    fwrite(&vidas, sizeof(int), 1, f);


    for (int i = 0; i < LinhaMatriz; i++) {
        fwrite(matriz[i], sizeof(char), ColunaMatriz, f); 
    }
    
    fclose(f);
}

void carregarjogo(char **matriz, personagem *pacman_ptr, int *pontuacao_ptr, int *vidas_ptr) {
    FILE *f = fopen("save.bin", "rb");
    if (f == NULL) {
        printf("Nenhum jogo salvo encontrado.\n");
        return; 
    }

    fread(pacman_ptr, sizeof(personagem), 1, f);
    fread(pontuacao_ptr, sizeof(int), 1, f);
    fread(vidas_ptr, sizeof(int), 1, f);

    for (int i = 0; i < LinhaMatriz; i++) {
        fread(matriz[i], sizeof(char), ColunaMatriz, f);
    }
    
    fclose(f);
}

void sairjogo() {
    CloseWindow(); // encerra a janela do Raylib
}

void menu(TELA *tela_ptr, char **matriz, personagem *pacman_ptr, int *pontuacao_ptr, int *vidas_ptr) {
    
    //Abrir o menu 
    if (IsKeyPressed(KEY_TAB)) {
        if (*tela_ptr == jogo) {
            *tela_ptr = pausa; // Pausa o jogo
        } else {
            // Se o usuário apertar TAB no menu, ele volta para o jogo
            *tela_ptr = jogo; 
            return;
        }
    }

    if (*tela_ptr != pausa) {
        return;
    }

    if (IsKeyPressed(KEY_N)) {
        novojogo(matriz, pacman_ptr, pontuacao_ptr, vidas_ptr); 
        *tela_ptr = jogo;
    }
    
    if (IsKeyPressed(KEY_C)) {
        carregarjogo(matriz, pacman_ptr, pontuacao_ptr, vidas_ptr);
        *tela_ptr = jogo;
    }
    
    if (IsKeyPressed(KEY_S)) {
        salvarjogo(matriz, *pacman_ptr, *pontuacao_ptr, *vidas_ptr); 
    }
    
    if (IsKeyPressed(KEY_Q)) {
        sairjogo(); 
    }

    if (IsKeyPressed(KEY_V)) {
        *tela_ptr = jogo; 
    }
}

