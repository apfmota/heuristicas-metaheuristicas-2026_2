#include "../release_utils.h"

int main(int argc, char **argv) {
    InstanciaBase I;
    SolucaoRef S;
    int *inicio = NULL, *r = NULL, *afetada = NULL;
    int cmax = 0, atraso, every = 4, maq, pos, j, qtd = 0;
    double ratio = 0.20;

    if (argc < 4 || argc > 6) {
        fprintf(stderr, "Uso: %s <instancia_base> <solucao_referencia> <arquivo_saida> [delay_ratio] [every]\n", argv[0]);
        return 1;
    }
    if (argc >= 5) ratio = atof(argv[4]);
    if (argc >= 6) every = atoi(argv[5]);
    if (ratio <= 0.0 || every < 0) falha("delay_ratio deve ser positivo e every deve ser >= 0.");

    if (ler_instancia_base(argv[1], &I) != 0) falha("nao foi possivel ler a instancia-base.");
    if (ler_solucao(argv[2], I.m, I.n, &S) != 0) { liberar_instancia(&I); falha("nao foi possivel ler a solucao de referencia."); }

    inicio = (int*)malloc((size_t)I.n * sizeof(int));
    r = (int*)malloc((size_t)I.n * sizeof(int));
    afetada = (int*)calloc((size_t)I.n, sizeof(int));
    if (!inicio || !r || !afetada) falha("memoria insuficiente.");
    if (calcular_referencia(&I, &S, inicio, &cmax) != 0) falha("erro ao calcular os instantes de referencia.");

    for (j = 0; j < I.n; j++) r[j] = inicio[j];
    atraso = (int)ceil((double)cmax * ratio);
    if (atraso < 1) atraso = 1;

    for (maq = 0; maq < I.m; maq++) {
        for (pos = 0; pos < S.len[maq]; pos++) {
            int tarefa = S.seq[maq][pos];
            if (pos == 0 || (every > 0 && pos % every == 0)) {
                r[tarefa] = inicio[tarefa] + atraso;
                if (!afetada[tarefa]) { afetada[tarefa] = 1; qtd++; }
            }
        }
    }

    if (copiar_base_e_escrever_release(argv[1], argv[3], r, I.n) != 0) falha("erro ao gravar a instancia com release dificil.");

    printf("Gerada: %s\n", argv[3]);
    printf("Cmax de referencia: %d\n", cmax);
    printf("Atraso aplicado: %d (delay_ratio=%.3f)\n", atraso, ratio);
    printf("Tarefas afetadas (%d):", qtd);
    for (j = 0; j < I.n; j++) if (afetada[j]) printf(" %d", j);
    printf("\n");
    printf("p_j e s_ij foram preservados; somente os %d valores r_j foram anexados em uma unica linha.\n", I.n);

    free(inicio); free(r); free(afetada);
    liberar_solucao(&S);
    liberar_instancia(&I);
    return 0;
}
