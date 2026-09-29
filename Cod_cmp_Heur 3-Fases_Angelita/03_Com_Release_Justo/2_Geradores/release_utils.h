#ifndef RELEASE_UTILS_H
#define RELEASE_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <math.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

typedef struct {
    int m;
    int n;
    int total;
    int *p;
    int *setup;
} InstanciaBase;

typedef struct {
    int m;
    int n;
    int **seq;
    int *len;
} SolucaoRef;

static void falha(const char *msg) {
    fprintf(stderr, "ERRO: %s\n", msg);
    exit(EXIT_FAILURE);
}

static int arquivo_existe(const char *path) {
    struct stat st;
    return stat(path, &st) == 0 && S_ISREG(st.st_mode);
}

static int pasta_existe(const char *path) {
    struct stat st;
    return stat(path, &st) == 0 && S_ISDIR(st.st_mode);
}

static int criar_pasta(const char *path) {
    if (mkdir(path, 0777) == 0 || errno == EEXIST) return 0;
    return -1;
}

static int limpar_arquivos_dados(const char *dir) {
    DIR *d = opendir(dir);
    struct dirent *e;
    char p[PATH_MAX];
    if (!d) return -1;
    while ((e = readdir(d)) != NULL) {
        if (strncmp(e->d_name, "dados_trab_", 11) == 0) {
            snprintf(p, sizeof(p), "%s/%s", dir, e->d_name);
            remove(p);
        }
    }
    closedir(d);
    return 0;
}

static int parse_nome_base(const char *nome, int *m, int *n, char *tipo, int *inst) {
    int pos = 0;
    char t[32];
    if (sscanf(nome, "dados_trab_%d_%d_%31[^_]_%d%n", m, n, t, inst, &pos) != 4) return 0;
    if (nome[pos] != '\0') return 0;
    if (strcmp(t, "struc") != 0 && strcmp(t, "nonstruc") != 0) return 0;
    strcpy(tipo, t);
    return 1;
}

static void liberar_instancia(InstanciaBase *I) {
    free(I->p);
    free(I->setup);
    memset(I, 0, sizeof(*I));
}

static int ler_instancia_base(const char *path, InstanciaBase *I) {
    FILE *f = fopen(path, "r");
    int i;
    long extra;
    if (!f) return -1;
    memset(I, 0, sizeof(*I));
    if (fscanf(f, "%d", &I->m) != 1 || fscanf(f, "%d", &I->n) != 1) { fclose(f); return -2; }
    I->total = I->n + I->m;
    if (I->m <= 0 || I->n <= 0 || I->total <= 0) { fclose(f); return -3; }
    I->p = (int*)malloc((size_t)I->total * sizeof(int));
    I->setup = (int*)malloc((size_t)I->total * (size_t)I->total * sizeof(int));
    if (!I->p || !I->setup) { fclose(f); liberar_instancia(I); return -4; }
    for (i = 0; i < I->total; i++) if (fscanf(f, "%d", &I->p[i]) != 1) { fclose(f); liberar_instancia(I); return -5; }
    for (i = 0; i < I->total * I->total; i++) if (fscanf(f, "%d", &I->setup[i]) != 1) { fclose(f); liberar_instancia(I); return -6; }
    if (fscanf(f, "%ld", &extra) == 1) { fclose(f); liberar_instancia(I); return -7; }
    fclose(f);
    return 0;
}

static void liberar_solucao(SolucaoRef *S) {
    int k;
    if (S->seq) {
        for (k = 0; k < S->m; k++) free(S->seq[k]);
    }
    free(S->seq);
    free(S->len);
    memset(S, 0, sizeof(*S));
}

static int ler_solucao(const char *path, int m_esperado, int n_esperado, SolucaoRef *S) {
    FILE *f = fopen(path, "r");
    char linha[1024], nome[512], marca[64];
    int m, n, ordem, tarefa, maq, k, vistos = 0;
    int *used = NULL;
    if (!f) return -1;
    memset(S, 0, sizeof(*S));
    if (!fgets(linha, sizeof(linha), f)) { fclose(f); return -2; }
    marca[0] = '\0';
    if (sscanf(linha, "%511s %d %d %63s", nome, &m, &n, marca) < 3) { fclose(f); return -3; }
    if (m != m_esperado || n != n_esperado) { fclose(f); return -4; }
    S->m = m; S->n = n;
    S->seq = (int**)calloc((size_t)m, sizeof(int*));
    S->len = (int*)calloc((size_t)m, sizeof(int));
    used = (int*)calloc((size_t)n, sizeof(int));
    if (!S->seq || !S->len || !used) { fclose(f); free(used); liberar_solucao(S); return -5; }
    for (k = 0; k < m; k++) {
        int j;
        S->seq[k] = (int*)malloc((size_t)n * sizeof(int));
        if (!S->seq[k]) { fclose(f); free(used); liberar_solucao(S); return -5; }
        for (j = 0; j < n; j++) S->seq[k][j] = -1;
    }
    while (fgets(linha, sizeof(linha), f)) {
        if (sscanf(linha, "%d %d %d", &ordem, &tarefa, &maq) != 3) continue;
        if (ordem == -1 && tarefa == -1 && maq == -1) break;
        if (maq < 0 || maq >= m || tarefa < 0 || tarefa >= n || ordem < 0 || ordem >= n) {
            fclose(f); free(used); liberar_solucao(S); return -6;
        }
        if (used[tarefa]) { fclose(f); free(used); liberar_solucao(S); return -7; }
        if (S->seq[maq][ordem] != -1) { fclose(f); free(used); liberar_solucao(S); return -8; }
        S->seq[maq][ordem] = tarefa;
        if (ordem + 1 > S->len[maq]) S->len[maq] = ordem + 1;
        used[tarefa] = 1;
        vistos++;
    }
    fclose(f);
    if (vistos != n) { free(used); liberar_solucao(S); return -9; }
    for (k = 0; k < m; k++) {
        int j;
        for (j = 0; j < S->len[k]; j++) {
            if (S->seq[k][j] < 0) { free(used); liberar_solucao(S); return -10; }
        }
    }
    free(used);
    return 0;
}

static int calcular_referencia(const InstanciaBase *I, const SolucaoRef *S, int *inicio_ref, int *cmax_ref) {
    int maq, j, prev, tarefa, sij, tempo, cmax = 0;
    for (j = 0; j < I->n; j++) inicio_ref[j] = -1;
    for (maq = 0; maq < I->m; maq++) {
        int dep = I->n + maq;
        tempo = 0;
        prev = dep;
        for (j = 0; j < S->len[maq]; j++) {
            tarefa = S->seq[maq][j];
            inicio_ref[tarefa] = tempo;
            sij = I->setup[prev * I->total + tarefa];
            if (sij >= 500) return -1;
            tempo += sij + I->p[tarefa];
            prev = tarefa;
        }
        if (S->len[maq] > 0) {
            sij = I->setup[prev * I->total + dep];
            if (sij >= 500) return -2;
            tempo += sij;
        }
        if (tempo > cmax) cmax = tempo;
    }
    for (j = 0; j < I->n; j++) if (inicio_ref[j] < 0) return -3;
    *cmax_ref = cmax;
    return 0;
}

static int copiar_base_e_escrever_release(const char *base_path, const char *out_path, const int *r, int n) {
    FILE *in = fopen(base_path, "rb");
    FILE *out;
    char buf[8192];
    size_t lidos;
    long tam;
    int ultimo = '\n', j;
    if (!in) return -1;
    out = fopen(out_path, "wb");
    if (!out) { fclose(in); return -2; }
    while ((lidos = fread(buf, 1, sizeof(buf), in)) > 0) {
        if (fwrite(buf, 1, lidos, out) != lidos) { fclose(in); fclose(out); return -3; }
        ultimo = (unsigned char)buf[lidos - 1];
    }
    fclose(in);
    if (ultimo != '\n') fputc('\n', out);
    /* Vetor r_j em UMA unica linha, para ficar legivel. */
    for (j = 0; j < n; j++) {
        if (j) fputc(' ', out);
        fprintf(out, "%d", r[j]);
    }
    fputc('\n', out);
    tam = ftell(out);
    (void)tam;
    fclose(out);
    return 0;
}

static int find_file_recursive(const char *root, const char *target, char *found, size_t found_sz) {
    DIR *d = opendir(root);
    struct dirent *e;
    if (!d) return 0;
    while ((e = readdir(d)) != NULL) {
        char p[PATH_MAX];
        struct stat st;
        if (strcmp(e->d_name, ".") == 0 || strcmp(e->d_name, "..") == 0) continue;
        snprintf(p, sizeof(p), "%s/%s", root, e->d_name);
        if (stat(p, &st) != 0) continue;
        if (S_ISREG(st.st_mode) && strcmp(e->d_name, target) == 0) {
            snprintf(found, found_sz, "%s", p);
            closedir(d);
            return 1;
        }
        if (S_ISDIR(st.st_mode) && find_file_recursive(p, target, found, found_sz)) {
            closedir(d);
            return 1;
        }
    }
    closedir(d);
    return 0;
}

static int localizar_solucao(const char *solutions_root, const char *nome_base, char *found, size_t found_sz) {
    char target[PATH_MAX];
    snprintf(target, sizeof(target), "solucaoTabu.%s", nome_base);
    return find_file_recursive(solutions_root, target, found, found_sz);
}

typedef struct {
    char path[PATH_MAX];
    char name[512];
    char tipo[32];
    int m, n, inst;
} BaseItem;

static int cmp_baseitem(const void *a, const void *b) {
    const BaseItem *x = (const BaseItem*)a;
    const BaseItem *y = (const BaseItem*)b;
    return strcmp(x->name, y->name);
}

static int listar_bases(const char *base_root, BaseItem **out_items, int *out_count) {
    const char *subs[2] = {"01_Estruturadas", "02_Nao_Estruturadas"};
    BaseItem *items = NULL;
    int count = 0, cap = 0, si;
    for (si = 0; si < 2; si++) {
        char dir[PATH_MAX];
        DIR *d;
        struct dirent *e;
        snprintf(dir, sizeof(dir), "%s/%s", base_root, subs[si]);
        d = opendir(dir);
        if (!d) { free(items); return -1; }
        while ((e = readdir(d)) != NULL) {
            int m, n, inst;
            char tipo[32];
            char p[PATH_MAX];
            struct stat st;
            if (!parse_nome_base(e->d_name, &m, &n, tipo, &inst)) continue;
            snprintf(p, sizeof(p), "%s/%s", dir, e->d_name);
            if (stat(p, &st) != 0 || !S_ISREG(st.st_mode)) continue;
            if (count == cap) {
                int ncap = cap ? cap * 2 : 64;
                BaseItem *tmp = (BaseItem*)realloc(items, (size_t)ncap * sizeof(BaseItem));
                if (!tmp) { free(items); return -2; }
                items = tmp; cap = ncap;
            }
            snprintf(items[count].path, sizeof(items[count].path), "%s", p);
            snprintf(items[count].name, sizeof(items[count].name), "%s", e->d_name);
            snprintf(items[count].tipo, sizeof(items[count].tipo), "%s", tipo);
            items[count].m = m; items[count].n = n; items[count].inst = inst;
            count++;
        }
        closedir(d);
    }
    qsort(items, (size_t)count, sizeof(BaseItem), cmp_baseitem);
    *out_items = items;
    *out_count = count;
    return 0;
}

#endif
