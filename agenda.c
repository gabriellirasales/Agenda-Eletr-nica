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

/*
 * PARTE DA PESSOA B - entrada, validacao e exibicao
 * Cole este conteudo no agenda.c, no lugar do marcador
 * ">>>>> COLE AQUI O BLOCO DA PESSOA B <<<<<".
 * Depende das constantes, structs e prototipos definidos na parte da Pessoa A.
 */

/* ===========================================================================
 *  FUNCOES DE ENTRADA (leitura segura)
 *  Usamos fgets em vez de scanf: o scanf deixa '\n' no buffer e nao aceita
 *  espacos em strings (ex.: "Maria da Silva"), alem de poder estourar o vetor.
 * ========================================================================= */

/*
 * ler_linha: le uma linha do teclado para 'destino' (ate tamanho-1 chars),
 * removendo o '\n' final.
 * Retorno:  1 = leitura ok
 *           0 = fim da entrada (EOF, ex.: Ctrl+Z no Windows)
 *          -1 = linha digitada era maior que o permitido (excesso descartado)
 */
int ler_linha(char *destino, int tamanho)
{
    int len;
    int c;

    if (fgets(destino, tamanho, stdin) == NULL) {
        return 0;
    }

    len = (int) strlen(destino);

    /* Caso normal: a linha coube inteira e terminou em '\n' */
    if (len > 0 && destino[len - 1] == '\n') {
        destino[len - 1] = '\0';
        return 1;
    }

    /* Nao achou '\n': ou a linha era grande demais, ou acabou o arquivo.
     * Olhamos o proximo caractere para decidir. */
    c = getchar();
    if (c == EOF || c == '\n') {
        return 1;           /* coube exatamente, ou ultima linha sem '\n' */
    }

    /* Linha grande demais: descarta o resto para nao sujar a proxima leitura */
    while (c != '\n' && c != EOF) {
        c = getchar();
    }
    return -1;
}

/* so_espacos: retorna 1 se a string for vazia ou tiver apenas espacos */
int so_espacos(const char *s)
{
    while (*s != '\0') {
        if (!isspace((unsigned char) *s)) {
            return 0;
        }
        s++;
    }
    return 1;
}

/*
 * ler_texto: pede um texto ao usuario e repete ate ser valido.
 *   rotulo      - nome do campo mostrado ao usuario
 *   destino     - vetor que recebe o texto
 *   max         - maximo de caracteres permitidos
 *   obrigatorio - 1: nao aceita vazio; 0: Enter em branco e permitido
 */
void ler_texto(const char *rotulo, char *destino, int max, int obrigatorio)
{
    char temp[256];  /* buffer auxiliar maior que qualquer campo */
    int r;

    for (;;) {
        printf("%s%s: ", rotulo, obrigatorio ? "" : " (opcional)");
        r = ler_linha(temp, max + 1);

        if (r == 0) {  /* EOF: nao ha como continuar pedindo dados */
            printf("\nEntrada encerrada. Programa finalizado.\n");
            exit(1);
        }
        if (r == -1) {
            printf("  Erro: maximo de %d caracteres. Tente novamente.\n", max);
            continue;
        }
        if (obrigatorio && so_espacos(temp)) {
            printf("  Erro: este campo e obrigatorio.\n");
            continue;
        }
        if (so_espacos(temp)) {
            destino[0] = '\0';  /* campo opcional deixado em branco */
        } else {
            strcpy(destino, temp);
        }
        return;
    }
}

/*
 * telefone_valido: aceita digitos, espaco, '(', ')' e '-'.
 * Exige DDD: o numero deve ter 10 digitos (fixo: DDD + 8) ou
 * 11 digitos (celular: DDD + 9), e o DDD nao pode comecar com 0.
 * Exemplos validos: "(88) 3611-1234" e "(88) 99999-1234".
 */
int telefone_valido(const char *s)
{
    int digitos = 0;
    char primeiro = '\0';   /* primeiro digito (inicio do DDD) */

    while (*s != '\0') {
        if (isdigit((unsigned char) *s)) {
            if (digitos == 0) {
                primeiro = *s;
            }
            digitos++;
        } else if (*s != ' ' && *s != '(' && *s != ')' && *s != '-') {
            return 0;  /* caractere nao permitido */
        }
        s++;
    }
    return (digitos == 10 || digitos == 11) && primeiro != '0';
}

/*
 * so_digitos: copia para 'saida' apenas os digitos de 's'.
 * Usado para comparar telefones ignorando a formatacao:
 * "(88) 99999-1234" e "88999991234" viram o mesmo texto "88999991234".
 * 'saida' deve ter espaco para TAM_FONE + 1 caracteres.
 */
void so_digitos(const char *s, char *saida)
{
    while (*s != '\0') {
        if (isdigit((unsigned char) *s)) {
            *saida = *s;
            saida++;
        }
        s++;
    }
    *saida = '\0';
}

/*
 * mesmo_telefone: retorna 1 se 'a' e 'b' tem exatamente os mesmos digitos
 * (ignora parenteses, espacos e hifen). Campo vazio nunca e considerado
 * igual a outro, para que dois campos opcionais em branco nao contem
 * como repetidos.
 */
int mesmo_telefone(const char *a, const char *b)
{
    char dig_a[TAM_FONE + 1];
    char dig_b[TAM_FONE + 1];

    so_digitos(a, dig_a);
    so_digitos(b, dig_b);
    return dig_a[0] != '\0' && strcmp(dig_a, dig_b) == 0;
}

/*
 * telefone_repetido: retorna 1 se 'tel' ja existe
 *   (a) nos outros telefones do contato que esta sendo cadastrado (atual), ou
 *   (b) em qualquer telefone de qualquer contato ja cadastrado (agenda).
 */
int telefone_repetido(const char *tel, const Contato *atual,
                      const Contato *agenda, int total)
{
    int i;

    /* (a) mesmo contato: fixo, celular 1 e celular 2 devem ser diferentes */
    if (mesmo_telefone(tel, atual->fixo) ||
        mesmo_telefone(tel, atual->celular1) ||
        mesmo_telefone(tel, atual->celular2)) {
        return 1;
    }

    /* (b) contatos ja cadastrados (inclui os carregados do arquivo) */
    for (i = 0; i < total; i++) {
        if (mesmo_telefone(tel, agenda[i].fixo) ||
            mesmo_telefone(tel, agenda[i].celular1) ||
            mesmo_telefone(tel, agenda[i].celular2)) {
            return 1;
        }
    }
    return 0;
}

/*
 * ler_telefone: le, valida (DDD + 10/11 digitos) e confere se o numero
 * nao esta repetido. So grava em 'destino' quando o numero e aceito.
 *   atual  - contato em preenchimento (para comparar com os outros telefones)
 *   agenda - contatos ja cadastrados; total - quantidade deles
 */
void ler_telefone(const char *rotulo, char *destino, int obrigatorio,
                  const Contato *atual, const Contato *agenda, int total)
{
    char temp[TAM_FONE + 1];   /* le primeiro aqui, sem alterar 'destino' */

    for (;;) {
        ler_texto(rotulo, temp, TAM_FONE, obrigatorio);

        if (temp[0] == '\0') {      /* campo opcional deixado em branco */
            destino[0] = '\0';
            return;
        }
        if (!telefone_valido(temp)) {
            printf("  Erro: o telefone deve ter 10 ou 11 digitos"
                   " (DDD + numero), sem contar ( ) e -.\n"
                   "  Exemplos: (88) 99999-1234 ou (88) 3611-1234.\n");
            continue;
        }
        if (telefone_repetido(temp, atual, agenda, total)) {
            printf("  Erro: este telefone ja foi cadastrado"
                   " (neste contato ou em outro). Digite outro.\n");
            continue;
        }
        strcpy(destino, temp);
        return;
    }
}

/*
 * email_valido: validacao simples - sem espacos, exatamente um '@',
 * com texto antes dele e um '.' depois (nao no inicio nem no fim).
 */
int email_valido(const char *s)
{
    const char *arroba;
    const char *ponto;

    if (strchr(s, ' ') != NULL) {
        return 0;
    }
    arroba = strchr(s, '@');
    if (arroba == NULL || arroba == s) {
        return 0;                       /* sem '@' ou nada antes dele */
    }
    if (strchr(arroba + 1, '@') != NULL) {
        return 0;                       /* mais de um '@' */
    }
    ponto = strchr(arroba + 1, '.');
    if (ponto == NULL || ponto == arroba + 1 || ponto[1] == '\0') {
        return 0;                       /* sem '.' valido apos o '@' */
    }
    return 1;
}

/* ler_email: le e valida o e-mail */
void ler_email(char *destino)
{
    for (;;) {
        ler_texto("E-mail", destino, TAM_EMAIL, 1);
        if (email_valido(destino)) {
            return;
        }
        printf("  Erro: e-mail invalido (exemplo: nome@dominio.com).\n");
    }
}

/*
 * ler_inteiro: le um inteiro entre minimo e maximo (inclusive), repetindo
 * ate o usuario digitar um valor valido. Usa strtol para detectar lixo
 * (ex.: "12abc") - coisa que o scanf("%d") nao detecta.
 */
int ler_inteiro(const char *mensagem, int minimo, int maximo)
{
    char temp[32];
    char *fim;
    long valor;
    int r;

    for (;;) {
        printf("%s", mensagem);
        r = ler_linha(temp, (int) sizeof(temp));

        if (r == 0) {
            printf("\nEntrada encerrada. Programa finalizado.\n");
            exit(1);
        }
        if (r == -1 || so_espacos(temp)) {
            printf("  Erro: digite um numero inteiro.\n");
            continue;
        }

        valor = strtol(temp, &fim, 10);
        /* *fim deve apontar para o fim da string (sem sobra de lixo) */
        while (isspace((unsigned char) *fim)) {
            fim++;
        }
        if (*fim != '\0') {
            printf("  Erro: digite apenas numeros.\n");
            continue;
        }
        if (valor < minimo || valor > maximo) {
            printf("  Erro: o valor deve estar entre %d e %d.\n",
                   minimo, maximo);
            continue;
        }
        return (int) valor;
    }
}

/* ===========================================================================
 *  LEITURA E EXIBICAO DE UM CONTATO
 * ========================================================================= */

/*
 * somente_letras: (usada para o nome e para a rua) aceita apenas letras, espacos, hifen, apostrofo e ponto
 * (ex.: "Maria da Silva", "Ana-Clara", "Joao D'Avila", "Carlos Jr.").
 * Letras acentuadas sao aceitas: no Windows elas chegam como caracteres
 * de codigo maior que 127, que tratamos como letras.
 * Numeros e outros simbolos (@, #, etc.) sao recusados, e o nome precisa
 * ter pelo menos uma letra.
 */
int somente_letras(const char *s)
{
    int letras = 0;
    unsigned char c;

    while (*s != '\0') {
        c = (unsigned char) *s;
        if (isalpha(c) || c > 127) {
            letras++;                      /* letra (com ou sem acento) */
        } else if (c != ' ' && c != '-' && c != '\'' && c != '.') {
            return 0;                      /* digito ou simbolo: invalido */
        }
        s++;
    }
    return letras > 0;
}

/* ler_nome: le o nome completo e repete ate conter so letras e separadores */
void ler_nome(char *destino)
{
    for (;;) {
        ler_texto("Nome completo", destino, TAM_NOME, 1);
        if (somente_letras(destino)) {
            return;
        }
        printf("  Erro: o nome deve conter apenas letras"
               " (sem numeros ou simbolos).\n");
    }
}

/* ler_rua: le a rua e repete ate conter so letras e separadores.
 * O numero da casa/predio NAO vai aqui: ele tem o proprio campo "Numero". */
void ler_rua(char *destino)
{
    for (;;) {
        ler_texto("Rua", destino, TAM_RUA, 1);
        if (somente_letras(destino)) {
            return;
        }
        printf("  Erro: a rua deve conter apenas letras"
               " (sem numeros ou simbolos).\n"
               "  O numero do imovel e informado no campo seguinte.\n");
    }
}

/* ler_complemento: campo opcional. Pode ficar em branco; se for preenchido,
 * deve conter apenas letras (ex.: "Fundos", "Bloco B"). */
void ler_complemento(char *destino)
{
    for (;;) {
        ler_texto("Complemento", destino, TAM_COMPL, 0);
        if (destino[0] == '\0' || somente_letras(destino)) {
            return;  /* em branco (permitido) ou valido */
        }
        printf("  Erro: o complemento deve conter apenas letras"
               " (sem numeros ou simbolos).\n");
    }
}

/* ler_bairro: campo obrigatorio. Repete ate conter so letras e separadores
 * (ex.: "Centro", "Sumare", "Jardim Sao Paulo"). */
void ler_bairro(char *destino)
{
    for (;;) {
        ler_texto("Bairro", destino, TAM_BAIRRO, 1);
        if (somente_letras(destino)) {
            return;
        }
        printf("  Erro: o bairro deve conter apenas letras"
               " (sem numeros ou simbolos).\n");
    }
}

/*
 * cidade_estado_valido: o campo segue as mesmas regras de somente_letras
 * (sem numeros e sem simbolos), mas aceita uma unica barra '/' para separar
 * a cidade do estado (ex.: "Sobral/CE"). Para isso, copiamos o texto sem a
 * barra e validamos a copia com somente_letras.
 */
int cidade_estado_valido(const char *s)
{
    char copia[TAM_CIDADE + 1];   /* texto sem as barras */
    int barras = 0;
    int i = 0;

    while (*s != '\0') {
        if (*s == '/') {
            barras++;
        } else {
            copia[i] = *s;
            i++;
        }
        s++;
    }
    copia[i] = '\0';
    return barras <= 1 && somente_letras(copia);
}

/* ler_cidade_estado: campo obrigatorio; repete ate ser valido */
void ler_cidade_estado(char *destino)
{
    for (;;) {
        ler_texto("Cidade/Estado (ex.: Sobral/CE)", destino, TAM_CIDADE, 1);
        if (cidade_estado_valido(destino)) {
            return;
        }
        printf("  Erro: use apenas letras, com no maximo uma barra"
               " separando cidade e estado (ex.: Sobral/CE).\n");
    }
}

/*
 * ler_contato: preenche todos os campos de um contato via teclado.
 * Recebe a agenda atual para impedir telefones repetidos.
 */
void ler_contato(Contato *c, const Contato *agenda, int total)
{
    /* Telefones comecam vazios: assim, ao validar um deles, os que ainda
     * nao foram digitados nao entram na comparacao de repetidos */
    c->fixo[0] = '\0';
    c->celular1[0] = '\0';
    c->celular2[0] = '\0';

    printf("\n--- Dados pessoais ---\n");
    ler_nome(c->nome);

    printf("\n--- Telefones ---\n");
    ler_telefone("Telefone fixo com DDD", c->fixo, 0, c, agenda, total);
    ler_telefone("Celular 1 com DDD", c->celular1, 1, c, agenda, total);
    ler_telefone("Celular 2 com DDD", c->celular2, 0, c, agenda, total);

    printf("\n--- E-mail ---\n");
    ler_email(c->email);

    printf("\n--- Endereco ---\n");
    ler_rua(c->endereco.rua);
    ler_texto("Numero", c->endereco.numero, TAM_NUMERO, 1);
    ler_complemento(c->endereco.complemento);
    ler_bairro(c->endereco.bairro);
    ler_cidade_estado(c->endereco.cidade_estado);

    printf("\n--- Estado civil ---\n");
    printf("  1 - Solteiro\n  2 - Casado\n  3 - Separado\n  4 - Viuvo\n");
    /* O usuario digita 1..4; guardamos 0..3 (indice do vetor de nomes) */
    c->estado_civil = ler_inteiro("Escolha o estado civil (1-4): ", 1, 4) - 1;
}

/* exibir_contato: imprime um contato formatado (campos vazios viram "-") */
void exibir_contato(const Contato *c, int indice)
{
    printf("\n[%d] %s\n", indice, c->nome);
    printf("    Fixo     : %s\n", c->fixo[0] ? c->fixo : "-");
    printf("    Celular 1: %s\n", c->celular1);
    printf("    Celular 2: %s\n", c->celular2[0] ? c->celular2 : "-");
    printf("    E-mail   : %s\n", c->email);
    printf("    Endereco : %s, %s", c->endereco.rua, c->endereco.numero);
    if (c->endereco.complemento[0] != '\0') {
        printf(" - %s", c->endereco.complemento);
    }
    printf("\n");
    printf("               %s - %s\n", c->endereco.bairro,
           c->endereco.cidade_estado);
    printf("    Estado civil: %s\n", NOMES_ESTADO_CIVIL[c->estado_civil]);
}

/* exibir_todos: lista toda a agenda */
void exibir_todos(const Contato *agenda, int total)
{
    int i;

    if (total == 0) {
        printf("\nA agenda esta vazia.\n");
        return;
    }
    printf("\n========== AGENDA (%d registro(s)) ==========", total);
    for (i = 0; i < total; i++) {
        exibir_contato(&agenda[i], i + 1);
    }
    printf("\n=============================================\n");
}

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
