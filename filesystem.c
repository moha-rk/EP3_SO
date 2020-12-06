#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include "filesystem.h"

//Retorno da função a ser definido, podendo ser um ponteiro FILE
FILE *mount(char *nome_arquivo)
{
    FILE *f;

    if (f = fopen(nome_arquivo, "r+"))
    {
        /*Carregar FAT e bitmap na memória nas váriaveis correspondentes*/
        carrega_FAT(f);
        carrega_bitmap(f);
        //fclose(f);
        return f;
    }
    /*Cria sistema de arquivos com esse nome*/
    if (!(f = fopen(nome_arquivo, "w+")))
    {
        fprintf(stderr, "Acesso ao arquivo %s negado\n", nome_arquivo);
        return NULL;
    }
    printf("Criando sistema de arquivos...\n");
    cria_FAT(f);
    cria_bitmap(f);
    cria_root(f);
    for (int i = BLOCOS_FAT + BLOCOS_BITMAP + 1; i < N_BLOCOS; i++)
        preenche_bloco_vazio(f, i, 0);
    volta_pro_root(f);

    return f;
}

//Recebe um arquivo vazio e escreve a FAT nele, também a armazenando na memória
void cria_FAT(FILE *f)
{
    int i;
    //Primeiros 32 blocos pertencem a FAT
    for (i = 0; i < BLOCOS_FAT - 1; i++)
        FAT[i] = i + 1;
    FAT[i] = -1;
    //Proximos 7 blocos pertencem ao bitmap
    for (i = 32; i < BLOCOS_FAT + BLOCOS_BITMAP - 1; i++)
        FAT[i] = i + 1;
    FAT[i] = -1;
    //Primeiro bloco pertence ao '/'
    FAT[i + 1] = -1;
    //Depois vejo se preciso zerar o resto dos elementos
    for (i = i + 2; i < N_BLOCOS; i++)
        FAT[i] = 0;

    //Devo escrever no arquivo já aqui?
    for (i = 0; i < N_BLOCOS; i++)
        fprintf(f, "%05d", FAT[i]);
    preenche_bloco_vazio(f, BLOCOS_FAT - 1, (N_BLOCOS * FAT_ENTRY) % TAMANHO_BLOCO);
}

//Recebe um arquivo vazio e carrega a FAT escrita nele
void carrega_FAT(FILE *f)
{
    fseek(f, 0, SEEK_SET);
    char buf[6];
    for (int i = 0; i < N_BLOCOS; i++)
    {
        fgets(buf, FAT_ENTRY + 1, f);
        FAT[i] = atoi(buf);
    }
    //for (int i = 0; i < 50; i++) printf("FAT[%d] = %d\n", i, FAT[i]);
}

//Recebe um arquivo contendo apenas FAT e escreve o bitmap nele, também o armazenando na memória
void cria_bitmap(FILE *f)
{
    //BLOCOS_FAT+BLOCOS_BITMAP+1 engloba blocos destas duas estruturas + bloco inicial do '/'
    int i;
    for (i = 0; i < BLOCOS_FAT + BLOCOS_BITMAP + 1; i++)
    {
        bitmap[i] = OCUPADO;
        fputc(OCUPADO, f);
    }
    for (int j = i; j < N_BLOCOS; j++)
    {
        bitmap[j] = LIVRE;
        fputc(LIVRE, f);
    }
    preenche_bloco_vazio(f, BLOCOS_FAT + BLOCOS_BITMAP - 1, N_BLOCOS % TAMANHO_BLOCO);
}

//Recebe um arquivo vazio e carrega o bitmap escrita nele
void carrega_bitmap(FILE *SA)
{
    fseek(SA, TAMANHO_BLOCO * (BLOCOS_FAT), SEEK_SET);
    char buf[1];
    for (int i = 0; i < N_BLOCOS; i++)
    {
        bitmap[i] = fgetc(SA);
    }
    //for (int i = 0; i < 50; i++) printf("bitmap[%d] = %d\n", i, bitmap[i]);
}

//O root conterá os próprios metadados?
void cria_root(FILE *f)
{
    fputc('/', f);
    time(&rawtime);
    fprintf(f, "%ld ", rawtime);
    fprintf(f, "%ld ", rawtime);
    fprintf(f, "%ld|", rawtime);
}

void preenche_bloco_vazio(FILE *f, int n_bloco, int offset)
{
    fseek(f, (n_bloco * TAMANHO_BLOCO + offset), SEEK_SET);
    for (int i = offset; i < TAMANHO_BLOCO; i++)
    {
        fputc(0, f);
    }
}

void volta_pro_root(FILE *f)
{
    fseek(f, TAMANHO_BLOCO * BLOCO_ROOT + TAMANHO_METADADOS_ROOT, SEEK_SET);
}

//SA = Sistema de arquivos
//add_arquivo deve conferir se há tamanho suficiente para o arquivo, buscar o diretorio onde deve ser salvo o arquivo, salvar seus metadados nesse diretorio e então salvar o arquivo em um ou mais blocos
//add_arquivo sempre partirá do root
int add_arquivo(FILE *SA, char *nome_origem, char *nome_destino)
{
    int tamanho, blocos_arquivo;
    char *diretorio_atual = "/";
    FILE *arq;
    if (!(arq = fopen(nome_origem, "r")))
    {
        fprintf(stderr, "O arquivo %s não pôde ser aberto\n", nome_origem);
        return -1;
    }
    fseek(arq, 0, SEEK_END);
    tamanho = ftell(arq);
    blocos_arquivo = tamanho / TAMANHO_BLOCO;
    if (tamanho % TAMANHO_BLOCO != 0)
        blocos_arquivo++;
    rewind(arq);

    int cont = 0, primeiro, segundo;
    for (int i = BLOCO_ROOT; i < N_BLOCOS; i++)
    {
        if (bitmap[i] == LIVRE)
        {
            cont++;
            if (cont == 1)
                primeiro = i;
            if (cont >= blocos_arquivo)
                break;
        }
    }
    if (cont < blocos_arquivo)
    {
        fprintf(stderr, "Não há espaço suficiente para adicionar %s ao sistema\n", nome_origem);
        fclose(arq);
        return -1;
    }

    //Aqui, devemos iniciar uma busca até encontrarmos o diretório no qual o arquivo deverá ser salvo
    int dir = busca_diretorio_pai(SA, "/", nome_destino, BLOCO_ROOT);
    if (dir == -1)
    {
        fprintf(stderr, "O diretório não foi encontrado\n");
        return -1;
    }
    remove_dirs_nome(nome_destino);
    int tam_metadados = calcula_tamanho_metadados(nome_destino, -1);

    dir = busca_espaco_metadados(SA, dir, tam_metadados, &primeiro, segundo, blocos_arquivo);
    if (dir == -1)
    {
        fprintf(stderr, "Não há espaço suficiente para adicionar %s ao sistema\n", nome_origem);
        fclose(arq);
        return -1;
    }
    //Resta apenas alocar aqui o primeiro bloco (talvez outros) para o arquivo em questão e salvar seus dados lá
    adiciona_metadados_arquivo(SA, dir, nome_destino, tamanho, primeiro);

    aloca_bloco(SA, primeiro);
    for (int i = 0; i < blocos_arquivo; i++)
    {
        if (tamanho > TAMANHO_BLOCO)
        {
            for (int j = 0; j < TAMANHO_BLOCO; j++)
                fputc(fgetc(arq), SA);
            tamanho -= TAMANHO_BLOCO;
            for (segundo = primeiro + 1; bitmap[segundo] != LIVRE; segundo++)
                ;

            estende_bloco(SA, primeiro, segundo);
            primeiro = segundo;
        }
        else
        {
            for (int j = 0; j < tamanho; j++)
                fputc(fgetc(arq), SA);
            tamanho = 0;
        }
    }
    return 0; //Acho
}

//Chamar com tamanho = 0 para arquivo vazio (touch) e tamanho = -1 para mkdir
int add_arquivo_vazio(FILE *SA, char *nome, int tamanho)
{
    int blocos_arquivo;
    char *diretorio_atual = "/";
    blocos_arquivo = 1;

    int cont = 0, primeiro, segundo = -1;
    for (int i = BLOCO_ROOT; i < N_BLOCOS; i++)
    {
        if (bitmap[i] == LIVRE)
        {
            cont++;
            if (cont == 1)
                primeiro = i;
            if (cont >= blocos_arquivo)
                break;
        }
    }
    if (cont < blocos_arquivo)
    {
        fprintf(stderr, "Não há espaço suficiente para adicionar %s ao sistema\n", nome);
        return -1;
    }

    //Aqui, devemos iniciar uma busca até encontrarmos o diretório no qual o arquivo deverá ser salvo
    int dir = busca_diretorio_pai(SA, "/", nome, BLOCO_ROOT);
    if (dir == -1)
    {
        fprintf(stderr, "O diretório não foi encontrado\n");
        return -1;
    }
    remove_dirs_nome(nome);
    int tam_metadados = calcula_tamanho_metadados(nome, tamanho);

    dir = busca_espaco_metadados(SA, dir, tam_metadados, &primeiro, segundo, blocos_arquivo);
    if (dir == -1)
    {
        fprintf(stderr, "Não há espaço suficiente para adicionar %s ao sistema\n", nome);
        return -1;
    }
    adiciona_metadados_arquivo(SA, dir, nome, tamanho, primeiro);

    aloca_bloco(SA, primeiro);
    return 0; //Acho
}

int busca_espaco_metadados(FILE *SA, int dir, int tam_metadados, int *primeiro, int segundo, int blocos_arquivo)
{
    if (espaco_restante_diretorio(SA, &dir) < tam_metadados + digitos(tam_metadados) + 1)
    {
        //Aloco novo bloco para o diretorio
        int cont = 0;
        for (int i = *primeiro + 1; i < N_BLOCOS; i++)
        {
            if (bitmap[i] == LIVRE)
            {
                cont++;
                if (cont == 1)
                    segundo = i;
                if (cont == blocos_arquivo)
                    break;
            }
        }
        if (cont < blocos_arquivo)
            return -1;

        estende_bloco(SA, dir, *primeiro);
        *primeiro = segundo;
    }
    return dir;
}

//Aloca um bloco e aponta SA para ele
void aloca_bloco(FILE *SA, int dir1)
{
    FAT[dir1] = -1;
    bitmap[dir1] = OCUPADO;
    //Escrever FAT
    fseek(SA, dir1 * FAT_ENTRY, SEEK_SET);
    fprintf(SA, "%05d", FAT[dir1]);

    //Escrever bitmap
    fseek(SA, (TAMANHO_BLOCO * BLOCOS_FAT) + dir1, SEEK_SET);
    fputc(OCUPADO, SA);

    fseek(SA, dir1 * TAMANHO_BLOCO, SEEK_SET);
}

//Aloca mais um bloco e aponta SA para ele
void estende_bloco(FILE *SA, int dir1, int dir2)
{
    FAT[dir1] = dir2;
    FAT[dir2] = -1;
    bitmap[dir2] = OCUPADO;
    //Escrever FAT
    fseek(SA, dir1 * FAT_ENTRY, SEEK_SET);
    fprintf(SA, "%05d", FAT[dir1]);
    fseek(SA, dir2 * FAT_ENTRY, SEEK_SET);
    fprintf(SA, "%05d", FAT[dir2]);

    //Escrever bitmap
    fseek(SA, (TAMANHO_BLOCO * BLOCOS_FAT) + dir2, SEEK_SET);
    fputc(OCUPADO, SA);

    fseek(SA, dir2 * TAMANHO_BLOCO, SEEK_SET);
}

void remove_dirs_nome(char *nome)
{
    int i, j = 0;
    char aux[255];
    for (i = 0; i < strlen(nome); i++)
    {
        aux[j] = nome[i];
        j++;
        if (nome[i] == '/')
            j = 0;
    }
    aux[j] = '\0';
    strcpy(nome, aux);
}

void nome_dir_pai(char *nome)
{
    int i, j = 0;
    char aux[255];
    for (i = strlen(nome) - 1; i >= 0; i--)
    {
        if (nome[i] == '/')
            break;
    }
    aux[0] = '/';
    for (j = 1; j < i; j++)
        aux[j] = nome[j];
    aux[j] = '\0';
    strcpy(nome, aux);
}

//dir_atual deve ser chamado com "/" e bloco_dir com o bloco do root
//Devo alimentar a função com um caminho contendo o nome do arquivo no final?
//entradas serão do tipo "/tmp/d1/arquivo.txt" ou "/tmp/d1"?
//Acho que vou fazer uma função que busca o penultimo diretorio. Nos exemplos acima, deve buscar d1 e tmp.

//Esta função devolve o bloco do diretorio anterior à última entrada (arquivo.txt ou d1 no exemplo acima)
//dir_atual deve ser chamado com "/" e bloco_dir com o bloco do root, e nome_atual é o q queremos buscar
int busca_diretorio_pai(FILE *SA, char *dir_atual, char *nome_atual, int bloco_dir)
{
    if (strcmp(dir_atual, "/") == 0)
        volta_pro_root(SA);
    if (nome_atual[0] != '/')
    {
        //Chegamos no diretório
        return bloco_dir;
    }

    char dir_aux[255];
    int i;
    for (i = 1; i < strlen(nome_atual); i++)
    {
        if (nome_atual[i] == '/')
            break;
        dir_aux[i - 1] = nome_atual[i];
    }

    dir_aux[i - 1] = '\0';
    if (i == strlen(nome_atual))
    {
        nome_atual = dir_aux;
        return busca_diretorio_pai(SA, dir_atual, nome_atual, bloco_dir);
    }
    else
    {
        //Preciso mexer *SA para o bloco de dir_aux antes de chamar a função novamente
        bloco_dir = procura_nome_e_devolve_info(SA, dir_aux, GETFAT, bloco_dir);
        if (bloco_dir == -1)
        {
            fprintf(stderr, "%s não foi encontrado em %s\n", dir_aux, dir_atual);
            //volta_pro_root(SA);
            return -1;
        }
        fseek(SA, TAMANHO_BLOCO * bloco_dir, SEEK_SET);
        int j = 0;
        char aux[255];
        for (i; i < strlen(nome_atual); i++)
        {
            aux[j] = nome_atual[i];
            j++;
        }
        aux[j] = '\0';
        nome_atual = aux;
        return busca_diretorio_pai(SA, dir_aux, nome_atual, bloco_dir);
    }
}

//Procura o nome de um arquivo (regular ou diretorio) e devolve a informação pedida supondo que SA está no início do diretório a ser vasculhado
int procura_nome_e_devolve_info(FILE *SA, char *nome, int info, int bloco_dir)
{
    //cont armazenará quantos bytes foram lidos do bloco atual
    int cont = 0, i = 0, tamanho_entrada;

    if (bloco_dir == BLOCO_ROOT)
    {
        volta_pro_root(SA);
        cont += TAMANHO_METADADOS_ROOT;
    }
    else
        fseek(SA, bloco_dir * TAMANHO_BLOCO, SEEK_SET);

    char nome_lido[255];
    nome_lido[0] = '\0';

    while (strcmp(nome_lido, nome) != 0)
    {
        if (cont >= TAMANHO_BLOCO)
            if (!busca_continuacao_dir(SA, &bloco_dir, &cont))
                return -1;

        //Aqui, nome_lido contém o nome errado e SA aponta para o primeiro caracter após \0

        while ((nome_lido[i] = fgetc(SA)) != '\0')
        {
            i++;
            cont++;
            //Não sei se deve ser == ou >
            if (cont >= TAMANHO_BLOCO)
                if (!busca_continuacao_dir(SA, &bloco_dir, &cont))
                    return -1;
        }
        if (nome_lido[0] == '\0')
            return -1;
        cont++;
        if (cont >= TAMANHO_BLOCO)
            if (!busca_continuacao_dir(SA, &bloco_dir, &cont))
                return -1;
        tamanho_entrada = atoi(nome_lido);
        i = 0;
        while ((nome_lido[i] = fgetc(SA)) != '\0')
        {
            i++;
            cont++;
            if (cont >= TAMANHO_BLOCO)
                if (!busca_continuacao_dir(SA, &bloco_dir, &cont))
                    return -1;
        }
        cont++;
        i = 0;
        cont += tamanho_entrada - (strlen(nome_lido) + 1);

        fseek(SA, tamanho_entrada - (strlen(nome_lido) + 1), SEEK_CUR);
        //Aqui, estou no nome da próxima entrada (ou em \0 se o diretorio tiver acabado)
    }
    fseek(SA, -tamanho_entrada + (strlen(nome_lido) + 1), SEEK_CUR);
    //*SA aponta para o tamanho (caso seja arquivo regular) ou para o primeiro tempo caso seja um diretorio
    //Depois adiciono mais infos para devolver, por enquanto devolverei apenas o diretorio FAT
    if (info == 1) //Numero generico
    {
        fseek(SA, tamanho_entrada - (strlen(nome_lido) + 1) - FAT_ENTRY - 1, SEEK_CUR);

        for (i = 0; i < 5; i++)
            nome_lido[i] = fgetc(SA);
        nome_lido[i] = '\0';
        return atoi(nome_lido); //FAT
    }
    else if (info == 2) //Função que serve para posicionar SA no inicio do tempo de acesso (tempo2)
    {
        fseek(SA, tamanho_entrada - (strlen(nome_lido) + 1) - (FAT_ENTRY + 1 + 2 * TAMANHO_TIME), SEEK_CUR);
        return 1;
    }
}

void df(FILE *SA, int bloco_dir_atual, int *ndirs, int *narquivos, int primeira_chamada)
{
    //Primeiro, faremos uma busca em profundidade para encontrar o numeor de arquivos e diretorios (sem contar o root)
    int cont = 0, i = 0, tamanho_entrada;

    if (bloco_dir_atual == BLOCO_ROOT)
    {
        volta_pro_root(SA);
        cont += TAMANHO_METADADOS_ROOT;
    }
    else
        fseek(SA, bloco_dir_atual * TAMANHO_BLOCO, SEEK_SET);
    char nome_lido[255];
    while (1)
    {
        if (cont >= TAMANHO_BLOCO)
            if (!busca_continuacao_dir(SA, &bloco_dir_atual, &cont))
                break;
        //Aqui, nome_lido contém o nome errado e SA aponta para o primeiro caracter após \0

        while ((nome_lido[i] = fgetc(SA)) != '\0')
        {
            i++;
            cont++;
            //Não sei se deve ser == ou >
            if (cont >= TAMANHO_BLOCO)
                if (!busca_continuacao_dir(SA, &bloco_dir_atual, &cont))
                    break;
        }
        //fprintf(stderr, "nomelido=%s\n", nome_lido);
        if (nome_lido[0] == '\0' || cont >= TAMANHO_BLOCO)
            break;
        cont++;
        if (cont >= TAMANHO_BLOCO)
            if (!busca_continuacao_dir(SA, &bloco_dir_atual, &cont))
                break;
        tamanho_entrada = atoi(nome_lido);
        i = 0;
        while ((nome_lido[i] = fgetc(SA)) != '\0')
        {
            i++;
            cont++;
            if (cont >= TAMANHO_BLOCO)
                if (!busca_continuacao_dir(SA, &bloco_dir_atual, &cont))
                    break;
        }
        if (cont >= TAMANHO_BLOCO)
            break;
        cont++;
        i = 0;
        fseek(SA, tamanho_entrada - (strlen(nome_lido) + 1) - (FAT_ENTRY + 1), SEEK_CUR);
        cont += tamanho_entrada - (strlen(nome_lido) + 1) - (FAT_ENTRY + 1);
        if (cont >= TAMANHO_BLOCO)
            if (!busca_continuacao_dir(SA, &bloco_dir_atual, &cont))
                break;

        if (tamanho_entrada == calcula_tamanho_metadados(nome_lido, -1))
        {
            //Achei um diretorio
            *ndirs += 1;
            int j;
            for (j = 0; j < FAT_ENTRY; j++)
            {
                nome_lido[j] = fgetc(SA);
                cont++;
                if (cont >= TAMANHO_BLOCO)
                    if (!busca_continuacao_dir(SA, &bloco_dir_atual, &cont))
                        break;
            }
            nome_lido[j] = '\0';
            fgetc(SA); //Para passar o |
            cont++;
            int bloco_entrada = atoi(nome_lido);
            long pos_anterior = ftell(SA);
            df(SA, bloco_entrada, ndirs, narquivos, 0);
            fseek(SA, pos_anterior, SEEK_SET);
        }
        else
        {
            *narquivos += 1;
            cont += FAT_ENTRY + 1;
            fseek(SA, FAT_ENTRY + 1, SEEK_CUR);
            if (cont >= TAMANHO_BLOCO)
                if (!busca_continuacao_dir(SA, &bloco_dir_atual, &cont))
                    break;
        }
        //Aqui, estou no nome da próxima entrada (ou em \0 se o diretorio tiver acabado)
    }
    //Saindo aqui, tenho o numero de arquivos e de diretorios
    if (!primeira_chamada)
        return;

    fprintf(stderr, "Numero de arquivos regulares: %d\n", *narquivos);
    fprintf(stderr, "Numero de diretórios: %d\n", *ndirs);

    long blocos_livres = 0;
    long memoria_desperdicada = 0;

    for (i = 0; i < N_BLOCOS; i++)
    {
        if (bitmap[i] == LIVRE)
            blocos_livres++;
        else
            memoria_desperdicada += espaco_restante_bloco(SA, i);
    }
    fprintf(stderr, "Espaço livre (em bytes): %ld\n", blocos_livres * TAMANHO_BLOCO);
    fprintf(stderr, "Espaço desperdiçado (em bytes): %ld\n", memoria_desperdicada);
}

void find(FILE *SA, char *diretorio, char *nome, int primeira_chamada, int bloco_diretorio_pai, char *dir_pai)
{
    int cont = 0, i = 0, tamanho_entrada, bloco_dir;
    if (primeira_chamada)
    {
        bloco_diretorio_pai = busca_diretorio_pai(SA, "/", diretorio, BLOCO_ROOT);
        dir_pai = diretorio;
    }
    if (strcmp(diretorio, "/") == 0)
    {
        bloco_dir = BLOCO_ROOT;
        dir_pai = "";
    }
    else
    {
        char dir_reduzido[255];
        strcpy(dir_reduzido, diretorio);
        remove_dirs_nome(dir_reduzido);
        bloco_dir = procura_nome_e_devolve_info(SA, dir_reduzido, GETFAT, bloco_diretorio_pai);
    }

    //Ir para o bloco do diretorio e ler as entradas, ver se sao arquivos e apagá-las e ver se são diretorios e chamar a função de novo

    if (bloco_dir == BLOCO_ROOT)
    {
        volta_pro_root(SA);
        cont += TAMANHO_METADADOS_ROOT;
    }
    else
        fseek(SA, bloco_dir * TAMANHO_BLOCO, SEEK_SET);
    char nome_lido[255];
    while (1)
    {
        if (cont >= TAMANHO_BLOCO)
            if (!busca_continuacao_dir(SA, &bloco_dir, &cont))
                return;
        //Aqui, nome_lido contém o nome errado e SA aponta para o primeiro caracter após \0
        i = 0;
        while ((nome_lido[i] = fgetc(SA)) != '\0')
        {
            i++;
            cont++;
            //Não sei se deve ser == ou >
            if (cont >= TAMANHO_BLOCO)
                if (!busca_continuacao_dir(SA, &bloco_dir, &cont))
                    return;
        }
        if (nome_lido[0] == '\0')
            return;
        cont++;
        if (cont >= TAMANHO_BLOCO)
            if (!busca_continuacao_dir(SA, &bloco_dir, &cont))
                return;
        tamanho_entrada = atoi(nome_lido);
        i = 0;
        while ((nome_lido[i] = fgetc(SA)) != '\0')
        {
            i++;
            cont++;
            if (cont >= TAMANHO_BLOCO)
                if (!busca_continuacao_dir(SA, &bloco_dir, &cont))
                    return;
        }
        
        cont++;
        i = 0;
        cont += tamanho_entrada - (strlen(nome_lido) + 1);

        if (tamanho_entrada == calcula_tamanho_metadados(nome_lido, -1))
        {
            //Achei um diretorio
            char dir_aux[250];
            dir_aux[0] = '\0';
            strcat(dir_aux, dir_pai);
            if (strcmp(diretorio, "/") != 0)
            {   
                strcat(dir_aux, "/");
                strcat(dir_aux, diretorio);
            }
            int pos_atual = ftell(SA);
            find(SA, nome_lido, nome, 0, bloco_dir, dir_aux);
            fseek(SA, pos_atual, SEEK_SET);
        }
        if (strcmp(nome_lido, nome) == 0)
        {
             if (strcmp(diretorio, "/") != 0)
                fprintf(stderr, "%s/%s/%s\n", dir_pai, diretorio, nome);
            else
                fprintf(stderr, "/%s\n", nome);
        }

        fseek(SA, tamanho_entrada - (strlen(nome_lido) + 1), SEEK_CUR);
        //Aqui, estou no nome da próxima entrada (ou em \0 se o diretorio tiver acabado)
    }
}

//Esta função apaga os metadados de um arquivo de seu diretorio pai e então apaga seus dados do sistema de arquivos
int remove_arquivo(FILE *SA, char *nome)
{
    int cont = 0, i = 0, tamanho_entrada;
    int bloco_dir = busca_diretorio_pai(SA, "/", nome, BLOCO_ROOT);

    if (bloco_dir == BLOCO_ROOT)
    {
        volta_pro_root(SA);
        cont += TAMANHO_METADADOS_ROOT;
    }
    else if (bloco_dir == -1)
    {
        fprintf(stderr, "%s não foi encontrado\n", nome);
        return -1;
    }

    char nome_lido[255], c;

    remove_dirs_nome(nome);
    nome_lido[0] = '\0';

    while (strcmp(nome_lido, nome) != 0)
    {
        if (cont >= TAMANHO_BLOCO)
            if (!busca_continuacao_dir(SA, &bloco_dir, &cont))
            {
                fprintf(stderr, "O arquivo não foi encontrado\n");
                return -1;
            }

        //Aqui, nome_lido contém o nome errado e SA aponta para o primeiro caracter após \0
        while ((nome_lido[i] = fgetc(SA)) != '\0')
        {
            i++;
            cont++;
            //Não sei se deve ser == ou >
            if (cont >= TAMANHO_BLOCO)
                if (!busca_continuacao_dir(SA, &bloco_dir, &cont))
                {
                    fprintf(stderr, "O arquivo %s não foi encontrado\n", nome);
                    return -1;
                }
        }
        if (nome_lido[0] == '\0')
        {
            fprintf(stderr, "O arquivo %s não foi encontrado\n", nome);
            return -1;
        }
        cont++;
        if (cont >= TAMANHO_BLOCO)
            if (!busca_continuacao_dir(SA, &bloco_dir, &cont))
            {
                fprintf(stderr, "O arquivo %s não foi encontrado\n", nome);
                return -1;
            }
        tamanho_entrada = atoi(nome_lido);
        i = 0;
        while ((nome_lido[i] = fgetc(SA)) != '\0')
        {
            i++;
            cont++;
            if (cont >= TAMANHO_BLOCO)
                if (!busca_continuacao_dir(SA, &bloco_dir, &cont))
                {
                    fprintf(stderr, "O arquivo %s não foi encontrado\n", nome);
                    return -1;
                }
        }
        cont++;
        i = 0;
        cont += tamanho_entrada - (strlen(nome_lido) + 1);

        fseek(SA, tamanho_entrada - (strlen(nome_lido) + 1), SEEK_CUR);
        //Aqui, estou no nome da próxima entrada (ou em \0 se o diretorio tiver acabado)
    }
    fseek(SA, -FAT_ENTRY - 1, SEEK_CUR);
    for (i = 0; i < FAT_ENTRY; i++)
    {
        nome_lido[i] = fgetc(SA);
        cont++;
        if (cont >= TAMANHO_BLOCO)
            if (!busca_continuacao_dir(SA, &bloco_dir, &cont))
            {
                fprintf(stderr, "O arquivo %s não foi encontrado\n", nome);
                return -1;
            }
    }
    nome_lido[i] = '\0';
    fgetc(SA); //Para passar o |
    int bloco_entrada = atoi(nome_lido);
    int distancia = tamanho_entrada + digitos(tamanho_entrada) + 1;
    fseek(SA, -distancia, SEEK_CUR);
    //*SA aponta para o tamanho (caso seja arquivo regular) ou para o primeiro tempo caso seja um diretorio
    long pos_anterior;
    while (1)
    {
        //Talvez <=
        if (ftell(SA) + distancia < (bloco_dir + 1) * TAMANHO_BLOCO)
        {
            pos_anterior = ftell(SA);
            fseek(SA, distancia, SEEK_CUR);
            c = fgetc(SA);
            fseek(SA, pos_anterior, SEEK_SET);
            fputc(c, SA);
        }
        else if (FAT[bloco_dir] != -1)
        {
            pos_anterior = ftell(SA);
            fseek(SA, (FAT[bloco_dir] * TAMANHO_BLOCO) + (TAMANHO_BLOCO - (pos_anterior % TAMANHO_BLOCO) + distancia), SEEK_SET);
            c = fgetc(SA);
            fseek(SA, pos_anterior, SEEK_SET);
            fputc(c, SA);
            if (ftell(SA) >= (bloco_dir + 1) * TAMANHO_BLOCO)
            {
                bloco_dir = FAT[bloco_dir];
                fseek(SA, bloco_dir * TAMANHO_BLOCO, SEEK_SET);
            }
        }
        else
        {
            for (int j = 0; j < distancia; j++)
                fputc('\0', SA);
            break;
        }
    }
    do
    {
        bloco_entrada = limpa_bloco(SA, bloco_entrada);
    } while (bloco_entrada != -1);
    return 1;
}

int limpa_bloco(FILE *SA, int bloco)
{
    long pos_anterior = ftell(SA);
    bitmap[bloco] = LIVRE;
    fseek(SA, (TAMANHO_BLOCO * BLOCOS_FAT) + bloco, SEEK_SET);
    fputc(LIVRE, SA);

    int aux = FAT[bloco];
    FAT[bloco] = 0;
    fseek(SA, bloco*FAT_ENTRY, SEEK_SET);
    fprintf(SA, "%05d", FAT[bloco]);
    fseek(SA, pos_anterior, SEEK_SET);
    return aux;
}

int remove_diretorio(FILE *SA, int bloco_diretorio_pai, char *diretorio, int primeira_chamada)
{
    int cont = 0, i = 0, tamanho_entrada, bloco_dir;
    if (primeira_chamada)
        bloco_diretorio_pai = busca_diretorio_pai(SA, "/", diretorio, BLOCO_ROOT);

    char dir_reduzido[255];
    strcpy(dir_reduzido, diretorio);
    remove_dirs_nome(dir_reduzido);
    bloco_dir = procura_nome_e_devolve_info(SA, dir_reduzido, GETFAT, bloco_diretorio_pai);

    //Ir para o bloco do diretorio e ler as entradas, ver se sao arquivos e apagá-las e ver se são diretorios e chamar a função de novo

    if (bloco_dir == BLOCO_ROOT)
    {
        volta_pro_root(SA);
        cont += TAMANHO_METADADOS_ROOT;
    }
    else if (bloco_dir == -1)
    {
        fprintf(stderr, "O diretório não foi encontrado\n");
        return -1;
    }
    else
    {
        fseek(SA, bloco_dir * TAMANHO_BLOCO, SEEK_SET);
    }

    char nome_lido[255];

    while (1)
    {
        if (cont >= TAMANHO_BLOCO)
            if (!busca_continuacao_dir(SA, &bloco_dir, &cont))
                break;
        i = 0;
        while ((nome_lido[i] = fgetc(SA)) != '\0')
        {
            i++;
            cont++;
            //Não sei se deve ser == ou >
            if (cont >= TAMANHO_BLOCO)
                if (!busca_continuacao_dir(SA, &bloco_dir, &cont))
                    break;
        }
        if (nome_lido[0] == '\0' || cont >= TAMANHO_BLOCO)
            break;
        cont++;
        if (cont >= TAMANHO_BLOCO)
            if (!busca_continuacao_dir(SA, &bloco_dir, &cont))
                break;
        tamanho_entrada = atoi(nome_lido);
        i = 0;
        while ((nome_lido[i] = fgetc(SA)) != '\0')
        {
            i++;
            cont++;
            if (cont >= TAMANHO_BLOCO)
                if (!busca_continuacao_dir(SA, &bloco_dir, &cont))
                    break;
        }
        if (cont >= TAMANHO_BLOCO)
            break;
        cont++;
        i = 0;
        //Aqui já lemos o nome
        fseek(SA, tamanho_entrada - (strlen(nome_lido) + 1) - FAT_ENTRY - 1, SEEK_CUR);
        char fat_buf[6];
        for (i = 0; i < FAT_ENTRY; i++)
        {
            fat_buf[i] = fgetc(SA);
            cont++;
            if (cont >= TAMANHO_BLOCO)
                if (!busca_continuacao_dir(SA, &bloco_dir, &cont))
                    break ;
        }
        if (cont >= TAMANHO_BLOCO)
            break;
        fat_buf[i] = '\0';
        fgetc(SA); //Para passar o |
        int bloco_entrada = atoi(fat_buf);

        if (tamanho_entrada > calcula_tamanho_metadados(nome_lido, -1))
        {
            //Arquivo
            do
            {
                bloco_entrada = limpa_bloco(SA, bloco_entrada);
            } while (bloco_entrada != -1);
            fprintf(stderr, "%s/%s\n", dir_reduzido, nome_lido);
        }
        else
        {
            //Diretorio
            int j;
            int pos_anterior = ftell(SA);
            remove_diretorio(SA, bloco_dir, nome_lido, 0);
            do
            {
                bloco_entrada = limpa_bloco(SA, bloco_entrada);
            } while (bloco_entrada != -1);
            fseek(SA, pos_anterior, SEEK_SET);
            fprintf(stderr, "%s/%s/\n", dir_reduzido, nome_lido);
        }
    }
    if (primeira_chamada) //apagar metadados
    {
        remove_arquivo(SA, diretorio);
        fprintf(stderr, "%s/\n", diretorio);
    }
    return 1;
}

void lista_itens_diretorio(FILE *SA, char *nome)
{
    int cont = 0, i = 0, tamanho_entrada, bloco_dir;
    char buf[255];
    buf[0] = '\0';
    strcat(buf, nome);
    if (strcmp(nome, "/") != 0)
        strcat(buf, "/a");
    bloco_dir = busca_diretorio_pai(SA, "/", buf, BLOCO_ROOT);

    if (bloco_dir == BLOCO_ROOT)
    {
        volta_pro_root(SA);
        cont += TAMANHO_METADADOS_ROOT;
    }
    else if (bloco_dir == -1)
    {
        fprintf(stderr, "O diretório %s não foi encontrado\n", nome);
        return;
    }
    fprintf(stderr, "Nome            Tamanho (bytes)   Última modificação\n");

    char nome_lido[255];

    while (1)
    {
        if (cont >= TAMANHO_BLOCO)
            if (!busca_continuacao_dir(SA, &bloco_dir, &cont))
                return;

        while ((nome_lido[i] = fgetc(SA)) != '\0')
        {
            i++;
            cont++;
            if (cont >= TAMANHO_BLOCO)
                if (!busca_continuacao_dir(SA, &bloco_dir, &cont))
                    return;
        }
        if (nome_lido[0] == '\0')
            return;
        cont++;
        if (cont >= TAMANHO_BLOCO)
            if (!busca_continuacao_dir(SA, &bloco_dir, &cont))
                return;
        tamanho_entrada = atoi(nome_lido);
        i = 0;
        while ((nome_lido[i] = fgetc(SA)) != '\0')
        {
            i++;
            cont++;
            if (cont >= TAMANHO_BLOCO)
                if (!busca_continuacao_dir(SA, &bloco_dir, &cont))
                    return;
        }
        cont++;
        i = 0;

        //Aqui já lemos o nome

        if (tamanho_entrada > calcula_tamanho_metadados(nome_lido, -1))
        {
            //Arquivo
            fprintf(stderr, "%s ", nome_lido);
            int j, digitos_bytes;
            for (j = 0; j < 15 - strlen(nome_lido); j++) fprintf(stderr, " ");
            
            digitos_bytes = tamanho_entrada - calcula_tamanho_metadados(nome_lido, -1);
            for (j = 0; j < digitos_bytes; j++)
                fprintf(stderr, "%c", fgetc(SA));

            for (j = 0; j < 17 - digitos_bytes; j++) fprintf(stderr, " ");

            fseek(SA, TAMANHO_TIME * 2, SEEK_CUR);
            for (j = 0; j < TAMANHO_TIME; j++)
            {
                nome_lido[j] = fgetc(SA);
            }
            nome_lido[j] = '\0';
            currtime = atol(nome_lido);
            fprintf(stderr, " %s", ctime(&currtime));
        }
        else
        {
            //Diretorio
            fprintf(stderr, "%s/ ", nome_lido);
            int j;
            for (j = 0; j < 15 - strlen(nome_lido); j++) fprintf(stderr, " ");
            fseek(SA, TAMANHO_TIME * 2, SEEK_CUR);
            for (j = 0; j < TAMANHO_TIME; j++)
            {
                nome_lido[j] = fgetc(SA);
            }
            nome_lido[j] = '\0';
            currtime = atol(nome_lido);
            fprintf(stderr, " %s", ctime(&currtime));
        }
        fseek(SA, FAT_ENTRY + 1, SEEK_CUR);
    }
}

//Essa função altera o valor de bloco_dir para o proximo valor na FAT (caso != -1), altera cont usado na função acima e devolve 1 caso o diretorio possua mais um bloco e 0 caso contrário
int busca_continuacao_dir(FILE *SA, int *bloco_dir, int *cont)
{
    if (FAT[*bloco_dir] != -1)
    {
        *bloco_dir = FAT[*bloco_dir];
        *cont = *cont - TAMANHO_BLOCO;
        fseek(SA, (*bloco_dir) * TAMANHO_BLOCO + *cont, SEEK_SET);
        return 1;
    }
    else
    {
        return 0;
    }
}

//Nome+tamanho(arquivos)+tempos+endereço
//"tamanho = -1" se for diretorio. "tamanho = bytes" caso arquivo regular
int calcula_tamanho_metadados(char *nome, int tamanho)
{
    int tam_digitos = digitos(tamanho);
    //30 é a soma dos tempos (acesso, modificação...), 5 é entrada da FAT e 1 é |
    return strlen(nome) + 1 + 30 + tam_digitos + 5 + 1;
}

//Por enquanto, esta função irá manter o ponteiro SA na primeira posição vazia do bloco
int espaco_restante_bloco(FILE *SA, int n_bloco)
{
    int pos_anterior = ftell(SA), pos_atual;
    fseek(SA, TAMANHO_BLOCO * n_bloco, SEEK_SET);
    int ant = 0;
    char penultimo, aux;

    for (int i = 0; i < TAMANHO_BLOCO; i++)
    {
        if ((aux = getc(SA)) == '\0')
        {
            ant++;
            if (ant == 2)
                break;
        }
        else
        {
            ant = 0;
            penultimo = aux;
        }
    }
    if (ant == 1 && penultimo == '|')
        return 1;
    if (ant <= 1)
    {
        //fseek(SA, pos_anterior, SEEK_SET);
        return 0;
    }
    pos_atual = ftell(SA) - 2;

    //fseek(SA, pos_anterior, SEEK_SET);
    fseek(SA, -2, SEEK_CUR);

    return (TAMANHO_BLOCO * (n_bloco + 1)) - pos_atual;
}

//Essa função não é muito legal. Diz quanto espaço tem mas não em qual bloco. Poderia
//passar um ponteiro de booleano como parametro e deixá-lo verdadeiro caso tenha espaço e devolver o numero
//do bloco e falso caso não tenha espaço e o numero do bloco
//Talvez seja util da forma que está. Bastaria adicionar alocação de novo bloco na próxima função
int espaco_restante_diretorio(FILE *SA, int *bloco_dir)
{
    int espaco_atual = espaco_restante_bloco(SA, *bloco_dir);
    while (FAT[*bloco_dir] != -1 && espaco_atual == 0)
    {
        *bloco_dir = FAT[*bloco_dir];
        espaco_atual = espaco_restante_bloco(SA, *bloco_dir);
    }
    return espaco_atual;
}

//Adiciona entrada com metadados de um arquivo a um diretorio
//bloco_dir = bloco do diretorio pai, nome = nome do arquivo, tamanho = -1 se diretorio, bloco_alocado = bloco alocado para o arquivo
//Acho que farei com que sempre que essa função seja chamada, o diretorio ja tenha espaço para essa entrada
//Da forma que está, SA já está posicionado no byte que quero escrever
void adiciona_metadados_arquivo(FILE *SA, int bloco_dir, char *nome, int tamanho, int bloco_alocado)
{
    int tamanho_entrada = calcula_tamanho_metadados(nome, tamanho);
    char e1[5], e2[40];
    time(&rawtime);
    sprintf(e1, "%d", tamanho_entrada);
    sprintf(e2, "%10ld%10ld%10ld%05d|", rawtime, rawtime, rawtime, bloco_alocado);
    for (int i = 0; i < strlen(e1); i++)
    {
        acabou_bloco(SA, &bloco_dir);
        fputc(e1[i], SA);
    }
    acabou_bloco(SA, &bloco_dir);
    fputc('\0', SA);
    for (int i = 0; i < strlen(nome); i++)
    {
        acabou_bloco(SA, &bloco_dir);
        fputc(nome[i], SA);
    }
    acabou_bloco(SA, &bloco_dir);
    fputc('\0', SA);
    if (tamanho != -1)
    {
        char tam[10];
        sprintf(tam, "%d", tamanho);
        for (int i = 0; i < strlen(tam); i++)
        {
            acabou_bloco(SA, &bloco_dir);
            fputc(tam[i], SA);
        }
    }
    for (int i = 0; i < strlen(e2); i++)
    {
        acabou_bloco(SA, &bloco_dir);
        fputc(e2[i], SA);
    }
}

void acabou_bloco(FILE *SA, int *bloco_dir)
{
    if (ftell(SA) % TAMANHO_BLOCO == 0)
    {
        *bloco_dir = FAT[*bloco_dir];
        fseek(SA, TAMANHO_BLOCO * (*bloco_dir), SEEK_SET);
    }
}

//Função para ser usada com cat
void imprime_arquivo(FILE *SA, char *nome)
{
    int cont = 0, i = 0, tamanho_entrada;
    int bloco_dir = busca_diretorio_pai(SA, "/", nome, BLOCO_ROOT);

    if (bloco_dir == BLOCO_ROOT)
    {
        volta_pro_root(SA);
        cont += TAMANHO_METADADOS_ROOT;
    }

    char nome_lido[255], c;
    remove_dirs_nome(nome);
    nome_lido[0] = '\0';


    while (strcmp(nome_lido, nome) != 0)
    {

        if (cont >= TAMANHO_BLOCO)
            if (!busca_continuacao_dir(SA, &bloco_dir, &cont))
                return;

        //Aqui, nome_lido contém o nome errado e SA aponta para o primeiro caracter após \0
        while ((nome_lido[i] = fgetc(SA)) != '\0')
        {
            i++;
            cont++;
            //Não sei se deve ser == ou >
            if (cont >= TAMANHO_BLOCO)
                if (!busca_continuacao_dir(SA, &bloco_dir, &cont))
                    return;
        }
        if (nome_lido[0] == '\0')
            return;
        cont++;
        if (cont >= TAMANHO_BLOCO)
            if (!busca_continuacao_dir(SA, &bloco_dir, &cont))
                return;

        tamanho_entrada = atoi(nome_lido);
        i = 0;
        while ((nome_lido[i] = fgetc(SA)) != '\0')
        {
            i++;
            cont++;
            if (cont >= TAMANHO_BLOCO)
                if (!busca_continuacao_dir(SA, &bloco_dir, &cont))
                    return;
        }
        cont++;
        i = 0;
        cont += tamanho_entrada - (strlen(nome_lido) + 1);

        fseek(SA, tamanho_entrada - (strlen(nome_lido) + 1), SEEK_CUR);
        //Aqui, estou no nome da próxima entrada (ou em \0 se o diretorio tiver acabado)
    }
    fseek(SA, -FAT_ENTRY - 1, SEEK_CUR);
    for (i = 0; i < FAT_ENTRY; i++)
    {
        nome_lido[i] = fgetc(SA);
        cont++;
        if (cont >= TAMANHO_BLOCO)
            if (!busca_continuacao_dir(SA, &bloco_dir, &cont))
                return;
    }
    nome_lido[i] = '\0';
    int bloco_entrada = atoi(nome_lido);
    fseek(SA, bloco_entrada * TAMANHO_BLOCO, SEEK_SET);

    char aux;
    do
    {
        for (i = 0; i < TAMANHO_BLOCO && ((aux = fgetc(SA)) != '\0'); i++)
            fprintf(stderr, "%c", aux);
        bloco_entrada = FAT[bloco_entrada];
        fseek(SA, bloco_entrada * TAMANHO_BLOCO, SEEK_SET);
    } while (bloco_entrada != -1);
    fprintf(stderr, "\n");
}

void atualiza_tempos(FILE *SA, char *dir_pai)
{
    int bloco_dir_pai = busca_diretorio_pai(SA, "/", dir_pai, BLOCO_ROOT);
    char dir_reduzido[255];
    strcpy(dir_reduzido, dir_pai);
    remove_dirs_nome(dir_reduzido);
    int t = procura_nome_e_devolve_info(SA, dir_reduzido, SETPOSITIONTOTIME, bloco_dir_pai);
    if (t != -1)
    {
        //Aqui, SA aponta para o primeiro byte de tempo de acesso
        time(&rawtime);
        fprintf(SA, "%ld", rawtime);
        fprintf(SA, "%ld", rawtime);
    }
    else if (bloco_dir_pai == BLOCO_ROOT)
    {
        time(&rawtime);
        fseek(SA, BLOCO_ROOT * TAMANHO_BLOCO + 1, SEEK_SET);
        fprintf(SA, "%ld ", rawtime);
        fprintf(SA, "%ld", rawtime);
    }
    else
    {
        fprintf(stderr, "O diretório %s não foi encontrado\n", dir_pai);
    }
}

int digitos(int n)
{
    int tam_digitos = 1;
    if (n == -1)
        tam_digitos = 0;
    while (n > 9)
    {
        n /= 10;
        tam_digitos++;
    }
    return tam_digitos;
}
