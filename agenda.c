/*
 * ============================================================================
 *  TRABALHO FINAL - PROGRAMACAO COMPUTACIONAL - 2026.2
 *  Tema 10: Agenda Eletronica (structs + arquivo binario dados.dat)
 *  Prof.: Fernando Rodrigues de Almeida Junior
 *
 *  Equipe: <NUMERO DA EQUIPE>
 *  Membros: <Nome 1> - <Matricula 1>
 *           <Nome 2> - <Matricula 2>
 *           <Nome 3> - <Matricula 3>
 *
 *  Padrao: C ANSI (C89/C90). Compilar com:
 *      gcc -ansi -pedantic -Wall -o agenda agenda.c
 *
 *  Bibliotecas usadas (todas permitidas): stdio, stdlib, string, ctype, locale
 * ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <locale.h>

/* ---------------------------------------------------------------------------
 * CONSTANTES
 * Tamanhos dos campos = limite de caracteres + 1 (espaco para o '\0').
 * ------------------------------------------------------------------------ */
#define TAM_NOME     50   /* enunciado: nome com ate 50 caracteres */
#define TAM_FONE     20
#define TAM_EMAIL    60
#define TAM_RUA      60
#define TAM_NUMERO   10
#define TAM_COMPL    30
#define TAM_BAIRRO   40
#define TAM_CIDADE   40   /* campo "Cidade/Estado", ex.: Sobral/CE */

#define ARQUIVO_DADOS "dados.dat"
#define CAPACIDADE_INICIAL 10

/* Valores possiveis do estado civil (so estes 4 sao aceitos) */
#define SOLTEIRO   0
#define CASADO     1
#define SEPARADO   2
#define VIUVO      3

/* Nomes exibidos para cada estado civil, indexados pelas constantes acima */
static const char *NOMES_ESTADO_CIVIL[4] = {
    "Solteiro", "Casado", "Separado", "Viuvo"
};

/* ---------------------------------------------------------------------------
 * ESTRUTURAS
 * IMPORTANTE: nenhuma struct contem ponteiros, so vetores de char e int.
 * Assim podemos gravar a struct inteira no arquivo binario com fwrite e
 * le-la de volta com fread, sem perder dados.
 * ------------------------------------------------------------------------ */

/* Endereco: struct aninhada dentro do contato */
typedef struct {
    char rua[TAM_RUA + 1];
    char numero[TAM_NUMERO + 1];       /* char para aceitar "S/N", "12A" */
    char complemento[TAM_COMPL + 1];   /* opcional */
    char bairro[TAM_BAIRRO + 1];
    char cidade_estado[TAM_CIDADE + 1];
} Endereco;

/* Contato: um registro completo da agenda */
typedef struct {
    char nome[TAM_NOME + 1];
    char fixo[TAM_FONE + 1];           /* opcional */
    char celular1[TAM_FONE + 1];
    char celular2[TAM_FONE + 1];       /* opcional */
    char email[TAM_EMAIL + 1];
    Endereco endereco;
    int estado_civil;                  /* SOLTEIRO, CASADO, SEPARADO, VIUVO */
} Contato;

/* ---------------------------------------------------------------------------
 * PROTOTIPOS
 * ------------------------------------------------------------------------ */
int  ler_linha(char *destino, int tamanho);
int  so_espacos(const char *s);
void ler_texto(const char *rotulo, char *destino, int max, int obrigatorio);
void ler_telefone(const char *rotulo, char *destino, int obrigatorio,
                  const Contato *atual, const Contato *agenda, int total);
void ler_email(char *destino);
int  somente_letras(const char *s);
void ler_nome(char *destino);
void ler_rua(char *destino);
void ler_complemento(char *destino);
void ler_bairro(char *destino);
int  cidade_estado_valido(const char *s);
void ler_cidade_estado(char *destino);
int  ler_inteiro(const char *mensagem, int minimo, int maximo);
int  telefone_valido(const char *s);
void so_digitos(const char *s, char *saida);
int  mesmo_telefone(const char *a, const char *b);
int  telefone_repetido(const char *tel, const Contato *atual,
                       const Contato *agenda, int total);
int  email_valido(const char *s);

void ler_contato(Contato *c, const Contato *agenda, int total);
void exibir_contato(const Contato *c, int indice);
void exibir_todos(const Contato *agenda, int total);

int  adicionar_contato(Contato **agenda, int *total, int *capacidade,
                       const Contato *novo);
int  carregar_dados(Contato **agenda, int *total, int *capacidade);
int  salvar_dados(const Contato *agenda, int total);
void cadastrar_varios(Contato **agenda, int *total, int *capacidade, int n);

/* >>>>> COLE AQUI O BLOCO DA PESSOA B <<<<< */
/* (veja o arquivo parte_B_entrada_validacao_exibicao.c) */

/* ===========================================================================
 *  GERENCIAMENTO DA AGENDA (memoria dinamica)
 *  A agenda e um vetor alocado com malloc e ampliado com realloc quando
 *  enche; assim nao ha limite fixo de contatos.
 * ========================================================================= */

/*
 * adicionar_contato: copia 'novo' para o fim da agenda, aumentando a
 * capacidade (dobrando) se necessario.
 * Recebe Contato ** porque realloc pode mudar o endereco do vetor.
 * Retorno: 1 = sucesso, 0 = falta de memoria.
 */
int adicionar_contato(Contato **agenda, int *total, int *capacidade,
                      const Contato *novo)
{
    Contato *tmp;
    int nova_cap;

    if (*total >= *capacidade) {
        nova_cap = (*capacidade) * 2;
        tmp = (Contato *) realloc(*agenda, nova_cap * sizeof(Contato));
        if (tmp == NULL) {
            return 0;   /* *agenda continua valido, nada se perde */
        }
        *agenda = tmp;
        *capacidade = nova_cap;
    }
    (*agenda)[*total] = *novo;   /* copia a struct inteira */
    (*total)++;
    return 1;
}

/* cadastrar_varios: le e adiciona n contatos em sequencia */
void cadastrar_varios(Contato **agenda, int *total, int *capacidade, int n)
{
    int i;
    Contato novo;

    for (i = 1; i <= n; i++) {
        printf("\n########## Cadastro %d de %d ##########\n", i, n);
        /* Passa a agenda para ler_contato conferir telefones repetidos */
        ler_contato(&novo, *agenda, *total);
        if (!adicionar_contato(agenda, total, capacidade, &novo)) {
            printf("\nErro: memoria insuficiente. Cadastro interrompido.\n");
            return;
        }
        /* Salva logo apos cada cadastro: nada se perde se o programa for
         * fechado de repente (Ctrl+C, janela fechada, etc.) */
        if (!salvar_dados(*agenda, *total)) {
            printf("\nAviso: nao foi possivel gravar %s.\n", ARQUIVO_DADOS);
        }
        printf("\nContato cadastrado com sucesso!\n");
    }
}

/* ===========================================================================
 *  ARQUIVO BINARIO (dados.dat)
 * ========================================================================= */

/*
 * carregar_dados: le todos os registros de dados.dat para a memoria.
 * Se o arquivo nao existe (primeira execucao), simplesmente retorna 1
 * com total = 0. Retorno: 1 = ok, 0 = erro de memoria.
 */
int carregar_dados(Contato **agenda, int *total, int *capacidade)
{
    FILE *arq;
    Contato lido;

    arq = fopen(ARQUIVO_DADOS, "rb");
    if (arq == NULL) {
        return 1;   /* primeira execucao: ainda nao ha arquivo */
    }

    /* fread devolve quantos registros completos leu; 1 enquanto houver dado */
    while (fread(&lido, sizeof(Contato), 1, arq) == 1) {
        /* O arquivo pode estar corrompido ou ter sido editado: garante o
         * terminador '\0' em todas as strings para o printf nao ler alem
         * do vetor */
        lido.nome[TAM_NOME] = '\0';
        lido.fixo[TAM_FONE] = '\0';
        lido.celular1[TAM_FONE] = '\0';
        lido.celular2[TAM_FONE] = '\0';
        lido.email[TAM_EMAIL] = '\0';
        lido.endereco.rua[TAM_RUA] = '\0';
        lido.endereco.numero[TAM_NUMERO] = '\0';
        lido.endereco.complemento[TAM_COMPL] = '\0';
        lido.endereco.bairro[TAM_BAIRRO] = '\0';
        lido.endereco.cidade_estado[TAM_CIDADE] = '\0';

        /* Descarta registro com estado civil invalido: usado como indice
         * do vetor NOMES_ESTADO_CIVIL, um valor fora de 0..3 causaria
         * acesso invalido a memoria */
        if (lido.estado_civil < SOLTEIRO || lido.estado_civil > VIUVO) {
            continue;
        }
        if (!adicionar_contato(agenda, total, capacidade, &lido)) {
            fclose(arq);
            return 0;
        }
    }
    fclose(arq);
    return 1;
}

/*
 * salvar_dados: regrava dados.dat com todos os contatos da memoria.
 * Retorno: 1 = ok, 0 = falha ao abrir/gravar.
 */
int salvar_dados(const Contato *agenda, int total)
{
    FILE *arq;
    size_t gravados;

    arq = fopen(ARQUIVO_DADOS, "wb");
    if (arq == NULL) {
        return 0;
    }
    /* Grava o vetor inteiro de uma vez; confere se tudo foi escrito */
    gravados = fwrite(agenda, sizeof(Contato), (size_t) total, arq);
    fclose(arq);
    return gravados == (size_t) total;
}

/* ===========================================================================
 *  PROGRAMA PRINCIPAL
 * ========================================================================= */
int main(void)
{
    Contato *agenda;      /* vetor dinamico de contatos */
    int total = 0;        /* quantidade de contatos em uso */
    int capacidade;       /* quantidade de contatos que cabem no vetor */
    int opcao;
    int n;

    setlocale(LC_ALL, "");   /* usa a configuracao regional do sistema */

    /* Aloca o vetor inicial da agenda */
    capacidade = CAPACIDADE_INICIAL;
    agenda = (Contato *) malloc(capacidade * sizeof(Contato));
    if (agenda == NULL) {
        printf("Erro: memoria insuficiente.\n");
        return 1;
    }

    printf("=====================================\n");
    printf("        AGENDA ELETRONICA\n");
    printf("=====================================\n");

    /* Requisito 4: ao iniciar, carrega e exibe os dados do arquivo */
    if (!carregar_dados(&agenda, &total, &capacidade)) {
        printf("Erro: memoria insuficiente ao carregar os dados.\n");
        free(agenda);
        return 1;
    }

    if (total > 0) {
        printf("\nDados carregados de %s.\n", ARQUIVO_DADOS);
        exibir_todos(agenda, total);
    } else {
        /* Requisitos 2 e 3: agenda vazia -> pede N e cadastra os iniciais */
        printf("\nNenhum dado encontrado. Vamos cadastrar os registros"
               " iniciais.\n");
        n = ler_inteiro("Quantos registros deseja inserir inicialmente"
                        " (N > 0)? ", 1, 10000);
        cadastrar_varios(&agenda, &total, &capacidade, n);
    }

    /* Requisito 5: menu para cadastrar novos dados ou exibir os existentes */
    do {
        printf("\n---------------- MENU ----------------\n");
        printf("1 - Cadastrar novo contato\n");
        printf("2 - Exibir contatos cadastrados\n");
        printf("0 - Salvar e sair\n");
        opcao = ler_inteiro("Opcao: ", 0, 2);

        switch (opcao) {
        case 1:
            cadastrar_varios(&agenda, &total, &capacidade, 1);
            break;
        case 2:
            exibir_todos(agenda, total);
            break;
        case 0:
            break;
        }
    } while (opcao != 0);

    /* Requisito 3: ao final do programa, salva tudo em dados.dat */
    if (salvar_dados(agenda, total)) {
        printf("\n%d contato(s) salvo(s) em %s. Ate logo!\n", total,
               ARQUIVO_DADOS);
    } else {
        printf("\nERRO: nao foi possivel gravar %s!\n", ARQUIVO_DADOS);
    }

    free(agenda);   /* libera a memoria alocada */
    return 0;
}
