/**
 * @file interface.c
 * @authors @lkmotta, @Dirceu06, @boonfim
 * @brief Implementacao das funcoes de interface grafica do jogo
 *
 * LICENSE: MIT
 * @copyright Copyright (c) 2025
 *
 */

#include "interface.h"
#include "funcaux.h"
#include "filechange.h"
#include "game.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h> // necessária para srand
#include "raylib.h"

#define SILVER (Color){192, 192, 192, 255}
#define BRONZE (Color){205, 127, 50, 255}

static float hue = 0.0f;

void interface(Cartas *cartas, int size_cartas, int quant_cartas_baralho)
{
    //GameState estadoAtual = CARREGANDO_TELA_INICIAL; // primeiro vamos gerar os baralhos
    GameState estadoAtual = NOVO_BARALHO; // primeiro vamos gerar os baralhos
    Cartas *baralho_jogador = NULL, *baralho_cpu = NULL;
    bool player_joga = true;             // booleano -> 0 = cpu joga, 1 = player joga
    int quem_ganhou = -1;                 // 0 = cpu, 1 = player, -1 = empate
    const char *atributos[] = {"Força", "Habilidade", "Velocidade", "Poderes", "Poder Cura"};
    int atributo = 0;                    // atributo escolhido (indice)
    const char *atributo_nome = "Força"; // nome do atributo escolhido (forca por padrao)
    bool maior_menor = true;             // booleano -> 0 = menor, 1 = maior
    bool maior_menor_selecionado = true; // booleano -> 0 = menor selecionado, 1 = maior selecionado
    int quant_cartas_jogador = quant_cartas_baralho, quant_cartas_cpu = quant_cartas_baralho;
    Cartas carta_jogador, carta_cpu;

    int rodada = 1;
    Cartas *cartas_empate_jogador = NULL;
    Cartas *cartas_empate_cpu = NULL;
    int quant_cartas_empate = 0;

    // estatisticas
    int vitorias = 0, empates = 0, pontuacao = 0;
    Historico partidaHist;

    // variáveis das telas (para evitar redeclarar)
    char nickname[4] = "";
    int tempo_contagem_regressiva, tempo_agora,tempo_inicial;
    int carta_x, carta_y, circulo_x, circulo_y, raio_circulo;
    int atributo_x = 0, atributo_y = 0;
    char informacao_rodada[30];

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Projeto Super-Trunfo"); // iniciando a janela
    SetTargetFPS(FPS);                                               // definindo a taxa de quadros por segundo


    // carregando musica de fundo
    InitAudioDevice();
    //Music musica_fundo = LoadMusicStream("assets/sounds/loading.wav");
    //PlayMusicStream(musica_fundo); 
    
    //carregando audio de atributos e de confirmação/maior/menor
    Sound som_atributos=LoadSound("assets/sounds/atributos.wav");
    Sound som_resto=LoadSound("assets/sounds/resto.wav");
    SetSoundVolume(som_resto, 0.8);
    Sound som_tecla=LoadSound("assets/sounds/tecla.wav");

    Image imagem_fundo;

    while (!WindowShouldClose()){
        BeginDrawing();
        ClearBackground(COR_FUNDO);
        
        switch (estadoAtual){
            case CARREGANDO_TELA_INICIAL: {
                if(quem_ganhou == 0){
                    imagem_fundo = LoadImage("assets/img/tela_inicial_cpuwin.png");
                }else if(quem_ganhou == 1){
                    imagem_fundo = LoadImage("assets/img/tela_inicial_playerwin.png");
                }else{
                    imagem_fundo = LoadImage("assets/img/tela_inicial.png");
                }
                
                if (!imagem_fundo.data) {
                    ClearBackground(COR_FUNDO);
                }else{
                    Texture2D textura_fundo = LoadTextureFromImage(imagem_fundo);
                    DrawTexture(textura_fundo, 0, 0, WHITE);
                    UnloadImage(imagem_fundo);
                }
                break;
            }

            case TELA_INICIAL: {
                // implementar os botões da tela inicial
                
                break;
            }
           
            case RESET: {
                player_joga = true;
                quem_ganhou = 0, atributo = 0;
                atributo_nome = "Força";
                maior_menor = true, maior_menor_selecionado = true;
                quant_cartas_jogador = quant_cartas_baralho, quant_cartas_cpu = quant_cartas_baralho;
                rodada = 1, vitorias = 0, empates = 0, pontuacao = 0;
                
                quant_cartas_empate = 0;
                if (cartas_empate_jogador != NULL) {
                    free(cartas_empate_jogador);
                    cartas_empate_jogador = NULL;
                }
                if (cartas_empate_cpu != NULL) {
                    free(cartas_empate_cpu);
                    cartas_empate_cpu = NULL;
                }
                estadoAtual = NOVO_BARALHO;
                break;
            }

            case NOVO_BARALHO: {
                int *cartas_usadas = NULL;
                int size_cartas_usadas = 0;

                baralho_jogador = get_baralho(cartas, size_cartas, quant_cartas_baralho, &size_cartas_usadas, &cartas_usadas);
                baralho_cpu = get_baralho(cartas, size_cartas, quant_cartas_baralho, &size_cartas_usadas, &cartas_usadas);
                
                /* printf("\nCartas jogador:\n");
                listar_cartas(baralho_jogador, quant_cartas_baralho);
                printf("\nCartas CPU:\n");
                listar_cartas(baralho_cpu, quant_cartas_baralho); */
                free(cartas_usadas);

                estadoAtual = TELA_CARREGAMENTO;
                break;
            }

            case TELA_CARREGAMENTO: {
                int largura_barra = 400;
                int altura_barra = 30;
                int pos_x_barra = (SCREEN_WIDTH - largura_barra) / 2;
                int pos_y_barra = SCREEN_HEIGHT - 100;
                int progresso = 0;
                int tempo_carregamento = GetRandomValue(2, 3) * FPS; // de 2 a 3 segundos
                Rectangle barra_progresso = {pos_x_barra, pos_y_barra, (largura_barra * progresso) / tempo_carregamento, altura_barra};

                while (progresso < tempo_carregamento && !WindowShouldClose())
                {
                    progresso++;

                    ClearBackground((Color){30, 30, 30, 255}); // fundo da janela
                    DrawText("Super Trunfo", SCREEN_WIDTH / 2 - MeasureText("Super Trunfo", 40) / 2, 50, 40, RAYWHITE);
                    DrawText("Liga da Justiça", SCREEN_WIDTH / 2 - MeasureText("Liga da Justiça", 20) / 2, 100, 20, RAYWHITE);

                    DrawText("Embaralhando...", SCREEN_WIDTH / 2 - MeasureText("Embaralhando...", 20) / 2, pos_y_barra - 40, 20, RAYWHITE);

                    DrawRectangleRounded(barra_progresso, 0.3, 0, (Color){251, 59, 0, 255});  // barra de progresso
                    barra_progresso.width = (largura_barra * progresso) / tempo_carregamento; // incrementando barra de progresso

                    BeginDrawing();
                    EndDrawing();
                }
                estadoAtual = TELA_PLAYER_ESCOLHENDO_ATRIBUTO;
                break;
            }

            case TELA_PLAYER_ESCOLHENDO_ATRIBUTO: {
                carta_jogador = baralho_jogador[0];
                carta_cpu = baralho_cpu[0];

                tempo_inicial = GetTime();

                maior_menor=maior_menor_selecionado;

                // placar
                DrawText(TextFormat("Jogador:%10d", quant_cartas_jogador), 10, 10, 20, BLUE);
                DrawText(TextFormat("CPU:%15d", quant_cartas_cpu), 10, 40, 20, RED);

                // carta do jogador
                carta_x = (SCREEN_WIDTH - 300) / 2;
                carta_y = (SCREEN_HEIGHT - 400) / 2;

                hue += 0.001f;
                if (hue > 1.0f) hue = 0.0f;

                Color cor = ColorFromHSV(hue * 360.0f, 1.0f, 1.0f);
                
                // CARTA COM BORDA !!!!!!!!!!!!!!!!!!!!!!!!!
                Rectangle rec= {carta_x-5, carta_y-5, 305, 420};
                if(carta_jogador.super_trunfo){
                    DrawRectangleRounded(rec, 0.1, 10, cor);
                }else{
                    DrawRectangleRounded(rec, 0.1, 10, LIGHTGRAY);
                }

                // desenhando círculo com borda
                raio_circulo = 25;
                circulo_x = carta_x + raio_circulo + 5;
                circulo_y = carta_y + raio_circulo + 5;

                //CIRCULO COM A BORDA CERTA
                if (carta_jogador.super_trunfo){
                    DrawCircle(circulo_x, circulo_y, raio_circulo+3.5, BLACK);
                    DrawCircle(circulo_x, circulo_y, raio_circulo, LIGHTGRAY);
                }else{
                    DrawCircle(circulo_x, circulo_y, raio_circulo+3.5, RED);
                    DrawCircle(circulo_x, circulo_y, raio_circulo, LIGHTGRAY);
                }
                

                // Texto da letra e número da carta
                char texto[10];
                snprintf(texto, sizeof(texto), "%c%d", carta_jogador.letra, carta_jogador.num);
                int texto_largura = MeasureText(texto, 20);
                int texto_altura = 20;
                DrawText(texto, circulo_x - texto_largura / 2, circulo_y - texto_altura / 2, 20, BLACK);

                int nome_largura = MeasureText(carta_jogador.nome, 20);
                DrawText(carta_jogador.nome, carta_x + (300 - nome_largura) / 2, carta_y + 100, 20, BLACK);

                // atributos da carta
                int valores[] = {carta_jogador.forca, carta_jogador.habilidade, carta_jogador.velocidade, carta_jogador.poderes, carta_jogador.poder_cura};
                for (int i = 0; i < 5; i++)
                {
                    atributo_x = carta_x + 10;
                    atributo_y = carta_y + 150 + i * 50;
                    Rectangle retangulo_atributo = {atributo_x, atributo_y, 200, 30};

                    if (CheckCollisionPointRec(GetMousePosition(), retangulo_atributo) || IsKeyDown(  KEY_ONE + i))
                    { // verificando se o mouse est? em cima do atributo
                        DrawRectangle(atributo_x-2.5, atributo_y-2.5, 205, 35, BLACK);
                        DrawRectangleRec(retangulo_atributo, GRAY);

                        if (i == atributo){
                            DrawRectangle(atributo_x-2.5, atributo_y-2.5, 205, 35, BLACK);
                            DrawRectangleRec(retangulo_atributo, GRAY);
                        }
                        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) || IsKeyPressed(KEY_ONE + i))
                        {                                                                                   // se o botao do mouse for clicado
                            int atributo_y_selecionado_anterior = carta_y + 150 + atributo * 50;            // pegando o retangulo selecionado antes
                            DrawRectangleLines(atributo_x, atributo_y_selecionado_anterior, 200, 30, GRAY); // apagando a borda
                            DrawRectangle(atributo_x-2.5, atributo_y-2.5, 205, 35, BLACK);                 // desenhando a borda no novo atributo selecionado
                            atributo = i;                                                                   // indice do atributo escolhido
                            atributo_nome = atributos[i];                                                   // nome do atributo escolhido
                            PlaySound(som_atributos);
                        }
                    }
                    else
                    {
                        DrawRectangleRec(retangulo_atributo, DARKGRAY);
                        if (i == atributo){
                            DrawRectangle(atributo_x-2.5, atributo_y-2.5, 205, 35, RED);
                            DrawRectangleRec(retangulo_atributo, GRAY);
                        }
                    }
                    DrawText(atributos[i], atributo_x + 5, atributo_y + 5, 20, RAYWHITE);
                    DrawText(TextFormat("%d", valores[i]), atributo_x + 160, atributo_y + 5, 20, RAYWHITE);                              
                }

                int maior_x = SCREEN_WIDTH - 150, maior_y = SCREEN_HEIGHT - 180;
                int menor_x = SCREEN_WIDTH - 150, menor_y = SCREEN_HEIGHT - 130;
                Rectangle maiorRect = {maior_x, maior_y, 100, 30};
                Rectangle menorRect = {menor_x, menor_y, 100, 30};

                // BOTAO MAIOR
                if (CheckCollisionPointRec(GetMousePosition(), maiorRect) || IsKeyDown(KEY_UP))
                {
                    DrawRectangle(maior_x-2, maior_y-2, 105, 35, RED);
                    DrawRectangleRec(maiorRect, GRAY);

                    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) || IsKeyPressed(KEY_UP))
                    {
                        PlaySound(som_resto);
                        if (!maior_menor_selecionado)
                        {
                            DrawRectangleLines(menor_x, menor_y, 100, 30, GRAY); // apagando a borda do outro
                            maior_menor_selecionado = true;
                        }
                        maior_menor = true;
                    }
                }
                else
                {
                    DrawRectangleRec(maiorRect, GRAY);
                    if (maior_menor_selecionado){
                        DrawRectangle(maior_x-2, maior_y-2, 105, 35, RED);
                        DrawRectangleRec(maiorRect, GRAY);
                    }
                }
                DrawText("Maior", SCREEN_WIDTH - 140, SCREEN_HEIGHT - 175, 20, RAYWHITE);

                // BOTAO MENOR
                if (CheckCollisionPointRec(GetMousePosition(), menorRect) || IsKeyDown(KEY_DOWN))
                {
                    DrawRectangle(menor_x-2, menor_y-2, 105, 35, BLUE);
                    DrawRectangleRec(menorRect, GRAY);

                    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) || IsKeyPressed(KEY_DOWN))
                    {
                        if (maior_menor_selecionado)
                        {
                            DrawRectangleLines(maior_x, maior_y, 100, 30, GRAY); // apagando a borda do outro
                            maior_menor_selecionado = false;
                        }
                        PlaySound(som_resto);
                        maior_menor = false;
                    }
                }else{
                    DrawRectangleRec(menorRect, GRAY);
                    if (!maior_menor_selecionado){
                        DrawRectangle(menor_x-2, menor_y-2, 105, 35, BLUE);
                        DrawRectangleRec(menorRect, GRAY);
                    }
                }
                DrawText("Menor", SCREEN_WIDTH - 140, SCREEN_HEIGHT - 125, 20, RAYWHITE);

                // BOTAO JOGAR
                Rectangle retangulo_jogar = {SCREEN_WIDTH - 150, SCREEN_HEIGHT - 80, 100, 50};
                if (CheckCollisionPointRec(GetMousePosition(), retangulo_jogar) || IsKeyDown(KEY_ENTER) ){ // se o mouse tiver em cima do botao
                    DrawRectangleRec(retangulo_jogar, DARKGREEN);
                    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) || IsKeyPressed(KEY_ENTER) ){ // se o botao for clicado
                        PlaySound(som_resto);
                        atributo+=1; // o enum dos atributos começa com 1
                        estadoAtual = VERIFICANDO_GANHADOR;
                    }
                }
                else{
                    DrawRectangleRec(retangulo_jogar, (Color){79, 79, 79, 255});
                }
                DrawText("Jogar", SCREEN_WIDTH - 130, SCREEN_HEIGHT - 65, 20, RAYWHITE);

                DrawText(TextFormat("Rodada: %d", rodada), 10, SCREEN_HEIGHT - 30, 20, RAYWHITE);

                break;
            }

            case TELA_CPU_ESCOLHENDO_ATRIBUTO: {
                carta_jogador = baralho_jogador[0];
                carta_cpu = baralho_cpu[0];


                srand(time(NULL));
                atributo = rand() % 5 + 1; // +1 para n�o pegar 0
                maior_menor = rand() % 2;

                // definição das vari�veis
                int progresso_cpu = 0;
                int tempo_carregamento_cpu = GetRandomValue( 3 ,4 ) * FPS; // de 1 a 2 segundos
                int largura_barra_cpu = 400;
                int altura_barra_cpu = 20;
                int pos_x_barra_cpu = (SCREEN_WIDTH - largura_barra_cpu) / 2;
                int pos_y_barra_cpu = 550;
                Rectangle barra_progresso_cpu = { pos_x_barra_cpu, pos_y_barra_cpu, 0, altura_barra_cpu };

                while (progresso_cpu < tempo_carregamento_cpu && !WindowShouldClose()) {
                    progresso_cpu++;

                    BeginDrawing();
                    ClearBackground((Color){30, 30, 30, 255}); // fundo da janela

                    // Texto "Vez da CPU" 
                    DrawText("Vez da CPU", (SCREEN_WIDTH - 120) - MeasureText("Vez da CPU", 40) / 2, 20, 35, RED);

                    // Texto "CPU escolhendo atributo..."
                    DrawText("CPU escolhendo atributo...", (SCREEN_WIDTH / 2) - MeasureText("CPU escolhendo atributo...", 20) / 2, pos_y_barra_cpu - 30, 20, RAYWHITE);

                    // Barra de progresso
                    barra_progresso_cpu.width = (largura_barra_cpu * progresso_cpu) / tempo_carregamento_cpu; // incrementando barra de progresso
                    DrawRectangleRounded(barra_progresso_cpu, 0.3, 0, (Color){251, 59, 0, 255});  // barra de progresso

                    // Placar
                    DrawText(TextFormat("Jogador:%10d", quant_cartas_jogador), 10, 10, 20, BLUE);
                    DrawText(TextFormat("CPU:%15d", quant_cartas_cpu), 10, 40, 20, RED);

                    // Carta do jogador
                    //int carta_x = (SCREEN_WIDTH - 300) / 2;
                    //int carta_y = (SCREEN_HEIGHT - 400) / 2;


                    // carta do jogador
                    carta_x = (SCREEN_WIDTH - 300) / 2;
                    carta_y = (SCREEN_HEIGHT - 400) / 2;

                    hue += 0.001f;
                    if (hue > 1.0f) hue = 0.0f;

                    Color cor = ColorFromHSV(hue * 360.0f, 1.0f, 1.0f);
                    
                    // CARTA COM BORDA !!!!!!!!!!!!!!!!!!!!!!!!!
                    Rectangle rec= {carta_x-5, carta_y-5, 305, 420};
                    if(carta_jogador.super_trunfo){
                        DrawRectangleRounded(rec, 0.1, 10, cor);
                    }else{
                        DrawRectangleRounded(rec, 0.1, 10, LIGHTGRAY);
                    }

                    // desenhando círculo com borda
                    raio_circulo = 25;
                    circulo_x = carta_x + raio_circulo + 5;
                    circulo_y = carta_y + raio_circulo + 5;

                    //CIRCULO COM A BORDA CERTA
                    if (carta_jogador.super_trunfo){
                        DrawCircle(circulo_x, circulo_y, raio_circulo+3.5, BLACK);
                        DrawCircle(circulo_x, circulo_y, raio_circulo, LIGHTGRAY);
                    }else{
                        DrawCircle(circulo_x, circulo_y, raio_circulo+3.5, RED);
                        DrawCircle(circulo_x, circulo_y, raio_circulo, LIGHTGRAY);
                    }

                    // Texto da letra e número da carta
                    char texto[10];
                    snprintf(texto, sizeof(texto), "%c%d", carta_jogador.letra, carta_jogador.num);
                    int texto_largura = MeasureText(texto, 20);
                    int texto_altura = 20;
                    DrawText(texto, circulo_x - texto_largura/2, circulo_y - texto_altura/2, 20, BLACK);

                    int nome_largura = MeasureText(carta_jogador.nome, 20);
                    DrawText(carta_jogador.nome, carta_x + (300 - nome_largura) / 2, carta_y + 100, 20, BLACK);

                    // Atributos da carta
                    int valores[] = {carta_jogador.forca, carta_jogador.habilidade, carta_jogador.velocidade, carta_jogador.poderes, carta_jogador.poder_cura};

                    for (int i = 0; i < 5; i++) {
                        Rectangle retangulo_atributo = {carta_x + 10, carta_y + 150 + i * 50, 200, 30};
                        DrawRectangleRec(retangulo_atributo, DARKGRAY);
                        DrawText(atributos[i], carta_x + 20, carta_y + 155 + i * 50, 20, RAYWHITE);
                        DrawText(TextFormat("%d", valores[i]), carta_x + 180, carta_y + 155 + i * 50, 20, RAYWHITE);
                    }
                    //rodadas
                    DrawText(TextFormat("Rodada: %d", rodada), 10, SCREEN_HEIGHT - 30, 20, RAYWHITE);

                    EndDrawing();
                }

                tempo_inicial = GetTime();
                estadoAtual = VERIFICANDO_GANHADOR;
                break;
            }

            case VERIFICANDO_GANHADOR: {
                if(maior_menor) quem_ganhou = verifica_maior(&carta_jogador, &carta_cpu, atributo);
                else quem_ganhou = verifica_menor(&carta_jogador, &carta_cpu, atributo);
                
                if(quem_ganhou == -1){ // empate
                    printf("\nEMPATE\n");
                    empates++;
                    lidar_com_empate(&baralho_jogador, &quant_cartas_jogador, &baralho_cpu, &quant_cartas_cpu, &cartas_empate_jogador, &cartas_empate_cpu, &quant_cartas_empate);
                }else if(quem_ganhou){ // jogador ganhou
                    printf("\nGANHOU\n");
                    vitorias++;
                    // adicionando as cartas de empate ao baralho do jogador
                    if(quant_cartas_empate > 0){
                        for (int i = 0; i < quant_cartas_empate; i++) {
                            adicionar_carta_vencedor(&baralho_jogador, &cartas_empate_jogador, &quant_cartas_jogador, &quant_cartas_empate);
                            adicionar_carta_vencedor(&baralho_jogador, &cartas_empate_cpu, &quant_cartas_jogador, &quant_cartas_empate);
                        }
                        quant_cartas_empate = 0;
                        if (cartas_empate_jogador != NULL) {
                            free(cartas_empate_jogador);
                            cartas_empate_jogador = NULL;
                        }
                        if (cartas_empate_cpu != NULL) {
                            free(cartas_empate_cpu);
                            cartas_empate_cpu = NULL;
                        }
                    }
                    adicionar_carta_vencedor(&baralho_jogador, &baralho_cpu, &quant_cartas_jogador, &quant_cartas_cpu);
                }else{                // CPU ganhou
                    printf("\nPERDEU\n");
                    // adicionando as cartas de empate ao baralho do CPU
                    if(quant_cartas_empate > 0){
                        for (int i = 0; i < quant_cartas_empate; i++) {
                            adicionar_carta_vencedor(&baralho_cpu, &cartas_empate_jogador, &quant_cartas_cpu, &quant_cartas_empate);
                            adicionar_carta_vencedor(&baralho_cpu, &cartas_empate_cpu, &quant_cartas_cpu, &quant_cartas_empate);
                        }
                        quant_cartas_empate = 0;
                        if (cartas_empate_jogador != NULL) {
                            free(cartas_empate_jogador);
                            cartas_empate_jogador = NULL;
                        }
                        if (cartas_empate_cpu != NULL) {
                            free(cartas_empate_cpu);
                            cartas_empate_cpu = NULL;
                        }
                    }
                    adicionar_carta_vencedor(&baralho_cpu, &baralho_jogador, &quant_cartas_cpu, &quant_cartas_jogador);
                }
                
                atributo-=1; // voltando o valor do atributo para índice
                estadoAtual = TELA_RESULTADO;
                break;
            }

            case TELA_RESULTADO: {
                // ------------------------------------- CARTA DO JOGADOR -------------------------------------
                carta_x = (SCREEN_WIDTH - 700) / 2;
                carta_y = (SCREEN_HEIGHT - 500) / 2;

                hue += 0.001f;
                if (hue > 1.0f) hue = 0.0f;

                Color cor = ColorFromHSV(hue * 360.0f, 1.0f, 1.0f);
                
                // CARTA COM BORDA !!!!!!!!!!!!!!!!!!!!!!!!!
                DrawText("Sua carta", carta_x + 100, (SCREEN_HEIGHT - 550) / 2, 20, DARKGREEN);
                Rectangle rec= {carta_x-5, carta_y-5, 305, 420};
                if(carta_jogador.super_trunfo){
                    DrawRectangleRounded(rec, 0.1, 10, cor);
                }else{
                    DrawRectangleRounded(rec, 0.1, 10, LIGHTGRAY);
                }

                // desenhando círculo com borda
                raio_circulo = 25;
                circulo_x = carta_x + raio_circulo + 5;
                circulo_y = carta_y + raio_circulo + 5;

                //CIRCULO COM A BORDA CERTA
                if (carta_jogador.super_trunfo){
                    DrawCircle(circulo_x, circulo_y, raio_circulo+3.5, BLACK);
                    DrawCircle(circulo_x, circulo_y, raio_circulo, LIGHTGRAY);
                }else{
                    DrawCircle(circulo_x, circulo_y, raio_circulo+3.5, RED);
                    DrawCircle(circulo_x, circulo_y, raio_circulo, LIGHTGRAY);
                }

                // texto da letra e numero da carta
                char texto[10];
                snprintf(texto, sizeof(texto), "%c%d", carta_jogador.letra, carta_jogador.num);
                int texto_largura = MeasureText(texto, 20);
                int texto_altura = 20;
                DrawText(texto, circulo_x - texto_largura/2, circulo_y - texto_altura/2, 20, BLACK);

                // nome da carta
                int nome_largura = MeasureText(carta_jogador.nome, 20);
                DrawText(carta_jogador.nome, carta_x + (300 - nome_largura) / 2, carta_y + 100, 20, BLACK);

                // atributos da carta
                int valores[] = {carta_jogador.forca, carta_jogador.habilidade, carta_jogador.velocidade, carta_jogador.poderes, carta_jogador.poder_cura};

                for (int i = 0; i < 5; i++) {
                    Rectangle retangulo_atributo = {carta_x + 10, carta_y + 150 + i * 50, 200, 30};

                    if (i == atributo) {
                        if (quem_ganhou == -1) {
                            DrawRectangle(carta_x + 10-2.5, carta_y + 150 + i * 50-2.5, 205, 35, YELLOW);
                            DrawRectangleRec(retangulo_atributo, DARKGRAY);
                        } else if (quem_ganhou == 0) {
                            DrawRectangleRec(retangulo_atributo, RED);
                        } else if (quem_ganhou == 1) {
                            DrawRectangleRec(retangulo_atributo, GREEN);
                        }
                    }else DrawRectangleRec(retangulo_atributo, DARKGRAY);

                    DrawText(atributos[i], carta_x + 20, carta_y + 155 + i * 50, 20, RAYWHITE);
                    DrawText(TextFormat("%d", valores[i]), carta_x + 180, carta_y + 155 + i * 50, 20, RAYWHITE);
                }

                // DESENHANDO UM X ENTRE AS DUAS CARTAS
                DrawText("x", (SCREEN_WIDTH - MeasureText("X", 64)) / 2, (SCREEN_HEIGHT - 100 - MeasureText("X", 64)) / 2, 64, RAYWHITE);

                // --------------------------------------- CARTA DA CPU ----------------------------------------
                carta_x = (SCREEN_WIDTH - 350);
                carta_y = (SCREEN_HEIGHT - 500) / 2;
                
                DrawText("Carta CPU", carta_x + 100, (SCREEN_HEIGHT - 550) / 2, 20, (Color){ 156, 0, 0, 255 }); // #9C0000

                // CARTA COM BORDA !!!!!!!!!!!!!!!!!!!!!!!!!
                Rectangle rec1= {carta_x-5, carta_y-5, 305, 420};
                if(carta_cpu.super_trunfo){
                    DrawRectangleRounded(rec1, 0.1, 10, cor);
                }else{
                    DrawRectangleRounded(rec1, 0.1, 10, (Color){ 167, 167, 167, 255 });
                }

                // desenhando círculo com borda
                raio_circulo = 25;
                circulo_x = carta_x + raio_circulo + 5;
                circulo_y = carta_y + raio_circulo + 5;

                //CIRCULO COM A BORDA CERTA
                if (carta_cpu.super_trunfo){
                    DrawCircle(circulo_x, circulo_y, raio_circulo+3.5, BLACK);
                    DrawCircle(circulo_x, circulo_y, raio_circulo, LIGHTGRAY);
                }else{
                    DrawCircle(circulo_x, circulo_y, raio_circulo+3.5, RED);
                    DrawCircle(circulo_x, circulo_y, raio_circulo, LIGHTGRAY);
                }
                // texto da letra e numero da carta
                snprintf(texto, sizeof(texto), "%c%d", carta_cpu.letra, carta_cpu.num);
                texto_largura = MeasureText(texto, 20);
                texto_altura = 20;
                DrawText(texto, circulo_x - texto_largura/2, circulo_y - texto_altura/2, 20, BLACK);

                // nome da carta
                nome_largura = MeasureText(carta_cpu.nome, 20);
                DrawText(carta_cpu.nome, carta_x + (300 - nome_largura) / 2, carta_y + 100, 20, BLACK);

                // atributos da carta
                int valores_cpu[] = {carta_cpu.forca, carta_cpu.habilidade, carta_cpu.velocidade, carta_cpu.poderes, carta_cpu.poder_cura};

                for (int i = 0; i < 5; i++) {
                    Rectangle retangulo_atributo = {carta_x + 10, carta_y + 150 + i * 50, 200, 30};
                    if (i == atributo) {
                        if (quem_ganhou == -1) {
                            DrawRectangle(carta_x + 10-2.5, carta_y + 150 + i * 50-2.5, 205, 35, YELLOW);
                            DrawRectangleRec(retangulo_atributo, DARKGRAY);
                        } else if (quem_ganhou == 0) {
                            DrawRectangleRec(retangulo_atributo, GREEN);
                        } else if (quem_ganhou == 1) {
                            DrawRectangleRec(retangulo_atributo, RED);
                        }
                    }else DrawRectangleRec(retangulo_atributo, DARKGRAY);

                    DrawText(atributos[i], carta_x + 20, carta_y + 155 + i * 50, 20, RAYWHITE);
                    DrawText(TextFormat("%d", valores_cpu[i]), carta_x + 180, carta_y + 155 + i * 50, 20, RAYWHITE);
                }

                // --------------------------------------- BARRAS DE VIDA ----------------------------------------
                float proporcao_jogador = (float)quant_cartas_jogador / (2 * quant_cartas_baralho);
                float proporcao_cpu = (float)quant_cartas_cpu / (2 * quant_cartas_baralho);

                int largura_barra_dentro = 300;  // largura da barra de dentro
                int largura_maxima_barra = 280; // largura máxima da barra de vida

                // largura das barras baseada nas proporções
                int largura_barra_jogador = largura_maxima_barra * proporcao_jogador;
                int largura_barra_cpu = largura_maxima_barra * proporcao_cpu;

                DrawRectangle(50, 470, largura_barra_dentro, 20, WHITE); // barra de dentro
                DrawRectangle(60, 475, largura_barra_jogador, 10, GREEN); // barra do jogador

                DrawRectangle(SCREEN_WIDTH-350, 470, largura_barra_dentro, 20, WHITE); // barra de dentro                
                DrawRectangle(SCREEN_WIDTH-340, 475, largura_barra_cpu, 10, RED); // barra da CPU
                
                // TEXTO DAS BARRAS
                char texto_quant_jogador[20];
                char texto_quant_cpu[20];

                sprintf(texto_quant_jogador, "%d cartas", quant_cartas_jogador);
                sprintf(texto_quant_cpu, "%d cartas", quant_cartas_cpu);

                int texto_jogador_largura = MeasureText(texto_quant_jogador, 16);
                int texto_cpu_largura = MeasureText(texto_quant_cpu, 16);

                DrawText(texto_quant_jogador, 50 + (300 - texto_jogador_largura) / 2, 500, 16, RAYWHITE); // Texto do jogador
                DrawText(texto_quant_cpu, SCREEN_WIDTH - 350 + (300 - texto_cpu_largura) / 2, 500, 16, RAYWHITE); // Texto da CPU

                // RODADAS
                DrawText(TextFormat("Rodada: %d", rodada), 10, SCREEN_HEIGHT - 30, 20, RAYWHITE);

                // INFORMA��ES DA COMPARA��O
                sprintf(informacao_rodada, "%s - %s", atributo_nome, maior_menor ? "MAIOR" : "MENOR");
                DrawText(informacao_rodada, SCREEN_WIDTH - 10 - MeasureText(informacao_rodada, 20), SCREEN_HEIGHT - 30, 20, RAYWHITE); 

                // botao OK
                Rectangle botaook = {SCREEN_WIDTH / 2 - 50, SCREEN_HEIGHT - 80, 100, 50};
                Color botaocor = CheckCollisionPointRec(GetMousePosition(), botaook) ? DARKGREEN : GREEN;
                DrawRectangleRec(botaook, botaocor);
                DrawRectangleLines(botaook.x, botaook.y, botaook.width, botaook.height, WHITE);
                DrawText("OK", botaook.x + botaook.width / 2 - MeasureText("OK", 20) / 2, botaook.y + botaook.height / 2 - 10, 20, WHITE);

                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) || IsKeyPressed(KEY_ENTER)){
                    PlaySound(som_resto);
                    if (CheckCollisionPointRec(GetMousePosition(), botaook) || IsKeyPressed(KEY_ENTER)){
                        if(quant_cartas_cpu == 0) estadoAtual = JOGADOR_VENCEU;
                        else if(quant_cartas_jogador == 0) estadoAtual = CPU_VENCEU;
                        else {
                            player_joga = !player_joga;
                            rodada++; // NOVA RODADA
                            if(player_joga) estadoAtual = TELA_PLAYER_ESCOLHENDO_ATRIBUTO;
                            else estadoAtual = TELA_CPU_ESCOLHENDO_ATRIBUTO;
                        }
                    }
                }
                
                tempo_contagem_regressiva = tempo_inicial + 30; // 30 segundos
                tempo_agora = GetTime();
                
                if (tempo_agora >= tempo_contagem_regressiva){
                    if(quant_cartas_cpu == 0) estadoAtual = JOGADOR_VENCEU;
                    else if(quant_cartas_jogador == 0) estadoAtual = CPU_VENCEU;
                    else {
                        player_joga = !player_joga;
                        rodada++; // NOVA RODADA
                        if(player_joga) estadoAtual = TELA_PLAYER_ESCOLHENDO_ATRIBUTO;
                        else estadoAtual = TELA_CPU_ESCOLHENDO_ATRIBUTO;
                    }
                }

                DrawText(TextFormat("%d", (tempo_contagem_regressiva - tempo_agora)), 
                    SCREEN_WIDTH / 2 - MeasureText(TextFormat("%d",(tempo_contagem_regressiva - tempo_agora)), 20) / 2,
                    botaook.y + botaook.height + 10, 20, WHITE);
                break;
            }

            case JOGADOR_VENCEU: {
                DrawText("Fim de Jogo", SCREEN_WIDTH / 2 - MeasureText("Fim de Jogo", 50) / 2, SCREEN_HEIGHT / 2 - 250, 50, WHITE);
                DrawText("Voce ganhou", SCREEN_WIDTH / 2 - MeasureText("Voce ganhou", 40) / 2, SCREEN_HEIGHT / 2 - 130, 40, GREEN);
                pontuacao = (10 * ((100 * vitorias) / rodada))+50*empates;
                DrawText(TextFormat("Rodadas: %d", rodada), SCREEN_WIDTH / 2 - MeasureText(TextFormat("Rodadas: %d", rodada), 30) / 2, SCREEN_HEIGHT / 2 - 30, 30, WHITE);
                DrawText(TextFormat("Vitórias: %d", vitorias), SCREEN_WIDTH / 2 - MeasureText(TextFormat("Vit�rias: %d", vitorias), 30) / 2, SCREEN_HEIGHT / 2, 30, WHITE);
                DrawText(TextFormat("Empates: %d", empates), SCREEN_WIDTH / 2 - MeasureText(TextFormat("Empates: %d", empates), 30) / 2, SCREEN_HEIGHT / 2 + 30, 30, WHITE);
                DrawText(TextFormat("Pontuação: %d", pontuacao), SCREEN_WIDTH / 2 - MeasureText(TextFormat("Pontua��o: %d", pontuacao), 30) / 2, SCREEN_HEIGHT / 2 + 60, 30, WHITE);
                // botao
                Rectangle botaook = {SCREEN_WIDTH / 2 - 50, SCREEN_HEIGHT / 2 + 180, 100, 50};
                Color botaocor = CheckCollisionPointRec(GetMousePosition(), botaook) ? DARKGREEN : GREEN;
                DrawRectangleRec(botaook, botaocor);
                DrawText("OK", botaook.x + botaook.width / 2 - MeasureText("OK", 20) / 2, botaook.y + botaook.height / 2 - 10, 20, WHITE);

                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) || IsKeyPressed(KEY_ENTER)){
                    if (CheckCollisionPointRec(GetMousePosition(), botaook) || IsKeyPressed(KEY_ENTER)){
                        PlaySound(som_resto);
                        estadoAtual = NICKNAME;
                    }
                }
                
                //strcpy(partidaHist.vencedor, "CPU");
                strcpy(partidaHist.vencedor, "jogador"); //vai ficar "jogador" enquanto n�o tiver a tela para nickname do ranking
                partidaHist.rodadas = rodada;
                partidaHist.vitorias = vitorias;
                partidaHist.empates = empates;
                partidaHist.pontuacao = pontuacao;

                //pegando a data atual usando time.h
                time_t t = time(NULL);
                struct tm tm = *localtime(&t);
                partidaHist.dia=tm.tm_mday;
                partidaHist.mes=tm.tm_mon+1;

                break;
            }

            case CPU_VENCEU: {
                DrawText("Fim de Jogo", SCREEN_WIDTH / 2 - MeasureText("Fim de Jogo", 50) / 2, SCREEN_HEIGHT / 2 - 250, 50, WHITE);
                DrawText("CPU ganhou", SCREEN_WIDTH / 2 - MeasureText("Voce ganhou", 40) / 2, SCREEN_HEIGHT / 2 - 130, 40, RED);
                pontuacao = 10 * ((100 * vitorias) / rodada);
                DrawText(TextFormat("Rodadas: %d", rodada), SCREEN_WIDTH / 2 - MeasureText(TextFormat("Rodadas: %d", rodada), 30) / 2, SCREEN_HEIGHT / 2 - 30, 30, WHITE);
                DrawText(TextFormat("Vitórias: %d", vitorias), SCREEN_WIDTH / 2 - MeasureText(TextFormat("Vit�rias: %d", vitorias), 30) / 2, SCREEN_HEIGHT / 2, 30, WHITE);
                DrawText(TextFormat("Empates: %d", empates), SCREEN_WIDTH / 2 - MeasureText(TextFormat("Empates: %d", empates), 30) / 2, SCREEN_HEIGHT / 2 + 30, 30, WHITE);
                DrawText(TextFormat("Pontuação: %d", pontuacao), SCREEN_WIDTH / 2 - MeasureText(TextFormat("Pontua��o: %d", pontuacao), 30) / 2, SCREEN_HEIGHT / 2 + 60, 30, WHITE);
                // botao
                Rectangle botaook = {SCREEN_WIDTH / 2 - 50, SCREEN_HEIGHT / 2 + 180, 100, 50};
                Color DARKRED = (Color){139, 0, 0, 255};
                Color botaocor = CheckCollisionPointRec(GetMousePosition(), botaook) ? DARKRED : RED;
                DrawRectangleRec(botaook, botaocor);
                DrawText("OK", botaook.x + botaook.width / 2 - MeasureText("OK", 20) / 2, botaook.y + botaook.height / 2 - 10, 20, WHITE);

                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) || IsKeyPressed(KEY_ENTER)){
                    if (CheckCollisionPointRec(GetMousePosition(), botaook) || IsKeyPressed(KEY_ENTER)){
                        PlaySound(som_resto);
                        estadoAtual = JOGAR_NOVAMENTE;
                    }
                }

                strcpy(partidaHist.vencedor, "CPU");
                partidaHist.rodadas = rodada;
                partidaHist.vitorias = vitorias;
                partidaHist.empates = empates;
                partidaHist.pontuacao = pontuacao;
                //pegando a data atual usando time.h
                time_t t = time(NULL);
                struct tm tm = *localtime(&t);
                partidaHist.dia=tm.tm_mday;
                partidaHist.mes=tm.tm_mon+1;


                break;
            }

            case NICKNAME: {
                DrawText("Digite seu nickname", SCREEN_WIDTH / 2 - MeasureText("Digite seu nickname", 50) / 2, SCREEN_HEIGHT / 2 - 80, 50, WHITE);
                DrawText("Nickname (3 letras):", SCREEN_WIDTH / 2 - MeasureText("Nickname (3 letras):", 30) / 2, SCREEN_HEIGHT / 2 - 20, 30, WHITE);

                //caixas da letra
                int larguracaixa = 50;
                int alturacaixa = 50;
                int comecoX = (SCREEN_WIDTH - (3 * larguracaixa + 20)) / 2;
                for (int i = 0; i < 3; i++) {
                    DrawRectangle(comecoX + i * (larguracaixa + 10), SCREEN_HEIGHT / 2 + 20, larguracaixa, alturacaixa, WHITE);
                    if (i < strlen(nickname)) {
                        DrawText(TextFormat("%c", nickname[i]), comecoX + i * (larguracaixa + 10) + larguracaixa / 2 - MeasureText(TextFormat("%c", nickname[i]), 40) / 2, SCREEN_HEIGHT / 2 + 25, 40, BLACK);
                    }
                }

                int key = GetKeyPressed();
                if (key >= 32 && key <= 125 && strlen(nickname) < 3) {
                    PlaySound(som_tecla);
                    int len = strlen(nickname);
                    nickname[len] = (char)key;
                    nickname[len + 1] = '\0';
                }
                if (IsKeyPressed(KEY_BACKSPACE) && strlen(nickname) > 0) {
                    nickname[strlen(nickname) - 1] = '\0';
                }

                // botao ok
                Rectangle botaook = {SCREEN_WIDTH / 2 - 50, SCREEN_HEIGHT / 2 + 120, 100, 50};
                Color botaocor = CheckCollisionPointRec(GetMousePosition(), botaook) ? DARKGREEN : GREEN;
                DrawRectangleRec(botaook, botaocor);
                DrawText("OK", botaook.x + botaook.width / 2 - MeasureText("OK", 30) / 2, botaook.y + botaook.height / 2 - 15, 30, WHITE);

                static bool nickinvalido = false;
                static double nickinvalidoTempo = 0;

                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) || IsKeyPressed(KEY_ENTER)){
                    if (CheckCollisionPointRec(GetMousePosition(), botaook) || IsKeyPressed(KEY_ENTER)){
                        PlaySound(som_resto);
                        if (strlen(nickname) < 3) {
                            nickinvalido = true;
                            nickinvalidoTempo = GetTime();
                        } else {
                            strncpy(partidaHist.vencedor, nickname, sizeof(partidaHist.vencedor) - 1);
                            partidaHist.vencedor[sizeof(partidaHist.vencedor) - 1] = '\0';

                            FILE *historico = fopen("assets/data/historico.dat", "rb+");
                            if (historico == NULL){
                                historico = fopen("assets/data/historico.dat", "wb+");
                                if (historico == NULL)
                                {
                                    perror("\n\033[1;91mErro ao criar historico.dat\033[m");
                                    exit(1);
                                }
                                fseek(historico, 0, SEEK_END);
                                fwrite(&partidaHist, sizeof(Historico), 1, historico);
                                fclose(historico);
                            }
                            else{
                                fseek(historico, 0, SEEK_END);
                                fwrite(&partidaHist, sizeof(Historico), 1, historico);
                                fclose(historico);
                            }
                            estadoAtual = RANKING;
                        }
                    }
                }

                if (nickinvalido && GetTime() - nickinvalidoTempo < 4) {
                    DrawText("Nickname inválido! Deve ter exatamente 3 caracteres.", SCREEN_WIDTH / 2 - MeasureText("Nickname inválido! Deve ter exatamente 3 caracteres.", 20) / 2, SCREEN_HEIGHT / 2 + 90, 20, RED);
                } else {
                    nickinvalido = false;
                }

                break;
            }
            
            case RANKING: {
                DrawText("Ranking", SCREEN_WIDTH / 2 - MeasureText("Ranking", 50) / 2, 20, 50, WHITE);
                DrawText("Nickname", 50, 100, 40, WHITE);
                DrawText("Pontos", 315, 100, 40, WHITE);
                DrawText("Data", 500, 100, 40, WHITE);

                FILE *historico = fopen("assets/data/historico.dat", "rb");
                if (historico == NULL) {
                    DrawText("Erro ao abrir o arquivo de histórico.", 50, 150, 20, RED);
                } else {
                    Historico historicos[100];
                    int aux = 0;
                    while (fread(&historicos[aux], sizeof(Historico), 1, historico) && aux < 100) {
                        aux++;
                    }
                    fclose(historico);

                    //ordenar
                    for (int i = 0; i < aux - 1; i++) {
                        for (int j = i + 1; j < aux; j++) {
                            if (historicos[i].pontuacao < historicos[j].pontuacao) {
                                Historico temp = historicos[i];
                                historicos[i] = historicos[j];
                                historicos[j] = temp;
                            }
                        }
                    }

                    bool top5 = false;
                    for (int i = 0; i < aux && i < 5; i++) {
                        Color cor;
                        if (i == 0) cor = GOLD;
                        else if (i == 1) cor = SILVER;
                        else if (i == 2) cor = BRONZE;
                        else cor = WHITE;

                        DrawText(historicos[i].vencedor, 60, 150 + i * 40, 30, cor);
                        DrawText(TextFormat("%d", historicos[i].pontuacao), 325, 150 + i * 40, 30, cor);
                        DrawText(TextFormat("%02d/%02d", historicos[i].dia, historicos[i].mes), 500, 150 + i * 40, 30, cor);

                        if (strcmp(historicos[i].vencedor, partidaHist.vencedor) == 0 && historicos[i].pontuacao == partidaHist.pontuacao) {
                            top5 = true;
                        }
                    }

                    if (!top5) {
                        for (int i = 5; i < aux; i++) {
                            if (strcmp(historicos[i].vencedor, partidaHist.vencedor) == 0 && historicos[i].pontuacao == partidaHist.pontuacao) {
                                DrawText(TextFormat("Sua posição: %d°", i + 1), 50, 350, 30, YELLOW);
                                break;
                            }
                        }
                    }
                }

                Rectangle botaook = {SCREEN_WIDTH / 2 - 50, SCREEN_HEIGHT - 80, 100, 50};
                Color botaocor = CheckCollisionPointRec(GetMousePosition(), botaook) ? DARKGREEN : GREEN;
                DrawRectangleRec(botaook, botaocor);
                DrawText("OK", botaook.x + botaook.width / 2 - MeasureText("OK", 20) / 2, botaook.y + botaook.height / 2 - 10, 20, WHITE);

                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) || IsKeyPressed(KEY_ENTER)) {
                    if (CheckCollisionPointRec(GetMousePosition(), botaook) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                        PlaySound(som_resto);
                        estadoAtual = JOGAR_NOVAMENTE;
                    }else if (IsKeyPressed(KEY_ENTER)){
                        PlaySound(som_resto);
                        estadoAtual = JOGAR_NOVAMENTE;
                    }
                    
                }

                break;
            }
            
            case JOGAR_NOVAMENTE: {

                DrawText("Jogar Novamente?", SCREEN_WIDTH / 2 - MeasureText("Jogar Novamente?", 40) / 2, SCREEN_HEIGHT / 2 - 60, 40, WHITE);

                // botao sim
                Rectangle botaoSim = {SCREEN_WIDTH / 2 - 110, SCREEN_HEIGHT / 2, 100, 50};
                DrawRectangleRec(botaoSim, GREEN);
                DrawText("Sim", botaoSim.x + botaoSim.width / 2 - MeasureText("Sim", 20) / 2, botaoSim.y + botaoSim.height / 2 - 10, 20, RAYWHITE);

                // botao nao
                Rectangle botaoNao = {SCREEN_WIDTH / 2 + 10, SCREEN_HEIGHT / 2, 100, 50};
                DrawRectangleRec(botaoNao, RED);
                DrawText("Não", botaoNao.x + botaoNao.width / 2 - MeasureText("Não", 20) / 2, botaoNao.y + botaoNao.height / 2 - 10, 20, RAYWHITE);

                // verificando se o botão sim foi clicado
                if (CheckCollisionPointRec(GetMousePosition(), botaoSim) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
                    // RESETAR AS VARIAVEIS POR PADRAO AQUI, OU CRIAR UM CASE ANTES DE NOVO BARALHO PRA TRATA
                    PlaySound(som_resto);
                    estadoAtual = RESET;
                }

                // verificando se o botão nao foi clicado
                if (CheckCollisionPointRec(GetMousePosition(), botaoNao) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
                    PlaySound(som_resto);
                    estadoAtual = FECHAR_JANELA;
                }
                break;
            }

            default:
                break;
            }
        if (estadoAtual == FECHAR_JANELA)
            break;
        EndDrawing();
    }


    // parando e descarregando a musica
    /* StopMusicStream(musica_fundo);
    UnloadMusicStream(musica_fundo);
    CloseAudioDevice(); */

    CloseWindow(); // fechando a janela
    return;
} 