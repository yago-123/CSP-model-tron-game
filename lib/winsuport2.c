/*****************************************************************************/
/*									     */
/*				 winsuport2.c				     */
/*									     */
/* Rutines de suport al control de pantalla, utilitzant la llibreria 'curses'*/
/*	(gcc -c winsuport2.c -o winsuport2.o)				     */
/*									     */
/* Aquesta segona versio de les funcions de "winsuport" es diferencia de la  */
/* primera en que esta preparada per accedir a l'entorn de dibuix des de di- */
/* ferents processos, utilitzan una zona de memoria compartida que ha de     */
/* crear el proces pare i que sera accessible des de tots els processos fill.*/
/*									     */
/* La descripcio general del funcionament de les rutines, aixi com la decla- */
/* racio de constants, es troben al fitxer 'winsuport2.h'		     */
/*									     */
/*****************************************************************************/

#include <stdio.h>
#include <signal.h>
#include <curses.h>
#include "winsuport2.h"

WINDOW* win;                /* finestra d'acces al terminal */
void	(*oldsig)(int);	    /* referencia a l'antiga rutina de signal */
bool	hi_ha_win = FALSE;  /* flag de finestra creada */
int	Nfil=0, Ncol=0;	    /* mides actuals del camp de joc */
chtype	* p_camp;	    /* punter al camp de joc */
int	Mcamp;		    /* mida del camp de joc */


int win_ini(int *fil, int *col, char creq, unsigned int inv)
{
	int ufil,ucol,p_fil,p_col;
	chtype wc;

	if (hi_ha_win) return(-1);	/* error si ja s'havia creat la finestra */
	if (initscr() == NULL) return(-2);	/* error en iniciar les curses */
	if ((*fil > LINES) || (*col > COLS))    /* error en les mides */
	{ endwin();
		return(-3);
	}
	if (*fil == 0) *fil = LINES;	/* cas de mida maxima */
	if (*col == 0) *col = COLS;
	p_fil = (LINES - *fil)/ 2;		/* posicio de la finestra centrada */
	p_col = (COLS - *col) / 2;
	/* canviar interrupcio de final de proces */
	oldsig = signal(SIGINT,win_fi);
	/* configuracio preliminar de les curses */
	cbreak();			/* rep tecles sense buffering */
	noecho();			/* no fa echo de les tecles llegides */
	nonl();			/* el newline \n no es tradueix a CR i LF */
	intrflush(stdscr,FALSE);	/* ^C no s'esborra del buffer d'entrada */
	keypad(stdscr,TRUE);		/* s'activa el teclat numeric i mov. cursor */

	win = newwin(*fil, *col, p_fil, p_col);

	if (win == NULL)	      /* error en la creacio de la finestra */
	{ endwin();
		return(-4);
	}
	hi_ha_win = TRUE;

	leaveok(win,TRUE);	/* dibuixar en pantalla no modificara el cursor */
	curs_set(0);		/* fixa el cursor com a invisible */
	nodelay(win,TRUE);
	refresh();		/* refresca la pantalla (inicialment l'esborra) */

	ufil = *fil;			/* memoritza numero files */
	ucol = *col;			/* memoritza numero columnes */
	wc = creq;				/* dibuixa el requadre */
	if (inv) wc |= A_REVERSE;
	mvwhline(win,0,0,wc,ucol);		/* fila superior */
	mvwhline(win,ufil-2,0,wc,ucol);	/* fila inferior */
	mvwvline(win,1,0,wc,ufil-2);		/* columna dreta */
	mvwvline(win,1,ucol-1,wc,ufil-2);	/* columna esquerra */
	wrefresh(win);		/* redibuixa el contingut de la finestra */

	return(ufil*ucol*sizeof(chtype)*2);	/* retorna mida de zona de memoria */
}


void win_set(void *p_mem, int n_fil, int n_col)
{
	int i,j;
	chtype wc;

	p_camp = (chtype *) p_mem;	/* memoritza punter i dimensions del camp */
	Nfil = n_fil;
	Ncol = n_col;
	Mcamp = Nfil*Ncol;

	/* Comentari d'implementacio:						*/
	/*	No podem deduir els valors de n_fil i n_col de la crida		*/
	/*	a la funcio win_ini(), tot i que s'emmagatzemen a les		*/
	/*	variables globals Nfil,Ncol: aquestes variables s'inicialitzen	*/
	/*	a la crida win_ini() que fa el proces pare i, per tant, el seu	*/
	/*	valor NO es traspassa als processos fill. Per aix�, els		*/
	/*	processos fill hauran d'invocar la win_set() especificant num.	*/
	/*	de files i de columnes del camp de joc (que poden rebre a	*/
	/*	traves d'arguments de programa), per tal que la copia de Nfil i	*/
	/*	Ncol corresponent al fill pugui memoritzar les dimensions del	*/
	/*	camp de joc d'alguna forma.					*/

	if (hi_ha_win)		/* actualitzar camp de joc dins de memoria */
	{ for (i=0; i<Nfil; i++)		/* per a totes les files */
		for (j=0; j<Ncol; j++)			/* per a totes les columnes */
		{
			wc = mvwinch(win,i,j);		/* captura caracter posicio (i,j) */
			p_camp[i*Ncol+j] = wc;		/* actualitza primer camp */
			p_camp[Mcamp+i*Ncol+j] = wc;	/* actualitza segon camp */
		}
	}
}


void win_fi()
{
	if (hi_ha_win)
	{ delwin(win);			/* allibera recursos de la finestra */
		refresh();
		endwin();			/* finalitza l'entorn de curses */
		setvbuf(stdout,NULL, _IOLBF, BUFSIZ);
		signal(SIGINT,oldsig);		/* restaura antiga rutina de signal */
		hi_ha_win = FALSE;
	}
}


void win_escricar(int f, int c, char car, unsigned int invers)
{
	chtype wc;

	if ((f >= 0) && (f < Nfil) && (c >= 0) && (c < Ncol))
	{ wc = car;
		if (invers) wc |= A_REVERSE;
		p_camp[f*Ncol+c] = wc;
	}
}


char win_quincar(int f, int c)
{
	char result;
	chtype wc;

	result = ' ';			/* resultat per defecte */
	if ((f >= 0) && (f < Nfil) && (c >= 0) && (c < Ncol))
	{
		wc = p_camp[f*Ncol+c];		/* captura caracter */
		result = wc & A_CHARTEXT;		/* elimina atributs */
		if ((result < ' ') || (result > '~')) /* filtra valors fora del rang */
			result = ' ';	/* retorna blanc si no es un caracter ASCII */
	}
	return(result);
}


int win_quinatri(int f, int c)
{
	int result = 0;
	chtype wc;

	if ((f >= 0) && (f < Nfil) && (c >= 0) && (c < Ncol))
	{
		wc = p_camp[f*Ncol+c];		/* captura caracter */
		result = wc & A_REVERSE;	/* determina si es invers o no */
	}
	return(result);
}


void win_escristr(char *str)
{
	int j;

	j = 0;
	while ((str[j]!=0) && (j < Ncol))
	{ p_camp[(Nfil-1)*Ncol+j] = str[j];    /* escriu string en la ultima linia */
		j++;
	}
	while (j < Ncol)
	{ p_camp[(Nfil-1)*Ncol+j] = ' ';    /* esborra fins la ultima columna */
		j++;
	}
}


int win_gettec(void)
{
	int result = 0;

	if (hi_ha_win)
	{ result = wgetch(win);
		if (result == ERR) result = 0;
	}
	return(result);
}


void win_retard(int ms)
{
	napms(ms);
}


void win_update(void)
{
	int i,j;
	chtype wc;
	bool	refresc = FALSE;    /* flag de refresc de contingut de finestra */

	if (hi_ha_win)	/* si hi ha finestra */
	{
		for (i=0; i<Nfil; i++)		/* per a totes les files */
		{
			for (j=0; j<Ncol; j++)			/* per a totes les columnes */
			{
				wc = p_camp[i*Ncol+j];
				if (wc != p_camp[Mcamp+i*Ncol+j])	  /* si hi ha canvi */
				{
					mvwaddch(win,i,j,wc);			/* actualitza finestra */
					p_camp[Mcamp+i*Ncol+j] = wc;		/* actualitza segon camp */
					refresc = TRUE;
				}
			}
		}
		if (refresc)	/* si hi han hagut canvis */
		{   wrefresh(win);
			refresh();
		}
	}
}


int win_carregatauler(char *nom_fitxer, int n_f, int n_c, char creq)
{
	int i,j, error;
	chtype wc;
	FILE *fit_ent;

	error = 0;
	if (hi_ha_win)        /* si ja s'ha creat la finestra */
	{
		if (n_f > Nfil-1) return(-4);	/* numero de files massa gran */
		if (n_c > Ncol) return(-3);		/* numero de columnes massa gran */

		fit_ent = fopen(nom_fitxer,"rt");    /* obrir fitxer text per lectura */
		if (fit_ent)                         /* si ha pogut obrir el fitxer */
		{
			i = 0;
			while (!feof(fit_ent) && (i < n_f) && (error == 0))
			{ j = 0;
				do
				{ wc = fgetc(fit_ent);          /* captar un caracter */
					if (wc == creq)               /* si es igual al caracter paret */
						wc |= A_REVERSE;           /* imprimir-lo invertit */
					p_camp[i*Ncol+j] = wc;        /* mvwaddch(win,i,j,wc); */
					j++;                          /* repetir mentre hi hagi linia */
				} while ((j < n_c) && (wc != EOL) && (wc != EOF));
				if (j == n_c)
				{ wc = fgetc(fit_ent);          /* capturar caracter fi de linia */
					if (wc != EOL) error = -2;	/* error en format de linia */
				}
				else error = -2;		/* error en format de taulell */
				i++;
			}
			fclose(fit_ent);
			if (error == 0)                   /* si tot correcte */
			{ wrefresh(win);                  /* actualitza dibuix tauler */
				refresh();
			}
		}
		else error = -1;            /* error en nom del fitxer */
	}
	else error = -5;              /* error finestra no oberta */
	return(error);                /* retorna 0 si no hi ha error */
}
