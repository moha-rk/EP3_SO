#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "filesystem.h"
#include "filesystem.c"
#include "commands.c"

#define MAX_PARAMETERS 10
#define MAX_PROMPT_SIZE 10
#define SPACESTR " "
#define END_CHAR '\0'

/*Prototipos de funções de leitura e interpretação de comandos*/

void read_command(char **command, char **parameters);
bool internal_commands(char *command, char **parameters);

char *prompt = "[ep3]: ";
FILE *SA = NULL;

int main(int argc, char **argv)
{
	char *command = NULL;
	char *parameters[MAX_PARAMETERS];

	bool repeat = true;

	while (repeat)
	{
		free(command);

		read_command(&command, parameters);

		if (!internal_commands(command, parameters))
			fprintf(stderr, "Comando não reconhecido\n");
	}
	return 0;
}

void read_command(char **command, char **parameters)
{
	int i;
	char *buf;

	buf = readline(prompt);

	/*Salva entrada no historico de comandos*/
	if (strlen(buf) > 0)
	{
		add_history(buf);
	}

	i = 0;
	while ((parameters[i] = strsep(&buf, SPACESTR)) != NULL)
		i++;

	*command = parameters[0];

	free(buf);
}

bool internal_commands(char *command, char **parameters)
{
	if (strcmp(command, "mount") == 0)
	{
		if (parameters[1] != NULL)
			/*Monta sistema de arquivos*/
			SA = mount(parameters[1]);
		else
			fprintf(stderr, "Chamada incorreta da função mount.\nChamada correta: mount arquivo\n");
		return true;
	}
	if (strcmp(command, "cp") == 0)
	{
		if (parameters[1] != NULL && parameters[2] != NULL && SA != NULL)
			/*copia de parameters[1] sera salva em parameters[2]*/
			//add_arquivo(SA, parameters[1], parameters[2]);
			cp(SA, parameters[1], parameters[2]);
		else if (SA == NULL)
			fprintf(stderr, "Não há um sistema de arquivos montado\n");
		else
			fprintf(stderr, "Chamada incorreta da função cp.\nChamada correta: cp origem destino\n");
		return true;
	}
	if (strcmp(command, "mkdir") == 0)
	{
		if (parameters[1] != NULL && SA != NULL)
			//add_arquivo_vazio(SA, parameters[1], -1);
			mkdir(SA, parameters[1]);
		else if (SA == NULL)
			fprintf(stderr, "Não há um sistema de arquivos montado\n");
		else
			fprintf(stderr, "Chamada incorreta da função mkdir.\nChamada correta: mkdir diretorio\n");
		return true;
	}
	if (strcmp(command, "rmdir") == 0)
	{
		if (parameters[1] != NULL && SA != NULL)
			//remove_diretorio(SA, BLOCO_ROOT, parameters[1], 1);
			rmdir(SA, parameters[1]);
		else if (SA == NULL)
			fprintf(stderr, "Não há um sistema de arquivos montado\n");
		else
			fprintf(stderr, "Chamada incorreta da função rmdir.\nChamada correta: rmdir diretorio\n");
		return true;
	}
	if (strcmp(command, "cat") == 0)
	{
		if (parameters[1] != NULL && SA != NULL)
			//imprime_arquivo(SA, parameters[1]);
			cat(SA, parameters[1]);
		else if (SA == NULL)
			fprintf(stderr, "Não há um sistema de arquivos montado\n");
		else
			fprintf(stderr, "Chamada incorreta da função cat.\nChamada correta: cat arquivo\n");
		return true;
	}
	if (strcmp(command, "touch") == 0)
	{
		//FALTA CHECAR SE O ARQUIVO EXISTE PARA DAR TOUCH
		if (parameters[1] != NULL && SA != NULL)
			//add_arquivo_vazio(SA, parameters[1], 0);
			touch(SA, parameters[1]);
		else if (SA == NULL)
			fprintf(stderr, "Não há um sistema de arquivos montado\n");
		else
			fprintf(stderr, "Chamada incorreta da função touch.\nChamada correta: touch arquivo\n");
		return true;
	}
	if (strcmp(command, "rm") == 0)
	{
		/*remove o arquivo parameters[1]*/
		if (parameters[1] != NULL && SA != NULL)

			rm(SA, parameters[1]);
		else if (SA == NULL)
			fprintf(stderr, "Não há um sistema de arquivos montado\n");
		else
			fprintf(stderr, "Chamada incorreta da função touch.\nChamada correta: rm arquivo\n");
		return true;
	}
	if (strcmp(command, "ls") == 0)
	{
		if (parameters[1] != NULL && SA != NULL)
			lista_itens_diretorio(SA, parameters[1]);
		else if (SA == NULL)
			fprintf(stderr, "Não há um sistema de arquivos montado\n");
		else
			fprintf(stderr, "Chamada incorreta da função ls.\nChamada correta: ls diretorio\n");
		return true;
	}
	if (strcmp(command, "find") == 0)
	{
		if (parameters[1] != NULL && parameters[2] && SA != NULL)
			find(SA, parameters[1], parameters[2], 1, BLOCO_ROOT, "/");
		else if (SA == NULL)
			fprintf(stderr, "Não há um sistema de arquivos montado\n");
		else
			fprintf(stderr, "Chamada incorreta da função find.\nChamada correta: find diretorio arquivo\n");
		return true;
	}
	if (strcmp(command, "df") == 0)
	{
		int a = 0, b = 0;
		df(SA, BLOCO_ROOT, &a, &b, 1);
		return true;
	}
	if (strcmp(command, "umount") == 0)
	{
		fprintf(stderr, "Desmontando o sistema de arquivos\n");
		fclose(SA);
		SA = NULL;
		return true;
	}
	if (strcmp(command, "sai") == 0)
	{
		/*sai do simulador*/
		//Provavelmente vai ser um exit(0)
		if (SA != NULL)
		{
			fprintf(stderr, "Desmontando o sistema de arquivos\n");
			fclose(SA);
			SA = NULL;
		}
		free(command);
		clear_history();
		exit(EXIT_SUCCESS);
	}
	return false;
}
