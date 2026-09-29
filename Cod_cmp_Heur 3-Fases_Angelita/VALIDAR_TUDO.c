#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <limits.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

static int parse_nome(const char *nome, int *m, int *n, char *tipo, int *inst) {
    int pos=0; char t[32];
    if (sscanf(nome,"dados_trab_%d_%d_%31[^_]_%d%n",m,n,t,inst,&pos)!=4) return 0;
    if (nome[pos]!='\0') return 0;
    if (strcmp(t,"struc") && strcmp(t,"nonstruc")) return 0;
    strcpy(tipo,t); return 1;
}

static int validar_par(const char *base, const char *rel, int n, int m) {
    FILE *a=fopen(base,"r"), *b=fopen(rel,"r");
    int i, va, vb, total=n+m, base_count=2+total+total*total, extra;
    if (!a || !b) { if(a)fclose(a); if(b)fclose(b); return 0; }
    for(i=0;i<base_count;i++) {
        if(fscanf(a,"%d",&va)!=1 || fscanf(b,"%d",&vb)!=1 || va!=vb) { fclose(a); fclose(b); return 0; }
    }
    if(fscanf(a,"%d",&extra)==1) { fclose(a); fclose(b); return 0; }
    for(i=0;i<n;i++) if(fscanf(b,"%d",&vb)!=1) { fclose(a); fclose(b); return 0; }
    if(fscanf(b,"%d",&extra)==1) { fclose(a); fclose(b); return 0; }
    fclose(a); fclose(b); return 1;
}

static int contar_e_validar(const char *root, int *count) {
    const char *subs[2]={"01_Estruturadas","02_Nao_Estruturadas"};
    int s;
    *count=0;
    for(s=0;s<2;s++) {
        char dir[PATH_MAX]; DIR *d; struct dirent *e;
        snprintf(dir,sizeof(dir),"%s/01_Sem_Release/03_Instancias_Geradas/%s",root,subs[s]);
        d=opendir(dir); if(!d) return 0;
        while((e=readdir(d))!=NULL) {
            int m,n,inst; char tipo[32];
            char base[PATH_MAX],fac[PATH_MAX],dif[PATH_MAX];
            if(!parse_nome(e->d_name,&m,&n,tipo,&inst)) continue;
            snprintf(base,sizeof(base),"%s/%s",dir,e->d_name);
            snprintf(fac,sizeof(fac),"%s/02_Com_Release_Facil/03_Instancias_Geradas/%s/dados_trab_%d_%d_%s_release_facil_%02d",root,subs[s],m,n,tipo,inst);
            snprintf(dif,sizeof(dif),"%s/03_Com_Release_Dificil/03_Instancias_Geradas/%s/dados_trab_%d_%d_%s_release_dificil_%02d",root,subs[s],m,n,tipo,inst);
            if(!validar_par(base,fac,n,m)) { fprintf(stderr,"ERRO na versao facil de %s\n",e->d_name); closedir(d); return 0; }
            if(!validar_par(base,dif,n,m)) { fprintf(stderr,"ERRO na versao dificil de %s\n",e->d_name); closedir(d); return 0; }
            (*count)++;
        }
        closedir(d);
    }
    return 1;
}

int main(int argc,char **argv) {
    int nbase=0;
    const char *root = argc>1 ? argv[1] : ".";
    if(!contar_e_validar(root,&nbase)) return 1;
    printf("Sem release : %d\n",nbase);
    printf("Release facil: %d\n",nbase);
    printf("Release dificil: %d\n",nbase);
    if(nbase!=320) { fprintf(stderr,"ERRO: esperado 320 arquivos em cada familia.\n"); return 2; }
    printf("VALIDACAO OK: as 3 familias estao completas; p_j e s_ij foram preservados; cada arquivo com release possui exatamente n valores r_j.\n");
    return 0;
}
