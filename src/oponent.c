#include "oponent.h"

// Falta paso por parametro
#define NUM_ARGUMENTS 11
int const DF[] = {-1,0,1,0};
int const DC[] = {0,-1,0,1};

pthread_t threadGestorMsg, threadTimerRastre;
dades_oponent_t dades_opo;

int semafor_pantalla, retard;

pthread_mutex_t mutex_gestor_jugador = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char *argv[]) {
	if(inicialitzaDadesOponent(argc, &(*argv), &dades_opo) == SUCCESS) {
		setbuf(stdout, NULL);
		pthread_create(&threadGestorMsg, NULL, gestor_msg, NULL);
		win_set(dades_opo.p_win, dades_opo.n_fil, dades_opo.n_col);
		jocOponent(&dades_opo);
		pthread_join(threadGestorMsg, NULL);
		free(dades_opo.p_opo);
	}

	pthread_mutex_destroy(&mutex_gestor_jugador);
	return 0;
}

void *gestor_msg(void *nul) {
	char msg[LEN_MISSATGE];
	char origen, accio, oponent;
	FILE *fp;

	receiveM(dades_opo.bustia_recepcio, msg);
	llegeixMissatge(msg, &origen, &accio, &oponent);
	while(accio != FINALITZA_PARTIDA){
		pthread_mutex_lock(&mutex_gestor_jugador);
		// EXECUTA EL RASTRE NO_INV
		if(accio == COLISIONA_INVERS) {
			if(dades_opo.timer_estat == 0) {
				/* SI EL TIMER ESTA DISPONIBLE L'EXECUTA */
				dades_opo.timer_estat = 1;
				pthread_create(&threadTimerRastre, NULL, timerRastre, NULL);
			} else if(dades_opo.timer_estat == -1) {
				/* SI EL TIMER HA FINALITZAT EL TORNA A EXECUTAR */
				dades_opo.timer_estat = 0;
				pthread_join(threadTimerRastre, NULL);

				pthread_create(&threadTimerRastre, NULL, timerRastre, NULL);
			}

		} else if(accio == COLISIONA_NO_INVERS) {
			esborrar_posicions(dades_opo.num_oponent+'0',
					dades_opo.p_opo, dades_opo.n_opo);
			dades_opo.fiOponent = 1;
		}

		pthread_mutex_unlock(&mutex_gestor_jugador);

		// COMPROVA SI ES IMPACTE NO_INVERS EL REBUT PER ACABAR
		receiveM(dades_opo.bustia_recepcio, msg);
		llegeixMissatge(msg, &origen, &accio, &oponent);
	}

	pthread_mutex_lock(&mutex_gestor_jugador);
	dades_opo.fiOponent = 1;
	pthread_mutex_unlock(&mutex_gestor_jugador);

	waitS(semafor_pantalla);
	fp = fopen(dades_opo.fitxer, "a");
	fprintf(fp, "El jugador %c ha escrit %i caracters\n", dades_opo.num_oponent+'0', dades_opo.n_opo);
	fclose(fp);
	signalS(semafor_pantalla);

	/* NECESARI PER SINCRONITZAR SORTIDA AMB GESTOR PRINCIPAL */
	creaMissatge(msg, dades_opo.num_oponent+'0', JUGADOR_FINALITZA, GESTOR);
	sendM(dades_opo.bustia_ingres, msg, LEN_MISSATGE);

	// No fiquem mutex perque pot bloquejarse infinitament
	if(dades_opo.timer_estat != 0) {
		pthread_join(threadTimerRastre, NULL);
	}

	pthread_exit(NULL);
}

int jocOponent() {
	char cars, msg[LEN_MISSATGE];
	int k, vk, nd, vd[3], canvi;
	tron seg;

	creaMissatge(msg, '0'+dades_opo.num_oponent, 	JUGADOR_PREPARAT, NUL);
	sendM(dades_opo.bustia_ingres, msg, LEN_MISSATGE);
	inicialitzaJocOponent(&dades_opo);

	/* INICI SECCIO CRITICA THREADS LOCALS */
	pthread_mutex_lock(&mutex_gestor_jugador);
	while (dades_opo.fiOponent == 0) {
		canvi = 0;
		seg.f = dades_opo.opo.f + DF[dades_opo.opo.d]; /* calcular seguent posicio */
		seg.c = dades_opo.opo.c + DC[dades_opo.opo.d];

		/* INICI SECCIO CRITICA CURSES */
		waitS(semafor_pantalla);
		cars = win_quincar(seg.f, seg.c); /* calcula caracter seguent posicio */

		/* Evita xoc amb  oponents inversos o ell mateix en forma no inversa */
		if ((cars == ('0'+dades_opo.num_oponent)) || (win_quinatri(seg.f, seg.c) != 0)) { /* si seguent posicio ocupada */
			canvi = 1; /* anotar que s'ha de produir un canvi de direccio */
		} else
			if (dades_opo.variacio > 0) /* si hi ha variabilitat */ {
				k = rand() % 10; /* prova un numero aleatori del 0 al 9 */
				if (k < dades_opo.variacio) canvi = 1; /* possible canvi de direccio */
			}

		if (canvi) /* si s'ha de canviar de direccio */ {
			nd = 0;
			for (k = -1; k <= 1; k++) /* provar direccio actual i dir. veines */ {
				vk = (dades_opo.opo.d + k) % 4; /* nova direccio */
				if (vk < 0) vk += 4; /* corregeix negatius */
				seg.f = dades_opo.opo.f + DF[vk]; /* calcular posicio en la nova dir.*/
				seg.c = dades_opo.opo.c + DC[vk];

				cars = win_quincar(seg.f, seg.c); /* calcula caracter seguent posicio */
				if ((cars == ' ') || ((win_quinatri(seg.f, seg.c) == 0) &&
							(cars != ((char)dades_opo.num_oponent+'0')))) {
					vd[nd] = vk; /* memoritza com a direccio possible */
					nd++; /* anota una direccio possible mes */
				}
			}
			if (nd == 0) {
				/* SEGUEIX EL CAMI QUE TENIA PREVIST */
				seg.f = dades_opo.opo.f + DF[dades_opo.opo.d];
				seg.c = dades_opo.opo.c + DC[dades_opo.opo.d];

				cars = win_quincar(seg.f, seg.c);
				if((cars == '+') || (cars == (dades_opo.num_oponent+'0'))) {
					/* COLISIONA AMB PARET, JUGADOR TE QUE MORIR */
					creaMissatge(msg, dades_opo.num_oponent+'0', COLISIONA_PARET, NUL);
				} else {
					/* COLISIONA AMB OPONENT INVERS, JUGADOR TE QUE MORIR */
					creaMissatge(msg, dades_opo.num_oponent+'0', COLISIONA_INVERS, cars);
				}
				/* FALTA COMPROBAR SI COLISIONA MAB ELL MATEIX ((COLISIONA_PARET)) */
				/* EN CAS DE COLISIONAR AMB ELEMENT INVERS */
				signalS(semafor_pantalla);
				esborrar_posicions(dades_opo.num_oponent+'0', dades_opo.p_opo, dades_opo.n_opo);
				waitS(semafor_pantalla);
				dades_opo.fiOponent = 1;

				sendM(dades_opo.bustia_ingres, msg, LEN_MISSATGE);
			} else {
				if (nd == 1) /* si nomes pot en una direccio */
					dades_opo.opo.d = vd[0]; /* li assigna aquesta */
				else /* altrament */
					dades_opo.opo.d = vd[rand() % nd]; /* segueix una dir. aleatoria */
			}
		}

		if (dades_opo.fiOponent == 0) /* si no ha col.lisionat amb res */ {
			dades_opo.opo.f = dades_opo.opo.f + DF[dades_opo.opo.d]; /* actualitza posicio */
			dades_opo.opo.c = dades_opo.opo.c + DC[dades_opo.opo.d];

			cars = win_quincar(dades_opo.opo.f, dades_opo.opo.c);
			if(cars != ' ') {
				creaMissatge(msg, dades_opo.num_oponent+'0', COLISIONA_NO_INVERS, cars);
				sendM(dades_opo.bustia_ingres, msg, LEN_MISSATGE);
			}

			win_escricar(dades_opo.opo.f, dades_opo.opo.c, '0' + dades_opo.num_oponent, dades_opo.t_escrit); /* dibuixa bloc oponent */
			/* FI SECCIO CRITICA CURSES */

			dades_opo.p_opo[dades_opo.n_opo].f = dades_opo.opo.f; /* memoritza posicio actual */
			dades_opo.p_opo[dades_opo.n_opo].c = dades_opo.opo.c;
			(dades_opo.n_opo)++;
		}

		/* POT MOURE AQUI */
		signalS(semafor_pantalla);
		pthread_mutex_unlock(&mutex_gestor_jugador);
		win_retard(dades_opo.retard);
		/* INICI SECCIO CRITICA THREADS LOCALS */
		/* Deixa mutex bloquejat per realitzar comparacio while */
		pthread_mutex_lock(&mutex_gestor_jugador);
	}

	// Desbloqueja el thread ja que estaba bloquejat per la comparacio
	// del while
	pthread_mutex_unlock(&mutex_gestor_jugador);
	/* FI SECCIO CRITICA THREADS LOCALS */
	// Finalitza thread, oponent ha finalitzat
	return 0;
}

void *timerRastre(void *nul) {
	int fiOponentLocal, i = 0;

	// Canvia tipus escritura
	pthread_mutex_lock(&mutex_gestor_jugador);
	dades_opo.t_escrit = NO_INV;
	pthread_mutex_unlock(&mutex_gestor_jugador);

	do {
		sleep(1);
		i++;

		pthread_mutex_lock(&mutex_gestor_jugador);
		fiOponentLocal = dades_opo.fiOponent;
		pthread_mutex_unlock(&mutex_gestor_jugador);
	} while((i < 10) && (fiOponentLocal == 0));

	// Retorna tipus de escritura al valor per defecte
	// i indica a la variable timer_estat que ha finalitzat
	pthread_mutex_lock(&mutex_gestor_jugador);
	dades_opo.timer_estat = -1;
	dades_opo.t_escrit = INVERS;
	pthread_mutex_unlock(&mutex_gestor_jugador);

	pthread_exit(NULL);
}

int inicialitzaDadesOponent(int argc, char *argv[], dades_oponent_t *dades_opo) {
	if(argc == NUM_ARGUMENTS) {
		dades_opo->timer_estat = 0;
		dades_opo->fiOponent = 0;
		dades_opo->num_oponent = atoi(argv[1]);
		dades_opo->id_pantalla = atoi(argv[2]);
		dades_opo->n_fil = atoi(argv[3]);
		dades_opo->n_col = atoi(argv[4]);
		// Retard passat per linia de comandes
		dades_opo->retard = atoi(argv[5]);

		dades_opo->variacio = atoi(argv[6]);
		// Bustia de recepcio procedent del proces pare
		dades_opo->bustia_recepcio = atoi(argv[7]);
		// Rep id de la bustia per enviar missatges proces pare
		dades_opo->bustia_ingres = atoi(argv[8]);
		// Rep id del semafor per seccio critica
		semafor_pantalla = atoi(argv[9]);
		sprintf(dades_opo->fitxer, "%s", argv[10]);

		srand(getpid());
		dades_opo->t_escrit = INVERS;
		dades_opo->p_win = map_mem(dades_opo->id_pantalla);
		dades_opo->p_opo = calloc((dades_opo->n_fil*dades_opo->n_col)/2, sizeof(pos));
		if(dades_opo->p_opo != NULL) {
			return SUCCESS;
		} else {
			return ERROR;
		}
	} else {
		return ERROR;
	}
}

void inicialitzaJocOponent(dades_oponent_t *dades_opo) {
	dades_opo->opo.f = (dades_opo->n_fil - 1) / (dades_opo->num_oponent + 4);
	dades_opo->opo.c = (dades_opo->n_col * 3) / (dades_opo->num_oponent +4);
	dades_opo->opo.d = rand() % 4;
	/* Seccio critica */
	waitS(semafor_pantalla);
	win_escricar(dades_opo->opo.f, dades_opo->opo.c,
			'0'+dades_opo->num_oponent, dades_opo->t_escrit);
	signalS(semafor_pantalla);
	/* Fi seccio critica */
	dades_opo->p_opo[0].f = dades_opo->opo.f;
	dades_opo->p_opo[0].c = dades_opo->opo.c;
	dades_opo->n_opo = 1;
}

void esborrar_posicions(char car_tron, pos p_pos[], int n_pos) {
	int i;

	//fprintf(fitxer, "%s %s tron acabat %c: %d\n", dia, hora, car_tron, n_pos);
	for (i = n_pos - 1; i >= 0; i--) /* de l'ultima cap a la primera */ {
		// Si forma part del rastre esborra
		/* Inici seccio critica */
		waitS(semafor_pantalla);
		if(win_quincar(p_pos[i].f, p_pos[i].c) == car_tron) {
			win_escricar(p_pos[i].f, p_pos[i].c, ' ', NO_INV); /* esborra una pos. */
		}
		signalS(semafor_pantalla);
		/* Fi seccio critica */

		win_retard(10);
	}
}
