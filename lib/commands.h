#include <stdio.h>

#ifndef COMMANDS_H
#define COMMANDS_H

void touch(FILE *SA, char *nome);
void mkdir(FILE *SA, char *nome);
void rmdir(FILE *SA, char *nome);
void cat(FILE *SA, char *nome);
void cp(FILE *SA, char *nome_origem, char *nome_destino);
void rm(FILE *SA, char *nome);

#endif