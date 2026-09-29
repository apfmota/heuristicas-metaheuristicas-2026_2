#include "../release_utils.h"

int main(int argc, char **argv) {
    const char *base_root, *solutions_root, *out_root;
    double ratio = 0.20;
    int every = 4;
    BaseItem *bases = NULL;
    int nbases = 0, k;
    char out_struc[PATH_MAX], out_non[PATH_MAX], manifest_path[PATH_MAX];
    FILE *manifest;

    if (argc < 4 || argc > 6) {
        fprintf(stderr, "Uso: %s <pasta_bases> <pasta_solucoes_referencia> <pasta_saida> [delay_ratio] [every]\n", argv[0]);
        return 1;
    }
    base_root = argv[1]; solutions_root = argv[2]; out_root = argv[3];
    if (argc >= 5) ratio = atof(argv[4]);
    if (argc >= 6) every = atoi(argv[5]);
    if (ratio <= 0.0 || every < 0) falha("delay_ratio deve ser positivo e every deve ser >= 0.");

    if (!pasta_existe(base_root)) falha("pasta de instancias-base inexistente.");
    if (!pasta_existe(solutions_root)) falha("pasta de solucoes de referencia inexistente.");
    if (criar_pasta(out_root) != 0) falha("nao foi possivel criar pasta de saida.");

    snprintf(out_struc, sizeof(out_struc), "%s/01_Estruturadas", out_root);
    snprintf(out_non, sizeof(out_non), "%s/02_Nao_Estruturadas", out_root);
    if (criar_pasta(out_struc) != 0 || criar_pasta(out_non) != 0) falha("nao foi possivel criar subpastas de saida.");
    limpar_arquivos_dados(out_struc);
    limpar_arquivos_dados(out_non);

    if (listar_bases(base_root, &bases, &nbases) != 0 || nbases == 0) falha("nenhuma instancia-base encontrada.");

    snprintf(manifest_path, sizeof(manifest_path), "%s/manifesto_release_dificil.csv", out_root);
    manifest = fopen(manifest_path, "w");
    if (!manifest) falha("nao foi possivel criar o manifesto.");
    fprintf(manifest, "base;tipo;m;n;instancia;cmax_referencia;arquivo_saida;atraso;tarefas_afetadas;regra\n");

    for (k = 0; k < nbases; k++) {
        InstanciaBase I;
        SolucaoRef S;
        int *inicio, *r, *afetada;
        int cmax, atraso, maq, pos, j;
        char sol[PATH_MAX], out[PATH_MAX], nome_out[512];
        const char *subout = strcmp(bases[k].tipo, "struc") == 0 ? out_struc : out_non;

        if (ler_instancia_base(bases[k].path, &I) != 0) falha("erro ao ler uma instancia-base.");
        if (!localizar_solucao(solutions_root, bases[k].name, sol, sizeof(sol))) {
            fprintf(stderr, "ERRO: solucao de referencia nao encontrada para %s\n", bases[k].name);
            return 2;
        }
        if (ler_solucao(sol, I.m, I.n, &S) != 0) falha("erro ao ler uma solucao de referencia.");

        inicio = (int*)malloc((size_t)I.n * sizeof(int));
        r = (int*)malloc((size_t)I.n * sizeof(int));
        afetada = (int*)calloc((size_t)I.n, sizeof(int));
        if (!inicio || !r || !afetada) falha("memoria insuficiente.");
        if (calcular_referencia(&I, &S, inicio, &cmax) != 0) falha("erro ao calcular referencia.");

        for (j = 0; j < I.n; j++) r[j] = inicio[j];
        atraso = (int)ceil((double)cmax * ratio);
        if (atraso < 1) atraso = 1;
        for (maq = 0; maq < I.m; maq++) {
            for (pos = 0; pos < S.len[maq]; pos++) {
                int tarefa = S.seq[maq][pos];
                if (pos == 0 || (every > 0 && pos % every == 0)) {
                    r[tarefa] = inicio[tarefa] + atraso;
                    afetada[tarefa] = 1;
                }
            }
        }

        snprintf(nome_out, sizeof(nome_out), "dados_trab_%d_%d_%s_release_dificil_%02d", I.m, I.n, bases[k].tipo, bases[k].inst);
        snprintf(out, sizeof(out), "%s/%s", subout, nome_out);
        if (copiar_base_e_escrever_release(bases[k].path, out, r, I.n) != 0) falha("erro ao escrever instancia com release dificil.");

        fprintf(manifest, "%s;%s;%d;%d;%d;%d;%s;%d;", bases[k].name, bases[k].tipo, I.m, I.n, bases[k].inst, cmax, out, atraso);
        {
            int first = 1;
            for (j = 0; j < I.n; j++) if (afetada[j]) {
                if (!first) fputc(',', manifest);
                fprintf(manifest, "%d", j);
                first = 0;
            }
        }
        fprintf(manifest, ";referencia + %.3f*Cmax em posicoes selecionadas\n", ratio);

        free(inicio); free(r); free(afetada);
        liberar_solucao(&S);
        liberar_instancia(&I);
        if ((k + 1) % 50 == 0 || k + 1 == nbases) printf("Processadas %d/%d...\n", k + 1, nbases);
    }

    fclose(manifest);
    free(bases);
    printf("Concluido: %d instancias com release dificil.\n", nbases);
    printf("Manifesto: %s\n", manifest_path);
    printf("ATENCAO: delay_ratio e every sao parametros metodologicos configuraveis.\n");
    return 0;
}
