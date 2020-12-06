/**********************************************************************/
/*                                                                    */
/*  Interficie de les rutines dels semafors.                          */
/*                                                                    */
/*  Semantica : Segons l'explicada a l'assignatura d'ISO.             */
/*  Implementacio : Fent servir les rutines del SO UNIX que implemen- */
/*                  ten semafors amb una semantica diferent.          */
/*  Notes : S'ha afegit una nova rutina "elim_sem" per questions      */
/*          d'adaptacio de la implementacio. En UNIX quan un proces   */
/*          acaba no allibera els semafors que ha creat o utilitzat.  */
/*                                                                    */
/*  Autor : Carles Aliagas                                            */
/**********************************************************************/

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <assert.h>
#include <stdio.h>

union semun {
	int 	val;
	struct semid_ds	*buf;
	ushort	*array;
};

int ini_sem(int valor);

void elim_sem (int id_sem);

void waitS (int id_sem);

void signalS (int id_sem);
