#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <malloc.h>
#include <sys/types.h>
#include <sys/times.h>
#define sqr(qw) ((qw) * (qw))
/*#define MAXK 5          Neighboors' number     #define MINK 6 */
#define NBGRAND 5
#define MAXP 90        /* Maximum processors' number            */
#define MAXN 286       /* Maximum tasks' number plus processors */
#define MAXKN 30       /* Maximum neighboors' number            */
#define MAXDIST 30000  /* Distance between depots               */
#define MAXREAL 1.7e38 /* Maximum real value in the program     */
#define demand(fact,msg)   {\
        if (!fact) \
          {\
           printf("msg \n");\
           exit(1);\
          }\
       }

/********BEGINNING OF THE STRUCTURES DEFINITION**********/

typedef struct tourneelem tourneelem;
struct tourneelem{
       int noeud, rang;
       tourneelem *precedent, *prochain;
       };

typedef struct {
       int satourne;
       tourneelem *ptrtourne;
       } coord;
coord g[MAXN+1], bg[MAXN+1];

typedef struct {
       int noeudinterne[MAXN+1];
       tourneelem *ptr;
       int nbredenoeuds;
       } tourne;

typedef struct {
       int nn[MAXKN+1], leplusloin;
       float maxdist;
       } proxnoeud;
typedef struct {
       proxnoeud p1[MAXN+1], p2[MAXN+1];
       } neighbour;
neighbour neigh[MAXP+1];

typedef struct {
       tourne tabt[MAXP+1];
       int    nodepot[MAXP+1], load[MAXP+1];
       } tdepot;
tdepot depot, bsolution;

typedef struct {
       int modifie, numbdepot;
       int mdelta;
       } tflag;

typedef struct {
       int load, proc, iter;
       } makes;
makes makespan;

/*********BEGINNING OF THE FUNCTIONS DEFINITION*************/

int mini (a,b)
    int a, b;
   {
    if (a <= b)
      return(a);
    else return(b);
   }

/**********END OF MINI****************/

float min (a,b)
     float a, b;
    {
     if (a <= b)
       return(a);
     else return(b);
    }

/**********END OF MIN****************/

int tabu_time (lim1,lim2)
    int lim1,lim2;

   {
    float var;
    double drand48();

    var = ( 0.8* (lim2-lim1)) + lim1 + .5;
    var = (drand48() * (lim2-lim1)) + lim1 + .5;
    return((int)var);
   }

/**********END OF TABU_TIME****************/

int suiv (a,g)
    int a;
    coord g[];

   {
    return(g[a].ptrtourne->prochain->noeud);
   }

/**********END OF SUIV****************/

int prec (a,g)
    int a;
    coord g[];

   {
    return(g[a].ptrtourne->precedent->noeud);
   }

/**********END OF PREC****************/

int ordre (a,g)
    int a;
    coord g[];
   {
    return(g[a].ptrtourne->rang);
   }

/**********END OF ORDRE****************/

float calculcoutt (t,dtp)

      tourne *t;
      int dtp[MAXN+1][MAXN+1];

     {
      tourneelem *i, *j;

      int c;

      c = 0;
      j = t->ptr;
      i = t->ptr->prochain;
      do {
          c += dtp[j->noeud][i->noeud];
          j = i;
          i = i->prochain;
         } while ( j != t->ptr);
      return(c);
     }

/**********END OF CALCULCOUTT****************/

/**********BEGINNING OF THE GLOBAL VARIABLES DEFINITION********/

int i, j, k, w, n, ij, np, k1, lngtotal, cptlng,
    minmakespan, minpl, minpm, cneighin, cneighout,
    xio, vio, pio, pa, aspiration, maxload,lastimp,imptrue,
    lasttabu[MAXN+1], procplus, maxtabu, mintabu, iteration,
    newprocplus, flag_change, xx, exc, cpt, custp, MAXK,
    MAXK1;

long int elap = 0;
char *fname;
long int nprob, ns, eucl, choix, maxdist, besttabu[11];


int d[MAXN+1][MAXN+1], dtp[MAXN+1][MAXN+1], tabulist[MAXP+1][MAXN+1],
    tproc[MAXN+1];

float timeia, timetab, timetb, timepot, timetot,
      ttimeia, ttimetab, ttimetb, ttimepot, ttimetot,
      soluias, solutab, solupot,
      tsoluias, tsolutab, tsolupot, alfa;


proxnoeud neighin[MAXN+1], neighout[MAXN+1];

tflag flag;

tourneelem *imppb;

coord g2[MAXN+1];

tourne t2;

char namef[30], nameout[30];

neighbour bneigh[MAXP+1];

FILE *outf;

/********BEGINNING OF THE MAIN PROGRAM****************/

int main()
{

    clock_t start, finish;

    strcpy (namef,"ns_g05_100_02_01");
	fname = namef;

 /*for (ij = 1 ; ij <= 10 ; ij++) besttabu[ij] = MAXDIST;*/
 for (alfa = 0.5 ; alfa <= .8 ; alfa+=.5)
 {
  if (alfa <= 0.5) alfa = 0.6;
  else if (alfa <= 1.0) alfa = 0.8;
  ttimeia = 0 ;
  ttimetab = 0;
  ttimetb = 0;
  ttimepot = 0;
  ttimetot = 0;
  tsoluias = 0;
  tsolutab = 0;
  tsolupot = 0;
 /*for (ij = 1 ; ij <= 1 ; ij++) {*/

 start = clock();
 init(&depot,lasttabu,tabulist);
 k1 = MAXK;
 generegraph(g,d,dtp,tproc,&depot);
 find_in_out_neighbour (neighin,neighout,d);
 initial_assignment (g,d,dtp,&depot,neigh);
 seed_generation ();
 /*printf("primeiro store ");
   printf ("\n primeiro store bneigh[1]->p1[6].nn[2]==%d",bneigh[1].p1[6].nn[2]);*/
 store_best_solution (g,&depot,bg,&bsolution,&makespan,
                      neigh,bneigh);
 procplus = makespan.proc;
 soluias = makespan.load;
 finish = clock();
 tsoluias = (finish - start)/CLOCKS_PER_SEC;

/* printf("\nEnter the minimum tabu size : ");*/
/* scanf("%d",&mintabu);                      */
/* printf("\nEnter the maximum tabu size : ");*/
/* scanf("%d",&maxtabu);                      */
 mintabu = 3;
 maxtabu = 7;

 /* ********  BEGINNING OF TABU PHASE  ********  */

 /*printf ("\n************TABU PHASE****************");*/

 start = clock();
 makespan.iter = 1;
 imptrue = 1;
 lastimp = 1;
 timetab = 0;
 for (iteration = 1 ; imptrue ; iteration++)
    {
     etime(1);
     minmakespan = MAXDIST;
     flag.modifie = 0;
     xio = depot.nodepot[procplus];
     vio = 0;
     pio = 0;
     for (i = 1 ; i <= (depot.tabt[procplus].nbredenoeuds - 1) ; i++)
        {
         minpm = MAXDIST;
         xio = suiv (xio,g);
         oterx (xio,&neigh[procplus],&depot.tabt[procplus],g,dtp,&flag);
         if (depot.tabt[procplus].nbredenoeuds == 2)
           flag.mdelta = - depot.load[procplus];
         minpl = depot.load[procplus] + flag.mdelta;
         for (j = 1 ; j <= np ; j++){
             if (j != procplus){
                cneighin = 0;
                cneighout = 0;
                for (k = 1 ; k <= k1 ; k++)
                   {
                    if (neighin[xio].nn[k] > n)
                      neighin[xio].nn[k] = j + n;
                    if (neighout[xio].nn[k] > n)
                      neighout[xio].nn[k] = j + n;
                    if (depot.tabt[j].noeudinterne[neighin[xio].nn[k]])
                      cneighin++;
                    if (depot.tabt[j].noeudinterne[neighout[xio].nn[k]])
                      cneighout++;
                   } /* END OF for (k = 1 ; k <= k1 ; k++) */
                if ((cneighin >= 1) || (cneighout >= 2) ||
                    (depot.tabt[j].nbredenoeuds <= 1))
                  {
                   ajoutx (xio,&neigh[j],&depot.tabt[j],g,dtp,&flag);
                   if ((depot.load[j] + flag.mdelta) <= minpm){
                      if (tabulist[j][xio] >= iteration)  {
                         if (((depot.load[j]+flag.mdelta)<makespan.load)
                              && (minpl < makespan.load)){
                            aspiration = 1;
                            for (k = 1 ; k <= np ; k++){
                                if ((k != procplus) && (k != j)){
                                   if (depot.load[k] > makespan.load)
                                     aspiration = 0;
                                  }
                               }
                            if (aspiration){
                               pa = j;
                               minpm = depot.load[j] + flag.mdelta;
                              }
                           }
                        } /* END THEN OF if (tabulist[j][xio]>=iter... */
                      else{
                         pa = j;
                         minpm = depot.load[j] + flag.mdelta;
                        } /* END ELSE OF if (tabulist[j][xio]>=iter... */
                     } /* END OF if((depot.load[j]+flag.mdelta<=minpm) */
                  } /* END OF if ((cneighin >= 1) || (cneighout >= 2)) */
               } /* END OF if (j != procplus) */
            } /* END OF for (j = 1 ; j <= np ; j++) */
         if (minpm <= minpl) minpm = minpl;
         if (minpm <= minmakespan){
            minmakespan = minpm;
            vio = xio;
            pio = pa;
           }
        } /* END OF for(i=1;i<=(depot.tabt[procplus].nbredenoeuds-1) */
     if ((vio == 0) || (pio == 0)) break;
     flag.modifie = 1;
     flag.numbdepot = procplus;
     oterx (vio,&neigh[procplus],&depot.tabt[procplus],g,dtp,&flag);
     depot.load[procplus] = calculcoutt (&depot.tabt[procplus],dtp);
     if (depot.tabt[procplus].nbredenoeuds == 1)
       depot.load[procplus] = 0;
     update_local_neighbourhood (vio,procplus,&neigh[procplus],
                                 &depot.tabt[procplus],g,d);
     flag.numbdepot = pio;
     ajoutx (vio,&neigh[pio],&depot.tabt[pio],g,dtp,&flag);
     depot.load[pio] = calculcoutt (&depot.tabt[pio],dtp);
     g[vio].satourne = pio;
     ajoutenoeudprox (&neigh[pio],vio,d,pio);

    /*timetab = timetab + (etime(0) / 60.);*/
 /*  printf ("\nIteration = %d",iteration);                          */
 /*  printf ("\nProc.out = %d - Var.out = %d - new cust = %d",       */
 /*          procplus,vio,depot.load[procplus]);                     */
 /*  printf ("\nProc.in = %d - new cust = %d\n",pio,depot.load[pio]);*/
     tabulist[procplus][vio] = tabu_time (mintabu,maxtabu) + iteration;
     lasttabu[vio] = iteration;
     maxload = 0;
     for (i = 1 ; i <= np ; i++){
         if (depot.load[i] > maxload){
            maxload = depot.load[i];
            procplus = i;
           }
        }
     if (maxload < makespan.load) {
        makespan.iter = iteration;
    /*    printf ("\n antes do store");
	      printf ("\n bneigh[1]->p1[6].nn[2]==%d",bneigh[1].p1[6].nn[2]);*/
	store_best_solution (g,&depot,bg,&bsolution,&makespan,
                             neigh,bneigh);
        lastimp = iteration;
        finish = clock();
        timetb = (finish - start)/CLOCKS_PER_SEC;
       }
     else {
        if ((iteration - lastimp) > (n*10)) imptrue = 0;
       }
    } /* END OF for (iteration = 1 ; imptrue ; iteration++) */
 solutab = makespan.load;
 finish = clock();
 timetab = (finish - start)/CLOCKS_PER_SEC;
 /*printf ("\n*****BEST SOLUTION AFTER TABU PHASE*******");*/
 /*printf ("\n\nIteration    = %d",makespan.iter);*/
 /*printf ("\nLoadest Proc = %d",makespan.proc);
   printf ("\nLoad         = %d\n",makespan.load);*/
 for (j = 1 ; j <= np ; j++)
    {
     imppb = bsolution.tabt[j].ptr;
   /*  printf ("\n  *********PROCESSOR %d ***********",j);
       printf("\n CUST = %d\n",bsolution.load[j]);*/
     for (i = 1 ; i <= bsolution.tabt[j].nbredenoeuds + 1 ; i++)
        {
       /*  printf("%d-", imppb->noeud);*/
         imppb = imppb->prochain;
        }
     /*printf("\n");*/
    }

 /************BEGINNING OF POST-OPTIMIZATION**************/
 start = clock();
 flag_change = 0;
 procplus = makespan.proc;
 etime(1);
 do
   {
    if (flag_change)
      {
       procplus = newprocplus;
       makespan.proc = newprocplus;
       makespan.load = bsolution.load[newprocplus];
      }
    newprocplus = 1;
    exc = bsolution.load[procplus];
    xx = bsolution.nodepot[procplus];
    flag.numbdepot = procplus;
    copietourne (&bsolution.tabt[procplus],bg,&t2,g2,xx);
    cpt = 0;
    flag.modifie = 1;
    do
      {
      /*printf("\n procplus==%d", procplus);*/
       oterx (xx,&bneigh[procplus],&t2,g2,dtp,&flag);
    /*   printf("\nNumero de nos depois do oterx %d", t2.nbredenoeuds);*/

       k1++;
       ajoutx (xx,&bneigh[procplus],&t2,g2,dtp,&flag);
     /*  printf("\nNumero de nos depois do ajoutx %d", t2.nbredenoeuds);*/
       k1--;
       custp = calculcoutt (&t2,dtp);
       if (exc > custp)
         {
          exc = custp;
          cpt = 0;
          copietourne (&t2,g2,&bsolution.tabt[procplus],bg,xx);
          bsolution.load[procplus] = custp;
          makespan.load = custp;
         }
       xx = suiv (xx , g2);
       cpt++;
      } while (cpt != bsolution.tabt[procplus].nbredenoeuds);
    custp = bsolution.load[newprocplus];
    if (bg[n+procplus].ptrtourne != bsolution.tabt[procplus].ptr)
      {
       bsolution.tabt[procplus].ptr = bg[n+procplus].ptrtourne;
       numerote_tourne(&bsolution.tabt[procplus]);
      }
    for (i = 2 ; i <= np ; i++)
       {
        if (bsolution.load[i] > custp)
          {
           newprocplus = i;
           custp = bsolution.load[i];
          }
       }
     if (procplus != newprocplus) flag_change = 1;
   } while (procplus != newprocplus);
 /*timepot = etime(0) / 60.;*/
 finish = clock();
 timepot = (finish - start)/CLOCKS_PER_SEC;
 solupot = makespan.load;

  strcpy(nameout, "SOL_");
  strcat(nameout, namef);
  fname = nameout;
  if ((outf = fopen(fname,"w")) == NULL)
   {
   printf("\nOUTPUT FILE CAN NOT BE OPEN \n ## END OF THE PROGRAM\n");
    exit(1);
   }

 printf ("\n**BEST SOLUTION AFTER POST-OPTIMIZATION**\n");
 fprintf (outf,"\n**BEST SOLUTION AFTER POST-OPTIMIZATION**\n");
 printf ("\nLoadest Proc. = %d",makespan.proc);
 printf ("\nLoad          = %d",makespan.load);
 fprintf (outf,"\nLoadest Proc. = %d",makespan.proc);
 fprintf (outf,"\nLoad          = %d",makespan.load);
 for (j = 1 ; j <= np ; j++)
    {
     imppb = bsolution.tabt[j].ptr;
     printf ("\n  *********PROCESSOR %d ***********",j);
     printf("\n COST = %d\n",bsolution.load[j]);
     fprintf (outf,"\n  *********PROCESSOR %d ***********",j);
     fprintf (outf,"\n COST = %d\n",bsolution.load[j]);
     for (i = 1 ; i <= bsolution.tabt[j].nbredenoeuds + 1 ; i++)
        {
         printf("%d-", imppb->noeud);
         fprintf(outf,"%d-", imppb->noeud);
         imppb = imppb->prochain;
        }
     printf("\n");
     fprintf(outf,"\n");
    }
 timetot = timeia + timetab + timepot;
 /*ttimeia += timeia;
 ttimetab += timetab;
 ttimetb += timetb;
 ttimepot += timepot;
 ttimetot += timetot;
 tsoluias += soluias;
 tsolutab += solutab;
 tsolupot += solupot;*/
 printf("\nInitial Solution       = %f",soluias);
 printf("\nTime Initial Assigment = %f",timeia);
 printf("\nTabu Solution          = %f",solutab);
 printf("\nTime Tabu Phase        = %f",timetab);
 printf("\nPos-Opt. Solution      = %f",solupot);
 printf("\nTime Post Optimization = %f",timepot);
 printf("\nTotal Time             = %f\n",timetot);
 fprintf(outf,"\nInitial Solution       = %f",soluias);
 fprintf(outf,"\nTime Initial Assigment = %f",timeia);
 fprintf(outf,"\nTabu Solution          = %f",solutab);
 fprintf(outf,"\nTime Tabu Phase        = %f",timetab);
 fprintf(outf,"\nPos-Opt. Solution      = %f",solupot);
 fprintf(outf,"\nTime Post Optimization = %f",timepot);
 fprintf(outf,"\nTotal Time             = %f\n",timetot);
 fclose(outf);

/*** CREATION OF THE OUTPUT FILE TO THE FORTRAN ROUTINE ***/

 /*if (makespan.load < besttabu[ij]) {
  besttabu[ij] = makespan.load;*/
/*  strcpy(namef, "r");
  strcat(namef, fname);
//  if (ij == 1) strcat(namef,"01");
//  if (ij == 2) strcat(namef,"02");
//  if (ij == 3) strcat(namef,"03");
//  if (ij == 4) strcat(namef,"04");
//  if (ij == 5) strcat(namef,"05");
//  if (ij == 6) strcat(namef,"06");
//  if (ij == 7) strcat(namef,"07");
//  if (ij == 8) strcat(namef,"08");
//  if (ij == 9) strcat(namef,"09");
//  if (ij == 10) strcat(namef,"10");

 if ((outf = fopen(namef,"w")) == NULL)
   {
   printf("\nOUTPUT FILE CAN NOT BE OPEN \n ## END OF THE PROGRAM\n");
    exit(1);
   }
 nprob = 1;
 ns = n + 1;
 choix = 1;
 eucl = 0;
 maxdist = makespan.load;

 fprintf(outf,"%5d%5d%5d%5d%5d%5d\n",nprob,ns,np,eucl,choix,maxdist);
 for (i = 1 ; i <= n ; i++)
    {
     for (j = (i + 1) ; j <= (n + 1) ; j++)
        {
         fprintf(outf,"%5d%5d\n",dtp[i][j],dtp[j][i]);
        }
    }
 fprintf(outf,"%10.3f",timetot);
 fclose(outf);*/

 /*}*/ /* END OF if (makespan.load <= besttabu[ij]) */
 /*} */ /* END OF for (ij = 1 ; ij <= 10 ; ij++) */
/* printf ("\n\n**************FINAL STATISTICS*****************\n");
 printf ("\nTabu Tag Min = %d      Tabu Tag Max = %d",mintabu,maxtabu);
 printf ("\nLocal neighbourhood    = %d", MAXK);
 printf ("\nGlobal neighbourhood   = %d", MAXK1);
 printf ("\nNumber of Problems     = %d",ij-1);
 printf ("\nNumber of Proc.        = %d",np);
 printf ("\nNumber of Tasks        = %d",n);
 printf ("\nInitial Solution       = %f     %f",
        (tsoluias/(ij-1)),(((tsoluias-tsolupot)*100)/tsolupot));
 printf ("\nTabu Solution          = %f     %f",
        (tsolutab/(ij-1)),(((tsolutab-tsolupot)*100)/tsolupot));
 printf ("\nPos-Opt. Solution      = %f     %f",
        (tsolupot/(ij-1)),(((tsolupot-tsolupot)*100)/tsolupot));
 printf ("\nTime Initial Solution  = %f     %f",
        (ttimeia/(ij-1)),((ttimeia*100)/ttimetot));
 printf ("\nTime To Best Solution  = %f     %f",
       (ttimetb/(ij-1)),((ttimetb*100)/ttimetot));
 printf ("\nTime Tabu Phase        = %f     %f",
        (ttimetab/(ij-1)),((ttimetab*100)/ttimetot));
 printf ("\nTime Post Optimization = %f     %f",
       (ttimepot/(ij-1)),((ttimepot*100)/ttimetot));
 printf ("\nTotal Time             = %f     %f",
        (ttimetot/(ij-1)),((ttimetot*100)/ttimetot));
 printf("\n");*/
} /* END OF for (alfa = 1 ; alfa <= 2 ; alfa++) */

} /*  FIM MAIN()  */

/******************************************************************/

init(depot,lasttabu,tabulist)

 tdepot *depot;

 int lasttabu[], tabulist[MAXP+1][MAXN+1];

{

 /* INITILIZATION OF THE RESULT VECTOR */

 int   ll1,ll2;

 for (ll1 = 0 ; ll1 <= MAXN ; ll1 ++)
    {
     lasttabu[ll1] = 0;
     for (ll2 = 0 ; ll2 <= MAXP ; ll2++)
        {
         depot->tabt[ll2].noeudinterne[ll1] = 0;
         tabulist[ll2][ll1] = 0;
        }
    }

} /* END OF INIT() */

/*******************************************************************/

generegraph(g,d,dtp,tproc,depot)

 coord g[];

 int d[MAXN+1][MAXN+1], dtp[MAXN+1][MAXN+1], tproc[MAXN+1];

 tdepot *depot;

{

 /* THIS PROCEDURE READ THE POINTS' COORDINATE FROM
    AN OUTSIDE FILE AND BUILD A DISTANCE MATRIX
    BETWEEN EACH NODE */


 FILE *inp;
 int  i, j, k, mach, task, cvali;
 float cval;


 if ((inp = fopen(fname,"r")) == NULL)
   {
    printf("\nFILE CAN NOT BE OPEN \n ## END OF THE PROGRAM ##\n");
    exit(1);
   }
 fscanf(inp,"%d\n",&mach);
 fscanf(inp,"%d\n",&task);
 cval = alfa  * sqrt (( (float) task / (float) mach)) + 0.99;
 MAXK = cval;
 if (MAXK <= 3) MAXK = 3;
 k1 = MAXK;
 cval = alfa * sqrt ( (float) task) + 0.99;
 MAXK1 = cval;
 if (MAXK1 <= 5) MAXK1 = 5;
 np = mach;
 n = task;
 for (i = 1 ; i <= (n + np) ; i++)
    {
     fscanf(inp,"%d\n",&cvali);
     tproc[i] = cvali;
    }
 for (i = 1 ; i <= (n + np) ; i++)
    {
     for (j = 1 ; j <= (n + np) ; j++)
      {
       fscanf(inp,"%d\n",&cvali);
       if (cvali >= 500) d[i][j] = MAXDIST;
       else d[i][j] = cvali;
      }
    }
 for (i = 1 ; i <= (n + np) ; i++)
    {
     for (j = 1 ; j <= (n + np) ; j++)
      {
       dtp[i][j] = d[i][j] + tproc[j];
      }
    }
 initialise_depot (depot,g);
 initprox (neigh);
 for (i = (n + 1) ; i <= (n + np) ; i++)
    ajoutenoeudprox (&neigh[i-n],i,d,(i-n));

} /* END OF GENEREGRAPH */

/*******************************************************************/

 initialise_depot (depot,g)

 tdepot *depot;

 coord g[];

 {
  int j;

  for (j = n + 1 ; j <= (n + np) ; j++)
     {
      nouvelletourne(&depot->tabt[j-n],j,g);
      g[j].satourne = j - n;
      depot->nodepot[j-n] = j;
     }

 } /* END OF INITIALISE_DEPOT */

/*******************************************************************/

initprox(neigh)

neighbour neigh[];

{

 /* THIS PROCEDURE ITITIALIZE THE K1 NEAREST NEIGHBOURS TABLEAU.
    TO EACH NODE THE LONGEST DISTANCE IS TO NODE NUMBER ONE
    (DISTANCE = MAXREAL = 1,7e38.) */

 proxnoeud w;
 int i, j;

 for (j = 1 ; j <= k1 + 1 ; j++) w.nn[j] = -1;
 w.leplusloin = 1;
 w.maxdist = MAXREAL;
 for (i = 1 ; i <= np ; i++)
    {
     for (j = 1 ; j <= (n + np) ; j++)
        {
         neigh[i].p1[j] = w;
         neigh[i].p2[j] = w;
        }
    }

} /* END OF INITPROX */

/********************************************************************/

find_in_out_neighbour(neighin,neighout,d)

 proxnoeud neighin[], neighout[];

 int d[MAXN+1][MAXN+1];

{

 int i,j,i1,j1,imin[MAXN+1],temp,exchange;
 float min[MAXN+1];
 for (i = 1 ; i <= n ; i++)
    {

     /* FINDING THE OUT NEIGHBOURHOOD : i->j */

     min[i] = MAXREAL;
     imin[i] = i;
     for (j = 1 ; j <= (n+1) ; j++)
        {
         if (j != i)
           {
            imin[j] = j;
            min[j] = d[i][j];
           }
        }
     exchange = 1;
     for (i1 = 1 ; ((i1 <= n) && (exchange)) ; i1++)
        {
         exchange = 0;
         for (j1 = 1 ; j1 <= (n + 1 - i1) ; j1++)
            {
             if (min[imin[j1]] > min[imin[j1+1]])
               {
                temp = imin[j1];
                imin[j1] = imin[j1+1];
                imin[j1+1] = temp;
                exchange = 1;
               }
            }
        }
     for (j = 1 ; j <= MAXK1 ; j++){
     	neighout[i].nn[j] = imin[j];
	/*printf("\n neighout[%i].nn[%i]==%d",i,j,neighout[i].nn[j]);*/
     }
     neighout[i].leplusloin = MAXK1;
     neighout[i].maxdist = min[imin[MAXK1]];

     /* FINDING THE IN NEIGHBOURHOOD : j->i */

     min[i] = MAXREAL;
     imin[i] = i;
     for (j = 1 ; j <= (n+1) ; j++)
        {
         if (j != i)
           {
            imin[j] = j;
            min[j] = d[j][i];
           }
        }
     exchange = 1;
     for (i1 = 1 ; ((i1 <= n) && (exchange)) ; i1++)
        {
         exchange = 0;
         for (j1 = 1 ; j1 <= (n + 1 - i1) ; j1++)
            {
             if (min[imin[j1]] > min[imin[j1+1]])
               {
                temp = imin[j1];
                imin[j1] = imin[j1+1];
                imin[j1+1] = temp;
                exchange = 1;
               }
            }
        }
     for (j = 1 ; j <= MAXK1 ; j++){
     	neighin[i].nn[j] = imin[j];
     	/*printf("\n neighin[%i].nn[%i]==%d",i,j,neighin[i].nn[j]);*/
     }
     neighin[i].leplusloin = MAXK1;
     neighin[i].maxdist = min[imin[MAXK1]];
    } /* END OF for (i = 1 ; i <= (n+1) ; i++) */

} /* END OF FIND_IN_OUT_NEIGHBOUR */

/********************************************************************/

initial_assignment(g,d,dtp,depot,neigh)

 coord g[];

 int d[MAXN+1][MAXN+1], dtp[MAXN+1][MAXN+1];

 tdepot *depot;

 neighbour neigh[];

{

 int i,j,dgap;
 float gap;
 tflag  flag;

 tourneelem *impp;

 etime(1);
 for (i = 1 ; i <= np ; i++)
    {
     ajoute_a_tourne(&depot->tabt[i],i,g);
     ajoutenoeudprox(&neigh[i],i,d,i);
     numerote_tourne(&depot->tabt[i]);
     depot->load[i] = dtp[i+n][i];
     g[i].satourne = i;
    }
 flag.modifie = 0;
 flag.mdelta = MAXDIST;
 gap = MAXDIST;
 for (i = (np + 1) ; i <= n ; i++)
    {
     for (j = 1 ; j <= np ; j++)
        {
         flag.numbdepot = j;
         ajoutx (i,&neigh[j],&depot->tabt[j],g,dtp,&flag);
         if ((flag.mdelta + depot->load[j]) <= gap)
           {
            gap = depot->load[j] + flag.mdelta;
            dgap = j;
           }
        }
     flag.modifie = 1;
     ajoutx(i,&neigh[dgap],&depot->tabt[dgap],g,dtp,&flag);
     depot->load[dgap] = calculcoutt (&depot->tabt[dgap],dtp);
     ajoutenoeudprox(&neigh[dgap],i,d,dgap);
     g[i].satourne = dgap;
     flag.modifie = 0;
     gap = MAXREAL;
    }
 timeia = etime(0) / 60.;
 /*printf ("\n************INITIAL ASSIGMENT****************");*/
 for (j = 1 ; j <= np ; j++)
    {
     impp = depot->tabt[j].ptr;
   /*  printf ("\n  *********PROCESSOR %d ***********",j);
     printf("\n CUST = %d\n",depot->load[j]);*/
     for (i = 1 ; i <= depot->tabt[j].nbredenoeuds + 1 ; i++)
        {
       /*  printf("%d-", impp->noeud);*/
         impp = impp->prochain;
        }
     /*printf("\n");*/
    }

} /* END OF INITIAL_ASSIGNMENT */

/******************************************************************/

nouvelletourne(t,x,g)

 tourne *t;

 int x;

 coord g[];

{

 /* THIS PROCEDURE BUILD A ROUTE WITH ONLY ONE NODE x.
    THE g[x] POINTER WILL BE SETTED IN THE EXIT OF THIS
    ELEMENT. */

 t->noeudinterne[x] = 1;
 t->nbredenoeuds = 1;
 t->ptr = (tourneelem *) malloc(sizeof(tourneelem));
 demand(t->ptr,memory overflow);
 t->ptr->noeud = x;
 t->ptr->prochain = t->ptr;
 t->ptr->precedent = t->ptr;
 g[x].ptrtourne = t->ptr;

} /* END OF NOUVELLETOURNE  */

/******************************************************************/

ajoutenoeudprox(neigh,aj,d,ndep)

 neighbour *neigh;

 int aj;

 int d[MAXN+1][MAXN+1];

 int ndep;


{

 /* THIS PROCEDURE UPDATE THE K1 NEAREST NEIGHBOURS TABLEAU. */

 int i, nmaximum, j;

 float vmaximum, dist;

 for (i = 1 ; i <= (n + 1) ; i++)
    {
     if (i == (n+1)) i = ndep + n;
     if (i != aj)
       {
        if (neigh->p1[i].maxdist > d[i][aj])
          {
           neigh->p1[i].nn[neigh->p1[i].leplusloin] = aj;
           if (neigh->p1[i].nn[k1+1] < 0)
             neigh->p1[i].nn[k1+1] = aj;
           else if (neigh->p1[i].maxdist < d[i][neigh->p1[i].nn[k1+1]])
                  neigh->p1[i].nn[k1+1] = aj;
           vmaximum = 0;
           for (j = 1 ; j <= k1 ; j++)
              {
               if (neigh->p1[i].nn[j] < 0)
                 {
                  nmaximum = j;
                  vmaximum = MAXREAL;
                  break;
                 }
               else
                 {
                  dist = d[i][neigh->p1[i].nn[j]];
                  if (dist >= vmaximum)
                    {
                     nmaximum = j;
                     vmaximum = dist;
                    }
                 }
              }
           neigh->p1[i].leplusloin = nmaximum;
           neigh->p1[i].maxdist = vmaximum;
         }
       else if (neigh->p1[i].nn[k1+1] < 0)
              neigh->p1[i].nn[k1+1] = aj;
            else if (d[i][aj] < d[i][neigh->p1[i].nn[k1+1]])
                   neigh->p1[i].nn[k1+1] = aj;
       if (neigh->p2[i].maxdist > d[aj][i])
         {
          neigh->p2[i].nn[neigh->p2[i].leplusloin] = aj;
          if (neigh->p2[i].nn[k1+1] < 0)
            neigh->p2[i].nn[k1+1] = aj;
          else if (neigh->p2[i].maxdist < d[neigh->p2[i].nn[k1+1]][i])
                 neigh->p2[i].nn[k1+1] = aj;
          vmaximum = 0;
          for (j = 1 ; j <= k1 ; j++)
             {
              if (neigh->p2[i].nn[j] < 0)
                {
                 nmaximum = j;
                 vmaximum = MAXREAL;
                 break;
                }
              else
                {
                 dist = d[neigh->p2[i].nn[j]][i];
                 if (dist >= vmaximum)
                   {
                    nmaximum = j;
                    vmaximum = dist;
                   }
                }
             }
           neigh->p2[i].leplusloin = nmaximum;
           neigh->p2[i].maxdist = vmaximum;
         }
       else if (neigh->p2[i].nn[k1+1] < 0)
              neigh->p2[i].nn[k1+1] = aj;
            else if (d[aj][i] < d[neigh->p2[i].nn[k1+1]][i])
                   neigh->p2[i].nn[k1+1] = aj;
      }
    }

} /* END OF AJOUTENOEUDPROX */

/****************************************************************/

update_local_neighbourhood(x,nproc,neigh,t,g,d)

 int x,nproc;

 neighbour *neigh;

 tourne *t;

 coord g[];

 int d[MAXN+1][MAXN+1];

{

int var,i,j,k,l,var2,sofar,maxd,maxv,vtrue1,vtrue2,eq,ident;

for (i = 1 ; i <= (n + 1) ; i++){
	var = i;
     	if (i == (n + 1)) var = n + nproc;
     	vtrue1 = 1;
     	if (t->noeudinterne[var]) ident = k1;
     	else ident = k1 - 1;
     	for (j = 1 ; ((j <= k1) && (vtrue1)) ; j++){
         	if (neigh->p1[var].nn[j] > n) neigh->p1[var].nn[j] = n + nproc;
         	if (neigh->p1[var].nn[j] == x){
            		if (t->nbredenoeuds <= ident){
               			for (k = j ; k <= (k1 - 1) ; k++)
	               			neigh->p1[var].nn[k] = neigh->p1[var].nn[k+1];
        	       		neigh->p1[var].nn[k] = -1;
               			vtrue2 = 1;
               			for (k = j ; vtrue2 ; k++){
                   			if (neigh->p1[var].nn[k] < 0){
                      				neigh->p1[var].leplusloin = k;
                      				neigh->p1[var].maxdist = MAXREAL;
                      				vtrue2 = 0;
                     			}
                 		}
              		} /* END THEN OF if (t->nbredenoeuds <= ident) */
            		else{
               			maxd = MAXDIST;
               			var2 = n + nproc;
               			if (neigh->p1[var].leplusloin == j) sofar = x;
               			else sofar = neigh->p1[var].nn[neigh->p1[var].leplusloin];
               			if ((var > n) && (neigh->p1[var].maxdist == 0)){
                			do {
                    				var2 = suiv (var2,g);
                    				vtrue2 = 1;
                    				eq = 1;
                    				for (k = 1 ; ((k <= k1) && (eq)) ; k++){
                        				if (k != j){
                           					eq = 1;
                           					if (var2 == neigh->p1[var].nn[k]) eq = 0;
                          				}
                       				}
                    				if (eq){
                       					neigh->p1[var].nn[j] = var2;
                       					vtrue2 = 0;
                      				}
                   			} while (vtrue2);
               			}
               			else{
               				for (k = 1 ; k <= t->nbredenoeuds ; k++){
                   				if ((var != var2) && (var2 != sofar)){
                      					if (d[var][var2] < maxd){
                         					if (d[var][var2] >= neigh->p1[var].maxdist){
                            						vtrue2 = 1;
                            						if (d[var][var2] == neigh->p1[var].maxdist){
                               							for (l = 1 ; l <= k1 ; l++){
                                   							if (var2 == neigh->p1[var].nn[l])
                                     								vtrue2 = 0;
                                  						}
                              						}
                            						if (vtrue2){
                               							maxd = d[var][var2];
                               							maxv = var2;
                              						}
                           					}
                        				}
                     				}
                   				var2 = suiv (var2,g);
                  			}
               				neigh->p1[var].nn[j] = maxv;
               				neigh->p1[var].leplusloin = j;
               				neigh->p1[var].maxdist = maxd;
               			}
              		} /* END ELSE OF if (t->nbredenoeuds <= ident) */
            		vtrue1 = 0;
           	} /* END OF if (neigh->p1[var].nn[j] == x) */
	} /* END OF for (j = 1 ; j <= k1 ; j++) */
     	if (neigh->p1[var].nn[k1+1] == x)
       	neigh->p1[var].nn[k1+1] =
       	neigh->p1[var].nn[neigh->p1[var].leplusloin];
} /* END OF for (i = 1 ; i <= t->nbredenoeuds ; i++) */
for (i = 1 ; i <= (n + 1) ; i++){
	var = i;
     	if (i == (n + 1)) var = n + nproc;
     	vtrue1 = 1;
     	if (t->noeudinterne[var]) ident = k1;
     	else ident = k1 - 1;
     	for (j = 1 ; ((j <= k1) && (vtrue1)) ; j++){
      	if (neigh->p2[var].nn[j] > n) neigh->p2[var].nn[j] = n + nproc;
        	if (neigh->p2[var].nn[j] == x){
            		if (t->nbredenoeuds <= ident){
               			for (k = j ; k <= (k1 - 1) ; k++)
                   			neigh->p2[var].nn[k] = neigh->p2[var].nn[k+1];
               			neigh->p2[var].nn[k] = -1;
               			vtrue2 = 1;
               			for (k = j ; vtrue2 ; k++){
                   			if (neigh->p2[var].nn[k] < 0){
                      				neigh->p2[var].leplusloin = k;
                      				neigh->p2[var].maxdist = MAXREAL;
                      				vtrue2 = 0;
                     			}
                  		}
	              	} /* END THEN OF if (t->nbredenoeuds <= ident) */
            		else{
               			maxd = MAXDIST;
               			var2 = n + nproc;
               			if (neigh->p2[var].leplusloin == j) sofar = x;
               			else sofar = neigh->p2[var].nn[neigh->p2[var].leplusloin];
               			if ((var > n) && (neigh->p2[var].maxdist == 0)){
                			do {
                    				var2 = suiv (var2,g);
                    				vtrue2 = 1;
                    				eq = 1;
                    				for (k = 1 ; ((k <= k1) && (eq)) ; k++){
                        				if (k != j){
                           					eq = 1;
                           					if (var2 == neigh->p2[var].nn[k]) eq = 0;
                          				}
                       				}
                    				if (eq){
                       					neigh->p2[var].nn[j] = var2;
                       					vtrue2 = 0;
                      				}
                   			} while (vtrue2);
               			}
               			else{
               				for (k = 1 ; k <= t->nbredenoeuds ; k++){
                   				if ((var != var2) && (var2 != sofar)){
                      					if (d[var2][var] < maxd){
                         					if (d[var2][var] >= neigh->p2[var].maxdist){
                            						vtrue2 = 1;
                            						if (d[var2][var] == neigh->p2[var].maxdist){
                               							for (l = 1 ; l <= k1 ; l++){
                                   							if (var2 == neigh->p2[var].nn[l])
                                     								vtrue2 = 0;
                                  						}
                              						}
                            						if (vtrue2){
                               							maxd = d[var2][var];
                               							maxv = var2;
                              						}
                           					}
                        				}
                     				}
                   				var2 = suiv (var2,g);
                  			}
               				neigh->p2[var].nn[j] = maxv;
               				neigh->p2[var].leplusloin = j;
               				neigh->p2[var].maxdist = maxd;
               			}
              		} /* END ELSE OF if (t->nbredenoeuds <= ident) */
            		vtrue1 = 0;
 		} /* END OF if (neigh->p1[var].nn[j] == x) */
 	} /* END OF for (j = 1 ; j <= k1 ; j++) */
     	if (neigh->p2[var].nn[k1+1] == x){
       		neigh->p2[var].nn[k1+1] = neigh->p2[var].nn[neigh->p2[var].leplusloin];
       		/*printf("\nneigh->p2[%i].nn[%i]==%d",var,(k1+1),neigh->p2[var].nn[k1+1]);*/
	}
} /* END OF for (i = 1 ; i <= t->nbredenoeuds ; i++) */

} /* END OF UPDATE_LOCAL_NEIGHBOURHOOD */

/****************************************************************/

ajoute_a_tourne(t,ind,g)

 tourne *t;

 int ind;

 coord g[];

{

 /* THIS PROCEDURE ADD TO THE ROUTE A NODE LABELED ind
    AND THE POINTER g[ind].ptr IS DIRECTIONED TO IT. */

 tourneelem *p;

 p = t->ptr->precedent;
 t->noeudinterne[ind] = 1;
 t->nbredenoeuds++;
 t->ptr->precedent = (tourneelem *) malloc(sizeof(tourneelem));
 demand(t->ptr->precedent,memory overflow);
 t->ptr->precedent->noeud = ind;
 t->ptr->precedent->precedent = p;
 t->ptr->precedent->prochain = t->ptr;
 p->prochain = t->ptr->precedent;
 g[ind].ptrtourne = t->ptr->precedent;

} /* END AJOUTE_A_TOURNE */

/****************************************************************/

numerote_tourne(t)

 tourne *t;

{

 /* THIS PROCEDURE ASSIGN A RANK TO EACH NODE IN THE ROUTE.
    THE NODE POINTERED IN t RECEIVE THE RANK = 1, THE NEXT
    ONE RANK = 2 AND SUCESSIVELY. */

 tourneelem *w;
 int i, j , indtrue, en[MAXN+1];

 for (i = 0 ; i <= (n+np+1) ; i++) en[i] = 0;
 indtrue = 1;
 w = t->ptr;
 for (i = 1 ; i <= MAXN ; i++)
    {
     w->rang = i;
     en[w->noeud] = 1;
     w = w->prochain;
     if (w == t->ptr) break;
    }
 for (j = 0 ; j <= (n+np+1) ; j++)
    {
     if (en[j] != t->noeudinterne[j])
       {
        indtrue = 0;
        break;
       }
    }
 if ((i != t->nbredenoeuds) || (!indtrue))
   {
    printf("\nWRONG ROUTE ASSIGNMENT ### END OF THE PROGRAM  \n");
    exit(1);
   }

} /* END OF NUMEROTE_TOURNE  */

/******************************************************************/

ajoutx(x,neigh,t,g,dtp,flag)

 int x;

 neighbour *neigh;

 tourne *t;

 coord g[];

 int dtp[MAXN+1][MAXN+1];

 tflag *flag;

{

 /* THIS PROCEDURE ADD THE NODE x TO THE ROUTE.
    NOTE THAT THIS PROCEDURE DON'T WORK IF THERE IS NO
    NODES IN THE ROUTE.
    THE K1 NEAREST NEIGHBOURS TABLEAU IS UPDATED. */

 struct oper {
              int x, i, j, k, l,
                  si, sj, sk, sl,
                  pi, pj, pk, pl,
                  typeajout;
             } noeuds;

 int som1, som2, i, j, k, l, xi, xj, xk, xl,
     xprecnj, xsuivni, ni, nl, nk, nj,
     suivni,suivnj, suivnl, suivnk,
     precnk,precnl,precnj,precni,
     xk1,yk1,delta, nouvdelta,
     voisinx_vu,deja_vuxi2,nlvaut0;

 tourneelem *px;

 i = 0; j = 0; k = 0; l = 0;
 ni = 0; nj = 0; nk = 0; nl = 0;
 xi = 0; xj = 0; xk = 0; xl = 0;
 xk1 = mini(t->nbredenoeuds , k1);
 yk1 = mini(t->nbredenoeuds - 1 , k1);
 delta = MAXDIST;
 if (xk1 <= 2)
   {
    ni = t->ptr->noeud;
    nj = suiv (ni,g);
    if (xk1 == 1)
      nouvdelta = dtp[ni][x];
    else
      {
       nouvdelta = dtp[ni][x] + dtp[x][nj] - dtp[ni][nj];
       if (nouvdelta > dtp[nj][x])
         {
          nouvdelta = dtp[nj][x];
          ni = nj;
          nj = suiv (ni,g);
         }
      }
    if (nouvdelta <= delta)
      {
       delta = nouvdelta;
       noeuds.typeajout = 5;
       noeuds.x = x;
       noeuds.i = ni;
       noeuds.j = nj;
      }
   } /* END THEN OF if (xk1 <= 2) */
 else
   {
    for (i = 1 ; i <= xk1 ; i++)
    {
     ni = neigh->p2[x].nn[i];
     if (ni < 0) ni = 0;
     if (t->noeudinterne[ni])
       {
        suivni = suiv ( ni, g );
        precni = prec ( ni, g );
        xi = ordre ( ni, g );
       }
     voisinx_vu = 0; /* FALSE */
     for (j = 1 ; (((j <= xk1) || (!voisinx_vu)) &&
                   (t->noeudinterne[ni])) ; j++)
        {
         nj = neigh->p1[x].nn[j];
         if (nj < 0) nj = 0;
         if (nj == suivni) voisinx_vu = 1; /* TRUE */
         if (( j > xk1 ) && (!voisinx_vu))
           {
            voisinx_vu = 1; /* TRUE */
            nj = suivni;
           }
         if (t->noeudinterne[nj]) xj = ordre(nj,g);
         if ((xj != xi) && (t->noeudinterne[nj]))
           {
            suivnj = suiv ( nj, g );
            precnj = prec ( nj, g );
            deja_vuxi2 = 0;  /* FALSE */
           for (l = 1 ; l <= ( 2*yk1+1 ) ; l++)
              {
               nlvaut0 = 0; /* FALSE */
               if (l <= yk1)
                 {
                  nl = neigh->p1[suivni].nn[l];
                  if (nl < 0) nl = 0;
                 }
               else
                 if ((l == (yk1+1)) && (!deja_vuxi2))
                   nl = suiv(suivni,g );
                 else
                   if (l > (yk1+1))
                     {
                      nl = neigh->p2[precnj].nn[l-yk1-1];
                      if (nl < 0) nl = 0;
                     }
                   else nlvaut0 = 1; /* TRUE */
               if ((nl == suiv(suivni,g)) && (l <= yk1))
                 deja_vuxi2 = 1; /* TRUE */
               if ((!nlvaut0) && (t->noeudinterne[nl]))
                 {
                  xl = ordre ( nl, g );
                  precnl = prec ( nl, g );
                  suivnl = suiv ( nl, g );
                 }
               if ((!((l == ( yk1+1 )) && (deja_vuxi2))) &&
                  (t->noeudinterne[nl]))
                 {
                  if (((xj > xi) && ((xl < xi) || (xl > xj))) ||
                      ((xj < xi) && ((xl > xj) && (xl < xi))))
                    {
                     if (l <= yk1)
                       {
                        nouvdelta = dtp[ni][x]+dtp[x][nj]+dtp[suivni][nl]+
                                    dtp[suivnj][suivnl]-dtp[ni][suivni]-
                                    dtp[nj][suivnj]-dtp[nl][suivnl];
                        som1 = nj;
                        while (som1 != suivni)
                             {
                              som2 = prec (som1,g);
                              nouvdelta = nouvdelta -
                                          dtp[som2][som1]+dtp[som1][som2];
                              som1 = som2;
                             }
                        som1 = nl;
                        while (som1 != suivnj)
                             {
                              som2 = prec (som1,g);
                              nouvdelta = nouvdelta -
                                          dtp[som2][som1]+dtp[som1][som2];
                              som1 = som2;
                             }
                        delta = mini (delta , nouvdelta);
                        if (delta == nouvdelta)
                          {
                           /*.s pour successeur et .p pour predecesseur*/
                           noeuds.typeajout = 1;
                           noeuds.i = ni;
                           noeuds.x = x;
                           noeuds.j = nj;
                           noeuds.si = suivni;
                           noeuds.l = nl;
                           noeuds.sj = suivnj;
                           noeuds.sl = suivnl;
                          }
                       } /* END THEN OF if (l <= yk1) */
                     else
                       {
                        nouvdelta = dtp[ni][x]+dtp[x][nj]+dtp[nl][precnj]+
                                    dtp[precnl][precni]-dtp[precni][ni]-
                                    dtp[precnj][nj]-dtp[precnl][nl];
                        som1 = precnj;
                        while (som1 != ni)
                             {
                              som2 = prec(som1,g);
                              nouvdelta = nouvdelta -
                                          dtp[som2][som1]+dtp[som1][som2];
                              som1 = som2;
                             }
                        som1 = precni;
                        while (som1 != nl)
                             {
                              som2 = prec(som1,g);
                              nouvdelta = nouvdelta -
                                          dtp[som2][som1]+dtp[som1][som2];
                              som1 = som2;
                             }
                        delta = mini (delta, nouvdelta );
                        if (delta == nouvdelta)
                          {
                           noeuds.typeajout = 3;
                           noeuds.i = ni;
                           noeuds.x = x;
                           noeuds.j = nj;
                           noeuds.l = nl;
                           noeuds.pi = precni;
                           noeuds.pj = precnj;
                           noeuds.pl = precnl;
                          }
                       } /* END ELSE OF if (l <= yk1) */
                     if ((((xl != ordre (suivnj,g)) && (l <= (yk1+1))) ||
                        ((xl != ordre ( precni,g )) && (l > (yk1+1)))) &&
                        (xj != ordre(suivni,g)))
                       {
                        for (k = 1 ; k <= yk1 ; k++)
                           {
                            if (l <= yk1+1)
                               nk = neigh->p2[suivnj].nn[k];
                            else
                               nk = neigh->p1[precni].nn[k];
                            if (nk < 0) nk = 0;
                            if (t->noeudinterne[nk])
                              xk = ordre ( nk, g );
                            xsuivni = ordre ( suivni, g );
                            xprecnj = ordre ( precnj, g );
                            if ((((((xj > xsuivni) && (xk > xsuivni) &&
                               (xk <= xj)) || ((xj < xsuivni) &&
                               ((xk > xsuivni) || (xk <= xj)))) &&
                               (l <= (yk1+1))) || ((((xprecnj > xi) &&
                               (xk >= xi) && (xk < xprecnj)) ||
                               ((xprecnj < xi) && ((xk >= xi) ||
                               (xk < xprecnj)))) && (l > (yk1+1)))) &&
                               (t->noeudinterne[nk]))
                              {
                               suivnk = suiv ( nk, g );
                               precnk = prec ( nk, g );
                               if (l <= (yk1+1))
                                 {
                                  nouvdelta = dtp[ni][x]+dtp[x][nj]+
                                     dtp[nk][suivnj]+
                                     dtp[precnl][precnk]+dtp[suivni][nl]-
                                     dtp[ni][suivni]-dtp[nj][suivnj]-
                                     dtp[precnl][nl]-dtp[precnk][nk];
                                  som1 = nj;
                                  while (som1 != nk)
                                       {
                                        som2 = prec(som1,g);
                                        nouvdelta = nouvdelta -
                                           dtp[som2][som1]+dtp[som1][som2];
                                        som1 = som2;
                                       }
                                  som1 = precnk;
                                  while (som1 != suivni)
                                       {
                                        som2 = prec(som1,g);
                                        nouvdelta = nouvdelta -
                                           dtp[som2][som1]+dtp[som1][som2];
                                        som1 = som2;
                                       }
                                  delta = mini (delta, nouvdelta );
                                  if (delta == nouvdelta)
                                    {
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
                                 } /* END THEN OF if (l <= (yk1+1)) */
                               else
                                 {
                                  nouvdelta = dtp[ni][x]+dtp[x][nj]+
                                      dtp[precni][nk]+
                                      dtp[suivnk][suivnl]+dtp[nl][precnj] -
                                      dtp[precni][ni]-dtp[precnj][nj]-
                                      dtp[nl][suivnl]-dtp[nk][suivnk];
                                  som1 = nk;
                                  while (som1 != ni)
                                       {
                                        som2 = prec(som1,g);
                                        nouvdelta = nouvdelta -
                                           dtp[som2][som1]+dtp[som1][som2];
                                        som1 = som2;
                                       }
                                  som1 = precnj;
                                  while (som1 != suivnk)
                                       {
                                        som2 = prec(som1,g);
                                        nouvdelta = nouvdelta -
                                           dtp[som2][som1]+dtp[som1][som2];
                                        som1 = som2;
                                       }
                                  delta = mini (delta, nouvdelta );
                                  if (delta == nouvdelta)
                                    {
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
                                 } /* END ELSE OF if (l <= (yk1+1)) */
                              } /* END OF if ((((((xj > xsuivni) && ...*/
                           } /* END OF for (k = 1 ; k <= yk1 ; k++) */
                       } /* END OF if ((((xl != ordre (suivnj,g)) ... */
                    } /* END OF if (((xj > xi) && ((xl < xi) ||... */
                 } /* END OF if ((!((l = ( yk1+1 )) && ... */
              } /* END OF for (l = 1 ; l <= ( 2*yk1+1 ) ; l++) */
           } /* END OF if ((xj != xi) && ... */
        } /* END OF for (j = 1 ; (((j <= xk1)... */
    } /* END OF for (i = 1 ; i <= xk1 ; i++) */
   } /* END ELSE OF if (xk1 <= 2) */
 flag->mdelta = delta;
 if (flag->modifie)
 {
 px = (tourneelem *) malloc(sizeof(tourneelem));
 demand(px,memory overflow);
 g[x].ptrtourne = px;
 px->noeud = noeuds.x;
 t->nbredenoeuds++;
 t->noeudinterne[noeuds.x] = 1;
 switch (noeuds.typeajout)
       {
         case 1 :
                 chemin ( noeuds.i, noeuds.x, g );
                 chemin ( noeuds.x, noeuds.j, g );
                 inverse( noeuds.j, noeuds.si, g );
                 chemin ( noeuds.si, noeuds.l, g );
                 inverse( noeuds.l, noeuds.sj, g );
                 chemin ( noeuds.sj, noeuds.sl, g );
                 break;
         case 2 :
                 chemin ( noeuds.i, noeuds.x, g );
                 chemin ( noeuds.x, noeuds.j, g );
                 inverse( noeuds.j, noeuds.k, g );
                 chemin ( noeuds.k, noeuds.sj,g );
                 chemin ( noeuds.pl, noeuds.pk, g );
                 inverse( noeuds.pk, noeuds.si, g );
                 chemin ( noeuds.si, noeuds.l, g );
                 break;
         case 3 :  /* inverse du type 1 */
                 chemin ( noeuds.pl, noeuds.pi, g );
                 inverse( noeuds.pi, noeuds.l, g );
                 chemin ( noeuds.l, noeuds.pj, g );
                 inverse( noeuds.pj, noeuds.i, g );
                 chemin ( noeuds.i, noeuds.x , g );
                 chemin ( noeuds.x, noeuds.j , g );
                 break;
         case 4 :  /* inverse du type 2 */
                 chemin ( noeuds.l,noeuds.pj, g );
                 inverse( noeuds.pj,noeuds.sk, g );
                 chemin ( noeuds.sk,noeuds.sl, g );
                 chemin ( noeuds.pi,noeuds.k, g );
                 inverse( noeuds.k, noeuds.i, g );
                 chemin ( noeuds.i,noeuds.x, g );
                 chemin ( noeuds.x, noeuds.j, g );
                 break;
         case 5 :
                 chemin ( noeuds.i, noeuds.x, g );
                 chemin ( noeuds.x, noeuds.j, g );
                 break;
        }
 numerote_tourne(t);
 } /* END OF  if (flag->modifie) */

} /* END OF AJOUTX */


/******************************************************************/

chemin (n1,n2,g)

 int n1, n2;
 coord g[];

{

 /* THIS PROCEDURE CHANGE THE ORIENTATION OF THE POINTERS
    OF THE NODE n1 WITH THE NODE n2 AND VICE-VERSA. */

 g[n1].ptrtourne->prochain = g[n2].ptrtourne;
 g[n2].ptrtourne->precedent = g[n1].ptrtourne;

} /* END OF CHEMIN  */

/********************************************************************/

inverse (depart,arrive,g)

 int depart, arrive;
 coord g[];

{

 /* THIS PROCEDURE REVERSE THE PATH FROM DEPARTURE TO ARRIVAL. */

 tourneelem *d, *a, *p, *pp;
 int lng;

 cptlng = cptlng + 1;
 lng = 0;
 a = g[depart].ptrtourne;
 d = g[arrive].ptrtourne;
 p = d->prochain;
 while (d != a)
      {
       lng = lng + 1;
       pp = p->prochain;
       p->prochain = d;
       d->precedent = p;
       d = p;
       p = pp;
      }
 lngtotal = lngtotal + lng;

} /* END OF INVERSE */

/********************************************************************/


int etime(int i)
{
 clock_t times();
 struct tms buffer;

 if (i) {
   times(&buffer);
   elap=buffer.tms_utime;
   return(elap);
 }
 else {
   times(&buffer);
   return(buffer.tms_utime-elap);
 }
}

/********************************************************************/

oterx (x,neigh,t,g,dtp,flag)

 int x;

 neighbour *neigh;

 tourne *t;

 coord g[];

 int dtp[MAXN+1][MAXN+1];

 tflag *flag;

{

 /* THIS PROCEDURE IS THE OPOSITE OPERATION OF THE PROCEDURE
    ajoutx.
    THE RESULT IS IN t AND IS UPDATED LIKE THE K1 NEAREST
    NEIGHBOORS.   */

 struct oper {
              int x, i, j, k, l, sj, sk, sl,
                  pj, pk, pl, i2,
                  typeretrait;
             } noeuds;

 int ni, ni2, ni3, nj, nk, nl, j, k, l, xi, xj, xk, xl, xi2,
     suivnj, suivnl, suivnk, precnj, precnl, precnk, som1, som2,
     vu_ni2, vu_ni3,
     xk1, o, delta, nouvdelta;

 xk1 = mini (k1 , t->nbredenoeuds - 1);
/*printf("\n xk1==%d",xk1);*/
 delta = MAXDIST;
 if (t->ptr->noeud == x)   {
    t->ptr = g[g[x].ptrtourne->prochain->noeud].ptrtourne;
    numerote_tourne ( t );
   } /* END OF if (t->ptr->noeud == x) */
 if (xk1 <= 2){
/* 	printf("entrou em xk1<=2");*/
 	ni = prec (x , g);
 	nj = suiv (x , g);

 	if (xk1 == 1) nouvdelta = - dtp[ni][x];
 	else nouvdelta = dtp[ni][nj] - dtp[ni][x] - dtp[x][nj];
 	if (nouvdelta <= delta) {
    		delta = nouvdelta;
    		noeuds.typeretrait = 5;
    		noeuds.x = x;
    		noeuds.i = ni;
    		noeuds.j = nj;
   	}
 } /* END THEN OF if (xk1 <= 2) */
 else
 {
 ni = prec ( x, g );
 xi = ordre ( ni, g );
 ni2 = suiv ( suiv ( ni, g ), g );
 xi2 = ordre ( ni2, g );
 if (ni2 == x)
   vu_ni2 = 1; /* TRUE */
 else vu_ni2 = 0; /* FALSE */
 for (j =1 ; ((j <= xk1) || (!vu_ni2)) ; j++)
    {
     if ( j > xk1 )
       {
        nj = ni2;
	vu_ni2 = 1; /* TRUE */
       }
     else
       {
	/*printf("\nneigh->p1[%i].nn[%i]==%d",ni,j,neigh->p1[ni].nn[j]);*/
        nj = neigh->p1[ni].nn[j];
        if (nj < 0) nj = 0;
       }
     if (nj != x)
       {
        suivnj = suiv ( nj, g );
	/*printf("\nsuivnj==%d", suivnj);
	printf("\nnj==%d",nj);*/
        xj  = ordre ( nj, g );
        ni3 = suiv(ni2,g);
        if ((ni3 == ni) || (ni3 == nj) || (ni3 == x))
          vu_ni3 = 1; /* TRUE */
        else vu_ni3 = 0; /* FALSE */
        for (l = 1 ; ((l <= xk1) || (!vu_ni3)) ; l++)
           {
            if ( l > xk1 )
              {
               nl = ni3;
               vu_ni3 = 1; /* TRUE */
              }
           else
              {
               nl = neigh->p1[ni2].nn[l];
               if (nl < 0) nl = 0;
              }
           if (!((nl == ni) || (nl == nj) || (nl == x)))
             {
              suivnl = suiv ( nl, g );
              precnl = prec ( nl, g );
              xl = ordre ( nl, g );
              if ((( xj > xi ) && (( xl > xj ) || ( xl < xi ))) ||
                 (( xj < xi ) && ( xl > xj ) && ( xl < xi )))
                {
                 nouvdelta = dtp[ni][nj]+dtp[ni2][nl]+dtp[suivnj][suivnl]-
                             dtp[ni][x] - dtp[x][ni2] - dtp[nj][suivnj] -
                             dtp[nl][suivnl];
                 som1 = nj;
                 while (som1 != ni2)
                      {
                       som2 = prec (som1,g);
                       nouvdelta = nouvdelta - dtp[som2][som1] +
                                               dtp[som1][som2];
                       som1 = som2;
                      }
                 som1 = nl;
                 while (som1 != suivnj)
                      {
                       som2 = prec (som1,g);
                       nouvdelta = nouvdelta - dtp[som2][som1] +
                                                dtp[som1][som2];
                       som1 = som2;
                      }
                 delta = mini (delta, nouvdelta );
                 if (delta == nouvdelta)
                   {
                    noeuds.typeretrait = 1;
                    noeuds.i = ni;
                    noeuds.x = x;
                    noeuds.j = nj;
                    noeuds.i2 = ni2;
                    noeuds.sj = suivnj;
                    noeuds.sl = suivnl;
                    noeuds.l = nl;
                   }
                } /* END THEN OF if ((( xj > xi ) && (( xl > xj )... */
              else
                {
                 for (k = 1 ; k <= xk1 ; k++)
                    {
                     nk = neigh->p2[suivnj].nn[k];
/*		     printf("\n neigh->p2[%i].nn[%i]==%d", suivnj,k,neigh->p2[suivnj].nn[k]);*/
                     if (nk < 0) nk = 0;
                     xk = ordre ( nk, g );
                     suivnk = suiv ( nk, g );
                     if (((xj > xl) && (xk < xj) && (xk >= xl)) ||
                        ((xj < xl)  && ((xk < xj) || (xk >= xl))))
                       {
                        nouvdelta = dtp[ni][nj] + dtp[suivnk][precnl] +
                                    dtp[ni2][nl] + dtp[nk][suivnj] -
                                    dtp[ni][x] - dtp[x][ni2] -
                                    dtp[precnl][nl] - dtp[nk][suivnk] -
                                    dtp[nj][suivnj];
                        som1 = nj;
                        while (som1 != suivnk)
                             {
                              som2 = prec (som1,g);
                              nouvdelta = nouvdelta -
                                          dtp[som2][som1]+dtp[som1][som2];
                              som1 = som2;
                             }
                        som1 = precnl;
                        while (som1 != ni2)
                             {
                              som2 = prec (som1,g);
                              nouvdelta = nouvdelta -
                                          dtp[som2][som1]+dtp[som1][som2];
                              som1 = som2;
                             }
                        delta = mini (delta, nouvdelta );
                        if (delta == nouvdelta)
                          {
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
                       } /* END OF if (((xj > xl) && (xk < xj) && ... */
                    } /* END OF for (k = 1 ; k <= xk1 ; k++) */
                } /* END ELSE OF if ((( xj > xi ) && (( xl > xj )... */
             } /* END OF if (!((nl==ni) || (nl==nj) || (nl==x))) */
           } /* END OF for (l = 1;((l <= xk1) || (!vu_ni3));l++) */
       } /* END OF if (nj != x) */
    } /* END OF for (j =1 ; ((j <= xk1) || (!vu_ni2)) ; j++) */

 for (j = 1 ; j <= xk1 ; j++)
    {
     nj = neigh->p2[ni2].nn[j];
     if (nj < 0) nj = 0;
     if (nj != x)
       {
        suivnj = suiv ( nj, g );
        precnj = prec ( nj, g );
        xj = ordre ( nj,g );
        for (l = 1 ; l <= xk1 ; l++)
           {
            nl = neigh->p2[ni].nn[l];
            if (nl < 0) nl = 0;
            suivnl = suiv ( nl, g );
            precnl = prec ( nl, g );
            xl = ordre ( nl, g );
            if (!((nl == ni2) || (nl == nj) || (nl == x)))
              {
               if (((xi2 > xj) && ((xl > xi2) || (xl < xj))) ||
                  ((xi2 < xj) && (xl > xi2) && (xl < xj )))
                 {
                  nouvdelta = dtp[nj][ni2]+dtp[nl][ni]+dtp[precnl][precnj]-
                              dtp[ni][x] - dtp[x][ni2] - dtp[precnj][nj] -
                              dtp[precnl][nl];
                  som1 = ni;
                  while (som1 != nj)
                       {
                        som2 = prec (som1,g);
                        nouvdelta = nouvdelta - dtp[som2][som1] +
                                                dtp[som1][som2];
                        som1 = som2;
                       }
                  som1 = precnj;
                  while (som1 != nl)
                       {
                        som2 = prec(som1,g);
                        nouvdelta = nouvdelta - dtp[som2][som1] +
                                                dtp[som1][som2];
                        som1 = som2;
                       }
                  delta = mini (delta,nouvdelta );
                  if (delta == nouvdelta)
                    {
                     noeuds.typeretrait = 3;
                     noeuds.i2 = ni2;
                     noeuds.j = nj;
                     noeuds.i = ni;
                     noeuds.l = nl;
                     noeuds.pj = precnj;
                     noeuds.pl = precnl;
                     noeuds.x = x;
                    }
                 } /* END THEN OF if (((xi2>xj) && ((xl>xi2) ||... */
               else
                 {
                  for (k = 1 ; k <= xk1 ; k++)
                     {
                      nk = neigh->p1[precnj].nn[k];
                      if (nk < 0) nk = 0;
                      suivnk = suiv ( nk, g );
                      precnk = prec ( nk,g );
                      xk = ordre ( nk, g );
                      if (((xl>xj) && (xk >xj) && (xk <= xl)) ||
                         ((xl<xj) && ((xk>xj) || (xk<=xl))))
                        {
                         nouvdelta = dtp[nj][ni2] + dtp[suivnl][precnk] +
                                     dtp[nl][ni] + dtp[precnj][nk] -
                                     dtp[ni][x] -
                                     dtp[x][ni2] - dtp[nl][suivnl] -
                                     dtp[precnk][nk] - dtp[precnj][nj];
                         som1 = ni;
                         while (som1 != suivnl)
                              {
                               som2 = prec(som1,g);
                               nouvdelta = nouvdelta -
                                           dtp[som2][som1]+dtp[som1][som2];
                               som1 = som2;
                              }
                         som1 = precnk;
                         while (som1 != nj)
                              {
                               som2 = prec(som1,g);
                               nouvdelta = nouvdelta -
                                           dtp[som2][som1]+dtp[som1][som2];
                               som1 = som2;
                              }
                         delta = mini (delta, nouvdelta );
                         if (delta == nouvdelta)
                           {
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
                        } /* END OF if (((xl>xj) && (xk >xj) && ... */
                     } /* END OF for (k = 1 ; k <= xk1 ; k++) */
                 } /* END ELSE OF if (((xi2>xj) && ((xl>xi2) ||... */
              } /* END OF if (!((nl==ni2) || (nl==nj) || (nl==x))) */
           } /* END OF for (l = 1 ; l <= xk1 ; l++) */
       } /* END OF if (nj != x) */
    } /* END OF for (j = 1 ; j <= xk1 ; j++) */
 } /* END ELSE OF if (xk1 <= 2) */

 /* THE FOLLOWING PROCEDURE IS SIMILAR TO THAT IN THE ajoutx.
    IT PERFORMS THE UNASSIGNMENT OF THE x NODE IN t AND HERE
    IT'S DONNE THE RENUMERATION OF THE ROUTE */

 flag->mdelta = delta;
 if (flag->modifie)
 {
 free (g[x].ptrtourne);
 demand(1,problem in freeing memory);
 t->nbredenoeuds--;
 t->noeudinterne[x] = 0;
 switch (noeuds.typeretrait)
       {
         case 1 :
                 chemin ( noeuds.i, noeuds.j, g );
                 inverse( noeuds.j, noeuds.i2, g );
                 chemin ( noeuds.i2, noeuds.l, g );
                 inverse( noeuds.l, noeuds.sj, g );
                 chemin ( noeuds.sj, noeuds.sl, g );
                 break;
         case 2 :
                 chemin ( noeuds.i, noeuds.j, g );
                 inverse( noeuds.j, noeuds.sk, g );
                 chemin ( noeuds.sk,noeuds.pl, g );
                 inverse( noeuds.pl, noeuds.i2, g );
                 chemin ( noeuds.i2, noeuds.l, g );
                 chemin ( noeuds.k, noeuds.sj, g );
                 break;
         case 3 :  /* inverse du type 1 */
                 inverse( noeuds.i, noeuds.j, g );
                 chemin ( noeuds.j, noeuds.i2, g );
                 chemin ( noeuds.pl,noeuds.pj, g );
                 inverse( noeuds.pj,noeuds.l,  g );
                 chemin ( noeuds.l, noeuds.i, g );
                 break;
         case 4 :  /* inverse du type 2 */
                 chemin  ( noeuds.pj,noeuds.k, g );
                 chemin  ( noeuds.l, noeuds.i, g );
                 inverse ( noeuds.i,noeuds.sl, g );
                 chemin  ( noeuds.sl,noeuds.pk, g );
                 inverse ( noeuds.pk, noeuds.j ,g );
                 chemin  ( noeuds.j, noeuds.i2, g );
                 break;
         case 5 :
                 chemin  ( noeuds.i,noeuds.j, g );
                 break;
        }
 numerote_tourne(t);
 } /* END OF if (flag->modifie) */

} /* END OF OTERX */


/******************************************************************/

copietourne(ts,g,td,g2,ndepotc)

 coord g[], g2[];

 tourne *ts, *td;

 int ndepotc;

{

 /* THIS PROCEDURE COPIES THE ROUTE t OF g IN t2 OF g2.
    NOTE THAT THIS PROCEDURE ONLY WORKS IF THE NUMBER OF
    ASSIGNED NODES IS EQUAL TO n.  */

 int i;

 tourneelem *pt;

 for (i = 0 ; i <= MAXN ; i++)
    {
     g2[i].satourne = g[i].satourne;
     g2[i].ptrtourne = NULL;
     td->noeudinterne[i] = ts->noeudinterne[i];
    }
 for (i = 1 ; i <= (n+np) ; i++)
    {
     if (g2[i].ptrtourne == NULL)
       {
        g2[i].ptrtourne = (tourneelem *) malloc(sizeof(tourneelem));
        demand(g2[i].ptrtourne,memory overflow);
       }
    }
 for (i = 1 ; i <= (n+np) ; i++)
    {
     pt = g2[i].ptrtourne;
     pt->noeud = i;
     pt->prochain = g2[g[i].ptrtourne->prochain->noeud].ptrtourne;
     pt->precedent = g2[g[i].ptrtourne->precedent->noeud].ptrtourne;
    }
 td->nbredenoeuds = ts->nbredenoeuds;
 td->ptr = g2[ndepotc].ptrtourne;
 numerote_tourne (td);

} /* END OF COPIETOURNE */

/********************************************************************/

store_best_solution(g,depot,bg,bsolution,makespan,neigh,bneigh)

 coord g[], bg[];

 tdepot *depot, *bsolution;

 makes *makespan;

 neighbour neigh[], bneigh[];

{

 /* THIS PROCEDURE COPIES THE ROUTE t OF g IN t2 OF g2.
    NOTE THAT THIS PROCEDURE ONLY WORKS IF THE NUMBER OF
    ASSIGNED NODES IS EQUAL TO n.  */

 int i1,j1,maxmake;

 tourneelem *pt;

 for (i1 = 0 ; i1 <= MAXN ; i1++)
    {
     bg[i1].satourne = g[i1].satourne;
     bg[i1].ptrtourne = NULL;
     for (j1 = 1 ; j1 <= np ; j1++)
        bsolution->tabt[j1].noeudinterne[i1] =
        depot->tabt[j1].noeudinterne[i1];
    }
 for (i1 = 1 ; i1 <= (n+np) ; i1++)
    {
     if (bg[i1].ptrtourne == NULL)
       {
        bg[i1].ptrtourne = (tourneelem *) malloc(sizeof(tourneelem));
        demand(bg[i1].ptrtourne,memory overflow);
       }
    }
 for (i1 = 1 ; i1 <= (n+np) ; i1++)
    {
     pt = bg[i1].ptrtourne;
     pt->noeud = i1;
     pt->prochain = bg[g[i1].ptrtourne->prochain->noeud].ptrtourne;
     pt->precedent = bg[g[i1].ptrtourne->precedent->noeud].ptrtourne;
    }
 maxmake = 0;
 for (i1 = 1 ; i1 <= np ; i1++)
    {
     bsolution->tabt[i1].nbredenoeuds = depot->tabt[i1].nbredenoeuds;
     bsolution->tabt[i1].ptr = bg[n+i1].ptrtourne;
     bsolution->nodepot[i1] = depot->nodepot[i1];
     bsolution->load[i1] = depot->load[i1];
     if (bsolution->load[i1] >= maxmake)
       {
        maxmake = bsolution->load[i1];
        makespan->load = maxmake;
        makespan->proc = i1;
       }
     numerote_tourne (&bsolution->tabt[i1]);
    }
 for (i1 = 1 ; i1 <= np ; i1++)
    {
     for (j1 = 1 ; j1 <= (n + np) ; j1++)
        {
         bneigh[i1].p1[j1] = neigh[i1].p1[j1];
	/* printf("\nbneigh[%i].p1[%i]==%d", i1,j1, bneigh[i1].p1[j1]);*/
         bneigh[i1].p2[j1] = neigh[i1].p2[j1];
	/* printf("\nbneigh[%i].p2[%i]==%d", i1,j1, bneigh[i1].p2[j1]);*/
        }
    }

} /* END OF STORE_BEST_SOLUTION */

/********************************************************************/

seed_generation()

{

 struct tm *tempo;
 long lt;
 double drand48();
 double log();
 int seed;


     time(&lt);
     tempo = localtime(&lt);
     seed = (tempo->tm_sec + tempo->tm_min + tempo->tm_hour +
             tempo->tm_mday + tempo->tm_mon + tempo->tm_year +
             tempo->tm_yday) * 132;
     srand48(seed);

} /* END OF SEED_GENERATION */
/********************************************************************/
