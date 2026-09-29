#include <stdio.h>      // Funções de entrada, saída e manipulação de arquivos
#include <stdlib.h>     // Funções gerais, alocação de memória e números aleatórios
#include <math.h>       // Funções matemáticas
#include <time.h>       // Funções relacionadas a tempo, data e medição de execução
#include <string.h>     // Funções para manipulação de strings
#include <malloc.h>     // Funções de alocação dinâmica de memória
#include <sys/types.h>  // Define tipos de dados utilizados pelo sistema operacional
#include <sys/times.h>  // Funções para medir o tempo de processamento do programa
#include <sys/time.h>   // gettimeofday()
#include <unistd.h>     // sysconf()
#include <dirent.h>      // Leitura de pastas no modo batch
#include <sys/stat.h>    // Verificar se argumento é pasta
#include <errno.h>       // Tratamento simples de erros
#include <ctype.h>       // Remoção de espaços no resumo CSV
#include <limits.h>      // INT_MAX para infinito numérico seguro
#define sqr(qw) ((qw) * (qw))
/*#define MAXK 5          Neighboors' number     #define MINK 6 */
#define NBGRAND 5      // Quantidade definida para NBGRAND; o uso exato depende do restante do código
#define MAXP 90        // Número máximo de processadores ou máquinas
#define MAXN 400       // Número máximo de tarefas mais processadores; valor original: 286
#define MAXKN 30       // Número máximo de vizinhos considerados pelo algoritmo
/******** INÍCIO DA CORREÇÃO 3: SEPARAÇÃO ENTRE MARCADOR, PENALIDADE E INFINITO ********/
#define CUSTO_INFINITO (INT_MAX / 4) // Usado apenas para inicializar comparações
/******** FIM DA CORREÇÃO 3 ************************************************************/
#define MAXREAL 1.7e38 // Valor real muito grande, geralmente utilizado como infinito inicial

/* Semente da execucao atual; usada apenas para repeticoes reproduziveis no modo batch. */
long seed_execucao_atual = 0;

// Verifica se uma condição é verdadeira.
// Caso seja falsa, apresenta uma mensagem e encerra o programa.
#define demand(fact,msg) { \
        if (!fact) {       \
            printf("msg \n"); \
            exit(1);       \
        }                  \
    }
/******** INÍCIO DA DEFINIÇÃO DAS ESTRUTURAS **********/

// Declara antecipadamente o tipo tourneelem,
// que representa um elemento de uma sequência de tarefas.
typedef struct tourneelem tourneelem;

// Estrutura utilizada como elemento de uma lista duplamente encadeada.
struct tourneelem {
  int noeud, rang;                    // Número da tarefa e sua posição na sequência
  tourneelem *precedent, *prochain;   // Ponteiros para o elemento anterior e o próximo
};

// Estrutura que associa uma tarefa à sequência em que ela está inserida.
typedef struct {
  int satourne;             // Número da sequência, máquina ou processador da tarefa
  tourneelem *ptrtourne;    // Ponteiro para a posição da tarefa nessa sequência
} coord;

// Vetores de localização das tarefas na solução atual e na melhor solução.
coord g[MAXN+1], bg[MAXN+1];

// Estrutura que representa a sequência de tarefas de uma máquina.
typedef struct {
  int noeudinterne[MAXN+1]; // Tarefas pertencentes à sequência
  tourneelem *ptr;           // Ponteiro para o início da lista de tarefas
  int nbredenoeuds;          // Número de tarefas existentes na sequência
} tourne;

// Estrutura que armazena os vizinhos mais próximos de uma tarefa.
typedef struct {
  int nn[MAXKN+1], leplusloin; // Lista de vizinhos e posição do vizinho mais distante
  float maxdist;               // Maior distância entre os vizinhos armazenados
} proxnoeud;

// Estrutura que armazena dois conjuntos de vizinhos para cada tarefa.
typedef struct {
  proxnoeud p1[MAXN+1], p2[MAXN+1]; // Vizinhanças do tipo p1 e p2
} neighbour;

// Vetor de estruturas de vizinhança para cada máquina ou processador.
neighbour neigh[MAXP+1];

// Estrutura que representa uma solução completa do problema.
typedef struct {
  tourne tabt[MAXP+1];           // Sequência de tarefas de cada máquina
  int nodepot[MAXP+1];           // Identificação associada a cada máquina ou depósito
  int load[MAXP+1];              // Carga ou tempo total de processamento de cada máquina
} tdepot;

// Solução atual e melhor solução encontrada pelo algoritmo.
tdepot depot, bsolution;

// Estrutura utilizada para indicar alterações durante a busca.
typedef struct {
  int modifie;       // Indica se ocorreu alguma modificação
  int numbdepot;     // Número da máquina ou depósito modificado
  int mdelta;        // Variação provocada pelo movimento realizado
} tflag;

// Estrutura que armazena informações sobre o makespan.
typedef struct {
  int load;          // Maior carga ou valor atual do makespan
  int proc;          // Máquina ou processador que possui a maior carga
  int iter;          // Iteração em que esse valor foi obtido
} makes;

// Variável que armazena as informações do makespan.
makes makespan;

/********* INÍCIO DA DEFININIÇÃO DAS FUNÇÕES *************/

/* Retorna o menor valor entre dois números inteiros. */
int mini (a,b)
int a, b;
{
  if (a <= b)
    return(a);          // Retorna a quando a for menor ou igual a b
  else
    return(b);          // Caso contrário, retorna b
}

/********** FIM DA FUNÇÃO MINI ****************/


/* Retorna o menor valor entre dois números reais do tipo float. */
float min (a,b)
float a, b;
{
  if (a <= b)
    return(a);          // Retorna a quando a for menor ou igual a b
  else
    return(b);          // Caso contrário, retorna b
}

/********** FIM DA FUNÇÃO MIN ****************/


/* Gera aleatoriamente o tempo de permanência de um movimento na lista tabu. */
int tabu_time (lim1,lim2)
int lim1,lim2;
{
  float var;            // Armazena temporariamente o tempo tabu calculado
  double drand48();     // Declara a função que gera um número aleatório entre 0 e 1

  /* Esta atribuição é imediatamente substituída pela linha seguinte
     e, portanto, não influencia o resultado final. */
  var = (0.8 * (lim2-lim1)) + lim1 + .5;

  /* Gera um valor aleatório entre lim1 e lim2.
     O valor 0.5 é usado antes da conversão para inteiro. */
  var = (drand48() * (lim2-lim1)) + lim1 + .5;

  return((int)var);     // Converte o valor calculado para inteiro e o retorna
}

/********** FIM DA FUNÇÃO TABU_TIME ****************/


/* Retorna o número da tarefa seguinte à tarefa a em sua sequência. */
int suiv (a,g)
int a;
coord g[];
{
  // Localiza a tarefa a e acessa o próximo elemento da lista encadeada
  return(g[a].ptrtourne->prochain->noeud);
}

/********** FIM DA FUNÇÃO SUIV ****************/


/* Retorna o número da tarefa anterior à tarefa a em sua sequência. */
int prec (a,g)
int a;
coord g[];
{
  // Localiza a tarefa a e acessa o elemento anterior da lista encadeada
  return(g[a].ptrtourne->precedent->noeud);
}

/********** FIM DA FUNÇÃO PREC ****************/


/* Retorna a posição da tarefa a dentro de sua sequência. */
int ordre (a,g)
int a;
coord g[];
{
  return(g[a].ptrtourne->rang); // Retorna a ordem ou posição da tarefa
}

/********** FIM DA FUNÇÃO ORDRE ****************/


/* Calcula o custo total de uma sequência, somando o custo
   entre cada par de tarefas consecutivas. */
float calculcoutt (t,dtp)

tourne *t;                         // Ponteiro para a sequência analisada
int dtp[MAXN+1][MAXN+1];          // Matriz de custos entre tarefas

{
  tourneelem *i, *j;              // Ponteiros para duas tarefas consecutivas
  int c;                          // Armazena o custo total da sequência

  c = 0;                          // Inicializa o custo total com zero
  j = t->ptr;                     // j aponta para o primeiro elemento da sequência
  i = t->ptr->prochain;           // i aponta para o elemento seguinte

  do {
    // Soma o custo de deslocamento ou transição entre j e i
    c += dtp[j->noeud][i->noeud];

    j = i;                        // Avança j para a posição atual de i
    i = i->prochain;              // Avança i para a próxima tarefa

  } while (j != t->ptr);          // Continua até retornar ao início da sequência

  return(c);                      // Retorna o custo total calculado
}

/********** FIM DA FUNÇÃO CALCULCOUTT ****************/

/********** INÍCIO DA DEFINIÇÃO DAS VARIÁVEIS GLOBAIS **********/

/* Variáveis inteiras utilizadas em diferentes etapas do algoritmo. */
int i, j, k, w, n, ij, np, k1, lngtotal, cptlng,
minmakespan, minpl, minpm, cneighin, cneighout,
xio, vio, pio, pa, aspiration, maxload, lastimp, imptrue,
lasttabu[MAXN+1], procplus, maxtabu, mintabu, iteration,
newprocplus, flag_change, xx, exc, cpt, custp, MAXK,
MAXK1;

/*
i, j, k e w: contadores usados em laços de repetição.
n: número de tarefas ou nós da instância.
np: número de processadores ou máquinas.
minmakespan: menor makespan encontrado.
aspiration: indica ou controla o critério de aspiração.
maxload: maior carga entre as máquinas.
lasttabu: armazena informações tabu relacionadas às tarefas.
procplus: máquina que apresenta a maior carga.
maxtabu e mintabu: limites máximo e mínimo do tempo tabu.
iteration: número da iteração atual.
flag_change: indica se ocorreu alguma alteração na solução.
MAXK e MAXK1: limites de vizinhança definidos durante a execução.

As demais variáveis são auxiliares e seu significado específico
depende das funções em que são utilizadas.
*/


/* Armazena o tempo decorrido durante a execução. */
long int elap = 0;


/* Ponteiro para o nome do arquivo de entrada. */
char *fname;


/* Variáveis inteiras de maior capacidade utilizadas para informações
   da instância e resultados da busca tabu. */
long int nprob, ns, eucl, choix, maxdist, besttabu[11];

/*
nprob: identificação ou número do problema.
ns: variável auxiliar relacionada à instância.
eucl: indica possível utilização de distância euclidiana.
choix: armazena uma escolha realizada pelo algoritmo.
maxdist: maior distância considerada.
besttabu: armazena os melhores resultados da busca tabu.
*/


/******** INÍCIO DA CORREÇÃO 3: PENALIDADE CALCULADA POR INSTÂNCIA ********/
/*
A penalidade é calculada depois da leitura da instância. Ela fica maior
que um limite superior de qualquer rota viável formada apenas por arcos
permitidos. Assim, não é confundida com o infinito usado nas comparações.
*/
int marcador_setup_proibido = 0; // Calculado internamente: maior setup válido + 1
int penalidade_transicao = 0;
/******** FIM DA CORREÇÃO 3 ***********************************************/

/* Matrizes e vetores utilizados para armazenar os dados do problema. */
int d[MAXN+1][MAXN+1],             // Matriz de distâncias ou custos originais
    dtp[MAXN+1][MAXN+1],           // Matriz de custos ou tempos de processamento
    tabulist[MAXP+1][MAXN+1],      // Lista tabu para cada máquina e tarefa
    tproc[MAXN+1];                  // Vetor relacionado aos processadores das tarefas


/* Variáveis reais utilizadas para armazenar tempos de execução,
   valores das soluções e parâmetros do algoritmo. */
float timeia, timetab, timetb, timepot, timetot,
      ttimeia, ttimetab, ttimetb, ttimepot, ttimetot,
      soluias, solutab, solupot,
      tsoluias, tsolutab, tsolupot, alfa;

/*
timeia, timetab, timetb e timepot: tempos consumidos por diferentes etapas.
timetot: tempo total de execução.
Variáveis iniciadas por ttime: tempos acumulados.
soluias, solutab e solupot: valores de soluções obtidas em diferentes etapas.
Variáveis iniciadas por tsolu: valores acumulados das soluções.
alfa: parâmetro numérico utilizado pelo algoritmo.
*/


/* Vetores que armazenam os vizinhos de entrada e de saída de cada tarefa. */
proxnoeud neighin[MAXN+1], neighout[MAXN+1];


/* Estrutura global utilizada para indicar modificações realizadas na solução. */
tflag flag;


/* Ponteiro para um elemento da sequência de tarefas. */
tourneelem *imppb;


/* Vetor auxiliar que armazena a localização das tarefas. */
coord g2[MAXN+1];


/* Estrutura auxiliar que representa uma sequência de tarefas. */
tourne t2;


/* Vetor utilizado para armazenar um nome de arquivo com até 39 caracteres. */
char namef[512];


/* Estrutura que armazena uma cópia das informações de vizinhança. */
neighbour bneigh[MAXP+1];


/* Ponteiro para o arquivo utilizado na gravação dos resultados. */
FILE *outf;


/*
Retorna o tempo atual do sistema em microssegundos.
Essa função é usada para medir o tempo de execução do algoritmo.
*/
double wtime() {
  struct timeval t;                  // Estrutura que armazena segundos e microssegundos

  gettimeofday(&t, NULL);            // Obtém o tempo atual do sistema

  return t.tv_sec * 1000000.0
       + t.tv_usec * 1.0;            // Converte todo o tempo para microssegundos

  /*
  Alternativa desativada que retornaria o tempo em milissegundos:
  return t.tv_sec * 1000.0 + t.tv_usec * 0.001;
  */
}





/******** FUNÇÕES AUXILIARES DO MODO BATCH ****************/

/*
Verifica se o caminho informado corresponde a uma pasta.
Retorna 1 quando for pasta e 0 caso contrário.
*/
int eh_pasta(const char *caminho)
{
  struct stat st;

  if (stat(caminho, &st) != 0)
    return 0;

  return S_ISDIR(st.st_mode);
}


/*
Remove espaços, tabulações e quebras de linha do começo e do fim de uma string.
*/
void limpar_espacos(char *texto)
{
  char *inicio;
  char *fim;
  int tamanho;

  inicio = texto;

  while (*inicio && isspace((unsigned char)*inicio))
    inicio++;

  if (inicio != texto)
    memmove(texto, inicio, strlen(inicio) + 1);

  tamanho = strlen(texto);

  if (tamanho == 0)
    return;

  fim = texto + tamanho - 1;

  while (fim >= texto && isspace((unsigned char)*fim)) {
    *fim = '\0';
    fim--;
  }
}


/*
Copia um arquivo de origem para destino.
É usado no modo batch para colocar temporariamente a instância
na mesma pasta do executável, preservando o comportamento original do tabu.c.
*/
int copiar_arquivo(const char *origem, const char *destino)
{
  FILE *in;
  FILE *out;
  char buffer[8192];
  size_t lidos;

  in = fopen(origem, "rb");
  if (in == NULL)
    return 0;

  out = fopen(destino, "wb");
  if (out == NULL) {
    fclose(in);
    return 0;
  }

  while ((lidos = fread(buffer, 1, sizeof(buffer), in)) > 0)
    fwrite(buffer, 1, lidos, out);

  fclose(in);
  fclose(out);

  return 1;
}


/*
Anexa o conteudo de um arquivo ao final de outro arquivo.
Usado somente no modo batch para reunir, em um unico arquivo de terminal,
as saidas de todas as execucoes da mesma instancia.
*/
int anexar_arquivo(const char *origem, const char *destino)
{
  FILE *in;
  FILE *out;
  char buffer[8192];
  size_t lidos;

  in = fopen(origem, "rb");
  if (in == NULL)
    return 0;

  out = fopen(destino, "ab");
  if (out == NULL) {
    fclose(in);
    return 0;
  }

  while ((lidos = fread(buffer, 1, sizeof(buffer), in)) > 0)
    fwrite(buffer, 1, lidos, out);

  fclose(in);
  fclose(out);

  return 1;
}


/*
Procura, dentro do arquivo de saída do terminal, uma linha contendo a chave informada.
Exemplo de chave: "Initial Solution" ou "Tabu Solution".
Quando encontra, copia o valor depois do sinal de igualdade para destino.
*/
void extrair_valor(const char *arquivo, const char *chave, char *destino, int tam_destino)
{
  FILE *fp;
  char linha[1024];
  char *pos;
  char *igual;

  destino[0] = '\0';

  fp = fopen(arquivo, "r");
  if (fp == NULL)
    return;

  while (fgets(linha, sizeof(linha), fp) != NULL) {

    pos = strstr(linha, chave);

    if (pos != NULL) {

      igual = strchr(linha, '=');

      if (igual != NULL) {
        igual++;
        strncpy(destino, igual, tam_destino - 1);
        destino[tam_destino - 1] = '\0';
        limpar_espacos(destino);
      }

      fclose(fp);
      return;
    }
  }

  fclose(fp);
}


/*
Evita tentar rodar arquivos que não são instâncias.
*/
int deve_ignorar_arquivo(const char *nome)
{
  int tam;

  if (nome[0] == '.')
    return 1;

  if (strncmp(nome, "SOL_", 4) == 0)
    return 1;

  if (strncmp(nome, "solucaoTabu.", 12) == 0)
    return 1;

  if (strncmp(nome, "terminal_", 9) == 0)
    return 1;

  if (strncmp(nome, "resumo", 6) == 0)
    return 1;

  if (strcmp(nome, "tabu") == 0)
    return 1;

  if (strcmp(nome, "tabu_batch") == 0)
    return 1;

  tam = strlen(nome);

  if (tam >= 2 && strcmp(nome + tam - 2, ".c") == 0)
    return 1;

  if (tam >= 4 && strcmp(nome + tam - 4, ".txt") == 0)
    return 1;

  if (tam >= 4 && strcmp(nome + tam - 4, ".csv") == 0)
    return 1;

  if (tam >= 3 && strcmp(nome + tam - 3, ".sh") == 0)
    return 1;

  return 0;
}


/*
Modo batch.
Quando o primeiro argumento for uma pasta, esta função roda todas as instâncias
existentes nela, uma por vez, usando o próprio executável.
*/
int rodar_pasta_de_instancias(const char *programa, const char *pasta, const char *tempo_extra)
{
  DIR *dir;
  struct dirent *ent;
  FILE *csv;
  FILE *terminal_fp;
  char caminho_origem[1024];
  char comando[2048];
  char terminal_saida[1024];
  char terminal_execucao[1024];
  char arq_solucao[1024];
  char arq_r[1024];
  char destino_solucao[1024];
  char destino_r[1024];
  char initial[128];
  char tabu_sol[128];
  char posopt[128];
  char walltime[128];
  int retorno;
  int total;
  int numero_execucoes;
  int execucao;
  int seed_execucao;

  printf("\nDigite o numero de execucoes por instancia: ");
  fflush(stdout);

  if (scanf("%d", &numero_execucoes) != 1 || numero_execucoes < 1) {
    printf("\nNumero de execucoes invalido. Informe um inteiro maior ou igual a 1.\n");
    return 1;
  }

  mkdir("resultados", 0777);

  csv = fopen("resultados/resumo_resultados.csv", "w");
  if (csv == NULL) {
    printf("\nNao foi possivel criar resultados/resumo_resultados.csv\n");
    return 1;
  }

  fprintf(csv, "instancia;execucao;seed;initial_solution;tabu_solution;posopt_solution;total_wall_time\n");

  dir = opendir(pasta);
  if (dir == NULL) {
    printf("\nNao foi possivel abrir a pasta: %s\n", pasta);
    fclose(csv);
    return 1;
  }

  total = 0;

  while ((ent = readdir(dir)) != NULL) {
    if (deve_ignorar_arquivo(ent->d_name))
      continue;

    snprintf(caminho_origem, sizeof(caminho_origem), "%s/%s", pasta, ent->d_name);
    if (eh_pasta(caminho_origem))
      continue;

    if (!copiar_arquivo(caminho_origem, ent->d_name)) {
      printf("Nao foi possivel copiar a instancia %s\n", ent->d_name);
      continue;
    }

    snprintf(terminal_saida, sizeof(terminal_saida), "resultados/terminal_%s.txt", ent->d_name);
    terminal_fp = fopen(terminal_saida, "w");
    if (terminal_fp == NULL) {
      printf("Nao foi possivel criar o arquivo de terminal para %s\n", ent->d_name);
      remove(ent->d_name);
      continue;
    }
    fprintf(terminal_fp, "Instance: %s\n", ent->d_name);
    fprintf(terminal_fp, "Numero de execucoes: %d\n", numero_execucoes);
    fclose(terminal_fp);

    printf("\n============================================\n");
    printf("Rodando instancia: %s\n", ent->d_name);
    printf("Numero de execucoes: %d\n", numero_execucoes);
    printf("============================================\n");

    for (execucao = 1; execucao <= numero_execucoes; execucao++) {
      seed_execucao = execucao - 1;
      printf("\nExecucao %d de %d - %s\n", execucao, numero_execucoes, ent->d_name);

      terminal_fp = fopen(terminal_saida, "a");
      if (terminal_fp != NULL) {
        fprintf(terminal_fp, "\n============================================================\n");
        fprintf(terminal_fp, "EXECUCAO %d DE %d\n", execucao, numero_execucoes);
        fprintf(terminal_fp, "SEED = %d\n", seed_execucao);
        fprintf(terminal_fp, "============================================================\n\n");
        fclose(terminal_fp);
      }

      snprintf(terminal_execucao, sizeof(terminal_execucao),
               "resultados/.tmp_terminal_%s_execucao_%02d.txt",
               ent->d_name, execucao);

      if (tempo_extra != NULL && strlen(tempo_extra) > 0) {
        snprintf(comando, sizeof(comando),
                 "\"%s\" \"%s\" \"%s\" \"%d\" > \"%s\" 2>&1",
                 programa, ent->d_name, tempo_extra, seed_execucao, terminal_execucao);
      } else {
        snprintf(comando, sizeof(comando),
                 "\"%s\" \"%s\" \"-100000\" \"%d\" > \"%s\" 2>&1",
                 programa, ent->d_name, seed_execucao, terminal_execucao);
      }

      retorno = system(comando);
      if (retorno != 0)
        printf("Aviso: a instancia %s, execucao %d, terminou com codigo %d\n",
               ent->d_name, execucao, retorno);

      if (!anexar_arquivo(terminal_execucao, terminal_saida))
        printf("Aviso: nao foi possivel anexar a saida da execucao %d de %s\n",
               execucao, ent->d_name);

      extrair_valor(terminal_execucao, "Initial Solution", initial, sizeof(initial));
      extrair_valor(terminal_execucao, "Tabu Solution", tabu_sol, sizeof(tabu_sol));
      extrair_valor(terminal_execucao, "Pos-Opt. Solution", posopt, sizeof(posopt));
      extrair_valor(terminal_execucao, "Total wall time", walltime, sizeof(walltime));

      fprintf(csv, "%s;%d;%d;%s;%s;%s;%s\n", ent->d_name, execucao, seed_execucao, initial, tabu_sol, posopt, walltime);
      fflush(csv);

      snprintf(arq_solucao, sizeof(arq_solucao), "solucaoTabu.%s", ent->d_name);
      snprintf(destino_solucao, sizeof(destino_solucao), "resultados/solucaoTabu.%s_execucao_%02d", ent->d_name, execucao);
      if (access(arq_solucao, F_OK) == 0)
        rename(arq_solucao, destino_solucao);

      snprintf(arq_r, sizeof(arq_r), "r%s", ent->d_name);
      snprintf(destino_r, sizeof(destino_r), "resultados/r%s_execucao_%02d", ent->d_name, execucao);
      if (access(arq_r, F_OK) == 0)
        rename(arq_r, destino_r);

      remove(terminal_execucao);
    }

    terminal_fp = fopen(terminal_saida, "a");
    if (terminal_fp != NULL) {
      fprintf(terminal_fp, "\n============================================================\n");
      fprintf(terminal_fp, "FIM DAS EXECUCOES DA INSTANCIA\n");
      fprintf(terminal_fp, "Instance: %s\n", ent->d_name);
      fprintf(terminal_fp, "Execucoes realizadas: %d\n", numero_execucoes);
      fprintf(terminal_fp, "============================================================\n");
      fclose(terminal_fp);
    }

    remove(ent->d_name);
    total++;
  }

  closedir(dir);
  fclose(csv);

  printf("\nProcessamento finalizado. Total de instancias rodadas: %d\n", total);
  printf("Execucoes realizadas por instancia: %d\n", numero_execucoes);
  printf("Veja a pasta resultados e o arquivo resultados/resumo_resultados.csv\n");
  return 0;
}

/******** INÍCIO DO PROGRAMA PRINCIPAL ****************/

int main(int argc, char **argv) {

  /*
  argc: quantidade de argumentos informados na execução do programa.
  argv: vetor que armazena os argumentos informados no terminal.
  */

  /*
  MODO BATCH:
  Se o primeiro argumento for uma pasta, o programa roda automaticamente
  todas as instâncias dentro dessa pasta e salva os resultados em CSV.

  Exemplo:
      ./tabu_batch instancias

  MODO NORMAL:
  Se o primeiro argumento for um arquivo, mantém o comportamento original:
      ./tabu_batch dados_trab_2_10_struc_01
  */
  if (argc < 2) {
    printf("\nUso normal : %s nome_da_instancia\n", argv[0]);
    printf("Uso batch  : %s pasta_de_instancias\n", argv[0]);
    return 1;
  }

  if (eh_pasta(argv[1])) {
    if (argc > 2)
      return rodar_pasta_de_instancias(argv[0], argv[1], argv[2]);
    else
      return rodar_pasta_de_instancias(argv[0], argv[1], NULL);
  }

  //---- Trecho inserido por Fernando ------

  double START, END, tabu_iter_start, post_start; // Tempos de alta resolução
  int outTime = 0;         // Indicador auxiliar relacionado ao limite de tempo

  START = wtime();         // Registra o instante inicial da execução

  double timeViz = -100000; // Inicializa o limite de tempo da vizinhança

  /*
  Caso seja informado um segundo argumento após o nome da instância,
  converte esse argumento de texto para número real e o armazena em timeViz.
  */
  if (argc > 2)
    timeViz = atof(argv[2]);

  /*
  No modo batch, o quarto argumento informa a semente da execucao atual.
  No modo normal, quando esse argumento nao e informado, a semente permanece zero.
  */
  if (argc > 3)
    seed_execucao_atual = atol(argv[3]);
  else
    seed_execucao_atual = 0;

  /*
  O primeiro argumento informado no terminal corresponde
  ao nome ou caminho do arquivo da instância.
  */
  fname = argv[1];

  // Exibe o nome da instância que será processada.
  printf("Instance: %s\n", argv[1]);

  //---------------------------------------


  /*
  Linha desativada que inicializaria as posições de 1 a 10
  do vetor besttabu com o valor máximo de distância.
  */
  // for (ij = 1; ij <= 10; ij++)
  //   besttabu[ij] = CUSTO_INFINITO;


  /*
  Executa o algoritmo para diferentes valores do parâmetro alfa.

  Apesar da estrutura do for, os valores efetivamente utilizados
  são alfa = 0.6 e alfa = 0.8.
  */
  for (alfa = 0.5; alfa <= 0.8; alfa += 0.5) {

    // Ajusta o primeiro valor de alfa de 0.5 para 0.6.
    if (alfa <= 0.5)
      alfa = 0.6;

    // Ajusta o valor seguinte de alfa para 0.8.
    else if (alfa <= 1.0)
      alfa = 0.8;


    // Inicializa os acumuladores de tempo com zero.
    ttimeia = 0;
    ttimetab = 0;
    ttimetb = 0;
    ttimepot = 0;
    ttimetot = 0;

    // Inicializa os acumuladores dos valores das soluções com zero.
    tsoluias = 0;
    tsolutab = 0;
    tsolupot = 0;


    /*
    Laço desativado que permitiria repetir a execução
    uma determinada quantidade de vezes.
    */
    // for (ij = 1; ij <= 1; ij++) {


    /*
    Inicializa a solução, o vetor lasttabu e a lista tabu.
    */
    init(&depot, lasttabu, tabulist);


    /*
    Copia o valor máximo da vizinhança para a variável auxiliar k1.
    */
    k1 = MAXK;


    /*
    Lê ou gera os dados do problema, preenchendo:
    - as coordenadas das tarefas;
    - a matriz de distâncias;
    - a matriz de tempos ou custos;
    - os processadores associados;
    - a estrutura da solução.
    */
    generegraph(g, d, dtp, tproc, &depot);


    /*
    Identifica os vizinhos de entrada e de saída
    de cada tarefa com base na matriz d.
    */
    find_in_out_neighbour(neighin, neighout, d);


    /*
    Constrói a solução inicial, atribuindo as tarefas
    às máquinas ou processadores.
    */
    initial_assignment(g, d, dtp, &depot, neigh);


    /*
    Inicializa ou configura o gerador de números aleatórios
    utilizado pelo algoritmo.
    */
    seed_generation();


    /*
    Linhas de teste desativadas, utilizadas anteriormente
    para verificar os dados armazenados em bneigh.
    */
    // printf("primeiro store ");
    // printf("\n primeiro store bneigh[1]->p1[6].nn[2] == %d",
    //        bneigh[1].p1[6].nn[2]);


    /*
    Armazena a solução inicial como a melhor solução conhecida,
    juntamente com suas coordenadas, makespan e vizinhanças.
    */
    store_best_solution(
        g,
        &depot,
        bg,
        &bsolution,
        &makespan,
        neigh,
        bneigh
    );


    /*
    Obtém a máquina ou processador que possui
    a maior carga na melhor solução armazenada.
    */
    procplus = makespan.proc;


    /*
    Armazena o makespan da solução inicial.
    */
    soluias = makespan.load;


    /*
    Trecho desativado que permitiria ao usuário
    informar os limites da lista tabu pelo teclado.
    */
    // printf("\nEnter the minimum tabu size: ");
    // scanf("%d", &mintabu);

    // printf("\nEnter the maximum tabu size: ");
    // scanf("%d", &maxtabu);


    /*
    Define diretamente os limites mínimo e máximo
    do tempo de permanência de um movimento na lista tabu.
    */
    mintabu = 3;
    maxtabu = 7;

    /* ******** INÍCIO DA FASE DE BUSCA TABU ******** */

// printf("\n************TABU PHASE****************");

/* A solução inicial foi obtida antes da primeira iteração tabu. */
makespan.iter = 1;

/* Controla a continuidade da busca tabu. */
imptrue = 1;

/* Registra a última iteração em que ocorreu melhoria. */
lastimp = 1;

/* Inicializa o tempo gasto na fase tabu. */
timetab = 0;


/* Executa a busca enquanto imptrue for diferente de zero. */
for (iteration = 1; imptrue; iteration++) {

  /* Inicia a medição de alta resolução desta iteração. */
  tabu_iter_start = wtime();

  /* Inicializa o melhor makespan candidato com um valor elevado. */
  minmakespan = CUSTO_INFINITO;

  /* Inicialmente, nenhuma solução foi modificada. */
  flag.modifie = 0;

  /*
  Seleciona o depósito ou nó inicial da máquina atualmente
  responsável pelo maior tempo de processamento.
  */
  xio = depot.nodepot[procplus];

  /*
  vio armazenará a tarefa escolhida para ser transferida.
  pio armazenará a máquina que receberá essa tarefa.
  */
  vio = 0;
  pio = 0;


  /*
  Analisa cada tarefa da máquina com maior carga para verificar
  se sua transferência para outra máquina melhora a solução.
  */
  for (i = 1;
       i <= (depot.tabt[procplus].nbredenoeuds - 1);
       i++) {

    /*
    Inicializa o menor custo encontrado para a máquina
    que poderá receber a tarefa.
    */
    minpm = CUSTO_INFINITO;

    /* Seleciona a próxima tarefa da máquina mais carregada. */
    xio = suiv(xio, g);

    /*
    Simula a retirada da tarefa xio da máquina procplus.
    A alteração de custo é armazenada em flag.mdelta.
    */
    oterx(
        xio,
        &neigh[procplus],
        &depot.tabt[procplus],
        g,
        dtp,
        &flag
    );

    /*
    Caso a sequência tenha somente o depósito e uma tarefa,
    a retirada dessa tarefa deixa a carga da máquina igual a zero.
    */
    if (depot.tabt[procplus].nbredenoeuds == 2)
      flag.mdelta = -depot.load[procplus];

    /*
    Calcula a carga estimada da máquina de origem
    após a retirada da tarefa.
    */
    minpl = depot.load[procplus] + flag.mdelta;


    /* Analisa todas as possíveis máquinas de destino. */
    for (j = 1; j <= np; j++) {

      /* A máquina de destino deve ser diferente da máquina de origem. */
      if (j != procplus) {

        /* Inicializa os contadores de vizinhos de entrada e saída. */
        cneighin = 0;
        cneighout = 0;


        /*
        Verifica se os vizinhos mais próximos da tarefa xio
        pertencem à máquina candidata j.
        */
        for (k = 1; k <= k1; k++) {

          /*
          Valores maiores que n representam depósitos.
          O depósito é ajustado para o depósito da máquina j.
          */
          if (neighin[xio].nn[k] > n)
            neighin[xio].nn[k] = j + n;

          if (neighout[xio].nn[k] > n)
            neighout[xio].nn[k] = j + n;

          /*
          Conta quantos vizinhos de entrada da tarefa xio
          estão presentes na máquina j.
          */
          if (depot.tabt[j].noeudinterne[neighin[xio].nn[k]])
            cneighin++;

          /*
          Conta quantos vizinhos de saída da tarefa xio
          estão presentes na máquina j.
          */
          if (depot.tabt[j].noeudinterne[neighout[xio].nn[k]])
            cneighout++;

        } /* Fim da análise dos vizinhos */


        /*
        A inserção é analisada quando:
        - existe pelo menos um vizinho de entrada;
        - existem pelo menos dois vizinhos de saída; ou
        - a máquina candidata não possui tarefas.
        */
        if ((cneighin >= 1) ||
            (cneighout >= 2) ||
            (depot.tabt[j].nbredenoeuds <= 1)) {

          /*
          Simula a inserção da tarefa xio na máquina j.
          A alteração de carga é armazenada em flag.mdelta.
          */
          ajoutx(
              xio,
              &neigh[j],
              &depot.tabt[j],
              g,
              dtp,
              &flag
          );

          /*
          Verifica se a carga resultante da máquina j é menor
          ou igual à melhor carga de destino encontrada até agora.
          */
          if ((depot.load[j] + flag.mdelta) <= minpm) {

            /*
            Verifica se o retorno da tarefa xio para a máquina j
            ainda está proibido pela lista tabu.
            */
            if (tabulist[j][xio] >= iteration) {

              /*
              Mesmo sendo tabu, o movimento poderá ser aceito
              pelo critério de aspiração quando produzir uma
              solução melhor que o melhor makespan conhecido.
              */
              if (((depot.load[j] + flag.mdelta) < makespan.load) &&
                  (minpl < makespan.load)) {

                aspiration = 1;

                /*
                Verifica se nenhuma das demais máquinas possui
                carga maior que o melhor makespan conhecido.
                */
                for (k = 1; k <= np; k++) {

                  if ((k != procplus) && (k != j)) {

                    if (depot.load[k] > makespan.load)
                      aspiration = 0;
                  }
                }

                /*
                Se o critério de aspiração for atendido,
                aceita a máquina j como possível destino.
                */
                if (aspiration) {

                  pa = j;

                  minpm = depot.load[j] + flag.mdelta;
                }
              }

            } /* Fim da verificação do movimento tabu */

            else {

              /*
              Caso o movimento não seja tabu,
              aceita a máquina j como possível destino.
              */
              pa = j;

              minpm = depot.load[j] + flag.mdelta;
            }

          } /* Fim da comparação com a melhor máquina de destino */

        } /* Fim da verificação dos vizinhos */

      } /* Fim da verificação da máquina de origem */

    } /* Fim da análise das máquinas de destino */


    /*
    O makespan do movimento candidato é o maior valor entre:
    - a carga da máquina de origem após a retirada;
    - a carga da máquina de destino após a inserção.
    */
    if (minpm <= minpl)
      minpm = minpl;


    /*
    Armazena o melhor movimento encontrado nesta iteração:
    vio recebe a tarefa e pio recebe a máquina de destino.
    */
    if (minpm <= minmakespan) {

      minmakespan = minpm;

      vio = xio;

      pio = pa;
    }

  } /* Fim da análise das tarefas da máquina mais carregada */


  /*
  Se nenhuma tarefa ou máquina de destino foi encontrada,
  encerra a fase tabu.
  */
  if ((vio == 0) || (pio == 0))
    break;


  /* Indica que será realizada uma modificação na solução. */
  flag.modifie = 1;

  /* Define a máquina de origem da tarefa selecionada. */
  flag.numbdepot = procplus;


  /* Retira definitivamente a tarefa vio da máquina de origem. */
  oterx(
      vio,
      &neigh[procplus],
      &depot.tabt[procplus],
      g,
      dtp,
      &flag
  );


  /* Recalcula a carga da máquina de origem. */
  depot.load[procplus] =
      calculcoutt(&depot.tabt[procplus], dtp);


  /*
  Se restar somente o depósito na sequência,
  a carga da máquina será igual a zero.
  */
  if (depot.tabt[procplus].nbredenoeuds == 1)
    depot.load[procplus] = 0;


  /*
  Atualiza as informações de vizinhança da máquina
  de origem após a retirada da tarefa.
  */
  update_local_neighbourhood(
      vio,
      procplus,
      &neigh[procplus],
      &depot.tabt[procplus],
      g,
      d
  );


  /* Define a máquina que receberá a tarefa. */
  flag.numbdepot = pio;


  /* Insere definitivamente a tarefa vio na máquina pio. */
  ajoutx(
      vio,
      &neigh[pio],
      &depot.tabt[pio],
      g,
      dtp,
      &flag
  );


  /* Recalcula a carga da máquina que recebeu a tarefa. */
  depot.load[pio] =
      calculcoutt(&depot.tabt[pio], dtp);


  /* Atualiza a máquina à qual a tarefa vio pertence. */
  g[vio].satourne = pio;


  /*
  Atualiza a estrutura de vizinhança da máquina
  que recebeu a tarefa.
  */
  ajoutenoeudprox(&neigh[pio], vio, d, pio);


  /*
  Finaliza a medição desta iteração e acumula
  o tempo gasto na fase tabu, em minutos.
  */
  timetab = timetab + ((wtime() - tabu_iter_start) / 1000000.0);


  /*
  Linhas desativadas que exibiriam informações
  detalhadas sobre cada movimento realizado.
  */
  /*
  printf("\nIteration = %d", iteration);

  printf(
      "\nProc.out = %d - Var.out = %d - new cust = %d",
      procplus,
      vio,
      depot.load[procplus]
  );

  printf(
      "\nProc.in = %d - new cust = %d\n",
      pio,
      depot.load[pio]
  );
  */


  /*
  Proíbe temporariamente que a tarefa vio retorne
  para sua máquina de origem.
  */
  tabulist[procplus][vio] =
      tabu_time(mintabu, maxtabu) + iteration;


  /* Registra a última iteração em que a tarefa vio foi movimentada. */
  lasttabu[vio] = iteration;


  /* Inicializa a procura pela nova máquina mais carregada. */
  maxload = 0;


  /*
  Percorre todas as máquinas para determinar:
  - a maior carga atual;
  - qual máquina possui essa carga.
  */
  for (i = 1; i <= np; i++) {

    if (depot.load[i] > maxload) {

      maxload = depot.load[i];

      procplus = i;
    }
  }


  /*
  Se a maior carga atual for menor que o melhor
  makespan conhecido, uma nova melhor solução foi encontrada.
  */
  if (maxload < makespan.load) {

    printf(
        "\nmakespan = %d - %8.2f [%d]",
        maxload,
        timetab,
        iteration
    );

    /* Registra a iteração da melhoria. */
    makespan.iter = iteration;


    /*
    Armazena a nova melhor solução encontrada.
    */
    store_best_solution(
        g,
        &depot,
        bg,
        &bsolution,
        &makespan,
        neigh,
        bneigh
    );


    /* Registra a última iteração em que houve melhoria. */
    lastimp = iteration;

    /* Registra o tempo em que a melhor solução foi encontrada. */
    timetb = timetab;

  } else {

    /*
    Caso não haja melhoria durante mais de n × 10 iterações,
    inicia o controle do tempo adicional de busca.
    */
    if (((iteration - lastimp) > (n * 100)) &&
        (outTime == 0)) {

      /*
      Soma o tempo já utilizado ao limite de tempo
      adicional definido para a busca.
      */
      timeViz += timetab;

      /* Indica que o controle de tempo adicional foi ativado. */
      outTime = 1;

      printf(" ## in %4.2f\n", timetab);


      /*
      Informa quando foi definido um tempo adicional
      positivo para continuar a busca.
      */
      if (timeViz > 0)
        printf(
            "---- Usando tempo extra predeterminado ... ----\n"
        );
    }


    /*
    Critério anterior de parada baseado somente no
    número de iterações sem melhoria. Está desativado.
    */
    // if ((iteration - lastimp) > (n * 100))
    //   imptrue = 0;


    /*
    Encerra a busca quando o tempo tabu alcança o
    limite definido e o controle de tempo está ativo.
    */
    if ((timetab >= timeViz) && (outTime == 1))
      imptrue = 0;
  }

} /* Fim das iterações da busca tabu */


/* Armazena o melhor makespan obtido após a fase tabu. */
solutab = makespan.load;


// printf("\n*****BEST SOLUTION AFTER TABU PHASE*******");


/* Exibe a iteração em que a melhor solução foi encontrada. */
printf("\n\nIteration    = %d", makespan.iter);


/*
Linhas desativadas que exibiriam a máquina mais carregada
e o valor do melhor makespan.
*/
// printf("\nLoadest Proc = %d", makespan.proc);
// printf("\nLoad         = %d\n", makespan.load);


/*
Percorre todas as máquinas da melhor solução armazenada.
*/
for (j = 1; j <= np; j++) {

  /*
  Posiciona o ponteiro no início da sequência
  da máquina j da melhor solução.
  */
  imppb = bsolution.tabt[j].ptr;


  /*
  Linhas desativadas que exibiriam o número da máquina
  e sua carga na melhor solução.
  */
  // printf("\n  *********PROCESSOR %d ***********", j);
  // printf("\n CUST = %d\n", bsolution.load[j]);


  /*
  Percorre todos os elementos da sequência da máquina j.
  O valor +1 inclui o depósito no percurso.
  */
  for (i = 1;
       i <= bsolution.tabt[j].nbredenoeuds + 1;
       i++) {

    /* Linha desativada que exibiria cada tarefa da sequência. */
    // printf("%d-", imppb->noeud);

    /* Avança para o próximo elemento da sequência. */
    imppb = imppb->prochain;
  }

  /* Linha desativada que separaria visualmente as máquinas. */
  // printf("\n");
}


   /************ INÍCIO DA PÓS-OTIMIZAÇÃO **************/

/* Indica inicialmente que a máquina crítica ainda não foi alterada. */
flag_change = 0;

/* Seleciona a máquina que possui o maior makespan na melhor solução. */
procplus = makespan.proc;

/* Inicia a medição de alta resolução da pós-otimização. */
post_start = wtime();

/*
Realiza melhorias internas nas sequências das máquinas mais carregadas.
O processo continua enquanto outra máquina se tornar a mais carregada.
*/
do {

  /*
  Se a máquina crítica mudou na iteração anterior,
  atualiza as informações do makespan.
  */
  if (flag_change) {
    procplus = newprocplus;
    makespan.proc = newprocplus;
    makespan.load = bsolution.load[newprocplus];
  }

  /* Inicializa a busca pela nova máquina mais carregada. */
  newprocplus = 1;

  /* Armazena o custo atual da máquina que será otimizada. */
  exc = bsolution.load[procplus];

  /* Obtém o nó depósito da máquina que será otimizada. */
  xx = bsolution.nodepot[procplus];

  /* Informa qual máquina está sendo modificada. */
  flag.numbdepot = procplus;

  /*
  Copia a sequência da máquina crítica para uma estrutura auxiliar.
  As alterações são testadas inicialmente em t2 e g2.
  */
  copietourne(
      &bsolution.tabt[procplus],
      bg,
      &t2,
      g2,
      xx
  );

  /*
  Conta quantos elementos foram analisados desde
  a última melhoria encontrada.
  */
  cpt = 0;

  /*
  Indica que as operações oterx e ajoutx devem
  efetivamente modificar a sequência auxiliar.
  */
  flag.modifie = 1;

  /*
  Percorre os nós da sequência procurando uma organização
  interna que reduza a carga da máquina.
  */
  do {

    // printf("\n procplus==%d", procplus);

    /*
    Retira o nó xx da sequência auxiliar e procura
    a melhor forma de reorganizar os demais nós.
    */
    oterx(
        xx,
        &bneigh[procplus],
        &t2,
        g2,
        dtp,
        &flag
    );

    // printf("\nNumero de nos depois do oterx %d", t2.nbredenoeuds);

    /*
    Aumenta temporariamente o tamanho da vizinhança
    considerada na reinserção do nó.
    */
    k1++;

    /*
    Reinsere o nó xx na melhor posição encontrada
    dentro da sequência auxiliar.
    */
    ajoutx(
        xx,
        &bneigh[procplus],
        &t2,
        g2,
        dtp,
        &flag
    );

    // printf("\nNumero de nos depois do ajoutx %d", t2.nbredenoeuds);

    /* Retorna o tamanho da vizinhança ao valor anterior. */
    k1--;

    /* Calcula o novo custo da sequência auxiliar. */
    custp = calculcoutt(&t2, dtp);

    /*
    Se a nova organização apresentar custo menor,
    armazena a sequência como nova melhor sequência da máquina.
    */
    if (exc > custp) {

      /* Atualiza o menor custo encontrado. */
      exc = custp;

      /*
      Reinicia o contador porque ocorreu uma melhoria.
      Assim, todos os nós serão novamente analisados.
      */
      cpt = 0;

      /*
      Copia a sequência auxiliar melhorada para
      a melhor solução armazenada.
      */
      copietourne(
          &t2,
          g2,
          &bsolution.tabt[procplus],
          bg,
          xx
      );

      /* Atualiza a carga da máquina na melhor solução. */
      bsolution.load[procplus] = custp;

      /* Atualiza o makespan armazenado. */
      makespan.load = custp;
    }

    /* Avança para o próximo nó da sequência auxiliar. */
    xx = suiv(xx, g2);

    /* Incrementa o número de nós analisados sem melhoria. */
    cpt++;

  /*
  Encerra quando todos os nós da sequência forem
  analisados sem encontrar uma nova melhoria.
  */
  } while (cpt != bsolution.tabt[procplus].nbredenoeuds);


  /*
  Inicializa o maior custo usando a carga da primeira máquina.
  */
  custp = bsolution.load[newprocplus];

  /*
  Garante que o ponteiro inicial da sequência corresponda
  ao depósito da máquina.
  */
  if (bg[n + procplus].ptrtourne !=
      bsolution.tabt[procplus].ptr) {

    bsolution.tabt[procplus].ptr =
        bg[n + procplus].ptrtourne;

    /* Renumera as posições dos nós na sequência. */
    numerote_tourne(&bsolution.tabt[procplus]);
  }

  /*
  Procura a máquina com a maior carga após
  a otimização da máquina atual.
  */
  for (i = 2; i <= np; i++) {

    if (bsolution.load[i] > custp) {

      newprocplus = i;
      custp = bsolution.load[i];
    }
  }

  /*
  Indica que houve mudança da máquina crítica.
  */
  if (procplus != newprocplus)
    flag_change = 1;

/*
Continua enquanto uma máquina diferente daquela que acabou
de ser otimizada possuir a maior carga.
*/
} while (procplus != newprocplus);


/* Calcula o tempo da pós-otimização em minutos. */
timepot = (wtime() - post_start) / 1000000.0;

/* Armazena o makespan obtido após a pós-otimização. */
solupot = makespan.load;


/* Exibe a melhor solução depois da pós-otimização. */
printf("\n**BEST SOLUTION AFTER POST-OPTIMIZATION**\n");

/* Exibe a máquina com maior carga. */
printf("\nLoadest Proc. = %d", makespan.proc);

/* Exibe o valor final do makespan. */
printf("\nLoad          = %d", makespan.load);


/*
Exibe a sequência de tarefas e o custo de cada máquina.
*/
for (j = 1; j <= np; j++) {

  /* Posiciona o ponteiro no início da sequência da máquina j. */
  imppb = bsolution.tabt[j].ptr;

  printf("\n  *********PROCESSOR %d ***********", j);

  /* Exibe a carga da máquina. */
  printf("\n COST = %d\n", bsolution.load[j]);

  /*
  Percorre a sequência da máquina.
  O valor adicional inclui o retorno ao depósito.
  */
  for (i = 1;
       i <= bsolution.tabt[j].nbredenoeuds + 1;
       i++) {

    /* Exibe o identificador do nó atual. */
    printf("%d-", imppb->noeud);

    /* Avança para o próximo nó. */
    imppb = imppb->prochain;
  }

  printf("\n");
}


/* Calcula o tempo total das três etapas do algoritmo. */
timetot = timeia + timetab + timepot;


/* Acumula os tempos das diferentes etapas. */
ttimeia += timeia;
ttimetab += timetab;
ttimetb += timetb;
ttimepot += timepot;
ttimetot += timetot;


/* Acumula os valores das soluções obtidas. */
tsoluias += soluias;
tsolutab += solutab;
tsolupot += solupot;


/* Exibe os resultados e tempos de cada etapa. */
printf("\nInitial Solution       = %f", soluias);
printf("\nTime Initial Assigment = %.9f", timeia);
printf("\nTabu Solution          = %f", solutab);
printf("\nTime Tabu Phase        = %.9f", timetab);
printf("\nPos-Opt. Solution      = %f", solupot);
printf("\nTime Post Optimization = %.9f", timepot);
END = wtime();
printf("\nTotal Time (sum)       = %.9f", timetot);
printf("\nTotal wall time        = %.9f\n", (END - START) / 1000000.0);
printf("CLK_TCK                = %ld\n", sysconf(_SC_CLK_TCK));


/*** CRIAÇÃO DO ARQUIVO DE SAÍDA PARA A ROTINA FORTRAN ***/


/*
Trecho desativado que permitiria criar o arquivo somente
quando o makespan fosse melhor que o valor anterior.
*/
// if (makespan.load < besttabu[ij]) {

// besttabu[ij] = makespan.load;


/*
Inicia o nome do arquivo com a letra "r".
*/
strcpy(namef, "r");

/*
Acrescenta ao nome do arquivo o nome da instância.
Exemplo: se fname for "instancia.txt", o arquivo será
chamado "rinstancia.txt".
*/
strcat(namef, fname);


/*
Trecho desativado que acrescentaria o número
da execução ao nome do arquivo.
*/
// if (ij == 1) strcat(namef,"01");
// if (ij == 2) strcat(namef,"02");
// if (ij == 3) strcat(namef,"03");
// if (ij == 4) strcat(namef,"04");
// if (ij == 5) strcat(namef,"05");
// if (ij == 6) strcat(namef,"06");
// if (ij == 7) strcat(namef,"07");
// if (ij == 8) strcat(namef,"08");
// if (ij == 9) strcat(namef,"09");
// if (ij == 10) strcat(namef,"10");


/*
Abre o arquivo de saída para escrita.
Se já existir um arquivo com o mesmo nome, seu conteúdo será substituído.
*/
if ((outf = fopen(namef, "w")) == NULL) {

  printf(
      "\nOUTPUT FILE CAN NOT BE OPEN "
      "\n ## END OF THE PROGRAM\n"
  );

  /* Encerra o programa se o arquivo não puder ser criado. */
  exit(1);
}


/* Define o número do problema. */
nprob = 1;

/* Define o número de nós, incluindo o depósito. */
ns = n + 1;

/* Define a opção utilizada pela rotina externa. */
choix = 1;

/* Indica que os valores não são coordenadas euclidianas. */
eucl = 0;

/* Armazena o makespan como valor máximo da solução. */
maxdist = makespan.load;


/*
Grava a primeira linha do arquivo contendo:
número do problema, número de nós, número de máquinas,
indicador euclidiano, opção escolhida e makespan.
*/
fprintf(
    outf,
    "%5d%5d%5d%5d%5d%5d\n",
    (int)nprob,
    (int)ns,
    (int)np,
    (int)eucl,
    (int)choix,
    (int)maxdist
);


/*
Grava os valores da matriz dtp para cada par de tarefas.
São armazenados os custos nas duas direções.
*/
for (i = 1; i <= n; i++) {

  for (j = i + 1; j <= n + 1; j++) {

    fprintf(
        outf,
        "%5d%5d\n",
        dtp[i][j],
        dtp[j][i]
    );
  }
}


/* Grava o tempo total de execução no arquivo. */
fprintf(outf, "%10.3f", timetot);


/* Fecha o arquivo de saída. */
fclose(outf);


// } /* Fim da comparação com besttabu */
// } /* Fim do antigo laço de dez execuções */


/*
Trecho desativado que exibiria estatísticas consolidadas
de várias execuções do algoritmo.
*/

// printf("\n\n**************FINAL STATISTICS*****************\n");
// printf("\nTabu Tag Min = %d      Tabu Tag Max = %d",
//        mintabu, maxtabu);
// printf("\nLocal neighbourhood    = %d", MAXK);
// printf("\nGlobal neighbourhood   = %d", MAXK1);
// printf("\nNumber of Problems     = %d", ij - 1);
// printf("\nNumber of Proc.        = %d", np);
// printf("\nNumber of Tasks        = %d", n);

// printf("\nInitial Solution       = %f     %f",
//        (tsoluias / (ij - 1)),
//        (((tsoluias - tsolupot) * 100) / tsolupot));

// printf("\nTabu Solution          = %f     %f",
//        (tsolutab / (ij - 1)),
//        (((tsolutab - tsolupot) * 100) / tsolupot));

// printf("\nPos-Opt. Solution      = %f     %f",
//        (tsolupot / (ij - 1)),
//        (((tsolupot - tsolupot) * 100) / tsolupot));

// printf("\nTime Initial Solution  = %f     %f",
//        (ttimeia / (ij - 1)),
//        ((ttimeia * 100) / ttimetot));

// printf("\nTime To Best Solution  = %f     %f",
//        (ttimetb / (ij - 1)),
//        ((ttimetb * 100) / ttimetot));

// printf("\nTime Tabu Phase        = %f     %f",
//        (ttimetab / (ij - 1)),
//        ((ttimetab * 100) / ttimetot));

// printf("\nTime Post Optimization = %f     %f",
//        (ttimepot / (ij - 1)),
//        ((ttimepot * 100) / ttimetot));

// printf("\nTotal Time             = %f     %f",
//        (ttimetot / (ij - 1)),
//        ((ttimetot * 100) / ttimetot));

printf("\n");

} /* Fim do laço que executa os valores de alfa */


/*** CRIAÇÃO DO ARQUIVO PARA A VIZINHANÇA DE LARGA ESCALA ***/


/* Define o prefixo do arquivo da solução tabu. */
strcpy(namef, "solucaoTabu.");

/* Acrescenta o nome da instância ao nome do arquivo. */
strcat(namef, fname);


/* Abre o arquivo de solução para escrita. */
if ((outf = fopen(namef, "w")) == NULL) {

  printf(
      "\nOUTPUT FILE CAN NOT BE OPEN "
      "\n ## END OF THE PROGRAM\n"
  );

  exit(1);
}


/*
Grava na primeira linha:
nome da instância, número de máquinas, número de tarefas
e o caractere "e".
*/
fprintf(
    outf,
    "%s %d %d e\n",
    fname,
    np,
    n
);


/*
Percorre todas as máquinas da melhor solução.
*/
for (j = 1; j <= np; j++) {

  /* Posiciona o ponteiro no início da sequência da máquina. */
  imppb = bsolution.tabt[j].ptr;

  /*
  Percorre os elementos armazenados na sequência.
  */
  for (i = 1;
       i <= bsolution.tabt[j].nbredenoeuds;
       i++) {

    /*
    Grava apenas os nós correspondentes a tarefas.
    Nós maiores que n representam depósitos.
    */
    if (imppb->noeud <= n) {

      /*
      Grava:
      - posição da tarefa na sequência;
      - número da tarefa;
      - número da máquina.

      Os valores são reduzidos em uma unidade para usar
      indexação iniciada em zero no arquivo de saída.
      */
      fprintf(
          outf,
          "%5d %5d %5d\n",
          i - 2,
          imppb->noeud - 1,
          j - 1
      );
    }

    /* Avança para o próximo nó da sequência. */
    imppb = imppb->prochain;
  }
}


/*
Grava a linha que indica o final das tarefas,
seguida pelo nome da instância, tempo da fase tabu
e valor da função objetivo.
*/
fprintf(
    outf,
    "-1 -1 -1\n%30s   time: %10.3f  fo: %10d\n",
    fname,
    timetab,
    makespan.load
);


/* Fecha o arquivo da solução tabu. */
fclose(outf);

printf("\n");

return 0;

} /* FIM DA FUNÇÃO MAIN() */


/******************************************************************/


/*
Inicializa os vetores e matrizes utilizados
pela busca tabu.
*/
init(depot, lasttabu, tabulist)

tdepot *depot;

int lasttabu[],
    tabulist[MAXP + 1][MAXN + 1];

{
  int ll1, ll2;

  /*
  Percorre todas as possíveis tarefas.
  */
  for (ll1 = 0; ll1 <= MAXN; ll1++) {

    /*
    Inicializa com zero a última iteração tabu
    associada a cada tarefa.
    */
    lasttabu[ll1] = 0;

    /*
    Percorre todas as possíveis máquinas.
    */
    for (ll2 = 0; ll2 <= MAXP; ll2++) {

      /*
      Inicialmente, nenhuma tarefa pertence
      a nenhuma sequência de máquina.
      */
      depot->tabt[ll2].noeudinterne[ll1] = 0;

      /*
      Inicializa todas as posições da lista tabu com zero.
      Portanto, nenhum movimento está inicialmente proibido.
      */
      tabulist[ll2][ll1] = 0;
    }
  }

} /* FIM DA FUNÇÃO INIT() */


/*******************************************************************/


/*
Lê a instância, constrói as matrizes de custos
e inicializa as estruturas das máquinas.
*/
generegraph(g, d, dtp, tproc, depot)

coord g[];

int d[MAXN + 1][MAXN + 1],
    dtp[MAXN + 1][MAXN + 1],
    tproc[MAXN + 1];

tdepot *depot;

{
  FILE *inp;

  int i, j, k, mach, task, cvali;
  int max_setup_valido;
  long long soma_processamentos;
  long long limite_superior_rota;

  float cval;


  /*
  Abre o arquivo da instância para leitura.
  */
  if ((inp = fopen(fname, "r")) == NULL) {

    printf(
        "\nFILE CAN NOT BE OPEN "
        "\n ## END OF THE PROGRAM ##\n"
    );

    exit(1);
  }


  /* Lê o número de máquinas. */
  fscanf(inp, "%d\n", &mach);

  /* Lê o número de tarefas. */
  fscanf(inp, "%d\n", &task);


  /*
  Calcula o tamanho da vizinhança local.

  O valor depende de:
  - alfa;
  - número de tarefas;
  - número de máquinas.
  */
  cval =
      alfa *
      sqrt((float)task / (float)mach) +
      0.99;

  /* Converte o valor calculado para inteiro. */
  MAXK = cval;

  /* Garante uma vizinhança local mínima de três elementos. */
  if (MAXK <= 3)
    MAXK = 3;

  /* Copia o tamanho da vizinhança para k1. */
  k1 = MAXK;


  /*
  Calcula o tamanho da vizinhança global
  com base no número de tarefas.
  */
  cval =
      alfa *
      sqrt((float)task) +
      0.99;

  MAXK1 = cval;

  /* Garante uma vizinhança global mínima de cinco elementos. */
  if (MAXK1 <= 5)
    MAXK1 = 5;


  /* Armazena globalmente o número de máquinas. */
  np = mach;

  /* Armazena globalmente o número de tarefas. */
  n = task;


  /******** INÍCIO DA CORREÇÃO 3: DADOS PARA O LIMITE SUPERIOR ********/
  soma_processamentos = 0;
  max_setup_valido = 0;
  /******** FIM DA CORREÇÃO 3 *****************************************/

  /*
  Lê os tempos de processamento das tarefas
  e dos nós associados aos depósitos.
  */
  for (i = 1; i <= n + np; i++) {

    fscanf(inp, "%d\n", &cvali);

    tproc[i] = cvali;

    /******** INÍCIO DA CORREÇÃO 3 ************************************/
    if (i <= n)
      soma_processamentos += (long long)cvali;
    /******** FIM DA CORREÇÃO 3 ***************************************/
  }


  /*
  Lê a matriz de custos ou distâncias sem transformar os valores.
  A identificação do marcador é realizada somente depois que toda
  a matriz estiver disponível.
  */
  for (i = 1; i <= n + np; i++) {

    for (j = 1; j <= n + np; j++) {

      if (fscanf(inp, "%d", &cvali) != 1) {
        printf("\nERRO: matriz de setup incompleta ou em formato invalido.\n");
        fclose(inp);
        exit(1);
      }

      d[i][j] = cvali;
    }
  }


  /******** INÍCIO DA CORREÇÃO 3: MARCADOR CALCULADO E PENALIDADE DINÂMICA *****/
  /*
  As transições proibidas são identificadas pela estrutura do problema,
  e não pelo valor numérico existente na instância:

  - tarefa -> ela mesma: i == j;
  - depósito -> depósito: i > n e j > n.

  Todos os demais elementos são setups válidos, inclusive valores acima de 500.
  O marcador interno é calculado como uma unidade acima do maior setup válido.
  O valor original escrito nas células proibidas da instância é ignorado.
  */
  max_setup_valido = 0;

  for (i = 1; i <= n + np; i++) {
    for (j = 1; j <= n + np; j++) {

      if (!((i == j) || ((i > n) && (j > n)))) {

        if (d[i][j] < 0) {
          printf("\nERRO: setup valido negativo encontrado na instancia.\n");
          fclose(inp);
          exit(1);
        }

        if (d[i][j] > max_setup_valido)
          max_setup_valido = d[i][j];
      }
    }
  }

  if (max_setup_valido >= CUSTO_INFINITO - 1) {
    printf("\nERRO: nao foi possivel calcular um marcador numericamente seguro.\n");
    fclose(inp);
    exit(1);
  }

  marcador_setup_proibido = max_setup_valido + 1;

  /*
  Uma rota com todas as n tarefas possui no máximo n + 1 arcos:
  depósito -> primeira tarefa, arcos entre tarefas e última -> depósito.

  limite superior = soma dos processamentos
                  + (n + 1) * maior setup permitido

  A penalidade fica uma unidade acima desse limite.
  */
  limite_superior_rota =
      soma_processamentos +
      ((long long)n + 1LL) * (long long)max_setup_valido;

  if (limite_superior_rota >= (long long)CUSTO_INFINITO - 1LL) {
    printf("\nERRO: os custos da instancia excedem o limite numerico seguro.\n");
    fclose(inp);
    exit(1);
  }

  penalidade_transicao = (int)(limite_superior_rota + 1LL);

  if (penalidade_transicao <= marcador_setup_proibido)
    penalidade_transicao = marcador_setup_proibido + 1;

  /*
  As células proibidas recebem diretamente a penalidade.
  Não é necessário substituir manualmente 500, 580 ou qualquer outro valor
  na diagonal da instância.
  */
  for (i = 1; i <= n + np; i++) {
    for (j = 1; j <= n + np; j++) {
      if ((i == j) || ((i > n) && (j > n)))
        d[i][j] = penalidade_transicao;
    }
  }

  printf(
      "Calculated setup marker = %d | Max valid setup = %d | Forbidden penalty = %d\n",
      marcador_setup_proibido,
      max_setup_valido,
      penalidade_transicao
  );
  /******** FIM DA CORREÇÃO 3 *****************************************************/


  /*
  Constrói a matriz dtp.

  Cada valor corresponde à soma:
  custo de transição de i para j + tempo de processamento de j.
  */
  for (i = 1; i <= n + np; i++) {

    for (j = 1; j <= n + np; j++) {

      dtp[i][j] = d[i][j] + tproc[j];
    }
  }


  /*
  Cria inicialmente uma sequência para cada máquina,
  contendo apenas seu respectivo depósito.
  */
  initialise_depot(depot, g);


  /*
  Inicializa as estruturas de vizinhança.
  */
  initprox(neigh);


  /*
  Adiciona os depósitos às estruturas de vizinhança
  das respectivas máquinas.
  */
  for (i = n + 1; i <= n + np; i++) {

    ajoutenoeudprox(
        &neigh[i - n],
        i,
        d,
        i - n
    );
  }

} /* FIM DA FUNÇÃO GENEREGRAPH */


/*******************************************************************/


/*
Inicializa uma sequência para cada máquina,
contendo inicialmente apenas seu depósito.
*/
initialise_depot(depot, g)

tdepot *depot;

coord g[];

{
  int j;

  /*
  Os nós de n + 1 até n + np representam os depósitos.
  Cada máquina recebe um depósito diferente.
  */
  for (j = n + 1; j <= n + np; j++) {

    /*
    Cria uma nova sequência contendo somente
    o depósito associado à máquina.
    */
    nouvelletourne(
        &depot->tabt[j - n],
        j,
        g
    );

    /*
    Informa a qual máquina o depósito pertence.
    */
    g[j].satourne = j - n;

    /*
    Registra o número do depósito da máquina.
    */
    depot->nodepot[j - n] = j;
  }

} /* FIM DA FUNÇÃO INITIALISE_DEPOT */


/*******************************************************************/


/*
Inicializa as listas de vizinhos mais próximos
de todos os nós e máquinas.
*/
initprox(neigh)

neighbour neigh[];

{
  proxnoeud w;

  int i, j;


  /*
  Inicializa as posições da lista de vizinhos com -1,
  indicando que ainda não existem vizinhos cadastrados.
  */
  for (j = 1; j <= k1 + 1; j++)
    w.nn[j] = -1;


  /*
  Inicialmente, a primeira posição é considerada
  a posição do vizinho mais distante.
  */
  w.leplusloin = 1;


  /*
  Inicializa a maior distância com um valor muito elevado.
  Isso permite que qualquer distância real seja aceita
  como vizinho durante o preenchimento da estrutura.
  */
  w.maxdist = MAXREAL;


  /*
  Inicializa as duas listas de vizinhança de cada nó
  para cada máquina.
  */
  for (i = 1; i <= np; i++) {

    for (j = 1; j <= n + np; j++) {

      /*
      p1 armazena vizinhos considerando uma direção
      das transições.
      */
      neigh[i].p1[j] = w;

      /*
      p2 armazena vizinhos considerando a direção oposta
      das transições.
      */
      neigh[i].p2[j] = w;
    }
  }

} /* FIM DA FUNÇÃO INITPROX */

/******************************************************************************************************/
/********************************************************************/

/*
Identifica, para cada tarefa, os MAXK1 vizinhos mais próximos
considerando duas direções:

neighout: custo de saída da tarefa i para a tarefa j, isto é, i -> j.
neighin: custo de entrada da tarefa j para a tarefa i, isto é, j -> i.
*/
find_in_out_neighbour(neighin, neighout, d)

proxnoeud neighin[], neighout[];

/* Matriz de distâncias ou custos entre os nós. */
int d[MAXN+1][MAXN+1];

{
  /*
  imin armazena os índices dos nós durante a ordenação.
  min armazena as distâncias associadas a esses nós.
  exchange indica se houve troca durante a ordenação.
  */
  int i, j, i1, j1, imin[MAXN+1], temp, exchange;
  float min[MAXN+1];

  /* Analisa cada tarefa do problema. */
  for (i = 1; i <= n; i++) {

    /******** PROCURA DOS VIZINHOS DE SAÍDA: i -> j ********/

    /*
    Impede que a própria tarefa i seja considerada
    vizinha de si mesma.
    */
    min[i] = MAXREAL;
    imin[i] = i;

    /*
    Armazena o custo de sair da tarefa i
    em direção a cada tarefa j.
    */
    for (j = 1; j <= (n + 1); j++) {

      if (j != i) {
        imin[j] = j;
        min[j] = d[i][j];
      }
    }

    /*
    Ordena os nós por distância crescente usando
    uma variação do método Bubble Sort.
    */
    exchange = 1;

    for (i1 = 1;
         ((i1 <= n) && (exchange));
         i1++) {

      /* Assume inicialmente que não ocorrerá nenhuma troca. */
      exchange = 0;

      for (j1 = 1;
           j1 <= (n + 1 - i1);
           j1++) {

        /*
        Se o nó atual possuir distância maior que o próximo,
        troca suas posições.
        */
        if (min[imin[j1]] > min[imin[j1+1]]) {

          temp = imin[j1];
          imin[j1] = imin[j1+1];
          imin[j1+1] = temp;

          /* Informa que houve troca e outra passagem será necessária. */
          exchange = 1;
        }
      }
    }

    /*
    Armazena os MAXK1 vizinhos de saída mais próximos da tarefa i.
    */
    for (j = 1; j <= MAXK1; j++) {

      neighout[i].nn[j] = imin[j];

      /*
      Linha de depuração desativada.
      */
      // printf("\n neighout[%i].nn[%i]==%d",
      //        i, j, neighout[i].nn[j]);
    }

    /*
    Registra a posição do vizinho mais distante
    dentro da lista selecionada.
    */
    neighout[i].leplusloin = MAXK1;

    /*
    Registra a distância do último vizinho selecionado,
    que é o mais distante entre os MAXK1 vizinhos.
    */
    neighout[i].maxdist = min[imin[MAXK1]];


    /******** PROCURA DOS VIZINHOS DE ENTRADA: j -> i ********/

    /*
    Impede novamente que a tarefa i seja considerada
    vizinha de si mesma.
    */
    min[i] = MAXREAL;
    imin[i] = i;

    /*
    Armazena o custo de cada tarefa j em direção à tarefa i.
    */
    for (j = 1; j <= (n + 1); j++) {

      if (j != i) {
        imin[j] = j;
        min[j] = d[j][i];
      }
    }

    /*
    Ordena os nós por custo crescente.
    */
    exchange = 1;

    for (i1 = 1;
         ((i1 <= n) && (exchange));
         i1++) {

      exchange = 0;

      for (j1 = 1;
           j1 <= (n + 1 - i1);
           j1++) {

        if (min[imin[j1]] > min[imin[j1+1]]) {

          temp = imin[j1];
          imin[j1] = imin[j1+1];
          imin[j1+1] = temp;

          exchange = 1;
        }
      }
    }

    /*
    Armazena os MAXK1 vizinhos de entrada mais próximos da tarefa i.
    */
    for (j = 1; j <= MAXK1; j++) {

      neighin[i].nn[j] = imin[j];

      /*
      Linha de depuração desativada.
      */
      // printf("\n neighin[%i].nn[%i]==%d",
      //        i, j, neighin[i].nn[j]);
    }

    /* Registra a posição do vizinho mais distante selecionado. */
    neighin[i].leplusloin = MAXK1;

    /* Registra a maior distância entre os vizinhos de entrada. */
    neighin[i].maxdist = min[imin[MAXK1]];

  } /* Fim da análise de todas as tarefas */

} /* FIM DA FUNÇÃO FIND_IN_OUT_NEIGHBOUR */


/********************************************************************/

/*
Constrói a solução inicial.

Primeiramente, atribui uma tarefa a cada máquina.
Depois, atribui as tarefas restantes à máquina que produzir
o menor aumento de carga.
*/
initial_assignment(g, d, dtp, depot, neigh)

coord g[];

/*
d: matriz de distâncias ou custos de transição.
dtp: matriz que inclui custo de transição e processamento.
*/
int d[MAXN+1][MAXN+1],
    dtp[MAXN+1][MAXN+1];

/* Estrutura que armazena as sequências e cargas das máquinas. */
tdepot *depot;

/* Estruturas de vizinhança de cada máquina. */
neighbour neigh[];

{
  /*
  dgap armazenará a máquina escolhida
  para receber a tarefa analisada.
  */
  int i, j, dgap;

  /*
  gap armazena o menor custo encontrado
  para a inserção da tarefa.
  */
  float gap;

  /* Estrutura auxiliar utilizada nas operações de inserção. */
  tflag flag;

  /* Ponteiro auxiliar para percorrer as sequências. */
  tourneelem *impp;
  double initial_start;


  /* Inicia a medição de alta resolução da atribuição inicial. */
  initial_start = wtime();


  /*
  Inicialmente, coloca uma tarefa em cada máquina.
  A tarefa i é atribuída à máquina i.
  */
  for (i = 1; i <= np; i++) {

    /*
    Adiciona a tarefa i à sequência da máquina i.
    Antes disso, a sequência continha somente o depósito.
    */
    ajoute_a_tourne(
        &depot->tabt[i],
        i,
        g
    );

    /*
    Atualiza a lista de vizinhos da máquina i
    após a inserção da tarefa.
    */
    ajoutenoeudprox(
        &neigh[i],
        i,
        d,
        i
    );

    /*
    Atualiza a posição de cada nó dentro da sequência.
    */
    numerote_tourne(
        &depot->tabt[i]
    );

    /******** INÍCIO DA CORREÇÃO 1: CARGA INICIAL COMPLETA ************/
    /*
    Recalcula a rota circular completa:
    depósito -> tarefa -> depósito.
    Assim, inclui o setup inicial, o processamento e o setup final.
    */
    depot->load[i] =
        calculcoutt(
            &depot->tabt[i],
            dtp
        );
    /******** FIM DA CORREÇÃO 1 ***************************************/

    /*
    Registra que a tarefa i pertence à máquina i.
    */
    g[i].satourne = i;
  }


  /*
  Indica inicialmente que as chamadas de ajoutx
  apenas avaliarão as inserções, sem modificar as sequências.
  */
  flag.modifie = 0;

  /* Inicializa a variação de custo com um valor elevado. */
  flag.mdelta = CUSTO_INFINITO;

  /* Inicializa o menor custo de inserção com um valor elevado. */
  gap = CUSTO_INFINITO;


  /*
  Atribui as tarefas restantes.

  As primeiras np tarefas já foram atribuídas,
  portanto a análise começa em np + 1.
  */
  for (i = np + 1; i <= n; i++) {

    /*
    Testa a inserção da tarefa i em cada máquina.
    */
    for (j = 1; j <= np; j++) {

      /* Define a máquina que está sendo avaliada. */
      flag.numbdepot = j;

      /*
      Avalia a melhor posição para inserir a tarefa i
      na sequência da máquina j.

      Como flag.modifie é zero, a sequência não é alterada.
      A variação de carga fica armazenada em flag.mdelta.
      */
      ajoutx(
          i,
          &neigh[j],
          &depot->tabt[j],
          g,
          dtp,
          &flag
      );

      /*
      Verifica se a carga resultante é menor ou igual
      à melhor carga encontrada até o momento.
      */
      if ((flag.mdelta + depot->load[j]) <= gap) {

        /* Atualiza o menor valor encontrado. */
        gap = depot->load[j] + flag.mdelta;

        /* Armazena a máquina escolhida. */
        dgap = j;
      }
    }


    /*
    Permite que a próxima chamada de ajoutx
    modifique efetivamente a solução.
    */
    flag.modifie = 1;

    /*
    Insere definitivamente a tarefa i
    na melhor máquina encontrada.
    */
    ajoutx(
        i,
        &neigh[dgap],
        &depot->tabt[dgap],
        g,
        dtp,
        &flag
    );

    /*
    Recalcula a carga completa da máquina
    que recebeu a tarefa.
    */
    depot->load[dgap] =
        calculcoutt(
            &depot->tabt[dgap],
            dtp
        );

    /*
    Atualiza a estrutura de vizinhança da máquina
    após a inserção da tarefa.
    */
    ajoutenoeudprox(
        &neigh[dgap],
        i,
        d,
        dgap
    );

    /*
    Registra a máquina à qual a tarefa i foi atribuída.
    */
    g[i].satourne = dgap;

    /*
    Volta ao modo de avaliação para a próxima tarefa.
    */
    flag.modifie = 0;

    /*
    Reinicializa o menor custo antes de analisar
    a próxima tarefa.
    */
    gap = MAXREAL;
  }


  /******** INÍCIO DA CORREÇÃO 1: VERIFICAÇÃO FINAL DAS CARGAS ******/
  /*
  Garante que todas as máquinas terminem a atribuição inicial com
  a carga obtida pela mesma função circular usada nas demais fases.
  */
  for (i = 1; i <= np; i++) {
    if (depot->tabt[i].nbredenoeuds > 1)
      depot->load[i] = calculcoutt(&depot->tabt[i], dtp);
    else
      depot->load[i] = 0;
  }
  /******** FIM DA CORREÇÃO 1 ***************************************/

  /*
  Finaliza a medição do tempo da atribuição inicial
  e converte o resultado para minutos.
  */
  timeia = (wtime() - initial_start) / 1000000.0;


  /*
  Linhas abaixo percorrem a solução inicial.
  As instruções de impressão estão desativadas.
  */

  // printf("\n************INITIAL ASSIGMENT****************");

  for (j = 1; j <= np; j++) {

    /* Posiciona o ponteiro no início da sequência da máquina j. */
    impp = depot->tabt[j].ptr;

    // printf("\n  *********PROCESSOR %d ***********", j);
    // printf("\n CUST = %d\n", depot->load[j]);

    /*
    Percorre todos os nós da sequência,
    incluindo o depósito.
    */
    for (i = 1;
         i <= depot->tabt[j].nbredenoeuds + 1;
         i++) {

      // printf("%d-", impp->noeud);

      /* Avança para o próximo nó da sequência. */
      impp = impp->prochain;
    }

    // printf("\n");
  }

} /* FIM DA FUNÇÃO INITIAL_ASSIGNMENT */


/******************************************************************/

/*
Cria uma nova sequência contendo apenas o nó x.

A sequência é representada por uma lista circular
duplamente encadeada.
*/
nouvelletourne(t, x, g)

tourne *t;

/* Nó que será inserido na nova sequência. */
int x;

/* Vetor que armazena os ponteiros para cada nó. */
coord g[];

{
  /*
  Indica que o nó x pertence à sequência.
  */
  t->noeudinterne[x] = 1;

  /*
  A sequência possui inicialmente somente um nó.
  */
  t->nbredenoeuds = 1;

  /*
  Reserva memória para o primeiro elemento da sequência.
  */
  t->ptr =
      (tourneelem *) malloc(sizeof(tourneelem));

  /*
  Verifica se a alocação de memória foi realizada.
  Caso contrário, encerra o programa.
  */
  demand(t->ptr, memory overflow);

  /* Armazena o número do nó. */
  t->ptr->noeud = x;

  /*
  Como existe somente um elemento, o próximo elemento
  é o próprio elemento.
  */
  t->ptr->prochain = t->ptr;

  /*
  Como existe somente um elemento, o elemento anterior
  também é o próprio elemento.
  */
  t->ptr->precedent = t->ptr;

  /*
  Armazena em g[x] o ponteiro para o nó criado.
  */
  g[x].ptrtourne = t->ptr;

} /* FIM DA FUNÇÃO NOUVELLETOURNE */


/******************************************************************/

/*
Atualiza as listas dos k1 vizinhos mais próximos
depois que o nó aj é adicionado a uma máquina.

p1 considera os custos i -> aj.
p2 considera os custos aj -> i.
*/
ajoutenoeudprox(neigh, aj, d, ndep)

/* Estrutura de vizinhança da máquina atual. */
neighbour *neigh;

/* Nó que foi adicionado à sequência. */
int aj;

/* Matriz de distâncias ou custos. */
int d[MAXN+1][MAXN+1];

/* Número da máquina ou depósito analisado. */
int ndep;

{
  /*
  nmaximum: posição do vizinho mais distante.
  vmaximum: valor da maior distância.
  dist: distância analisada.
  */
  int i, nmaximum, j;
  float vmaximum, dist;


  /*
  Analisa todas as tarefas e também o depósito
  da máquina atual.
  */
  for (i = 1; i <= (n + 1); i++) {

    /*
    Quando i chega à posição n + 1,
    substitui esse valor pelo nó correspondente
    ao depósito da máquina atual.
    */
    if (i == (n + 1))
      i = ndep + n;


    /*
    O nó adicionado não pode ser vizinho de si mesmo.
    */
    if (i != aj) {

      /******** ATUALIZAÇÃO DA VIZINHANÇA p1: i -> aj ********/

      /*
      Verifica se a distância de i até aj é menor
      que a maior distância atualmente armazenada.
      */
      if (neigh->p1[i].maxdist > d[i][aj]) {

        /*
        Substitui o vizinho mais distante pelo novo nó aj.
        */
        neigh->p1[i].nn[
            neigh->p1[i].leplusloin
        ] = aj;


        /*
        A posição k1 + 1 funciona como uma posição auxiliar
        para armazenar um vizinho adicional.
        */
        if (neigh->p1[i].nn[k1+1] < 0)

          neigh->p1[i].nn[k1+1] = aj;

        /*
        Caso a posição auxiliar já esteja ocupada,
        verifica se aj deve substituir o valor armazenado.
        */
        else if (
            neigh->p1[i].maxdist <
            d[i][neigh->p1[i].nn[k1+1]]
        )

          neigh->p1[i].nn[k1+1] = aj;


        /*
        Procura novamente qual é o vizinho mais distante
        entre as primeiras k1 posições.
        */
        vmaximum = 0;

        for (j = 1; j <= k1; j++) {

          /*
          Se existir uma posição vazia, ela passa
          a ser considerada a posição mais distante.
          */
          if (neigh->p1[i].nn[j] < 0) {

            nmaximum = j;
            vmaximum = MAXREAL;
            break;

          } else {

            /* Obtém a distância até o vizinho armazenado. */
            dist =
                d[i][neigh->p1[i].nn[j]];

            /*
            Atualiza a posição do vizinho mais distante.
            */
            if (dist >= vmaximum) {

              nmaximum = j;
              vmaximum = dist;
            }
          }
        }

        /*
        Atualiza a posição e a distância
        do vizinho mais distante.
        */
        neigh->p1[i].leplusloin = nmaximum;
        neigh->p1[i].maxdist = vmaximum;


      /*
      Se aj não estiver entre os k1 melhores,
      poderá ocupar a posição auxiliar k1 + 1.
      */
      } else if (neigh->p1[i].nn[k1+1] < 0) {

        neigh->p1[i].nn[k1+1] = aj;


      /*
      Caso a posição auxiliar já esteja ocupada,
      mantém nela o melhor vizinho adicional.
      */
      } else if (
          d[i][aj] <
          d[i][neigh->p1[i].nn[k1+1]]
      ) {

        neigh->p1[i].nn[k1+1] = aj;
      }


      /******** ATUALIZAÇÃO DA VIZINHANÇA p2: aj -> i ********/

      /*
      Verifica se a distância de aj até i é menor
      que a maior distância atualmente armazenada em p2.
      */
      if (neigh->p2[i].maxdist > d[aj][i]) {

        /*
        Substitui o vizinho mais distante pelo nó aj.
        */
        neigh->p2[i].nn[
            neigh->p2[i].leplusloin
        ] = aj;


        /*
        Armazena aj na posição auxiliar,
        caso essa posição ainda esteja vazia.
        */
        if (neigh->p2[i].nn[k1+1] < 0)

          neigh->p2[i].nn[k1+1] = aj;

        /*
        Verifica se aj deve substituir
        o vizinho auxiliar já armazenado.
        */
        else if (
            neigh->p2[i].maxdist <
            d[neigh->p2[i].nn[k1+1]][i]
        )

          neigh->p2[i].nn[k1+1] = aj;


        /*
        Procura o vizinho mais distante
        entre as primeiras k1 posições de p2.
        */
        vmaximum = 0;

        for (j = 1; j <= k1; j++) {

          if (neigh->p2[i].nn[j] < 0) {

            nmaximum = j;
            vmaximum = MAXREAL;
            break;

          } else {

            /*
            Em p2, a distância é calculada
            do vizinho armazenado até o nó i.
            */
            dist =
                d[neigh->p2[i].nn[j]][i];

            if (dist >= vmaximum) {

              nmaximum = j;
              vmaximum = dist;
            }
          }
        }

        /*
        Atualiza o maior valor e sua posição.
        */
        neigh->p2[i].leplusloin = nmaximum;
        neigh->p2[i].maxdist = vmaximum;


      /*
      Se aj não estiver entre os k1 vizinhos,
      tenta armazená-lo na posição auxiliar.
      */
      } else if (neigh->p2[i].nn[k1+1] < 0) {

        neigh->p2[i].nn[k1+1] = aj;


      /*
      Mantém na posição auxiliar o melhor
      vizinho adicional encontrado.
      */
      } else if (
          d[aj][i] <
          d[neigh->p2[i].nn[k1+1]][i]
      ) {

        neigh->p2[i].nn[k1+1] = aj;
      }
    }
  }

} /* FIM DA FUNÇÃO AJOUTENOEUDPROX */

/******************************************************************************************************/
/****************************************************************/

/*
Atualiza a vizinhança local de uma máquina após a retirada do nó x.

A função remove x das listas p1 e p2 e procura um nó substituto,
quando necessário.

p1 considera a direção var -> vizinho.
p2 considera a direção vizinho -> var.
*/
update_local_neighbourhood(x, nproc, neigh, t, g, d)

int x, nproc;

neighbour *neigh;

tourne *t;

coord g[];

int d[MAXN+1][MAXN+1];

{
  /*
  var: nó que está sendo analisado.
  var2: possível nó substituto.
  sofar: vizinho que deve ser desconsiderado na substituição.
  maxd: distância do candidato selecionado.
  maxv: nó candidato selecionado.
  vtrue1 e vtrue2: variáveis booleanas de controle.
  ident: quantidade máxima de vizinhos possível.
  */
  int var, i, j, k, l, var2, sofar, maxd, maxv,
      vtrue1, vtrue2, eq, ident;


  /******** ATUALIZAÇÃO DA VIZINHANÇA p1 ********/

  /*
  Analisa todas as tarefas e também o depósito
  da máquina nproc.
  */
  for (i = 1; i <= (n + 1); i++) {

    var = i;

    /*
    Na última posição, utiliza o nó que representa
    o depósito da máquina nproc.
    */
    if (i == (n + 1))
      var = n + nproc;

    /*
    Mantém a busca ativa enquanto o nó x
    não for encontrado na lista.
    */
    vtrue1 = 1;

    /*
    Se var pertence à sequência, podem existir k1 vizinhos.
    Caso contrário, considera-se k1 - 1.
    */
    if (t->noeudinterne[var])
      ident = k1;
    else
      ident = k1 - 1;


    /*
    Procura o nó removido x nas primeiras k1
    posições da vizinhança p1 de var.
    */
    for (j = 1;
         ((j <= k1) && (vtrue1));
         j++) {

      /*
      Normaliza qualquer nó de depósito para
      o depósito da máquina atual.
      */
      if (neigh->p1[var].nn[j] > n)
        neigh->p1[var].nn[j] = n + nproc;


      /*
      Verifica se a posição atual contém o nó x,
      que foi retirado da sequência.
      */
      if (neigh->p1[var].nn[j] == x) {

        /*
        Quando a sequência possui poucos nós, não existe
        um número suficiente de candidatos para preencher
        todas as posições da vizinhança.
        */
        if (t->nbredenoeuds <= ident) {

          /*
          Desloca os vizinhos posteriores uma posição
          para substituir o nó x.
          */
          for (k = j; k <= (k1 - 1); k++)
            neigh->p1[var].nn[k] =
                neigh->p1[var].nn[k+1];

          /*
          Marca a última posição como vazia.
          */
          neigh->p1[var].nn[k] = -1;

          /*
          Procura a primeira posição vazia da lista.
          */
          vtrue2 = 1;

          for (k = j; vtrue2; k++) {

            if (neigh->p1[var].nn[k] < 0) {

              /*
              A posição vazia passa a ser considerada
              a posição mais distante.
              */
              neigh->p1[var].leplusloin = k;

              /*
              MAXREAL indica que ainda não existe
              um vizinho válido nessa posição.
              */
              neigh->p1[var].maxdist = MAXREAL;

              vtrue2 = 0;
            }
          }

        } else {

          /*
          Quando existem nós suficientes, procura-se
          um substituto para o nó removido.
          */
          maxd = CUSTO_INFINITO;

          /*
          A busca começa pelo depósito da máquina.
          */
          var2 = n + nproc;

          /*
          Evita selecionar novamente o nó removido
          ou o vizinho anteriormente considerado mais distante.
          */
          if (neigh->p1[var].leplusloin == j)
            sofar = x;
          else
            sofar =
                neigh->p1[var].nn[
                    neigh->p1[var].leplusloin
                ];


          /*
          Caso var seja um depósito e a distância máxima
          armazenada seja zero, procura um nó ainda não
          existente na lista de vizinhos.
          */
          if ((var > n) &&
              (neigh->p1[var].maxdist == 0)) {

            do {

              /* Avança para o próximo nó da sequência. */
              var2 = suiv(var2, g);

              vtrue2 = 1;
              eq = 1;

              /*
              Verifica se var2 já está armazenado
              em outra posição da lista.
              */
              for (k = 1;
                   ((k <= k1) && (eq));
                   k++) {

                if (k != j) {

                  eq = 1;

                  if (var2 == neigh->p1[var].nn[k])
                    eq = 0;
                }
              }

              /*
              Se var2 não estiver repetido,
              utiliza-o para substituir x.
              */
              if (eq) {

                neigh->p1[var].nn[j] = var2;

                vtrue2 = 0;
              }

            } while (vtrue2);

          } else {

            /*
            Percorre os nós da sequência procurando
            um candidato para substituir x em p1.
            */
            for (k = 1;
                 k <= t->nbredenoeuds;
                 k++) {

              /*
              Desconsidera o próprio nó var e o nó sofar.
              */
              if ((var != var2) &&
                  (var2 != sofar)) {

                /*
                Procura a menor distância válida
                que seja maior ou igual ao limite atual.
                */
                if (d[var][var2] < maxd) {

                  if (d[var][var2] >=
                      neigh->p1[var].maxdist) {

                    vtrue2 = 1;

                    /*
                    Em caso de empate, verifica se o nó
                    já está armazenado na lista.
                    */
                    if (d[var][var2] ==
                        neigh->p1[var].maxdist) {

                      for (l = 1;
                           l <= k1;
                           l++) {

                        if (var2 ==
                            neigh->p1[var].nn[l])

                          vtrue2 = 0;
                      }
                    }

                    /*
                    Armazena o candidato encontrado.
                    */
                    if (vtrue2) {

                      maxd = d[var][var2];

                      maxv = var2;
                    }
                  }
                }
              }

              /* Avança para o próximo nó da sequência. */
              var2 = suiv(var2, g);
            }

            /*
            Insere o nó substituto na posição anteriormente
            ocupada por x.
            */
            neigh->p1[var].nn[j] = maxv;

            /*
            A posição substituída passa a representar
            o vizinho mais distante.
            */
            neigh->p1[var].leplusloin = j;

            /*
            Atualiza a maior distância armazenada.
            */
            neigh->p1[var].maxdist = maxd;
          }
        }

        /*
        Encerra a procura por x na lista atual.
        */
        vtrue1 = 0;
      }
    }


    /*
    Se x estiver na posição auxiliar k1 + 1,
    substitui-o pelo vizinho mais distante da lista principal.
    */
    if (neigh->p1[var].nn[k1+1] == x)

      neigh->p1[var].nn[k1+1] =
          neigh->p1[var].nn[
              neigh->p1[var].leplusloin
          ];
  }


  /******** ATUALIZAÇÃO DA VIZINHANÇA p2 ********/

  /*
  Repete o procedimento anterior para p2.
  A diferença é a direção utilizada na matriz d:
  em p2 considera-se var2 -> var.
  */
  for (i = 1; i <= (n + 1); i++) {

    var = i;

    if (i == (n + 1))
      var = n + nproc;

    vtrue1 = 1;

    if (t->noeudinterne[var])
      ident = k1;
    else
      ident = k1 - 1;


    /*
    Procura o nó x na lista p2 de var.
    */
    for (j = 1;
         ((j <= k1) && (vtrue1));
         j++) {

      /*
      Ajusta os identificadores dos depósitos.
      */
      if (neigh->p2[var].nn[j] > n)

        neigh->p2[var].nn[j] =
            n + nproc;


      /*
      Verifica se x está na posição atual.
      */
      if (neigh->p2[var].nn[j] == x) {

        /*
        Se não existirem nós suficientes,
        desloca a lista e deixa uma posição vazia.
        */
        if (t->nbredenoeuds <= ident) {

          for (k = j;
               k <= (k1 - 1);
               k++)

            neigh->p2[var].nn[k] =
                neigh->p2[var].nn[k+1];

          neigh->p2[var].nn[k] = -1;

          vtrue2 = 1;

          /*
          Procura a primeira posição vazia.
          */
          for (k = j; vtrue2; k++) {

            if (neigh->p2[var].nn[k] < 0) {

              neigh->p2[var].leplusloin = k;

              neigh->p2[var].maxdist = MAXREAL;

              vtrue2 = 0;
            }
          }

        } else {

          /*
          Procura um nó substituto para x.
          */
          maxd = CUSTO_INFINITO;

          var2 = n + nproc;

          if (neigh->p2[var].leplusloin == j)

            sofar = x;

          else

            sofar =
                neigh->p2[var].nn[
                    neigh->p2[var].leplusloin
                ];


          /*
          Tratamento específico quando var representa
          um depósito com distância máxima igual a zero.
          */
          if ((var > n) &&
              (neigh->p2[var].maxdist == 0)) {

            do {

              var2 = suiv(var2, g);

              vtrue2 = 1;
              eq = 1;

              /*
              Verifica se o candidato já está na lista.
              */
              for (k = 1;
                   ((k <= k1) && (eq));
                   k++) {

                if (k != j) {

                  eq = 1;

                  if (var2 ==
                      neigh->p2[var].nn[k])

                    eq = 0;
                }
              }

              /*
              Insere um candidato não repetido.
              */
              if (eq) {

                neigh->p2[var].nn[j] = var2;

                vtrue2 = 0;
              }

            } while (vtrue2);

          } else {

            /*
            Percorre a sequência procurando um substituto.
            Em p2, utiliza-se a distância var2 -> var.
            */
            for (k = 1;
                 k <= t->nbredenoeuds;
                 k++) {

              if ((var != var2) &&
                  (var2 != sofar)) {

                if (d[var2][var] < maxd) {

                  if (d[var2][var] >=
                      neigh->p2[var].maxdist) {

                    vtrue2 = 1;

                    /*
                    Evita inserir um nó duplicado em caso de empate.
                    */
                    if (d[var2][var] ==
                        neigh->p2[var].maxdist) {

                      for (l = 1;
                           l <= k1;
                           l++) {

                        if (var2 ==
                            neigh->p2[var].nn[l])

                          vtrue2 = 0;
                      }
                    }

                    if (vtrue2) {

                      maxd = d[var2][var];

                      maxv = var2;
                    }
                  }
                }
              }

              var2 = suiv(var2, g);
            }

            /*
            Substitui x pelo candidato encontrado.
            */
            neigh->p2[var].nn[j] = maxv;

            neigh->p2[var].leplusloin = j;

            neigh->p2[var].maxdist = maxd;
          }
        }

        vtrue1 = 0;
      }
    }


    /*
    Atualiza também a posição auxiliar k1 + 1,
    caso ela contenha o nó retirado.
    */
    if (neigh->p2[var].nn[k1+1] == x) {

      neigh->p2[var].nn[k1+1] =
          neigh->p2[var].nn[
              neigh->p2[var].leplusloin
          ];

      /*
      Linha de depuração desativada.
      */
      // printf("\nneigh->p2[%i].nn[%i]==%d",
      //        var, (k1+1),
      //        neigh->p2[var].nn[k1+1]);
    }
  }

} /* FIM DA FUNÇÃO UPDATE_LOCAL_NEIGHBOURHOOD */


/****************************************************************/

/*
Adiciona o nó ind ao final de uma sequência circular,
imediatamente antes do nó apontado por t->ptr.
*/
ajoute_a_tourne(t, ind, g)

tourne *t;

int ind;

coord g[];

{
  /* Ponteiro para o último elemento atual da sequência. */
  tourneelem *p;


  /*
  Obtém o elemento anterior ao primeiro nó.
  Em uma lista circular, esse elemento é o último nó.
  */
  p = t->ptr->precedent;


  /* Registra que o nó ind pertence à sequência. */
  t->noeudinterne[ind] = 1;


  /* Incrementa o número de nós da sequência. */
  t->nbredenoeuds++;


  /*
  Reserva memória para o novo elemento.
  O novo nó será o anterior ao primeiro elemento.
  */
  t->ptr->precedent =
      (tourneelem *) malloc(sizeof(tourneelem));


  /*
  Verifica se a memória foi alocada.
  */
  demand(t->ptr->precedent, memory overflow);


  /* Armazena o identificador do novo nó. */
  t->ptr->precedent->noeud = ind;


  /*
  O predecessor do novo nó será o antigo último nó.
  */
  t->ptr->precedent->precedent = p;


  /*
  O sucessor do novo nó será o primeiro nó da sequência.
  */
  t->ptr->precedent->prochain = t->ptr;


  /*
  Atualiza o sucessor do antigo último nó
  para apontar para o novo nó.
  */
  p->prochain = t->ptr->precedent;


  /*
  Armazena em g[ind] o endereço do elemento criado.
  */
  g[ind].ptrtourne = t->ptr->precedent;

} /* FIM DA FUNÇÃO AJOUTE_A_TOURNE */


/****************************************************************/

/*
Atribui uma posição, denominada rang, a cada nó da sequência.

Também verifica se a lista encadeada e o vetor
noeudinterne representam exatamente os mesmos nós.
*/
numerote_tourne(t)

tourne *t;

{
  /*
  w: ponteiro utilizado para percorrer a sequência.
  en: vetor que registra quais nós foram encontrados.
  indtrue: indica se a sequência é consistente.
  */
  tourneelem *w;

  int i, j, indtrue, en[MAXN+1];


  /*
  Inicializa o vetor de nós encontrados com zero.
  */
  for (i = 0; i <= (n + np + 1); i++)
    en[i] = 0;


  /* Assume inicialmente que a sequência está correta. */
  indtrue = 1;


  /* Começa pelo primeiro nó da sequência. */
  w = t->ptr;


  /*
  Percorre a lista circular atribuindo posições crescentes.
  */
  for (i = 1; i <= MAXN; i++) {

    /* Atribui a posição atual ao nó. */
    w->rang = i;

    /* Registra que o nó foi encontrado na lista. */
    en[w->noeud] = 1;

    /* Avança para o próximo nó. */
    w = w->prochain;

    /*
    Encerra quando retorna ao primeiro elemento,
    indicando que completou a volta na lista circular.
    */
    if (w == t->ptr)
      break;
  }


  /*
  Compara os nós encontrados na lista com o vetor
  noeudinterne da estrutura.
  */
  for (j = 0; j <= (n + np + 1); j++) {

    if (en[j] != t->noeudinterne[j]) {

      /* Indica inconsistência entre as estruturas. */
      indtrue = 0;

      break;
    }
  }


  /*
  Encerra o programa se:
  - o número percorrido for diferente do número registrado; ou
  - os nós da lista não coincidirem com noeudinterne.
  */
  if ((i != t->nbredenoeuds) ||
      (!indtrue)) {

    printf(
        "\nWRONG ROUTE ASSIGNMENT "
        "### END OF THE PROGRAM\n"
    );

    exit(1);
  }

} /* FIM DA FUNÇÃO NUMEROTE_TOURNE */


/******************************************************************/

/*
Avalia e, quando solicitado, executa a inserção do nó x
em uma sequência.

A função procura a melhor forma de inserir x, podendo também
inverter e reconectar partes da sequência.

Quando flag->modifie é zero:
- apenas calcula o menor aumento de custo;
- não altera a sequência.

Quando flag->modifie é diferente de zero:
- realiza efetivamente a melhor inserção encontrada.

O aumento ou redução do custo fica armazenado em flag->mdelta.
*/
ajoutx(x, neigh, t, g, dtp, flag)

int x;

neighbour *neigh;

tourne *t;

coord g[];

int dtp[MAXN+1][MAXN+1];

tflag *flag;

{
  /*
  Estrutura que armazena os nós envolvidos
  na melhor operação de inserção encontrada.

  Letras iniciadas por s representam sucessores.
  Letras iniciadas por p representam predecessores.
  typeajout identifica o tipo de reconexão.
  */
  struct oper {
    int x, i, j, k, l,
        si, sj, sk, sl,
        pi, pj, pk, pl,
        typeajout;
  } noeuds;


  /*
  Variáveis utilizadas para avaliar posições,
  sucessores, predecessores, ordens e custos.
  */
  int som1, som2, i, j, k, l,
      xi, xj, xk, xl,
      xprecnj, xsuivni,
      ni, nl, nk, nj,
      suivni, suivnj, suivnl, suivnk,
      precnk, precnl, precnj, precni,
      xk1, yk1, delta, nouvdelta,
      voisinx_vu, deja_vuxi2, nlvaut0;


  /* Ponteiro para o novo elemento que poderá ser criado. */
  tourneelem *px;


  /*
  Inicializa as variáveis auxiliares.
  */
  i = 0;
  j = 0;
  k = 0;
  l = 0;

  ni = 0;
  nj = 0;
  nk = 0;
  nl = 0;

  xi = 0;
  xj = 0;
  xk = 0;
  xl = 0;


  /*
  Limita a quantidade de vizinhos analisados
  ao menor valor entre o tamanho da sequência e k1.
  */
  xk1 = mini(t->nbredenoeuds, k1);


  /*
  yk1 desconsidera um elemento da sequência
  durante determinadas reconexões.
  */
  yk1 = mini(t->nbredenoeuds - 1, k1);


  /*
  Inicializa a melhor variação de custo
  com um valor elevado.
  */
  delta = CUSTO_INFINITO;


  /******** INSERÇÃO EM SEQUÊNCIAS PEQUENAS ********/

  /*
  Quando a sequência possui no máximo dois nós,
  utiliza uma inserção simples.
  */
  if (xk1 <= 2) {

    /* Obtém o primeiro nó da sequência. */
    ni = t->ptr->noeud;

    /* Obtém o sucessor de ni. */
    nj = suiv(ni, g);


    /******** INÍCIO DA CORREÇÃO 2: INSERÇÕES EM ROTAS PEQUENAS *******/
    /*
    Quando existe somente o depósito, a carga anterior da máquina é zero.
    A inserção cria a rota completa depósito -> x -> depósito.
    */
    if (xk1 == 1) {

      nouvdelta =
          dtp[ni][x] +
          dtp[x][ni];

      nj = ni;

    } else {

      /*
      Opção 1: inserir x entre ni e nj.
      Remove ni -> nj e adiciona ni -> x e x -> nj.
      */
      nouvdelta =
          dtp[ni][x] +
          dtp[x][nj] -
          dtp[ni][nj];

      /*
      Opção 2: inserir x depois de nj. Como a lista é circular,
      esta fórmula também trata corretamente o retorno ao depósito:
      remove nj -> sucessor(nj) e adiciona nj -> x -> sucessor(nj).
      */
      suivnj = suiv(nj, g);

      som1 =
          dtp[nj][x] +
          dtp[x][suivnj] -
          dtp[nj][suivnj];

      if (nouvdelta > som1) {

        nouvdelta = som1;

        ni = nj;

        nj = suivnj;
      }
    }
    /******** FIM DA CORREÇÃO 2 ***************************************/


    /*
    Armazena a inserção simples caso seja
    a melhor encontrada.
    */
    if (nouvdelta <= delta) {

      delta = nouvdelta;

      noeuds.typeajout = 5;

      noeuds.x = x;

      noeuds.i = ni;

      noeuds.j = nj;
    }

  } else {

    /******** INSERÇÃO EM SEQUÊNCIAS MAIORES ********/

    /*
    Percorre possíveis predecessores ni do nó x
    utilizando a vizinhança p2 de x.
    */
    for (i = 1; i <= xk1; i++) {

      ni = neigh->p2[x].nn[i];

      /*
      Valores negativos indicam posições vazias.
      */
      if (ni < 0)
        ni = 0;


      /*
      Continua somente se ni pertence à sequência.
      */
      if (t->noeudinterne[ni]) {

        suivni = suiv(ni, g);

        precni = prec(ni, g);

        xi = ordre(ni, g);
      }


      /*
      Indica se o sucessor original de ni
      já foi considerado como candidato.
      */
      voisinx_vu = 0;


      /*
      Percorre possíveis sucessores nj do nó x,
      utilizando a vizinhança p1 de x.
      */
      for (j = 1;
           (((j <= xk1) || (!voisinx_vu)) &&
            (t->noeudinterne[ni]));
           j++) {

        nj = neigh->p1[x].nn[j];

        if (nj < 0)
          nj = 0;


        /*
        Registra se o sucessor original de ni
        foi encontrado na vizinhança.
        */
        if (nj == suivni)
          voisinx_vu = 1;


        /*
        Caso o sucessor original não esteja entre
        os vizinhos, força sua avaliação.
        */
        if ((j > xk1) &&
            (!voisinx_vu)) {

          voisinx_vu = 1;

          nj = suivni;
        }


        /*
        Obtém a posição de nj na sequência.
        */
        if (t->noeudinterne[nj])
          xj = ordre(nj, g);


        /*
        Continua quando ni e nj são nós distintos
        e nj pertence à sequência.
        */
        if ((xj != xi) &&
            (t->noeudinterne[nj])) {

          suivnj = suiv(nj, g);

          precnj = prec(nj, g);

          deja_vuxi2 = 0;


          /*
          Analisa diferentes opções para o nó nl,
          utilizando vizinhanças de saída e entrada.
          */
          for (l = 1;
               l <= (2 * yk1 + 1);
               l++) {

            nlvaut0 = 0;


            /*
            Primeira parte: utiliza vizinhos p1
            do sucessor de ni.
            */
            if (l <= yk1) {

              nl =
                  neigh->p1[suivni].nn[l];

              if (nl < 0)
                nl = 0;


            /*
            Inclui explicitamente o segundo sucessor de ni,
            caso ele ainda não tenha sido avaliado.
            */
            } else if (
                (l == (yk1 + 1)) &&
                (!deja_vuxi2)
            )

              nl = suiv(suivni, g);


            /*
            Segunda parte: utiliza vizinhos p2
            do predecessor de nj.
            */
            else if (l > (yk1 + 1)) {

              nl =
                  neigh->p2[precnj]
                       .nn[l - yk1 - 1];

              if (nl < 0)
                nl = 0;

            } else {

              /*
              Indica uma condição em que nl
              não deve ser utilizado.
              */
              nlvaut0 = 1;
            }


            /*
            Evita avaliar duas vezes o segundo sucessor de ni.
            */
            if ((nl == suiv(suivni, g)) &&
                (l <= yk1))

              deja_vuxi2 = 1;


            /*
            Obtém posição, predecessor e sucessor de nl.
            */
            if ((!nlvaut0) &&
                (t->noeudinterne[nl])) {

              xl = ordre(nl, g);

              precnl = prec(nl, g);

              suivnl = suiv(nl, g);
            }


            /*
            Continua somente quando nl é válido
            e pertence à sequência.
            */
            if ((!((l == (yk1 + 1)) &&
                   (deja_vuxi2))) &&
                (t->noeudinterne[nl])) {


              /*
              Verifica a posição relativa dos nós
              na sequência circular.
              */
              if (((xj > xi) &&
                   ((xl < xi) || (xl > xj))) ||
                  ((xj < xi) &&
                   ((xl > xj) && (xl < xi)))) {


                /******** TIPO DE INSERÇÃO 1 ********/

                if (l <= yk1) {

                  /*
                  Calcula a variação de custo provocada
                  pela inserção de x e pelas reconexões.
                  */
                  nouvdelta =
                      dtp[ni][x] +
                      dtp[x][nj] +
                      dtp[suivni][nl] +
                      dtp[suivnj][suivnl] -
                      dtp[ni][suivni] -
                      dtp[nj][suivnj] -
                      dtp[nl][suivnl];


                  /*
                  Acrescenta ao cálculo os custos
                  provocados pelas inversões de segmentos.
                  */
                  som1 = nj;

                  while (som1 != suivni) {

                    som2 = prec(som1, g);

                    nouvdelta =
                        nouvdelta -
                        dtp[som2][som1] +
                        dtp[som1][som2];

                    som1 = som2;
                  }


                  som1 = nl;

                  while (som1 != suivnj) {

                    som2 = prec(som1, g);

                    nouvdelta =
                        nouvdelta -
                        dtp[som2][som1] +
                        dtp[som1][som2];

                    som1 = som2;
                  }


                  /*
                  Mantém a menor variação de custo.
                  */
                  delta =
                      mini(delta, nouvdelta);


                  /*
                  Armazena os dados da operação tipo 1.
                  */
                  if (delta == nouvdelta) {

                    /*
                    s representa sucessor.
                    p representa predecessor.
                    */
                    noeuds.typeajout = 1;

                    noeuds.i = ni;
                    noeuds.x = x;
                    noeuds.j = nj;
                    noeuds.si = suivni;
                    noeuds.l = nl;
                    noeuds.sj = suivnj;
                    noeuds.sl = suivnl;
                  }


                /******** TIPO DE INSERÇÃO 3 ********/

                } else {

                  /*
                  Calcula a variação de custo para
                  uma reconexão inversa ao tipo 1.
                  */
                  nouvdelta =
                      dtp[ni][x] +
                      dtp[x][nj] +
                      dtp[nl][precnj] +
                      dtp[precnl][precni] -
                      dtp[precni][ni] -
                      dtp[precnj][nj] -
                      dtp[precnl][nl];


                  /*
                  Inclui os efeitos da inversão dos segmentos.
                  */
                  som1 = precnj;

                  while (som1 != ni) {

                    som2 = prec(som1, g);

                    nouvdelta =
                        nouvdelta -
                        dtp[som2][som1] +
                        dtp[som1][som2];

                    som1 = som2;
                  }


                  som1 = precni;

                  while (som1 != nl) {

                    som2 = prec(som1, g);

                    nouvdelta =
                        nouvdelta -
                        dtp[som2][som1] +
                        dtp[som1][som2];

                    som1 = som2;
                  }


                  delta =
                      mini(delta, nouvdelta);


                  /*
                  Armazena os dados da operação tipo 3.
                  */
                  if (delta == nouvdelta) {

                    noeuds.typeajout = 3;

                    noeuds.i = ni;
                    noeuds.x = x;
                    noeuds.j = nj;
                    noeuds.l = nl;
                    noeuds.pi = precni;
                    noeuds.pj = precnj;
                    noeuds.pl = precnl;
                  }
                }


                /*
                Verifica se também é possível executar
                uma reconexão mais ampla, envolvendo o nó nk.
                */
                if (
                    (((xl != ordre(suivnj, g)) &&
                      (l <= (yk1 + 1))) ||
                     ((xl != ordre(precni, g)) &&
                      (l > (yk1 + 1)))) &&
                    (xj != ordre(suivni, g))
                ) {


                  /*
                  Percorre possíveis valores de nk.
                  */
                  for (k = 1; k <= yk1; k++) {

                    /*
                    Escolhe a vizinhança utilizada
                    conforme o valor de l.
                    */
                    if (l <= yk1 + 1)

                      nk =
                          neigh->p2[suivnj].nn[k];

                    else

                      nk =
                          neigh->p1[precni].nn[k];


                    if (nk < 0)
                      nk = 0;


                    /*
                    Obtém a posição de nk.
                    */
                    if (t->noeudinterne[nk])
                      xk = ordre(nk, g);


                    xsuivni =
                        ordre(suivni, g);

                    xprecnj =
                        ordre(precnj, g);


                    /*
                    Verifica se nk está em uma posição
                    compatível com a reconexão.
                    */
                    if (
                        (((((xj > xsuivni) &&
                            (xk > xsuivni) &&
                            (xk <= xj)) ||
                           ((xj < xsuivni) &&
                            ((xk > xsuivni) ||
                             (xk <= xj)))) &&
                          (l <= (yk1 + 1))) ||

                         ((((xprecnj > xi) &&
                            (xk >= xi) &&
                            (xk < xprecnj)) ||
                           ((xprecnj < xi) &&
                            ((xk >= xi) ||
                             (xk < xprecnj)))) &&
                          (l > (yk1 + 1)))) &&

                        (t->noeudinterne[nk])
                    ) {

                      suivnk = suiv(nk, g);

                      precnk = prec(nk, g);


                      /******** TIPO DE INSERÇÃO 2 ********/

                      if (l <= (yk1 + 1)) {

                        /*
                        Calcula a variação de custo
                        da operação tipo 2.
                        */
                        nouvdelta =
                            dtp[ni][x] +
                            dtp[x][nj] +
                            dtp[nk][suivnj] +
                            dtp[precnl][precnk] +
                            dtp[suivni][nl] -
                            dtp[ni][suivni] -
                            dtp[nj][suivnj] -
                            dtp[precnl][nl] -
                            dtp[precnk][nk];


                        /*
                        Inclui os custos das inversões.
                        */
                        som1 = nj;

                        while (som1 != nk) {

                          som2 = prec(som1, g);

                          nouvdelta =
                              nouvdelta -
                              dtp[som2][som1] +
                              dtp[som1][som2];

                          som1 = som2;
                        }


                        som1 = precnk;

                        while (som1 != suivni) {

                          som2 = prec(som1, g);

                          nouvdelta =
                              nouvdelta -
                              dtp[som2][som1] +
                              dtp[som1][som2];

                          som1 = som2;
                        }


                        delta =
                            mini(delta, nouvdelta);


                        /*
                        Armazena a operação tipo 2.
                        */
                        if (delta == nouvdelta) {

                          noeuds.typeajout = 2;

                          noeuds.i = ni;
                          noeuds.x = x;
                          noeuds.j = nj;
                          noeuds.k = nk;
                          noeuds.sj = suivnj;
                          noeuds.pk = precnk;
                          noeuds.pl = precnl;
                          noeuds.si = suivni;
                          noeuds.l = nl;
                        }


                      /******** TIPO DE INSERÇÃO 4 ********/

                      } else {

                        /*
                        Calcula a variação de custo
                        da operação tipo 4.
                        */
                        nouvdelta =
                            dtp[ni][x] +
                            dtp[x][nj] +
                            dtp[precni][nk] +
                            dtp[suivnk][suivnl] +
                            dtp[nl][precnj] -
                            dtp[precni][ni] -
                            dtp[precnj][nj] -
                            dtp[nl][suivnl] -
                            dtp[nk][suivnk];


                        /*
                        Inclui os custos das inversões.
                        */
                        som1 = nk;

                        while (som1 != ni) {

                          som2 = prec(som1, g);

                          nouvdelta =
                              nouvdelta -
                              dtp[som2][som1] +
                              dtp[som1][som2];

                          som1 = som2;
                        }


                        som1 = precnj;

                        while (som1 != suivnk) {

                          som2 = prec(som1, g);

                          nouvdelta =
                              nouvdelta -
                              dtp[som2][som1] +
                              dtp[som1][som2];

                          som1 = som2;
                        }


                        delta =
                            mini(delta, nouvdelta);


                        /*
                        Armazena a operação tipo 4.
                        */
                        if (delta == nouvdelta) {

                          noeuds.typeajout = 4;

                          noeuds.i = ni;
                          noeuds.x = x;
                          noeuds.j = nj;
                          noeuds.k = nk;
                          noeuds.pj = precnj;
                          noeuds.sk = suivnk;
                          noeuds.sl = suivnl;
                          noeuds.pi = precni;
                          noeuds.l = nl;
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }


  /*
  Retorna a melhor variação de custo encontrada.
  */
  flag->mdelta = delta;


  /*
  Quando flag->modifie é verdadeiro,
  executa efetivamente a melhor inserção.
  */
  if (flag->modifie) {

    /*
    Reserva memória para o novo elemento.
    */
    px =
        (tourneelem *) malloc(
            sizeof(tourneelem)
        );


    /*
    Verifica se a memória foi alocada.
    */
    demand(px, memory overflow);


    /*
    Associa o novo elemento ao nó x.
    */
    g[x].ptrtourne = px;


    /*
    Armazena o identificador do nó inserido.
    */
    px->noeud = noeuds.x;


    /*
    Incrementa o número de nós da sequência.
    */
    t->nbredenoeuds++;


    /*
    Registra que x pertence à sequência.
    */
    t->noeudinterne[noeuds.x] = 1;


    /*
    Executa a reconexão correspondente
    ao melhor tipo de inserção encontrado.
    */
    switch (noeuds.typeajout) {


    /*
    Tipo 1:
    insere x e inverte dois segmentos da sequência.
    */
    case 1:

      chemin(noeuds.i, noeuds.x, g);

      chemin(noeuds.x, noeuds.j, g);

      inverse(noeuds.j, noeuds.si, g);

      chemin(noeuds.si, noeuds.l, g);

      inverse(noeuds.l, noeuds.sj, g);

      chemin(noeuds.sj, noeuds.sl, g);

      break;


    /*
    Tipo 2:
    insere x e realiza uma reconexão envolvendo
    os nós i, j, k e l.
    */
    case 2:

      chemin(noeuds.i, noeuds.x, g);

      chemin(noeuds.x, noeuds.j, g);

      inverse(noeuds.j, noeuds.k, g);

      chemin(noeuds.k, noeuds.sj, g);

      chemin(noeuds.pl, noeuds.pk, g);

      inverse(noeuds.pk, noeuds.si, g);

      chemin(noeuds.si, noeuds.l, g);

      break;


    /*
    Tipo 3:
    operação inversa ao tipo 1.
    */
    case 3:

      chemin(noeuds.pl, noeuds.pi, g);

      inverse(noeuds.pi, noeuds.l, g);

      chemin(noeuds.l, noeuds.pj, g);

      inverse(noeuds.pj, noeuds.i, g);

      chemin(noeuds.i, noeuds.x, g);

      chemin(noeuds.x, noeuds.j, g);

      break;


    /*
    Tipo 4:
    operação inversa ao tipo 2.
    */
    case 4:

      chemin(noeuds.l, noeuds.pj, g);

      inverse(noeuds.pj, noeuds.sk, g);

      chemin(noeuds.sk, noeuds.sl, g);

      chemin(noeuds.pi, noeuds.k, g);

      inverse(noeuds.k, noeuds.i, g);

      chemin(noeuds.i, noeuds.x, g);

      chemin(noeuds.x, noeuds.j, g);

      break;


    /*
    Tipo 5:
    inserção simples de x entre i e j.
    */
    case 5:

      chemin(noeuds.i, noeuds.x, g);

      chemin(noeuds.x, noeuds.j, g);

      break;
    }


    /*
    Atualiza as posições dos nós e verifica
    a consistência da sequência modificada.
    */
    numerote_tourne(t);
  }

} /* FIM DA FUNÇÃO AJOUTX */


/******************************************************************************************************/
/********************************************************************/

/*
Conecta diretamente o nó n1 ao nó n2 na lista encadeada.

Após a execução:
- n2 passa a ser o sucessor de n1;
- n1 passa a ser o predecessor de n2.
*/
chemin(n1, n2, g)

int n1, n2;

coord g[];

{
  /*
  Faz o ponteiro de próximo de n1 apontar para n2.
  */
  g[n1].ptrtourne->prochain =
      g[n2].ptrtourne;

  /*
  Faz o ponteiro de anterior de n2 apontar para n1.
  */
  g[n2].ptrtourne->precedent =
      g[n1].ptrtourne;

} /* FIM DA FUNÇÃO CHEMIN */


/********************************************************************/

/*
Inverte a orientação do segmento da sequência
compreendido entre os nós depart e arrive.
*/
inverse(depart, arrive, g)

int depart, arrive;

coord g[];

{
  /*
  d: nó atualmente percorrido durante a inversão.
  a: nó que representa o limite final da inversão.
  p: próximo nó que será processado.
  pp: armazena temporariamente o sucessor de p.
  */
  tourneelem *d, *a, *p, *pp;

  /* Número de ligações invertidas. */
  int lng;


  /*
  Conta quantas vezes a função inverse foi chamada.
  */
  cptlng = cptlng + 1;

  /* Inicializa o comprimento invertido. */
  lng = 0;

  /*
  Obtém o ponteiro correspondente ao nó depart.
  */
  a = g[depart].ptrtourne;

  /*
  Obtém o ponteiro correspondente ao nó arrive.
  */
  d = g[arrive].ptrtourne;

  /*
  Começa pelo sucessor do nó arrive.
  */
  p = d->prochain;


  /*
  Percorre o segmento até alcançar o nó depart,
  invertendo a direção dos ponteiros.
  */
  while (d != a) {

    /* Conta mais uma ligação invertida. */
    lng = lng + 1;

    /*
    Guarda o sucessor original de p antes
    de modificar os ponteiros.
    */
    pp = p->prochain;

    /*
    Faz p apontar para trás, em direção a d.
    */
    p->prochain = d;

    /*
    Faz d considerar p como seu predecessor.
    */
    d->precedent = p;

    /*
    Avança d para o próximo nó do processo de inversão.
    */
    d = p;

    /*
    Avança p utilizando o ponteiro salvo anteriormente.
    */
    p = pp;
  }


  /*
  Acumula o número total de ligações invertidas
  durante toda a execução do algoritmo.
  */
  lngtotal = lngtotal + lng;

} /* FIM DA FUNÇÃO INVERSE */


/********************************************************************/

/*
Mede o tempo de CPU consumido pelo programa.

Quando i é diferente de zero:
- registra o instante inicial.

Quando i é igual a zero:
- retorna o tempo decorrido desde o último início.
*/
int etime(int i)
{
  /*
  Declaração da função do sistema utilizada
  para obter os tempos de execução.
  */
  clock_t times();

  /*
  Estrutura que recebe os tempos de CPU
  do processo e do sistema operacional.
  */
  struct tms buffer;


  /*
  Inicia a medição do tempo.
  */
  if (i) {

    /* Obtém o tempo atual do processo. */
    times(&buffer);

    /*
    Armazena globalmente o tempo de CPU
    utilizado pelo processo.
    */
    elap = buffer.tms_utime;

    /* Retorna o instante inicial registrado. */
    return(elap);

  } else {

    /*
    Finaliza a medição e retorna
    a diferença em relação ao instante inicial.
    */
    times(&buffer);

    return(buffer.tms_utime - elap);
  }
}


/********************************************************************/

/*
Avalia e, quando solicitado, executa a retirada do nó x
de uma sequência.

É a operação inversa da função ajoutx.

Quando flag->modifie é zero:
- apenas calcula a variação de custo da retirada;
- não modifica a sequência.

Quando flag->modifie é diferente de zero:
- executa efetivamente a retirada e as reconexões.

A variação de custo é armazenada em flag->mdelta.
*/
oterx(x, neigh, t, g, dtp, flag)

int x;

neighbour *neigh;

tourne *t;

coord g[];

int dtp[MAXN+1][MAXN+1];

tflag *flag;

{
  /*
  Estrutura que armazena os nós envolvidos
  na melhor operação de retirada encontrada.

  sj, sk e sl representam sucessores.
  pj, pk e pl representam predecessores.
  typeretrait identifica o tipo de reconexão.
  */
  struct oper {
    int x, i, j, k, l,
        sj, sk, sl,
        pj, pk, pl, i2,
        typeretrait;
  } noeuds;


  /*
  Variáveis utilizadas para representar nós,
  posições, sucessores, predecessores e custos.
  */
  int ni, ni2, ni3, nj, nk, nl,
      j, k, l,
      xi, xj, xk, xl, xi2,
      suivnj, suivnl, suivnk,
      precnj, precnl, precnk,
      som1, som2,
      vu_ni2, vu_ni3,
      xk1, o, delta, nouvdelta;


  /*
  Limita o número de vizinhos analisados
  ao menor valor entre k1 e a quantidade de nós
  restantes após a retirada de x.
  */
  xk1 =
      mini(
          k1,
          t->nbredenoeuds - 1
      );

  // printf("\n xk1==%d", xk1);


  /*
  Inicializa a melhor variação de custo
  com um valor elevado.
  */
  delta = CUSTO_INFINITO;


  /*
  Se x for o nó inicial da sequência,
  desloca o ponteiro inicial para o sucessor de x.
  */
  if (t->ptr->noeud == x) {

    t->ptr =
        g[
            g[x].ptrtourne->prochain->noeud
        ].ptrtourne;

    /*
    Atualiza a numeração dos nós após
    a mudança do início da sequência.
    */
    numerote_tourne(t);
  }


  /******** RETIRADA EM SEQUÊNCIAS PEQUENAS ********/

  /*
  Quando existem no máximo dois vizinhos relevantes,
  utiliza uma retirada simples.
  */
  if (xk1 <= 2) {

    // printf("entrou em xk1<=2");

    /* Obtém o predecessor de x. */
    ni = prec(x, g);

    /* Obtém o sucessor de x. */
    nj = suiv(x, g);


    /******** INÍCIO DA CORREÇÃO 2: RETIRADA EM ROTA PEQUENA **********/
    /*
    Quando a sequência contém somente o depósito e x, retirar x
    elimina os dois arcos da rota: depósito -> x e x -> depósito.
    A carga resultante da máquina é zero.
    */
    if (xk1 == 1)

      nouvdelta =
          -dtp[ni][x] -
          dtp[x][nj];

    else
    /******** FIM DA CORREÇÃO 2 ***************************************/

      /*
      Calcula a variação provocada pela retirada de x
      e pela ligação direta entre ni e nj.
      */
      nouvdelta =
          dtp[ni][nj] -
          dtp[ni][x] -
          dtp[x][nj];


    /*
    Armazena a retirada simples se ela for
    a melhor alternativa encontrada.
    */
    if (nouvdelta <= delta) {

      delta = nouvdelta;

      noeuds.typeretrait = 5;

      noeuds.x = x;

      noeuds.i = ni;

      noeuds.j = nj;
    }

  } else {

    /******** RETIRADA EM SEQUÊNCIAS MAIORES ********/

    /*
    Obtém o nó imediatamente anterior a x.
    */
    ni = prec(x, g);

    /*
    Obtém a posição de ni na sequência.
    */
    xi = ordre(ni, g);

    /*
    Como o sucessor de ni é x, ni2 corresponde
    ao nó localizado depois de x.
    */
    ni2 =
        suiv(
            suiv(ni, g),
            g
        );

    /*
    Obtém a posição de ni2.
    */
    xi2 = ordre(ni2, g);


    /*
    Indica se ni2 coincide com x.
    */
    if (ni2 == x)

      vu_ni2 = 1;

    else

      vu_ni2 = 0;


    /*
    Analisa possíveis nós nj para reconectar
    após a retirada de x.
    */
    for (j = 1;
         ((j <= xk1) || (!vu_ni2));
         j++) {

      /*
      Caso ni2 não esteja entre os vizinhos analisados,
      força sua inclusão.
      */
      if (j > xk1) {

        nj = ni2;

        vu_ni2 = 1;

      } else {

        /*
        Seleciona um vizinho de saída de ni.
        */
        nj = neigh->p1[ni].nn[j];

        if (nj < 0)
          nj = 0;
      }


      /*
      O nó candidato não pode ser o próprio nó x.
      */
      if (nj != x) {

        /* Obtém o sucessor de nj. */
        suivnj = suiv(nj, g);

        /* Obtém a posição de nj. */
        xj = ordre(nj, g);

        /*
        Define ni3 como o sucessor de ni2.
        */
        ni3 = suiv(ni2, g);


        /*
        Verifica se ni3 já corresponde a algum
        nó que não deve ser analisado novamente.
        */
        if ((ni3 == ni) ||
            (ni3 == nj) ||
            (ni3 == x))

          vu_ni3 = 1;

        else

          vu_ni3 = 0;


        /*
        Analisa possíveis nós nl.
        */
        for (l = 1;
             ((l <= xk1) || (!vu_ni3));
             l++) {

          /*
          Caso ni3 não esteja entre os vizinhos,
          força sua avaliação.
          */
          if (l > xk1) {

            nl = ni3;

            vu_ni3 = 1;

          } else {

            /*
            Seleciona um vizinho de saída de ni2.
            */
            nl = neigh->p1[ni2].nn[l];

            if (nl < 0)
              nl = 0;
          }


          /*
          Evita utilizar nós que já participam
          diretamente da retirada.
          */
          if (!((nl == ni) ||
                (nl == nj) ||
                (nl == x))) {

            /* Obtém o sucessor de nl. */
            suivnl = suiv(nl, g);

            /* Obtém o predecessor de nl. */
            precnl = prec(nl, g);

            /* Obtém a posição de nl. */
            xl = ordre(nl, g);


            /*
            Verifica a posição relativa dos nós
            na sequência circular.
            */
            if (((xj > xi) &&
                 ((xl > xj) || (xl < xi))) ||
                ((xj < xi) &&
                 (xl > xj) &&
                 (xl < xi))) {

              /******** TIPO DE RETIRADA 1 ********/

              /*
              Calcula a variação de custo para retirar x
              e reconectar os segmentos envolvidos.
              */
              nouvdelta =
                  dtp[ni][nj] +
                  dtp[ni2][nl] +
                  dtp[suivnj][suivnl] -
                  dtp[ni][x] -
                  dtp[x][ni2] -
                  dtp[nj][suivnj] -
                  dtp[nl][suivnl];


              /*
              Inclui os custos gerados pela inversão
              do segmento entre nj e ni2.
              */
              som1 = nj;

              while (som1 != ni2) {

                som2 = prec(som1, g);

                nouvdelta =
                    nouvdelta -
                    dtp[som2][som1] +
                    dtp[som1][som2];

                som1 = som2;
              }


              /*
              Inclui os custos gerados pela inversão
              do segmento entre nl e suivnj.
              */
              som1 = nl;

              while (som1 != suivnj) {

                som2 = prec(som1, g);

                nouvdelta =
                    nouvdelta -
                    dtp[som2][som1] +
                    dtp[som1][som2];

                som1 = som2;
              }


              /* Mantém a menor variação de custo. */
              delta =
                  mini(delta, nouvdelta);


              /*
              Armazena os nós da operação tipo 1.
              */
              if (delta == nouvdelta) {

                noeuds.typeretrait = 1;

                noeuds.i = ni;
                noeuds.x = x;
                noeuds.j = nj;
                noeuds.i2 = ni2;
                noeuds.sj = suivnj;
                noeuds.sl = suivnl;
                noeuds.l = nl;
              }

            } else {

              /******** PROCURA DE UMA RETIRADA TIPO 2 ********/

              /*
              Analisa possíveis nós nk relacionados
              à vizinhança de entrada de suivnj.
              */
              for (k = 1;
                   k <= xk1;
                   k++) {

                nk =
                    neigh->p2[suivnj].nn[k];

                if (nk < 0)
                  nk = 0;


                /*
                Obtém a posição e o sucessor de nk.
                */
                xk = ordre(nk, g);

                suivnk = suiv(nk, g);


                /*
                Verifica se nk está em uma posição
                compatível com a reconexão.
                */
                if (((xj > xl) &&
                     (xk < xj) &&
                     (xk >= xl)) ||
                    ((xj < xl) &&
                     ((xk < xj) ||
                      (xk >= xl)))) {

                  /*
                  Calcula a variação de custo
                  da retirada tipo 2.
                  */
                  nouvdelta =
                      dtp[ni][nj] +
                      dtp[suivnk][precnl] +
                      dtp[ni2][nl] +
                      dtp[nk][suivnj] -
                      dtp[ni][x] -
                      dtp[x][ni2] -
                      dtp[precnl][nl] -
                      dtp[nk][suivnk] -
                      dtp[nj][suivnj];


                  /*
                  Calcula os custos de inversão
                  entre nj e suivnk.
                  */
                  som1 = nj;

                  while (som1 != suivnk) {

                    som2 = prec(som1, g);

                    nouvdelta =
                        nouvdelta -
                        dtp[som2][som1] +
                        dtp[som1][som2];

                    som1 = som2;
                  }


                  /*
                  Calcula os custos de inversão
                  entre precnl e ni2.
                  */
                  som1 = precnl;

                  while (som1 != ni2) {

                    som2 = prec(som1, g);

                    nouvdelta =
                        nouvdelta -
                        dtp[som2][som1] +
                        dtp[som1][som2];

                    som1 = som2;
                  }


                  delta =
                      mini(delta, nouvdelta);


                  /*
                  Armazena os dados da operação tipo 2.
                  */
                  if (delta == nouvdelta) {

                    noeuds.typeretrait = 2;

                    noeuds.i = ni;
                    noeuds.k = nk;
                    noeuds.j = nj;
                    noeuds.sj = suivnj;
                    noeuds.sk = suivnk;
                    noeuds.pl = precnl;
                    noeuds.i2 = ni2;
                    noeuds.l = nl;
                    noeuds.x = x;
                  }
                }
              }
            }
          }
        }
      }
    }


    /******** PROCURA DOS TIPOS DE RETIRADA 3 E 4 ********/

    /*
    Esta segunda parte utiliza predominantemente
    as vizinhanças de entrada p2.
    */
    for (j = 1; j <= xk1; j++) {

      /*
      Seleciona um possível predecessor de ni2.
      */
      nj = neigh->p2[ni2].nn[j];

      if (nj < 0)
        nj = 0;


      if (nj != x) {

        /* Obtém sucessor, predecessor e posição de nj. */
        suivnj = suiv(nj, g);

        precnj = prec(nj, g);

        xj = ordre(nj, g);


        /*
        Analisa possíveis nós nl relacionados
        à vizinhança de entrada de ni.
        */
        for (l = 1; l <= xk1; l++) {

          nl = neigh->p2[ni].nn[l];

          if (nl < 0)
            nl = 0;


          /* Obtém sucessor, predecessor e posição de nl. */
          suivnl = suiv(nl, g);

          precnl = prec(nl, g);

          xl = ordre(nl, g);


          /*
          Evita nós que participam diretamente
          da retirada ou que causariam duplicidade.
          */
          if (!((nl == ni2) ||
                (nl == nj) ||
                (nl == x))) {


            /*
            Verifica a posição relativa dos nós.
            */
            if (((xi2 > xj) &&
                 ((xl > xi2) || (xl < xj))) ||
                ((xi2 < xj) &&
                 (xl > xi2) &&
                 (xl < xj))) {

              /******** TIPO DE RETIRADA 3 ********/

              /*
              Calcula a variação de custo
              da operação tipo 3.
              */
              nouvdelta =
                  dtp[nj][ni2] +
                  dtp[nl][ni] +
                  dtp[precnl][precnj] -
                  dtp[ni][x] -
                  dtp[x][ni2] -
                  dtp[precnj][nj] -
                  dtp[precnl][nl];


              /*
              Inclui o efeito das inversões de segmentos.
              */
              som1 = ni;

              while (som1 != nj) {

                som2 = prec(som1, g);

                nouvdelta =
                    nouvdelta -
                    dtp[som2][som1] +
                    dtp[som1][som2];

                som1 = som2;
              }


              som1 = precnj;

              while (som1 != nl) {

                som2 = prec(som1, g);

                nouvdelta =
                    nouvdelta -
                    dtp[som2][som1] +
                    dtp[som1][som2];

                som1 = som2;
              }


              delta =
                  mini(delta, nouvdelta);


              /*
              Armazena os dados da operação tipo 3.
              */
              if (delta == nouvdelta) {

                noeuds.typeretrait = 3;

                noeuds.i2 = ni2;
                noeuds.j = nj;
                noeuds.i = ni;
                noeuds.l = nl;
                noeuds.pj = precnj;
                noeuds.pl = precnl;
                noeuds.x = x;
              }

            } else {

              /******** PROCURA DE UMA RETIRADA TIPO 4 ********/

              /*
              Analisa possíveis nós nk da vizinhança
              de saída do predecessor de nj.
              */
              for (k = 1; k <= xk1; k++) {

                nk =
                    neigh->p1[precnj].nn[k];

                if (nk < 0)
                  nk = 0;


                /* Obtém sucessor, predecessor e posição de nk. */
                suivnk = suiv(nk, g);

                precnk = prec(nk, g);

                xk = ordre(nk, g);


                /*
                Verifica se nk pode participar
                da reconexão tipo 4.
                */
                if (((xl > xj) &&
                     (xk > xj) &&
                     (xk <= xl)) ||
                    ((xl < xj) &&
                     ((xk > xj) ||
                      (xk <= xl)))) {

                  /*
                  Calcula a variação de custo
                  da retirada tipo 4.
                  */
                  nouvdelta =
                      dtp[nj][ni2] +
                      dtp[suivnl][precnk] +
                      dtp[nl][ni] +
                      dtp[precnj][nk] -
                      dtp[ni][x] -
                      dtp[x][ni2] -
                      dtp[nl][suivnl] -
                      dtp[precnk][nk] -
                      dtp[precnj][nj];


                  /*
                  Inclui os custos das inversões.
                  */
                  som1 = ni;

                  while (som1 != suivnl) {

                    som2 = prec(som1, g);

                    nouvdelta =
                        nouvdelta -
                        dtp[som2][som1] +
                        dtp[som1][som2];

                    som1 = som2;
                  }


                  som1 = precnk;

                  while (som1 != nj) {

                    som2 = prec(som1, g);

                    nouvdelta =
                        nouvdelta -
                        dtp[som2][som1] +
                        dtp[som1][som2];

                    som1 = som2;
                  }


                  delta =
                      mini(delta, nouvdelta);


                  /*
                  Armazena os dados da operação tipo 4.
                  */
                  if (delta == nouvdelta) {

                    noeuds.typeretrait = 4;

                    noeuds.i2 = ni2;
                    noeuds.j = nj;
                    noeuds.pk = precnk;
                    noeuds.sl = suivnl;
                    noeuds.i = ni;
                    noeuds.l = nl;
                    noeuds.k = nk;
                    noeuds.pj = precnj;
                    noeuds.x = x;
                  }
                }
              }
            }
          }
        }
      }
    }
  }

  /*
  A continuação da função atribui delta a flag->mdelta
  e executa efetivamente a retirada quando flag->modifie
  for diferente de zero.
  */

/*
Executa efetivamente a retirada do nó x da sequência.

Esta parte é semelhante à etapa final da função ajoutx:
- armazena a variação de custo;
- remove o nó da memória;
- atualiza a quantidade de nós;
- reconecta os segmentos da sequência;
- renumera os nós.
*/

/* Armazena a melhor variação de custo encontrada para a retirada. */
flag->mdelta = delta;


/*
A retirada somente é executada quando flag->modifie
for diferente de zero.

Quando flag->modifie é zero, a função apenas avalia
o custo do movimento, sem modificar a sequência.
*/
if (flag->modifie) {

  /*
  Libera a memória ocupada pelo elemento da lista
  correspondente ao nó x.
  */
  free(g[x].ptrtourne);

  /*
  Esta verificação recebe sempre o valor 1 e, portanto,
  nunca identifica um erro na liberação de memória.
  Foi mantida porque pertence ao código original.
  */
  demand(1, problem in freeing memory);

  /* Reduz em uma unidade o número de nós da sequência. */
  t->nbredenoeuds--;

  /* Registra que o nó x não pertence mais à sequência. */
  t->noeudinterne[x] = 0;


  /*
  Executa a reconexão correspondente ao melhor
  tipo de retirada encontrado anteriormente.
  */
  switch (noeuds.typeretrait) {


  /*
  Retirada tipo 1:
  remove x e reconecta a sequência com duas inversões.
  */
  case 1:

    /* Liga diretamente o nó i ao nó j. */
    chemin(noeuds.i, noeuds.j, g);

    /* Inverte o segmento entre j e i2. */
    inverse(noeuds.j, noeuds.i2, g);

    /* Liga i2 ao nó l. */
    chemin(noeuds.i2, noeuds.l, g);

    /* Inverte o segmento entre l e o sucessor de j. */
    inverse(noeuds.l, noeuds.sj, g);

    /* Liga o sucessor de j ao sucessor de l. */
    chemin(noeuds.sj, noeuds.sl, g);

    break;


  /*
  Retirada tipo 2:
  remove x e realiza uma reconexão mais ampla,
  envolvendo os nós i, j, k, l e seus vizinhos.
  */
  case 2:

    /* Liga diretamente i ao nó j. */
    chemin(noeuds.i, noeuds.j, g);

    /* Inverte o segmento entre j e o sucessor de k. */
    inverse(noeuds.j, noeuds.sk, g);

    /* Liga o sucessor de k ao predecessor de l. */
    chemin(noeuds.sk, noeuds.pl, g);

    /* Inverte o segmento entre o predecessor de l e i2. */
    inverse(noeuds.pl, noeuds.i2, g);

    /* Liga i2 ao nó l. */
    chemin(noeuds.i2, noeuds.l, g);

    /* Liga o nó k ao sucessor de j. */
    chemin(noeuds.k, noeuds.sj, g);

    break;


  /*
  Retirada tipo 3:
  operação considerada inversa à retirada tipo 1.
  */
  case 3:

    /* Inverte o segmento entre i e j. */
    inverse(noeuds.i, noeuds.j, g);

    /* Liga j ao nó i2. */
    chemin(noeuds.j, noeuds.i2, g);

    /* Liga o predecessor de l ao predecessor de j. */
    chemin(noeuds.pl, noeuds.pj, g);

    /* Inverte o segmento entre o predecessor de j e l. */
    inverse(noeuds.pj, noeuds.l, g);

    /* Liga l ao nó i. */
    chemin(noeuds.l, noeuds.i, g);

    break;


  /*
  Retirada tipo 4:
  operação considerada inversa à retirada tipo 2.
  */
  case 4:

    /* Liga o predecessor de j ao nó k. */
    chemin(noeuds.pj, noeuds.k, g);

    /* Liga o nó l ao nó i. */
    chemin(noeuds.l, noeuds.i, g);

    /* Inverte o segmento entre i e o sucessor de l. */
    inverse(noeuds.i, noeuds.sl, g);

    /* Liga o sucessor de l ao predecessor de k. */
    chemin(noeuds.sl, noeuds.pk, g);

    /* Inverte o segmento entre o predecessor de k e j. */
    inverse(noeuds.pk, noeuds.j, g);

    /* Liga j ao nó i2. */
    chemin(noeuds.j, noeuds.i2, g);

    break;


  /*
  Retirada tipo 5:
  retirada simples do nó x.

  O predecessor de x é ligado diretamente ao sucessor de x.
  */
  case 5:

    chemin(noeuds.i, noeuds.j, g);

    break;
  }


  /*
  Atualiza as posições dos nós e verifica
  a consistência da sequência modificada.
  */
  numerote_tourne(t);

} /* Fim da execução efetiva da retirada */

} /* FIM DA FUNÇÃO OTERX */


/******************************************************************/

/*
Copia uma sequência de origem para uma sequência de destino.

ts: sequência de origem.
g: estrutura de coordenadas da origem.
td: sequência de destino.
g2: estrutura de coordenadas do destino.
ndepotc: identificador do depósito que será o início da cópia.

A função cria uma cópia independente da lista encadeada.
*/
copietourne(ts, g, td, g2, ndepotc)

coord g[], g2[];

tourne *ts, *td;

int ndepotc;

{
  int i;

  /* Ponteiro auxiliar utilizado para construir a cópia. */
  tourneelem *pt;


  /*
  Inicializa os dados da estrutura de destino.
  */
  for (i = 0; i <= MAXN; i++) {

    /*
    Copia a máquina à qual cada nó está associado.
    */
    g2[i].satourne = g[i].satourne;

    /*
    Inicializa o ponteiro do nó de destino como nulo.
    */
    g2[i].ptrtourne = NULL;

    /*
    Copia o indicador que informa se o nó
    pertence à sequência.
    */
    td->noeudinterne[i] =
        ts->noeudinterne[i];
  }


  /*
  Reserva memória para cada possível nó
  da instância.
  */
  for (i = 1; i <= (n + np); i++) {

    if (g2[i].ptrtourne == NULL) {

      g2[i].ptrtourne =
          (tourneelem *) malloc(
              sizeof(tourneelem)
          );

      /*
      Encerra o programa caso a alocação falhe.
      */
      demand(
          g2[i].ptrtourne,
          memory overflow
      );
    }
  }


  /*
  Reconstrói os ponteiros de sucessor e predecessor
  na nova estrutura.
  */
  for (i = 1; i <= (n + np); i++) {

    /* Obtém o elemento correspondente ao nó i. */
    pt = g2[i].ptrtourne;

    /* Armazena o identificador do nó. */
    pt->noeud = i;

    /*
    O sucessor do nó i na cópia será o elemento
    correspondente ao sucessor de i na estrutura original.
    */
    pt->prochain =
        g2[
            g[i].ptrtourne->prochain->noeud
        ].ptrtourne;

    /*
    O predecessor do nó i na cópia será o elemento
    correspondente ao predecessor de i na estrutura original.
    */
    pt->precedent =
        g2[
            g[i].ptrtourne->precedent->noeud
        ].ptrtourne;
  }


  /*
  Copia o número de nós da sequência.
  */
  td->nbredenoeuds =
      ts->nbredenoeuds;


  /*
  Define o depósito informado como início
  da sequência copiada.
  */
  td->ptr =
      g2[ndepotc].ptrtourne;


  /*
  Atualiza as posições dos nós e verifica
  a consistência da sequência copiada.
  */
  numerote_tourne(td);

} /* FIM DA FUNÇÃO COPIETOURNE */


/********************************************************************/

/*
Armazena uma cópia completa da melhor solução encontrada.

A função copia:
- a atribuição das tarefas;
- as sequências de todas as máquinas;
- as cargas das máquinas;
- o makespan;
- as estruturas de vizinhança.

g e depot representam a solução atual.
bg e bsolution representam a melhor solução armazenada.
*/
store_best_solution(
    g,
    depot,
    bg,
    bsolution,
    makespan,
    neigh,
    bneigh
)

coord g[], bg[];

tdepot *depot, *bsolution;

makes *makespan;

neighbour neigh[], bneigh[];

{
  /*
  i1 e j1 são contadores.
  maxmake armazena a maior carga encontrada.
  */
  int i1, j1, maxmake;

  /* Ponteiro auxiliar utilizado para criar as cópias. */
  tourneelem *pt;


  /*
  Inicializa a estrutura da melhor solução
  e copia as informações de atribuição.
  */
  for (i1 = 0; i1 <= MAXN; i1++) {

    /*
    Copia a máquina à qual cada nó pertence.
    */
    bg[i1].satourne =
        g[i1].satourne;

    /*
    Inicializa o ponteiro da cópia como nulo.
    */
    bg[i1].ptrtourne = NULL;


    /*
    Copia, para cada máquina, o vetor que informa
    quais nós pertencem à sua sequência.
    */
    for (j1 = 1; j1 <= np; j1++)

      bsolution->tabt[j1]
               .noeudinterne[i1] =

          depot->tabt[j1]
               .noeudinterne[i1];
  }


  /*
  Reserva memória para os elementos da lista
  que representarão a melhor solução.
  */
  for (i1 = 1; i1 <= (n + np); i1++) {

    if (bg[i1].ptrtourne == NULL) {

      bg[i1].ptrtourne =
          (tourneelem *) malloc(
              sizeof(tourneelem)
          );

      /*
      Verifica se a alocação de memória foi realizada.
      */
      demand(
          bg[i1].ptrtourne,
          memory overflow
      );
    }
  }


  /*
  Reconstrói os ponteiros de sucessor e predecessor
  da melhor solução.
  */
  for (i1 = 1; i1 <= (n + np); i1++) {

    /* Obtém o elemento da cópia associado ao nó i1. */
    pt = bg[i1].ptrtourne;

    /* Armazena o identificador do nó. */
    pt->noeud = i1;

    /*
    Copia a relação de sucessão da solução atual.
    */
    pt->prochain =
        bg[
            g[i1].ptrtourne->prochain->noeud
        ].ptrtourne;

    /*
    Copia a relação de precedência da solução atual.
    */
    pt->precedent =
        bg[
            g[i1].ptrtourne->precedent->noeud
        ].ptrtourne;
  }


  /*
  Inicializa a maior carga encontrada.
  */
  maxmake = 0;


  /*
  Copia as informações de cada máquina
  e determina o makespan da solução.
  */
  for (i1 = 1; i1 <= np; i1++) {

    /*
    Copia a quantidade de nós da sequência.
    */
    bsolution->tabt[i1].nbredenoeuds =
        depot->tabt[i1].nbredenoeuds;

    /*
    Define o depósito da máquina como início
    da sequência copiada.
    */
    bsolution->tabt[i1].ptr =
        bg[n + i1].ptrtourne;

    /*
    Copia o identificador do depósito.
    */
    bsolution->nodepot[i1] =
        depot->nodepot[i1];

    /*
    Copia a carga da máquina.
    */
    bsolution->load[i1] =
        depot->load[i1];


    /*
    Verifica se a máquina atual possui carga
    maior ou igual à maior carga já encontrada.
    */
    if (bsolution->load[i1] >= maxmake) {

      /* Atualiza a maior carga. */
      maxmake =
          bsolution->load[i1];

      /*
      Armazena o valor do makespan.
      */
      makespan->load =
          maxmake;

      /*
      Armazena a máquina responsável pelo makespan.
      */
      makespan->proc =
          i1;
    }


    /*
    Atualiza as posições dos nós e verifica
    a consistência da sequência copiada.
    */
    numerote_tourne(
        &bsolution->tabt[i1]
    );
  }


  /*
  Copia as estruturas de vizinhança
  de todas as máquinas e nós.
  */
  for (i1 = 1; i1 <= np; i1++) {

    for (j1 = 1;
         j1 <= (n + np);
         j1++) {

      /*
      Copia a vizinhança p1.
      */
      bneigh[i1].p1[j1] =
          neigh[i1].p1[j1];

      /*
      Linha de depuração desativada.
      */
      // printf(
      //     "\nbneigh[%i].p1[%i]==%d",
      //     i1,
      //     j1,
      //     bneigh[i1].p1[j1]
      // );


      /*
      Copia a vizinhança p2.
      */
      bneigh[i1].p2[j1] =
          neigh[i1].p2[j1];

      /*
      Linha de depuração desativada.
      */
      // printf(
      //     "\nbneigh[%i].p2[%i]==%d",
      //     i1,
      //     j1,
      //     bneigh[i1].p2[j1]
      // );
    }
  }

} /* FIM DA FUNÇÃO STORE_BEST_SOLUTION */
  
/********************************************************************/

/*
Inicializa o gerador de números pseudoaleatórios utilizado
pela função drand48().
*/
seed_generation()

{
  /*
  Ponteiro para uma estrutura que armazena informações
  sobre a data e a hora atuais.
  */
  struct tm *tempo;

  /* Armazena o tempo atual do sistema em segundos. */
  long lt;

  /* Declara a função que gera números pseudoaleatórios entre 0 e 1. */
  double drand48();

  /*
  Declaração da função log().
  Essa função não é utilizada dentro de seed_generation().
  */
  double log();

  /*
  Armazena o valor calculado para inicializar
  o gerador de números pseudoaleatórios.
  */
  int seed;


  /* Obtém o tempo atual do sistema. */
  time(&lt);

  /*
  Converte o tempo obtido para uma estrutura
  contendo segundos, minutos, horas, dia, mês e ano.
  */
  tempo = localtime(&lt);


  /*
  Calcula uma semente com base na data e na hora atuais.

  São utilizados:
  - segundos;
  - minutos;
  - horas;
  - dia do mês;
  - mês;
  - ano;
  - dia do ano.
  */
  seed =
      (tempo->tm_sec +
       tempo->tm_min +
       tempo->tm_hour +
       tempo->tm_mday +
       tempo->tm_mon +
       tempo->tm_year +
       tempo->tm_yday) * 132;


  /*
  Inicializa o gerador com a semente definida para a execucao atual.
  No modo batch, as execucoes usam sementes 0, 1, 2, ...;
  no modo normal, a semente permanece zero, preservando o comportamento anterior.
  */
  srand48(seed_execucao_atual);


  /*
  Linha desativada que inicializaria o gerador com a semente
  calculada a partir da data e da hora.

  Se esta linha fosse utilizada no lugar de srand48(0),
  cada execução poderia produzir uma sequência aleatória diferente.
  */
  // srand48(seed);

} /* FIM DA FUNÇÃO SEED_GENERATION */

/********************************************************************/
