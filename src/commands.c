#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include "../lib/commands.h"
#include "../lib/filesystem.h"

void touch(FILE *SA, char *nome)
{
    int bloco_dir_pai = busca_diretorio_pai(SA, "/", nome, BLOCO_ROOT);
    char dir_reduzido[255];
    strcpy(dir_reduzido, nome);
    remove_dirs_nome(dir_reduzido);
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
}

void mkdir(FILE *SA, char *nome)
{
    char dir_pai[255];
    strcpy(dir_pai, nome);
    nome_dir_pai(dir_pai);
    if (add_arquivo_vazio(SA, nome, -1) != -1)
        atualiza_tempos(SA, nome);
}

void rmdir(FILE *SA, char *nome)
{
    char dir_pai[255];
    strcpy(dir_pai, nome);
    nome_dir_pai(dir_pai);
    fprintf(stderr, "Os seguintes arquivos foram apagados:\n");
    if (remove_diretorio(SA, BLOCO_ROOT, nome, 1) != -1)
        atualiza_tempos(SA, dir_pai);
}

void cat(FILE *SA, char *nome)
{
    if (imprime_arquivo(SA, nome) == -1) fprintf(stderr, "%s não foi encontrado\n", nome);

    else atualiza_tempos(SA, nome);
}

void cp(FILE *SA, char *nome_origem, char *nome_destino)
{
    char dir_pai[255];
    strcpy(dir_pai, nome_destino);
    nome_dir_pai(dir_pai);
    add_arquivo(SA, nome_origem, nome_destino);
    atualiza_tempos(SA, dir_pai);
}

void rm(FILE *SA, char *nome)
{
    char dir_pai[255];
    strcpy(dir_pai, nome);
    nome_dir_pai(dir_pai);
    if (remove_arquivo(SA, nome) != -1)
        atualiza_tempos(SA, dir_pai);
}