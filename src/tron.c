#include"tron.h"

int const DF[] = {-1,0,1,0};
int const DC[] = {0,-1,0,1};

arg_comanda_t dades_arg;
pid_t tpid[MAX_OPONENTS];
int bustia_recepcio, bustia_ingres[MAX_JUGADORS];

time_t inici;

pthread_t threadUsuari, threadOponents, threadGestorMsg;
pthread_t threadGestorUsuari, threadTimerRastre;

// Mutex necesari per protegir entre usuari/gestor missatges del usuari
pthread_mutex_t mutex_gestor_usuari = PTHREAD_MUTEX_INITIALIZER;

int n_fil, n_col;
int id_pantalla, semafor_pantalla;

int acaba_partida = 0;

dades_usuari_t dades_usu;

int main(int argc, char *argv[]) {
	char temps[60];
	int *p_win, i;

	gestionaArgumentsJoc(argc, &(*argv), &dades_arg);
	inicialitzaBusties(&bustia_recepcio, bustia_ingres, dades_arg.n_thr);
	incialitzaTaulellJoc(&n_fil, &n_col, &id_pantalla);

	// Acces pantalla pare
	p_win = map_mem(id_pantalla);
	win_set(p_win, n_fil, n_col);

	// Incialitzacio semafor, fa mateixa funcio mutex
	semafor_pantalla = ini_sem(0);

	pthread_create(&threadGestorMsg, NULL, gestor_msg_principal, NULL);
	pthread_create(&threadGestorUsuari, NULL, gestor_usuari, NULL);
	pthread_create(&threadUsuari, NULL, mou_usuari, NULL);
	pthread_create(&threadOponents, NULL, mou_oponent, NULL);

	time(&inici);
	while(acaba_partida == 0) {
		calculaTemps(temps, inici);

		waitS(semafor_pantalla);
		win_escristr(temps);
		win_update();
		signalS(semafor_pantalla);

		win_retard(20);
	}

	pthread_join(threadUsuari, NULL);
	pthread_join(threadOponents, NULL);
	pthread_join(threadGestorUsuari, NULL);
	pthread_join(threadGestorMsg, NULL);

	// tanca pantalla i allibera dades
	win_fi();

	pthread_mutex_destroy(&mutex_gestor_usuari);

	elim_sem(semafor_pantalla);
	elim_mem(id_pantalla);
	elim_mis(bustia_recepcio);
	for(i = 0; i < (dades_arg.n_thr+1); i++) {
		elim_mis(bustia_ingres[i]);
	}

	return 0;
}

void *gestor_msg_principal(void *nul) {
	int i, n_oponents = dades_arg.n_thr;
	int n_usuari = 1, control[MAX_JUGADORS];

	char msg[LEN_MISSATGE];
	char origen, accio, oponent;

	// Espera a que tots els processos estiguin preparats
	for(i = 0; i < (dades_arg.n_thr+1); i++) {
		receiveM(bustia_recepcio, msg);
		llegeixMissatge(msg, &origen, &accio, &oponent);
		if(accio == JUGADOR_PREPARAT) {
			control[origen-'0'] = 0;
		}
	}

	// Allibera el semafor per iniciar la execucio
	signalS(semafor_pantalla);

	while((n_oponents != 0) && (n_usuari != 0)) {
		receiveM(bustia_recepcio, msg);
		llegeixMissatge(msg, &origen, &accio, &oponent);

		if(accio == COLISIONA_NO_INVERS) {
			if(control[oponent-'0'] == 0) {
				if(oponent == '0') {
					n_usuari--;
				} else {
					n_oponents--;
				}
				sendM(bustia_ingres[oponent-'0'], msg, LEN_MISSATGE);
				/* NECESARI PER QUAN IMPACTA MES D'UN COP CONTRA RASTRE NO INVERS */
				control[oponent-'0'] = 1;
			}
		} else if(accio == RETURN_USUARI) {
			// Return usuari
			n_usuari--;
		} else {
			if(control[origen-'0'] == 0) {
				if(origen == '0') {
					n_usuari--;
				} else {
					n_oponents--;
				}

				if(accio == COLISIONA_INVERS) {
					sendM(bustia_ingres[oponent-'0'], msg, LEN_MISSATGE);
				}

				/* Evita decrementar n_oponents a oponents que estan en proces de finalitzacio */
				control[origen-'0'] = 1;
			}
		}
	}

	// FALTA PONER ESTADOS DE LOS JUGADORES
	for(i = 0; i < (dades_arg.n_thr+1); i++) {
		creaMissatge(msg, GESTOR, FINALITZA_PARTIDA, NUL);
		sendM(bustia_ingres[i], msg, LEN_MISSATGE);
	}

	/* Evita acabar la funcio fins que tots els threads han acabat */
	i = 0;
	while(i < (dades_arg.n_thr+1)) {
		receiveM(bustia_recepcio, msg);
		llegeixMissatge(msg, &origen, &accio, &oponent);
		if(accio == JUGADOR_FINALITZA) {
			i++;
		}
	}

	acaba_partida = 1;

	pthread_exit(NULL);
}

void *gestor_usuari(void *nul) {
	char origen, accio, oponent;
	char msg[LEN_MISSATGE];
	FILE *fp;

	receiveM(bustia_ingres[0], msg);
	llegeixMissatge(msg, &origen, &accio, &oponent);
	while(accio != FINALITZA_PARTIDA){
		pthread_mutex_lock(&mutex_gestor_usuari);
		// EXECUTA EL RASTRE NO_INV

		if(accio == COLISIONA_INVERS) {
			if(dades_usu.timer_estat == 0) {
				/* SI EL TIMER ESTA DISPONIBLE L'EXECUTA */
				dades_usu.timer_estat = 1;
				pthread_create(&threadTimerRastre, NULL, timerRastre, NULL);
			} else if(dades_usu.timer_estat == -1) {
				/* SI EL TIMER HA FINALITZAT EL TORNA A EXECUTAR */
				dades_usu.timer_estat = 0;
				pthread_join(threadTimerRastre, NULL);

				pthread_create(&threadTimerRastre, NULL, timerRastre, NULL);
			}
			// Jugador choca contra rastre no invers i mata usuari
		} else if(accio == COLISIONA_NO_INVERS) {
			esborrar_posicions('0', dades_usu.p_usu, dades_usu.n_usu);
			dades_usu.fiUsuari = 1;
		}

		// COMPROVA SI ES IMPACTE NO_INVERS EL REBUT PER ACABAR
		pthread_mutex_unlock(&mutex_gestor_usuari);

		receiveM(bustia_ingres[0], msg);
		llegeixMissatge(msg, &origen, &accio, &oponent);
	}

	pthread_mutex_lock(&mutex_gestor_usuari);
	dades_usu.fiUsuari = 1;
	pthread_mutex_unlock(&mutex_gestor_usuari);

	// Escriu cuantitat de caracters
	waitS(semafor_pantalla);
	fp = fopen(dades_arg.fitxer, "a");
	fprintf(fp, "El jugador %c ha escrit %i caracters\n", '0', dades_usu.n_usu);
	fclose(fp);
	signalS(semafor_pantalla);

	/* NECESARI PER SINCRONITZAR SORTIDA AMB GESTOR PRINCIPAL */
	creaMissatge(msg, '0', JUGADOR_FINALITZA, GESTOR);
	sendM(bustia_recepcio, msg, LEN_MISSATGE);

	// No fiquem mutex perque pot bloquejarse infinitament
	if(dades_usu.timer_estat != 0) {
		pthread_join(threadTimerRastre, NULL);
	}

	pthread_exit(NULL);
}

void *mou_usuari(void *nul) {
	char cars, msg[LEN_MISSATGE];
	int tecla, tipus;
	tron seg;

	inicialitzaDadesUsuari(&dades_usu);

	creaMissatge(msg, '0', JUGADOR_PREPARAT, NUL);
	sendM(bustia_recepcio, msg, LEN_MISSATGE);
	inicialitzaJocUsuari(&dades_usu);
	// Espera a que tots els oponents estiguin

	pthread_mutex_lock(&mutex_gestor_usuari);
	while (dades_usu.fiUsuari == 0) {
		waitS(semafor_pantalla);
		tecla = win_gettec();
		if (tecla != 0) {
			switch (tecla) { /* modificar direccio usuari segons tecla */
				case TEC_AMUNT:
					dades_usu.usu.d = 0;
					break;
				case TEC_ESQUER:
					dades_usu.usu.d = 1;
					break;
				case TEC_AVALL:
					dades_usu.usu.d = 2;
					break;
				case TEC_DRETA:
					dades_usu.usu.d = 3;
					break;
				case TEC_RETURN:
					dades_usu.fiUsuari = -1;

					creaMissatge(msg, '0', RETURN_USUARI, NUL);
					sendM(bustia_recepcio, msg, LEN_MISSATGE);
					break;
			}
		}

		seg.f = dades_usu.usu.f + DF[dades_usu.usu.d]; /* calcular seguent posicio */
		seg.c = dades_usu.usu.c + DC[dades_usu.usu.d];
		/* Inici seccio critica */

		cars = win_quincar(seg.f, seg.c); /* calcular caracter seguent posicio */

		if (cars == ' ') { /* si seguent posicio lliure */
			dades_usu.usu.f = seg.f;
			dades_usu.usu.c = seg.c; /* actualitza posicio */

			win_escricar(dades_usu.usu.f, dades_usu.usu.c, '0', dades_usu.t_escrit); /* dibuixa bloc usuari */
			/* Fi seccio critica */

			dades_usu.p_usu[dades_usu.n_usu].f = dades_usu.usu.f; /* memoritza posicio actual */
			dades_usu.p_usu[dades_usu.n_usu].c = dades_usu.usu.c;
			(dades_usu.n_usu)++;
		} else {
			tipus = win_quinatri(seg.f, seg.c);

			if(cars == '+' || cars == '0') {
				/* COLISIONA AMB PARET, JUGADOR TE QUE MORIR */
				creaMissatge(msg, '0', COLISIONA_PARET, NUL);
				if(cars == '0') {
					tipus = 1;
				}
			} else if(tipus == 0) {
				/* COLISIONA AMB OPONENT NO_INV */
				creaMissatge(msg, '0', COLISIONA_NO_INVERS, cars);
			} else {
				/* COLISIONA AMB OPONENT INVERS, JUGADOR TE QUE MORIR */
				creaMissatge(msg, '0', COLISIONA_INVERS, cars);
			}
			/* FALTA COMPROBAR SI COLISIONA AMB ELL MATEIX (colisiona_paret) */
			/* EN CAS DE COLISIONAR AMB ELEMENT INVERS */
			if(tipus != 0) {
				signalS(semafor_pantalla);
				esborrar_posicions('0', dades_usu.p_usu, dades_usu.n_usu);
				waitS(semafor_pantalla);
				dades_usu.fiUsuari = 1;
			} else {
				dades_usu.usu.f = seg.f;
				dades_usu.usu.c = seg.c; /* actualitza posicio */
				win_escricar(dades_usu.usu.f, dades_usu.usu.c, '0', dades_usu.t_escrit);

				dades_usu.p_usu[dades_usu.n_usu].f = dades_usu.usu.f; /* memoritza posicio actual */
				dades_usu.p_usu[dades_usu.n_usu].c = dades_usu.usu.c;
				(dades_usu.n_usu)++;
			}

			sendM(bustia_recepcio, msg, LEN_MISSATGE);
		}
		signalS(semafor_pantalla);
		pthread_mutex_unlock(&mutex_gestor_usuari);

		win_retard(dades_arg.retard);
		pthread_mutex_lock(&mutex_gestor_usuari);
	}

	pthread_mutex_unlock(&mutex_gestor_usuari);
	pthread_exit(NULL);
}

void *timerRastre(void *nul) {
	int fiUsuariLocal, i = 0;

	// Canvia tipus escritura
	pthread_mutex_lock(&mutex_gestor_usuari);
	dades_usu.t_escrit = NO_INV;
	pthread_mutex_unlock(&mutex_gestor_usuari);

	do {
		sleep(1);
		i++;

		pthread_mutex_lock(&mutex_gestor_usuari);
		fiUsuariLocal = dades_usu.fiUsuari;
		pthread_mutex_unlock(&mutex_gestor_usuari);
	} while((i < 10) && (fiUsuariLocal == 0));

	// Retorna tipus de escritura al valor per defecte
	// i indica a la variable timer_estat que ha finalitzat
	pthread_mutex_lock(&mutex_gestor_usuari);
	dades_usu.timer_estat = -1;
	dades_usu.t_escrit = INVERS;
	pthread_mutex_unlock(&mutex_gestor_usuari);

	pthread_exit(NULL);
}

void *mou_oponent(void *nul) {
	int i, n = 0;
	char num_oponent[10], id_pantalla_str[10], semafor_pantalla_str[10];
	char n_fil_str[10], n_col_str[10], retard_str[10], variacio_str[10];
	char bustia_ingres_str[10], bustia_recepcio_str[10];

	sprintf(id_pantalla_str, "%i", id_pantalla);
	sprintf(n_fil_str, "%i", n_fil);
	sprintf(n_col_str, "%i", n_col);
	sprintf(retard_str, "%i", dades_arg.retard);
	sprintf(variacio_str, "%i", dades_arg.varia);
	sprintf(bustia_recepcio_str, "%i", bustia_recepcio);
	sprintf(semafor_pantalla_str, "%i", semafor_pantalla);

	for(i = 0; i < dades_arg.n_thr; i++) {
		tpid[i] = fork();
		if(tpid[i] == (pid_t) 0) {
			sprintf(num_oponent, "%i", (i+1));
			sprintf(bustia_ingres_str, "%i", bustia_ingres[i+1]);
			// executa nou binari
			execlp("./oponent", "oponent", num_oponent,
					id_pantalla_str, n_fil_str, n_col_str, retard_str, variacio_str,
					bustia_ingres_str, bustia_recepcio_str, semafor_pantalla_str, dades_arg.fitxer ,(char *) 0);
			fprintf(stderr, "Error: creacio thread erronea\n");
		} else if(tpid[i] > 0) {
			n++;
		}
	}

	// Espera a que tots els procesos acaben
	for(i = 0; i < dades_arg.n_thr; i++) {
		// Espera el retorn, status = NULL
		waitpid(tpid[i], NULL, 0);
	}

	pthread_exit(NULL);
}

int inicialitzaDadesUsuari(dades_usuari_t *dades_usu) {
	dades_usu->timer_estat = 0;
	dades_usu->fiUsuari = 0;
	dades_usu->n_usu = 0;
	dades_usu->t_escrit = INVERS;
	dades_usu->p_usu = calloc((n_fil*n_col)/2, sizeof(pos));
	if(dades_usu->p_usu != NULL) {
		return 0;
	} else {
		return -1;
	}
}

void inicialitzaJocUsuari(dades_usuari_t *dades_usu) {
	dades_usu->usu.f = (n_fil - 1) / 2;
	dades_usu->usu.c = n_col / 4;
	dades_usu->usu.d = 3;
	/* Inici seccio critica */
	waitS(semafor_pantalla);
	win_escricar(dades_usu->usu.f, dades_usu->usu.c, '0', dades_usu->t_escrit);
	signalS(semafor_pantalla);
	/* Fi seccio critica */
	dades_usu->p_usu[dades_usu->n_usu].f = dades_usu->usu.f;
	dades_usu->p_usu[dades_usu->n_usu].c = dades_usu->usu.c;
	(dades_usu->n_usu)++;
}

void inicialitzaBusties(int *bustia_recepcio, int bustia_ingres[], int n_thr) {
	int i;
	(*bustia_recepcio) = ini_mis();
	for(i = 0; i < (n_thr+1); i++) {
		bustia_ingres[i] = ini_mis();
	}
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

void incialitzaTaulellJoc(int *n_fil, int *n_col, int *id_pantalla) {
	int midaCamp;

	(*n_fil) = 0;
	(*n_col) = 0;
	midaCamp = win_ini(&(*n_fil), &(*n_col), '+', INVERS);
	if(midaCamp < 0) {
		fprintf(stderr, "incialitzaTaulellJoc: ");
		switch(midaCamp) {
			case -1:
				fprintf(stderr, "Error: camp de joc ja creat\n");
				break;
			case -2:
				fprintf(stderr, "Error: no s'ha pogut incialitzar entorn curses\n");
				break;
			case -3:
				fprintf(stderr, "Error: mides del camp massa grans\n");
				break;
			case -4:
				fprintf(stderr, "Error: no s'ha pogut crear la finestra\n");
				break;
			default:
				fprintf(stderr, "Error desconegut\n");
				break;
		}
		exit(2);
	} else {
		(*id_pantalla) = ini_mem(midaCamp);
	}
}

void gestionaArgumentsJoc(int argc, char *argv[], arg_comanda_t *dades_arg) {
	if (argc < 4) {
		fprintf(stderr, "Comanda: ./tron oponents fitxer variabilitat [retard]\n");
		fprintf(stderr, "         on \'oponents\' indica el nombre de rivals de la partida\n");
		fprintf(stderr, "         fitxer es el fitxer on acumularem la llargaria dels drons ),\n");
		fprintf(stderr, "         on \'variabilitat\' indica la frequencia de canvi de direccio\n");
		fprintf(stderr, "         de l'oponent: de 0 a 3 (0- gens variable, 3- molt variable),\n");
		fprintf(stderr, "         i \'retard\' es el numero de mil.lisegons que s'espera entre dos\n");
		exit(1);
	}

	dades_arg->n_thr = atoi(argv[1]);
	if(dades_arg->n_thr < 1) {
		dades_arg->n_thr = 1;
	} else if(dades_arg->n_thr > MAX_OPONENTS){
		dades_arg->n_thr = MAX_OPONENTS;
	}

	// Nom fitxer a escriure
	sprintf(dades_arg->fitxer, "%s", argv[2]);

	dades_arg->varia = atoi(argv[3]);
	if(dades_arg->varia < 0) {
		dades_arg->varia = 0;
	} else if(dades_arg->varia > 3){
		dades_arg->varia = 3;
	}

	if(argc == 5) {
		dades_arg->retard = atoi(argv[4]);
		if(dades_arg->retard < 10) {
			dades_arg->retard = 10;
		} else if(dades_arg->retard > 1000) {
			dades_arg->retard = 1000;
		}
	} else {
		dades_arg->retard = 100;
	}

	printf("Joc del Tron\n\tTecles: \'%c\', \'%c\', \'%c\', \'%c\', RETURN-> sortir\n",
			TEC_AMUNT, TEC_AVALL, TEC_DRETA, TEC_ESQUER);
	printf("prem una tecla per continuar:\n");
	getchar();
}

void calculaTemps(char temps[], time_t inici) {
	time_t final;
	int dif, min, seg;

	time(&final);
	dif = difftime(final, inici);
	min = dif / 60;
	seg = dif % 60;

	sprintf(temps, "Tecles: \'%c\', \'%c\', \'%c\', \'%c\', RETURN-> sortir, Temps: %i:%i\n",
			TEC_AMUNT, TEC_AVALL, TEC_DRETA, TEC_ESQUER, min, seg);
}
