#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>

#define MAX_NOS 1000
#define QTD_INSTANCIAS 10
#define VALOR_SUPERIOR 100
#define VALOR_PROIBIDO 500
#define SEMENTE_BASE_STRUCT 12345L
#define SEMENTE_BASE_NONSTRUCT 54321L

static int d[MAX_NOS][MAX_NOS];
static int tproc[MAX_NOS];

static void criar_pasta(const char *path) {
    if (mkdir(path, 0777) != 0 && errno != EEXIST) {
        perror(path);
        exit(1);
    }
}

static int valor_aleatorio(void) {
    double x = drand48() * (VALOR_SUPERIOR - 1) + 1;
    return (int)x; /* 1..99 quando VALOR_SUPERIOR=100 */
}

static int distancia_arredondada(double x1, double y1, double x2, double y2) {
    double dx = x1 - x2;
    double dy = y1 - y2;
    return (int)(sqrt(dx * dx + dy * dy) + 0.5);
}

static void escrever_instancia(const char *arquivo, int m, int n) {
    FILE *f;
    int i, j, total = n + m;
    f = fopen(arquivo, "w");
    if (!f) {
        perror(arquivo);
        exit(1);
    }
    fprintf(f, "%d\n%d\n", m, n);
    for (i = 1; i <= total; i++) fprintf(f, "%d\n", tproc[i]);
    for (i = 1; i <= total; i++)
        for (j = 1; j <= total; j++)
            fprintf(f, "%d\n", d[i][j]);
    fclose(f);
}

static void gerar_estruturada(int m, int n, int inst, const char *dir, FILE *manifesto) {
    double p1x[MAX_NOS], p1y[MAX_NOS], p2x[MAX_NOS], p2y[MAX_NOS];
    double q1x[MAX_NOS], q1y[MAX_NOS], q2x[MAX_NOS], q2y[MAX_NOS];
    int j, l, total = n + m;
    long seed = SEMENTE_BASE_STRUCT + inst + m * 1000L + n * 10L;
    char arquivo[512];

    srand48(seed);

    for (j = 1; j <= n; j++) tproc[j] = valor_aleatorio();
    for (j = n + 1; j <= total; j++) tproc[j] = 0;

    /* Mesma estrutura do gerador configuravel baseado no codigo do professor:
       dois conjuntos direcionais P/Q e um primeiro no ficticio de referencia. */
    for (j = 1; j <= n + 1; j++) {
        p1x[j] = drand48() * 99.0 + 1.0;
        p2x[j] = drand48() * 99.0 + 1.0;
        q1x[j] = drand48() * 99.0 + 1.0;
        q2x[j] = drand48() * 99.0 + 1.0;
        p1y[j] = drand48() * 99.0 + 1.0;
        p2y[j] = drand48() * 99.0 + 1.0;
        q1y[j] = drand48() * 99.0 + 1.0;
        q2y[j] = drand48() * 99.0 + 1.0;
    }
    for (j = n + 2; j <= total; j++) {
        p1x[j] = p1x[n + 1]; p2x[j] = p2x[n + 1];
        q1x[j] = q1x[n + 1]; q2x[j] = q2x[n + 1];
        p1y[j] = p1y[n + 1]; p2y[j] = p2y[n + 1];
        q1y[j] = q1y[n + 1]; q2y[j] = q2y[n + 1];
    }

    for (j = 1; j <= total; j++) {
        d[j][j] = VALOR_PROIBIDO;
        for (l = j + 1; l <= total; l++) {
            if (j >= n + 1 && l >= n + 1) {
                d[j][l] = VALOR_PROIBIDO;
                d[l][j] = VALOR_PROIBIDO;
            } else {
                d[j][l] = distancia_arredondada(p2x[j], p2y[j], p1x[l], p1y[l]);
                d[l][j] = distancia_arredondada(q2x[j], q2y[j], q1x[l], q1y[l]);
            }
        }
    }

    snprintf(arquivo, sizeof(arquivo), "%s/dados_trab_%d_%d_struc_%02d", dir, m, n, inst);
    escrever_instancia(arquivo, m, n);
    fprintf(manifesto, "%s;estruturada;%d;%d;%d;%ld\n", arquivo, m, n, inst, seed);
}

static void gerar_nao_estruturada(int m, int n, int inst, const char *dir, FILE *manifesto) {
    int j, l, total = n + m;
    long seed = SEMENTE_BASE_NONSTRUCT + inst + m * 1000L + n * 10L;
    char arquivo[512];

    srand48(seed);

    for (j = 1; j <= n; j++) tproc[j] = valor_aleatorio();
    for (j = n + 1; j <= total; j++) tproc[j] = 0;

    for (j = 1; j <= total; j++) {
        d[j][j] = VALOR_PROIBIDO;
        for (l = j + 1; l <= total; l++) {
            if (j >= n + 1 && l >= n + 1) {
                d[j][l] = VALOR_PROIBIDO;
                d[l][j] = VALOR_PROIBIDO;
            } else {
                d[j][l] = valor_aleatorio();
                d[l][j] = valor_aleatorio();
            }
        }
    }

    /* Mantem a convencao do gerador configuravel derivado do professor:
       todos os nos ficticios usam os mesmos custos do primeiro no ficticio. */
    for (j = 1; j <= n; j++) {
        for (l = n + 2; l <= total; l++) {
            d[j][l] = d[j][n + 1];
            d[l][j] = d[n + 1][j];
        }
    }

    snprintf(arquivo, sizeof(arquivo), "%s/dados_trab_%d_%d_nonstruc_%02d", dir, m, n, inst);
    escrever_instancia(arquivo, m, n);
    fprintf(manifesto, "%s;nao_estruturada;%d;%d;%d;%ld\n", arquivo, m, n, inst, seed);
}

int main(int argc, char **argv) {
    const int ns[] = {20, 30, 40, 50};
    const int ms[] = {2, 3, 4, 5};
    const char *raiz = (argc > 1) ? argv[1] : "../03_Instancias_Geradas";
    char dir_struct[512], dir_nonstruct[512], manifesto_path[512];
    FILE *manifesto;
    int im, in, inst;

    snprintf(dir_struct, sizeof(dir_struct), "%s/01_Estruturadas", raiz);
    snprintf(dir_nonstruct, sizeof(dir_nonstruct), "%s/02_Nao_Estruturadas", raiz);
    criar_pasta(raiz);
    criar_pasta(dir_struct);
    criar_pasta(dir_nonstruct);

    snprintf(manifesto_path, sizeof(manifesto_path), "%s/manifesto_instancias.csv", raiz);
    manifesto = fopen(manifesto_path, "w");
    if (!manifesto) { perror(manifesto_path); return 1; }
    fprintf(manifesto, "arquivo;tipo;maquinas;tarefas;instancia;semente\n");

    for (in = 0; in < 4; in++) {
        for (im = 0; im < 4; im++) {
            for (inst = 1; inst <= QTD_INSTANCIAS; inst++) {
                gerar_estruturada(ms[im], ns[in], inst, dir_struct, manifesto);
                gerar_nao_estruturada(ms[im], ns[in], inst, dir_nonstruct, manifesto);
            }
        }
    }

    fclose(manifesto);
    printf("\nGeracao concluida.\n");
    printf("Grade: n={20,30,40,50}; m={2,3,4,5}; 10 instancias por combinacao e por tipo.\n");
    printf("Total esperado: 320 instancias (160 estruturadas + 160 nao estruturadas).\n");
    printf("Saida: %s\n", raiz);
    return 0;
}
