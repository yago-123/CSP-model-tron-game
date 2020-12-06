#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>
#include<time.h>
#include<sys/types.h>

#include"../lib/winsuport2.h"
#include"../lib/memoria.h"
#include"../lib/semafor.h"
#include"../lib/missatge.h"

#include"codis.h"

#define SUCCESS 0
#define ERROR 1

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
	int t_escrit, bustia_recepcio, bustia_ingres;
	int num_oponent, id_pantalla, n_opo;
	int n_fil, n_col, *p_win, timer_estat;
	int fiOponent, variacio, retard;
	
	char fitxer[25]; 

	tron opo;
	pos *p_opo;
} dades_oponent_t;

void *gestor_msg(void *nul);

int jocOponent();
void* timerRastre(void *nul);
int inicialitzaDadesOponent(int argc, char *argv[], dades_oponent_t *dades_opo);
void inicialitzaJocOponent(dades_oponent_t *dades_opo);
void esborrar_posicions(char car_tron, pos p_pos[], int n_pos);
