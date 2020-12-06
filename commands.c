#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include "filesystem.h"

void touch(FILE *SA, char *nome)
{
    int bloco_dir_pai = busca_diretorio_pai(SA, "/", nome, BLOCO_ROOT);
    char *dir_reduzido = remove_dirs_nome(nome);
    int t = procura_nome_e_devolve_info(SA, dir_reduzido, 2, bloco_dir_pai);
    if (t == -1)
        add_arquivo_vazio(SA, nome, 0);
    //Caso arquivo não exista
    else
    {
        //Aqui, SA aponta para o primeiro byte de tempo de acesso
        time(&rawtime);
        fprintf(SA, "%ld", rawtime);
        fprintf(SA, "%ld", rawtime);
    }
    free(dir_reduzido);
}

void mkdir(FILE *SA, char *nome)
{
    char *dir_pai = nome_dir_pai(nome);
    add_arquivo_vazio(SA, nome, -1);
    atualiza_tempos(SA, dir_pai);
    free(dir_pai);
}

void rmdir(FILE *SA, char *nome)
{
    char *dir_pai = nome_dir_pai(nome);
    remove_diretorio(SA, BLOCO_ROOT, nome, 1);
    atualiza_tempos(SA, dir_pai);
    free(dir_pai);
}

void cat(FILE *SA, char *nome)
{
    imprime_arquivo(SA, nome);
    atualiza_tempos(SA, nome);
}

void cp(FILE *SA, char *nome_origem, char *nome_destino)
{
    char *dir_pai = nome_dir_pai(nome_destino);
    add_arquivo(SA, nome_origem, nome_destino);
    atualiza_tempos(SA, dir_pai);
    free(dir_pai);
}

void rm(FILE *SA, char *nome)
{
    char *dir_pai = nome_dir_pai(nome);
    remove_arquivo(SA, nome);
    atualiza_tempos(SA, dir_pai);
    free(dir_pai);
}