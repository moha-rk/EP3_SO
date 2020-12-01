#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define N_BLOCOS 25000
#define TAMANHO_BLOCO 4000
#define BLOCOS_FAT 32
#define BLOCOS_BITMAP 7
//Macros do Bitmap (Talvez tenha que inverter, não lembro o que o daniel disse que preferia)
#define LIVRE '1'
#define OCUPADO '0'

//Por enquanto ficará hardcoded já que sabemos o tamanho da tabela (acho q começa com zeros)
int FAT[N_BLOCOS];
char bitmap[N_BLOCOS];

FILE *mount (char *nome_arquivo);
void cria_FAT(FILE *f);
void cria_BitMap(FILE *f);

void preenche_bloco_vazio(FILE *f, int n_bloco, int offset);

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
    cria_BitMap(f);
    fclose(f);
    
    return NULL;
    
    
}

//Recebe um arquivo vazio e escreve a FAT nele, também a armazenando na memória
void cria_FAT(FILE *f)
{
    int i;
    //Primeiros 32 blocos pertencem a FAT
    for (i = 0; i < BLOCOS_FAT-1; i++) FAT[i] = i+1;
    FAT[i] = -1;
    //Proximos 7 blocos pertencem ao bitmap
    for (i = 32; i < 32+BLOCOS_BITMAP-1; i++) FAT[i] = i+1;
    FAT[i] = -1;
    //Primeiro bloco pertence ao /
    FAT[i+1] = -1;
    //Depois vejo se preciso zerar o resto dos elementos

    //Devo escrever no arquivo já aqui?
    for (i = 0; i < N_BLOCOS; i++) fprintf(f, "%05d", FAT[i]);
    preenche_bloco_vazio(f, BLOCOS_FAT-1, (N_BLOCOS*5)%TAMANHO_BLOCO);
}

void cria_BitMap(FILE *f)
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

void preenche_bloco_vazio(FILE *f, int n_bloco, int offset)
{
    fseek(f, (n_bloco*TAMANHO_BLOCO + offset), SEEK_SET);
    for (int i = offset; i < TAMANHO_BLOCO; i++)
    {
        fputc(0, f);
    }
}


/*if (access(path, F_OK) == 0)
		return true;
	else
		return false;*/