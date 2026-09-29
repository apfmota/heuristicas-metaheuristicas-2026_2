#include "../release_utils.h"

int main(int argc, char **argv) {
    InstanciaBase I;
    SolucaoRef S;
    int *inicio = NULL;
    int *r = NULL;
    int cmax = 0;
    int j;

    if (argc != 4) {
        fprintf(stderr, "Uso: %s <instancia_base> <solucao_referencia> <arquivo_saida>\n", argv[0]);
        return 1;
    }
    if (ler_instancia_base(argv[1], &I) != 0) falha("nao foi possivel ler a instancia-base.");
    if (ler_solucao(argv[2], I.m, I.n, &S) != 0) { liberar_instancia(&I); falha("nao foi possivel ler a solucao de referencia."); }

    inicio = (int*)malloc((size_t)I.n * sizeof(int));
    r = (int*)malloc((size_t)I.n * sizeof(int));
    if (!inicio || !r) falha("memoria insuficiente.");

    if (calcular_referencia(&I, &S, inicio, &cmax) != 0) falha("erro ao calcular os instantes de referencia.");
    for (j = 0; j < I.n; j++) r[j] = inicio[j];

    if (copiar_base_e_escrever_release(argv[1], argv[3], r, I.n) != 0) falha("erro ao gravar a instancia com release facil.");

    printf("Gerada: %s\n", argv[3]);
    printf("Cmax de referencia: %d\n", cmax);
    printf("Regra facil: r_j = instante de referencia do schedule sem release.\n");
    printf("p_j e s_ij foram preservados; somente os %d valores r_j foram anexados em uma unica linha.\n", I.n);

    free(inicio); free(r);
    liberar_solucao(&S);
    liberar_instancia(&I);
    return 0;
}
