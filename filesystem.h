#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#define N_BLOCOS 25000
#define TAMANHO_BLOCO 4000
#define TAMANHO_METADADOS_ROOT 34
#define TAMANHO_TIME 10
#define BLOCOS_FAT 32
#define BLOCOS_BITMAP 7
#define BLOCO_ROOT 39
//Tamanho das entradas na FAT
#define FAT_ENTRY 5
//Macros do Bitmap (Talvez tenha que inverter, não lembro o que o daniel disse que preferia)
#define LIVRE '1'
#define OCUPADO '0'

#define GETFAT 1
#define SETPOSITIONTOTIME 2

int FAT[N_BLOCOS];
char bitmap[N_BLOCOS];

FILE *mount(char *nome_arquivo);
void cria_FAT(FILE *f);
void carrega_FAT(FILE *f);
void cria_bitmap(FILE *f);
void carrega_bitmap(FILE *f);
void cria_root(FILE *f);
void volta_pro_root(FILE *f);

void preenche_bloco_vazio(FILE *f, int n_bloco, int offset);
void aloca_bloco(FILE *SA, int dir1);
void estende_bloco(FILE *SA, int dir1, int dir2);
int limpa_bloco(FILE *SA, int bloco);

int add_arquivo(FILE *SA, char *nome_origem, char *nome_destino);
int add_arquivo_vazio(FILE *SA, char *nome_destino, int tamanho);
int busca_espaco_metadados(FILE *SA, int dir, int tam_metadados, int *primeiro, int segundo, int blocos_arquivo);

int procura_nome_e_devolve_info(FILE *SA, char *nome, int info, int bloco_dir);
int remove_arquivo(FILE *SA, char *nome);
int remove_diretorio(FILE *SA, int bloco_diretorio_pai, char *diretorio, int primeira_chamada);
void atualiza_tempos(FILE *SA, char *dir_pai);

void lista_itens_diretorio(FILE *SA, char *nome);
int busca_continuacao_dir(FILE *SA, int *bloco_dir, int *cont);
int busca_diretorio_pai(FILE *SA, char *dir_atual, char *nome_atual, int bloco_dir);
int espaco_restante_diretorio(FILE *SA, int *bloco_dir);
int espaco_restante_bloco(FILE *SA, int bloco_dir);
void remove_dirs_nome(char *nome);
void nome_dir_pai(char *nome);

void find(FILE *SA, char *diretorio, char *nome, int primeira_chamada, int bloco_diretorio_pai, char *dir_pai);
void df(FILE *SA, int bloco_dir_atual, int *ndirs, int *narquivos, int primeira_chamada);

void adiciona_metadados_arquivo(FILE *SA, int bloco_dir, char *nome, int tamanho, int bloco_alocado);
void acabou_bloco(FILE *SA, int *bloco_dir);
int calcula_tamanho_metadados(char *nome, int tamanho);
int digitos(int n);

void imprime_arquivo(FILE *SA, char *nome);

time_t rawtime;
time_t currtime;

#endif