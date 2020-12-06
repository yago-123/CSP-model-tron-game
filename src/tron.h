#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<unistd.h>
#include<time.h>
#include<string.h>
#include<sys/wait.h>
#include<sys/types.h>

#include"../lib/winsuport2.h"
#include"../lib/memoria.h"
#include"../lib/semafor.h"
#include"../lib/missatge.h"
#include"codis.h"

#define MAX_OPONENTS 8
#define MAX_JUGADORS (MAX_OPONENTS+1)

typedef struct {
	int f;
	int c;
	int d;
} tron;

typedef struct {
	int f;
	int c;
} pos;

typedef struct {
	int t_escrit, n_usu;
	int fiUsuari, timer_estat;
	tron usu;
	pos *p_usu;
} dades_usuari_t;

typedef struct {
	int n_thr, varia, retard;
	char fitxer[25];
} arg_comanda_t;

void *gestor_msg_principal(void *nul);
void *gestor_usuari(void *nul);
void *mou_usuari(void *nul);
void *timerRastre(void *nul);
void *mou_oponent(void *nul);

void gestionaArgumentsJoc(int argc, char *argv[], arg_comanda_t *dades_arg);
void incialitzaTaulellJoc(int *n_fil, int *n_col, int *id_pantalla);
void inicialitzaJocUsuari(dades_usuari_t *dades_usu);
void inicialitzaBusties(int *bustia_recepcio, int bustia_jugador[], int n_thr);
int inicialitzaDadesUsuari(dades_usuari_t *dades_usu);
void esborrar_posicions(char car_tron, pos p_pos[], int n_pos);
void calculaTemps(char temps[], time_t inici);
