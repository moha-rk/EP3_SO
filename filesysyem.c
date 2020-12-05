#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#define N_BLOCOS 25000
#define TAMANHO_BLOCO 4000
#define TAMANHO_METADADOS_ROOT 34
#define BLOCOS_FAT 32
#define BLOCOS_BITMAP 7
#define BLOCO_ROOT 39
//Tamanho das entradas na FAT
#define FAT_ENTRY 5
//Macros do Bitmap (Talvez tenha que inverter, não lembro o que o daniel disse que preferia)
#define LIVRE '1'
#define OCUPADO '0'

//Por enquanto ficará hardcoded já que sabemos o tamanho da tabela (acho q começa com zeros)
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

int add_arquivo(FILE *SA, char *nome_origem, char *nome_destino);
int procura_nome_e_devolve_info(FILE *SA, char *nome, int info, int bloco_dir);

int cria_diretorio(FILE *SA, char *nome_origem);
void lista_itens_diretorio(FILE *SA, char *nome);
int busca_continuacao_dir(FILE *SA, int *bloco_dir, int *cont);
int busca_diretorio(FILE *SA, char *dir_atual, char *nome_atual, int bloco_dir);
int espaco_restante_diretorio(FILE *SA, int *bloco_dir);
char *remove_dirs_nome(char *nome);

void adiciona_metadados_arquivo(FILE *SA, int bloco_dir, char *nome, int tamanho, int bloco_alocado);
int calcula_tamanho_metadados(char *nome, int tamanho);
int digitos(int n);

time_t rawtime;

int main(int argc, char **argv)
{
    FILE *SA = mount("abc.txt");

    // fseek(SA, BLOCO_ROOT*TAMANHO_BLOCO + TAMANHO_METADADOS_ROOT, SEEK_SET);
    // fputs("39", SA);
    // fputc('\0', SA);
    // fputs("d1", SA);
    // fputc('\0', SA);
    // fputs("12345678901234567890123456789000321|", SA);
    // //FAT[321] = -1;
    // aloca_bloco(SA, 321);
    // printf ("\n%d\n", procura_nome_e_devolve_info(SA, "d1", 1, BLOCO_ROOT));
    // char *a = "/d1/arquivo.txt";
    // add_arquivo(SA, "ideias.txt", "/d1/arquivo.txt");
    // //printf ("\n%d\n", busca_diretorio(SA, "/", a, BLOCO_ROOT));
    // //printf("%s", a);

    fclose(SA);
    return 0;
}

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
    printf("criou arquivo");
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
//Adicionar forma de usar essa função para o touch
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
    int dir = busca_diretorio(SA, "/", nome_destino, BLOCO_ROOT);
    if (dir == -1)
        return -1;
    nome_destino = remove_dirs_nome(nome_destino);
    int tam_metadados = calcula_tamanho_metadados(nome_destino, -1);
    if (espaco_restante_diretorio(SA, &dir) < tam_metadados + digitos(tam_metadados) + 1)
    {
        //Aloco novo bloco para o diretorio
        cont = 0;
        for (int i = primeiro + 1; i < N_BLOCOS; i++)
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
        {
            fprintf(stderr, "Não há espaço suficiente para adicionar %s ao sistema\n", nome_origem);
            fclose(arq);
            return -1;
        }
        estende_bloco(SA, dir, primeiro);
        primeiro = segundo;
    }
    //Resta apenas alocar aqui o bloco primeiro (talvez outros) para o arquivo em questão e salvar seus dados lá
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
    free(nome_destino);
    return 0; //Acho
}

int cria_diretorio(FILE *SA, char *nome_origem)
{
    //aloca bloco para o diretorio

    //retorna bloco do diretorio
    return 0;
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

char *remove_dirs_nome(char *nome)
{
    int i, j = 0;
    char *aux;
    aux = malloc(strlen(nome) * sizeof(char));
    for (i = 0; i < strlen(nome); i++)
    {
        aux[j] = nome[i];
        j++;
        if (nome[i] == '/')
            j = 0;
    }
    aux[j + 1] = '\0';
    return aux;
}

//dir_atual deve ser chamado com "/" e bloco_dir com o bloco do root
//Devo alimentar a função com um caminho contendo o nome do arquivo no final?
//entradas serão do tipo "/tmp/d1/arquivo.txt" ou "/tmp/d1"?
//Acho que vou fazer uma função que busca o penultimo diretorio. Nos exemplos acima, deve buscar d1 e tmp.

//Esta função devolve o bloco do diretorio anterior à última entrada (arquivo.txt ou d1 no exemplo acima)
//dir_atual deve ser chamado com "/" e bloco_dir com o bloco do root, e nome_atual é o q queremos buscar
int busca_diretorio(FILE *SA, char *dir_atual, char *nome_atual, int bloco_dir)
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
        return busca_diretorio(SA, dir_atual, nome_atual, bloco_dir);
    }
    else
    {
        //Preciso mexer *SA para o bloco de dir_aux antes de chamar a função novamente
        bloco_dir = procura_nome_e_devolve_info(SA, dir_aux, 1, bloco_dir);
        if (bloco_dir == -1)
        {
            fprintf(stderr, "%s não foi encontrado em %s\n", dir_aux, dir_atual);
            volta_pro_root(SA);
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
        return busca_diretorio(SA, dir_aux, nome_atual, bloco_dir);
    }
}

//Procura o nome de um arquivo (regular ou diretorio) e devolve a informação pedida supondo que SA está no início do diretório a ser vasculhado
int procura_nome_e_devolve_info(FILE *SA, char *nome, int info, int bloco_dir)
{
    //cont armazenará quantos bytes foram lidos do bloco atual
    int cont = 0, i = 0, tamanho_entrada, aux, tam_digitos;

    if (bloco_dir == BLOCO_ROOT)
    {
        volta_pro_root(SA);
        cont += TAMANHO_METADADOS_ROOT;
    }

    char nome_lido[255];
    while ((nome_lido[i] = fgetc(SA)) != '\0')
        i++;
    if (nome_lido[0] == '\0')
    {
        return -1;
    }
    tamanho_entrada = atoi(nome_lido);
    i = 0;
    while ((nome_lido[i] = fgetc(SA)) != '\0')
        i++;
    i = 0;
    aux = tamanho_entrada;
    tam_digitos = 0;
    while (aux > 9)
    {
        aux /= 10;
        tam_digitos++;
    }
    cont += tamanho_entrada + tam_digitos + 1;

    while (strcmp(nome_lido, nome) != 0)
    {
        if (cont >= TAMANHO_BLOCO)
            if (!busca_continuacao_dir(SA, &bloco_dir, &cont))
                return -1;

        //Aqui, nome_lido contém o nome errado e SA aponta para o primeiro caracter após \0
        //tamanho_entrada\0nome\0(tamanho)tempo1tempo2tempo3FAT|
        //tamanho_entrada = strlen(nome)+1+ndigitosTamanho+30+5+1
        fseek(SA, ftell(SA) + tamanho_entrada - (strlen(nome_lido) + 1), SEEK_SET);
        //Aqui, estou no nome da próxima entrada (ou em \0 se o diretorio tiver acabado)

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
            //Não sei se deve ser == ou > (acho q ==, por isso vou manter assim)
            if (cont >= TAMANHO_BLOCO)
                if (!busca_continuacao_dir(SA, &bloco_dir, &cont))
                    return -1;
        }
        cont++;
        i = 0;
        cont += tamanho_entrada - (strlen(nome_lido) + 1);
    }
    //*SA aponta para o tamanho (caso seja arquivo regular) ou para o primeiro tempo caso seja um diretorio
    //Depois adiciono mais infos para devolver, por enquanto devolverei apenas o diretorio FAT
    if (info == 1) //Numero generico
    {
        fseek(SA, ftell(SA) + tamanho_entrada - (strlen(nome_lido) + 1) - FAT_ENTRY, SEEK_SET);

        for (i = 0; i < 5; i++)
            nome_lido[i] = fgetc(SA);
        nome_lido[i] = '\0';
        return atoi(nome_lido); //FAT
    }
}

void lista_itens_diretorio(FILE *SA, char *nome)
{
    //faz um loop com a função procura_nome_e_devolve_info
    //dai usa esse loop pra imprimir as informaço~es
    //printf("nomedoarquivo time1 time2 time3");
}

//Essa função altera o valor de bloco_dir para o proximo valor na FAT (caso != -1), altera cont usado na funcção acima e devolve 1 caso o diretorio possua mais um bloco e 0 caso contrário
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
    fseek(SA, ftell(SA) - 2, SEEK_SET);

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
        if (ftell(SA) - 1 % TAMANHO_BLOCO == 0)
        {
            bloco_dir = FAT[bloco_dir];
            fseek(SA, TAMANHO_BLOCO * bloco_dir, SEEK_SET);
        }
        fputc(e1[i], SA);
    }
    if (ftell(SA) - 1 % TAMANHO_BLOCO == 0)
    {
        bloco_dir = FAT[bloco_dir];
        fseek(SA, TAMANHO_BLOCO * bloco_dir, SEEK_SET);
    }
    fputc('\0', SA);
    for (int i = 0; i < strlen(nome); i++)
    {
        if (ftell(SA) - 1 % TAMANHO_BLOCO == 0)
        {
            bloco_dir = FAT[bloco_dir];
            fseek(SA, TAMANHO_BLOCO * bloco_dir, SEEK_SET);
        }
        fputc(nome[i], SA);
    }
    if (ftell(SA) - 1 % TAMANHO_BLOCO == 0)
    {
        bloco_dir = FAT[bloco_dir];
        fseek(SA, TAMANHO_BLOCO * bloco_dir, SEEK_SET);
    }
    fputc('\0', SA);
    if (tamanho != -1)
    {
        char tam[10];
        sprintf(tam, "%d", tamanho);
        for (int i = 0; i < strlen(tam); i++)
        {
            if (ftell(SA) - 1 % TAMANHO_BLOCO == 0)
            {
                bloco_dir = FAT[bloco_dir];
                fseek(SA, TAMANHO_BLOCO * bloco_dir, SEEK_SET);
            }
            fputc(tam[i], SA);
        }
    }
    for (int i = 0; i < strlen(e2); i++)
    {
        if (ftell(SA) - 1 % TAMANHO_BLOCO == 0)
        {
            bloco_dir = FAT[bloco_dir];
            fseek(SA, TAMANHO_BLOCO * bloco_dir, SEEK_SET);
        }
        fputc(e2[i], SA);
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
