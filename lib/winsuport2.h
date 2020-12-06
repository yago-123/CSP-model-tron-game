/***********************************************************/
/*							   */
/*			winsuport2.h			   */
/*							   */
/* definicio de constants i prototipus de funcions per	   */
/* utilitzar la interficie d'acces a "curses" implementada */
/* al modul winsuport2.c				   */
/*							   */
/***********************************************************/


/* Definicions generals per utilitzar les funcions de "winsuport2.c" */

#define	 INVERS	  1
#define	 NO_INV	  0

#define TEC_AMUNT   'w'
#define TEC_AVALL   's'
#define TEC_DRETA   'd'
#define TEC_ESQUER  'a'
#define TEC_RETURN  13  	/* tecla de Carry Return */

#define EOL  10			/* salt de linia en un fitxer UNIX */


/* Definicions de rutines publiques del fitxer "winsuport2.c" */


int win_ini(int *fil, int *col, char creq, unsigned int inv);
/* Rutina per crear i inicialitzar una finestra de sortida de text, */
/* utilitzant l'entorn de les curses. S'ha de passar per referencia */
/* el numero de files i de columnes que ha de tenir la finestra     */
/* (*fil, *col). Si s'especifica un 0 al valor dels dos parametres, */
/* la finestra ocupara tota la pantalla del terminal. L'ultima fila */
/* de la finestra es reserva per mostrar missatges de text. La      */
/* resta de files es reserva per a tauler de joc. Es dibuixa un     */
/* requadre a tot el tauler de joc, format per caracters creq, que  */
/* es mostren invertits si el parametre inv es diferent de zero.    */
/* Els errors que retorna aquesta rutina son els seguents:          */
/*	>0 -> cap problema, retorna el numero de files		    */
/*      -1  -> la finestra ja havia estat creada                    */
/*      -2  -> no es pot inicialitzar l'entorn de curses            */
/*      -3  -> les mides demanades de camp de joc son massa grans   */
/*      -4  -> la finestra finalment no s'ha pogut crear            */
/* Si el valor dels parametres *fil, *col era zero al cridar la ru- */
/* tina, en sortir de la rutina contindran el numero assignat de    */
/* files i de columnes (les dimensions de la finestra de terminal)  */
/* Per restaurar el terminal cal cridar a la rutina win_fi abans    */
/* de sortir del programa.                                          */ 
/*								    */
/* Aquesta rutina es diferencia de la win_ini() original (dins el   */
/* fitxer "winsuport.c") en que retorna la mida (en bytes) d'una    */
/* zona de memoria compartida que es necessitara per treballar amb  */
/* l'entorn de finestres des de diferents processos. Aquesta zona   */
/* de memoria s'ha de crear des del proces principal que crida a    */
/* la funcio win_ini(). El punter inicial i la seva mida s'han de   */
/* passar a la funcio win_set(), des de tots els processos que hi   */
/* vulguin accedir.						    */


void win_set(void *p_mem, int n_fil, int n_col);
/* Rutina per permetre l'acces a una finestra ja creada, on p_mem es un   */
/* punter a una zona de memoria compartida que contindra el dibuix del    */
/* camp de joc, les dimensions del qual venen indicades per n_fil, n_col. */
/* ATENCIO: aquesta rutina s'ha d'invocar despres de win_ini() i abans    */
/* que la resta de rutines d'acces al camp de joc. Se suposa que la zona  */
/* de memoria s'ha creat amb la mida retornada per la funcio win_ini().   */


void win_fi();
/* Rutina per eliminar la finestra i restaurar el terminal */ 


void win_escricar(int f, int c, char car, unsigned int invers);
/* Rutina per escriure un caracter en una posicio de la finestra,   */
/* on la primera posicio es la (0,0); si invers es diferent de zero,*/
/* el caracter surt invertit.                                       */


char win_quincar(int f, int c);
/* Rutina per llegir quin caracter hi ha en una posicio de finestra */
/* on la primera posicio es la (0,0).                               */


int win_quinatri(int f, int c);
/* Rutina per llegir quin atribut hi ha en una posicio de finestra. */
/* De fet, nomes determina si l'atribut es INVERS (retorna != 0), o */
/* si l'atribut no es INVERS (retorna = 0).			    */


void win_escristr(char *str);
/* Rutina per escriure un string en l'ultima fila de la finestra */


int  win_gettec(void);
/* Rutina per llegir una tecla; retorna zero si no hi ha tecla per llegir, */
/* o el codi ASCII d'una tecla normal.					   */
/* ATENCIO: aquesta rutina nomes es accessible des del proces que hagi     */
/* creat l'entorn de la finestra del camp de joc amb funcio win_ini().	   */


void win_retard(int ms);
/* Rutina per retardar ms mil.lisegons */


void win_update(void);
/* Rutina per actualizar la zona de dibuix de la finestra, d'acord amb les */
/* ultimes modificacions realitzades sobre la zona de memoria compartida.  */
/* ATENCIO: aquesta rutina nomes es accessible des del process que hagi    */
/* creat l'entorn de la finestra del camp de joc amb funcio win_ini().	   */



int win_carregatauler(char *nom_fitxer, int n_f, int n_c, char creq);
/* rutina per carregar un tauler de joc des d'un fitxer, i dibuixar-lo sobre la
   finestra actual de joc; els parametres n_f i n_c indicaran el numero de files
   i de columnes que es preten carregar; el fitxer es llegeis caracter a
   caracter, suposant que cada fila del tauler ocupa una linia del fitxer.
   Els codis ASCII que concideixin amb el parametre creq es visualitzaran en
   invers. Els errors que retorna son els seguents:
   0 ->   cap problema
   -1 ->   error en nom del fitxer
   -2 ->   error en el numero de columnes d'alguna fila no coincideix amb
   el numero global
   -3 ->   error en numero de columnes
   -4 ->   error en numero de files
   -5 ->   no hi ha finestra activa
 */
