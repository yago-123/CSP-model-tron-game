/**********************************************************************/
/*                                                                    */
/*  Codi de les rutines dels semafors.                                */
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
#include "semafor.h"

/**********************************************************************/
/*                                                                    */
/* Accio : ini_sem                                                    */
/*                                                                    */
/* Procediment que ens crea un semafor.                               */
/*                                                                    */
/*   Entrades : valor : valor inicial assignat al semafor.            */
/*   Sortides : Identificador del semafor.                            */
/*                                                                    */
/**********************************************************************/
int ini_sem(int valor)
{
	union semun a;
	int r,id_sem;

	id_sem = semget(IPC_PRIVATE,1,0600);	/* reserva d'un conjunt de semafors
						   del sistema, de forma privada, per al proces
						   actual i als seus fills. Nomes reservem un.
						   Nomes el propietari podra utilitzar-lo */
	assert ( id_sem != -1);

	a.val = valor;
	r = semctl(id_sem,0,SETVAL,a);        /* asignacio del valor original al
						 semafor reservat (el numero 0) */
	assert (r == 0);
	return(id_sem);
}

/**********************************************************************/
/*                                                                    */
/* Accio : elim_sem                                                   */
/*                                                                    */
/* Procediment que ens elimina un semafor.                            */
/*                                                                    */
/*   Entrades : id_sem : Identificador del semafor.                   */
/*   Sortides : --                                                    */
/*                                                                    */
/**********************************************************************/
void elim_sem(int id_sem)
{
	union semun a={0};
	int r;

	r = semctl(id_sem,0,IPC_RMID,a); 	/* esborrem el semafor */
	assert (r == 0);
}

/**********************************************************************/
/*                                                                    */
/* Accio : waitS                                                      */
/*                                                                    */
/* Procediment que fa l'operacio "Wait" sobre el semafor.             */
/*                                                                    */
/*   Entrades : id_sem : Identificador del semafor.                   */
/*   Sortides : --                                                    */
/*                                                                    */
/**********************************************************************/
void waitS (int id_sem)
{
	struct sembuf a[1];
	int r;

	a->sem_num = 0;	/* semafor sobre el que volem aplicar l'operacio 
			   (l'unic) */
	a->sem_op = -1;	/* Wait, demanem un recurs */
	a->sem_flg = 0;
	r = semop(id_sem,a,1);	/* aplicar una operacio sobre el cj de semafors 
				   reservats.*/
	assert (r != -1);
}

/**********************************************************************/
/*                                                                    */
/* Accio : signalS                                                    */
/*                                                                    */
/* Procediment que fa l'operacio "Signal" sobre el semafor.           */
/*                                                                    */
/*   Entrades : id_sem : Identificador del semafor.                   */
/*   Sortides : --                                                    */
/*                                                                    */
/**********************************************************************/
void signalS (int id_sem)
{
	struct sembuf a[1];
	int r;

	a->sem_num = 0;	/* semafor sobre el que volem aplicar l'operacio 
			   (l'unic) */
	a->sem_op = 1;		/* Signal, alliberem un recurs */
	a->sem_flg = 0;
	r = semop(id_sem,a,1);	/* aplicar una operacio sobre el cj de semafors 
				   reservats.*/
	assert (r != -1);
}
