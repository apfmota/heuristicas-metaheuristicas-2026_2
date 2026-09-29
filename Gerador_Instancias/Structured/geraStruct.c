#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>


float lim1, lim2, lim3;
int maq,tar, i, j, k, l,seed, tas,d[400][400];

char saida[30];

FILE *sai;

int conver();
void convIntToStr(int n,int casas, char *string);
int MountNameFile(int grupo, int intIni, int intEnd, int n, int m, int instNumber);


int main(void) {
  struct tm *tempo;
  long lt;
  double drand48();
  double log();
  double sqrt();
  int grupo;
  float p1x[400];
  float p1y[400];
  float p2x[400];
  float p2y[400];
  float q1x[400];
  float q1y[400];
  float q2x[400];
  float q2y[400];

  float dista;


    maq = 2;
    tar = 100;
    lim3= 9;
    grupo=1;


  for (i = 1 ; i <= 1 ; i++) {
    /*  GERACAO DA SEMENTE A SER UTILIZADA NO PROCESSO  */
 /*
    time(&lt);
    tempo = localtime(&lt);
    seed = (tempo->tm_sec + tempo->tm_min + tempo->tm_hour +
            tempo->tm_mday + tempo->tm_mon + tempo->tm_year +
            tempo->tm_yday) * 132;
*/
    seed=(i+maq+tar+lim3+grupo);
    srand48(seed);
    lim1 = 1;
    lim2 = 100;
    MountNameFile(grupo, 1, lim3, maq, tar, i);
    printf("\nNum. de Processadores   = %d",maq);
    printf("\nNum. de Tarefas         = %d",tar);
    printf("\nNum. do Problema        = %d",i);
    printf("\nLim. do Tam. da Tarefa  = %d",(int)lim2);
    printf("\nFile name               = %s",saida);

  if ((sai = fopen(saida,"w")) == NULL) {
    printf("\nARQUIVO NAO PODE SER ABERTO \n ## FIM ##");
    exit(1);
  }

  fprintf(sai,"%d\n",maq);
  fprintf(sai,"%d\n",tar);

  for (j = 1 ; j <= tar ; j++) {
    /*tas = conver();*/
    tas = 0;
    fprintf(sai,"%d\n",tas);
  }
  for (l = (tar + 1) ; l <= (tar + maq) ; l++) {
    tas = 0;
    fprintf(sai,"%d\n",tas);
  }

 for (j = 1 ; j <= tar+1 ; j++) {
    p1x[j] = drand48() * (lim2-lim1) + lim1;
    p2x[j] = drand48() * (lim2-lim1) + lim1;
    q1x[j] = drand48() * (lim2-lim1) + lim1;
    q2x[j] = drand48() * (lim2-lim1) + lim1;
    p1y[j] = drand48() * (lim2-lim1) + lim1;
    p2y[j] = drand48() * (lim2-lim1) + lim1;
    q1y[j] = drand48() * (lim2-lim1) + lim1;
    q2y[j] = drand48() * (lim2-lim1) + lim1;
  }
  for (l = (tar + 2) ; l <= (tar + maq) ; l++) {
    p1x[l] = p1x[tar+1];
    p2x[j] = p2x[tar+1];
    q1x[j] = q1x[tar+1];
    q2x[j] = q2x[tar+1];
    p1y[l] = p1y[tar+1];
    p2y[j] = p2y[tar+1];
    q1y[j] = q1y[tar+1];
    q2y[j] = q2y[tar+1];
  }



  for (j = 1 ; j<= tar+maq ; j++) {
        d[j][j] = 500;
    for (l = j+1 ; l <= tar+maq ; l++) {

          dista = sqrt (((p2x[j]-p1x[l])*(p2x[j]-p1x[l]))+((p2y[j]-p1y[l])*(p2y[j]-p1y[l])))+.5;
          d[j][l] = (int) dista;
          dista = sqrt (((q2x[j]-q1x[l])*(q2x[j]-q1x[l]))+((q2y[j]-q1y[l])*(q2y[j]-q1y[l])))+.5;
          d[l][j] = (int) dista;
          if ((j >= (tar+1)) && (l >= (tar+1)))
          {
              d[j][l] = 500;
              d[l][j] = 500;
          }

    }
  }


  for (j = 1 ; j<= (tar + maq) ; j++) {
    for (l = 1 ; l <= (tar + maq) ; l++) {
      tas = d[j][l];
      fprintf(sai,"%d\n",tas);
    }
  }
  fclose(sai);
  printf("\n");
  }
} /*  FIM */


int conver() {
  float var1;
  var1 = drand48() * (lim2-lim1) + lim1;
  return((int)var1);
}

void convIntToStr(int n,int casas, char *string) {
  int temp=n;
  int i,nulas;
  int j;
  int aux;
  i=0;
  string[0]='\0';
  while (temp>0) {
    temp=temp/10;
    i++;
  }
  nulas=casas-i;
  if (nulas<0)nulas=0;

  for (j=0; j<nulas; j++) {
    string[j]=(char)48;
  }
  temp=n;
  j=nulas+i;
  string[j]='\0';
  j--;
  while (temp>0) {
    aux=temp/10;
    string[j]=(char)(temp-aux*10)+48;
    j--;
    temp=temp/10;
  }
}

int MountNameFile(int grupo,int intIni, int intEnd, int maq, int tar, int instNumber) {
  char *temp;
  temp=malloc(30 * sizeof(char));
  saida[0]='\0';
  strcat(saida,"s_g");
  convIntToStr(grupo,2,temp);
  strcat(saida,temp);
  strcat(saida,"_");
  convIntToStr(intIni,2,temp);
  strcat(saida,temp);
  strcat(saida,"_");
  convIntToStr(intEnd,4,temp);
  strcat(saida,temp);
  strcat(saida,"_");
  convIntToStr(tar,3,temp);
  strcat(saida,temp);
  strcat(saida,"_");
  convIntToStr(maq,2,temp);
  strcat(saida,temp);
  strcat(saida,"_");
  convIntToStr(instNumber,2,temp);
  strcat(saida,temp);
  return 0;
}

