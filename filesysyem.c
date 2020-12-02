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
//Tamanho das entradas na FAT
#define FAT_ENTRY 5
//Macros do Bitmap (Talvez tenha que inverter, não lembro o que o daniel disse que preferia)
#define LIVRE '1'
#define OCUPADO '0'

//Por enquanto ficará hardcoded já que sabemos o tamanho da tabela (acho q começa com zeros)
int FAT[N_BLOCOS];
char bitmap[N_BLOCOS];

FILE *mount (char *nome_arquivo);
void cria_FAT(FILE *f);
void carrega_FAT(FILE *f);
void cria_bitmap(FILE *f);
void carrega_bitmap(FILE *f);
void cria_root(FILE* f);

void preenche_bloco_vazio(FILE *f, int n_bloco, int offset);

time_t rawtime;

int main(int argc, char **argv)
{
    mount("abc.txt");
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
    for (int i = BLOCOS_FAT+BLOCOS_BITMAP+1; i < N_BLOCOS; i++)
        preenche_bloco_vazio(f, i, 0);
    volta_pro_root(f);
    
    return f;
}

//Recebe um arquivo vazio e escreve a FAT nele, também a armazenando na memória
void cria_FAT(FILE *f)
{
    int i;
    //Primeiros 32 blocos pertencem a FAT
    for (i = 0; i < BLOCOS_FAT-1; i++) FAT[i] = i+1;
    FAT[i] = -1;
    //Proximos 7 blocos pertencem ao bitmap
    for (i = 32; i < BLOCOS_FAT+BLOCOS_BITMAP-1; i++) FAT[i] = i+1;
    FAT[i] = -1;
    //Primeiro bloco pertence ao '/'
    FAT[i+1] = -1;
    //Depois vejo se preciso zerar o resto dos elementos

    //Devo escrever no arquivo já aqui?
    for (i = 0; i < N_BLOCOS; i++) fprintf(f, "%05d", FAT[i]);
    preenche_bloco_vazio(f, BLOCOS_FAT-1, (N_BLOCOS*FAT_ENTRY)%TAMANHO_BLOCO);
}

//Recebe um arquivo vazio e carrega a FAT escrita nele
void carrega_FAT(FILE *f)
{
    fseek(f, 0, SEEK_SET);
    char buf[6];
    for (int i = 0; i < N_BLOCOS; i++)
    {
        fgets(buf, FAT_ENTRY+1, f);
        FAT[i] = atoi(buf);
    }
    //for (int i = 0; i < 50; i++) printf("FAT[%d] = %d\n", i, FAT[i]);
}

//Recebe um arquivo contendo apenas FAT e escreve o bitmap nele, também o armazenando na memória
void cria_bitmap(FILE *f)
{
    //BLOCOS_FAT+BLOCOS_BITMAP+1 engloba blocos destas duas estruturas + bloco inicial do '/'
    int i;
    for (i = 0; i < BLOCOS_FAT+BLOCOS_BITMAP+1; i++) 
    {
        bitmap[i] = OCUPADO;
        fputc(OCUPADO, f);
    }
    for (int j = i; j < N_BLOCOS; j++) 
    {
        bitmap[j] = LIVRE;
        fputc(LIVRE, f);
    }
    preenche_bloco_vazio(f, BLOCOS_FAT+BLOCOS_BITMAP-1, N_BLOCOS%TAMANHO_BLOCO);
}

//Recebe um arquivo vazio e carrega o bitmap escrita nele
void carrega_bitmap(FILE *f)
{
    fseek(f, TAMANHO_BLOCO*(BLOCOS_FAT), SEEK_SET);
    char buf[1];
    for (int i = 0; i < N_BLOCOS; i++)
    {
        fgets(buf, 2, f);
        bitmap[i] = atoi(buf);
    }
    //for (int i = 0; i < 50; i++) printf("bitmap[%d] = %d\n", i, bitmap[i]);
}

//O root conterá os próprios metadados?
void cria_root(FILE* f)
{
    fputc('/', f);
    time(&rawtime);
    fprintf(f, "%ld ", rawtime);
    fprintf(f, "%ld ", rawtime);
    fprintf(f, "%ld|", rawtime);
}

void preenche_bloco_vazio(FILE *f, int n_bloco, int offset)
{
    fseek(f, (n_bloco*TAMANHO_BLOCO + offset), SEEK_SET);
    for (int i = offset; i < TAMANHO_BLOCO; i++)
    {
        fputc(0, f);
    }
}

void volta_pro_root(FILE* f)
{
    fseek(f, TAMANHO_BLOCO*(BLOCOS_BITMAP+BLOCOS_FAT) + TAMANHO_METADADOS_ROOT, SEEK_SET);
}

//SA = Sistema de arquivos
//add_arquivo deve conferir se há tamanho suficiente para o arquivo, buscar o diretorio onde deve ser salvo o arquivo, salvar seus metadados nesse diretorio e então salvar o arquivo em um ou mais blocos
//add_arquivo sempre partirá do root
int add_arquivo(FILE *SA, char *nome_origem, char *nome_destino)
{
    int tamanho, blocos_arquivo;
    char *diretorio_atual = "/";
    FILE *arq;
    if(!(arq = fopen(nome_origem, "r")))
    {
        fprintf(stderr, "O arquivo %s não pôde ser aberto\n", nome_origem);
        return -1;
    }
    fseek(arq, 0, SEEK_END);
    tamanho = ftell(arq);
    blocos_arquivo = tamanho/TAMANHO_BLOCO;
    if (tamanho%TAMANHO_BLOCO != 0) blocos_arquivo++;
    rewind(arq);

    int cont = 0;
    //Ler de trás pra frente pois pro EP será mais rápido
    for (int i = N_BLOCOS-1; i >= 0; i--)
    {
        if (bitmap[i] == LIVRE) {
            cont++;
            if (cont == blocos_arquivo) break;
        }
    }
    if (cont < blocos_arquivo)
    {
        fprintf(stderr, "Não há espaço suficiente para adicionar %s ao sistema\n", nome_origem);
        fclose(arq);
        return -1;
    }

    //Aqui, devemos iniciar uma busca até encontrarmos o diretório no qual o arquivo deverá ser salvo
    cont = 0;
    for (int i = 0; i < strlen(nome_destino); i++)
    {
        if (nome_destino[i] == '/') cont++;
    }
    //Significa que o arquivo deve estar no root
    if (cont <= 1);

}

int busca_diretorio_para_add(FILE *SA, FILE *arq, char *dir_atual, char *nome_atual, int tamanho)
{
    if (nome_atual[0] != '/')
    {
        //Chegamos no diretório
        if (calcula_tamanho_metadados(nome_atual, tamanho) <= espaco_restante_bloco(SA, ftell(SA)%TAMANHO_BLOCO)){
            //Adiciona metadados e copia arquivo para um bloco vazio
        }
        else{
            //Avisa que não há espaço (ainda preciso adicionar as funções da FAT. Por enquanto ele olha um bloco só, tem q ver se o bloco continua (e isso é suave, só jogar o numero do bloco na fat e ver se é -1))
        }
    }
    char dir_aux[255];
    dir_aux[0] = '\0';
    int i;
    for (i = 1; i < strlen(nome_atual); i++)
    {
        if (nome_atual[i] == '/') break;
        dir_aux[i-1] = nome_atual[i];
    }
    if (i == strlen(nome_atual)) 
    {
        busca_diretorio_para_add(SA, arq, dir_atual, dir_aux, tamanho);
    
    }
    else
    {

    }


}

int encontra_bloco_diretorio(FILE *f, char *nome_dir)
{
    //Buscar no diretorio atual a entrada do proximo diretorio 'nome_dir', encontrando nela o valor do bloco do diretorio
}

//Nome+tamanho(arquivos)+tempos+endereço
int calcula_tamanho_metadados(char *nome, int tamanho)
{
    int tam_digitos = 1, aux = tamanho;
    if (aux == -1) tam_digitos = 0;
    while (aux > 9) {
        aux /= 10;
        tam_digitos++;
    }
    //1 será identificação se é arquivo ou diretório, 33 é a soma dos tempos (acesso, modificação...), 5 é entrada da FAT e 1 é |
    return 1+strlen(nome)+1+33+tam_digitos+5+1;
}

int espaco_restante_bloco(FILE *f, int n_bloco)
{
    int pos_anterior = ftell(f), pos_atual;
    fseek(f, TAMANHO_BLOCO*n_bloco, SEEK_SET);
    int ant = 0;

    for (int i = 0; i < TAMANHO_BLOCO; i++)
    {
        if(getc(f) == '\0')
        {
           ant++;
           if (ant == 2) break;
        }
        else ant = 0;
    }
    if (ant <= 1) 
    {
        fseek(f, pos_anterior, SEEK_SET);
        return 0;
    }
    pos_atual = ftell(f) - 2;

    fseek(f, pos_anterior, SEEK_SET);

    return (TAMANHO_BLOCO*(n_bloco+1)) - pos_atual;
}