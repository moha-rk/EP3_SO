#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>

#define MAX_PARAMETERS 10
#define MAX_PROMPT_SIZE 10
#define SPACESTR " "
#define END_CHAR '\0'

/*Prototipos de funções de leitura e interpretação de comandos*/

void read_command(char **command, char **parameters);
bool internal_commands(char *command, char **parameters);


char *prompt = "[ep3]: ";

int main(int argc, char **argv)
{
	char *command = NULL;
	char *parameters[MAX_PARAMETERS];

	bool repeat = true;

	while (repeat)
	{
		free(command);

		type_prompt();
		read_command(&command, parameters);

		if (internal_commands(command, parameters))
			continue;
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
		return true;
	}
    if (strcmp(command, "cp") == 0)
	{
		if (parameters[1] != NULL && parameters[2] != NULL)
			/*copia de parameters[1] sera salva em parameters[2]*/
		return true;
	}
	if (strcmp(command, "mkdir") == 0)
	{
		if (parameters[1] != NULL)
            /*Nova função de mkdir no sistema de arquivos simulado*/
			func_mkdir(parameters[1]);
		return true;
	}
    if (strcmp(command, "rmdir") == 0)
	{
		if (parameters[1] != NULL)
            /* apaga o diretorio parameters[1]. Se o diretorio nao estiver vazio, apaga tudo que estiver embaixo e avisa para o usuario tudo que tiver sido apagado*/
			
		return true;
	}
	if (strcmp(command, "cat") == 0)
	{
		if (parameters[1] != NULL)
			/* mostra o conteudo do arquivo parameters[1] na tela*/
		return true;
	}
    if (strcmp(command, "touch") == 0)
	{
		if (parameters[1] != NULL)
			/*se o arquivo parameters[1] existir, atualiza o instante de tempo de acesso para o
            instante de tempo atual. Se nao existir, cria um arquivo vazio*/
		return true;
	}
    if (strcmp(command, "rm") == 0)
	{
		if (parameters[1] != NULL)
			/*remove o arquivo parameters[1]*/
		return true;
	}
	if (strcmp(command, "ls") == 0)
	{
		if (parameters[1] != NULL)
			/* lista os arquivos e diretorios que estejam “embaixo” do diretorio parameters[1].
            Na listagem que sera exibida, para todos os arquivos existentes, deverao ser exibidos: nome, tamanho em bytes e data de ultima modificaçao. Diretorios devem ser exibidos com alguma informaçao a mais que os diferencie como diretorios*/
		return true;
	}
    if (strcmp(command, "find") == 0)
	{
		if (parameters[1] != NULL && parameters[2] != NULL)
			/*busca a partir de parameters[1] se ha algum arquivo com nome
            parameters[2]. Todos os arquivos encontrados devem ser exibidos na tela com os seus caminhos
            completos (apenas o nome dos arquivos, nao o conte ˜ udo)*/
		return true;
	}
    if (strcmp(command, "df") == 0)
	{
		/*imprime na tela as seguintes informaçoes do sistema de arquivos: quantidade de diretorios, ´
        quantidade de arquivos, espaço livre, espaço desperdiçado (considerando o espaço a mais necessario para cada arquivo ocupar exatamente multiplos do tamanho de 1 bloco)*/
		return true;
	}
	if (strcmp(command, "umount") == 0)
	{
		/*desmonta sistema de arquivos*/
		return true;
	}
    if (strcmp(command, "sai") == 0)
	{
		/*sai do simulador*/
		//Provavelmente vai ser um exit(0)
        exit(EXIT_SUCCESS);
        free(command);
	    clear_history();
	}
	return false;
}
