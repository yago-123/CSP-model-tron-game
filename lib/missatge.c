/**********************************************************************/
/*                                                                    */
/*  Codi de les rutines dels missatges.                               */
/*                                                                    */
/*  Semantica : Pas de missatges amb comunicacio indirecta, simetrica */
/*              i bloquejant a la funcio "receiveM" quan no hi ha mis-*/
/*		satges i a la funcio "sendM" quan la cua de missatges */
/*		esta plena.                                           */
/*  Implementacio : Fent servir les rutines del SO UNIX que implemen- */
/*                  ten missatges amb una semantica diferent.         */
/*  Notes : S'ha afegit una nova rutina "elim_mis" per questions      */
/*          d'adaptacio de la implementacio. En UNIX quan un proces   */
/*          acaba no allibera els missatges que ha creat o utilitzat. */
/*                                                                    */
/*  Autor : Carles Aliagas					      */
/*  modif : M.Angels Moncusi			      		      */
/**********************************************************************/
#include <stdlib.h>
#include <string.h>
#include "missatge.h"

/**********************************************************************/
/*                                                                    */
/* Accio : ini_mis                                                    */
/*                                                                    */
/* Procediment que ens crea una cua de missatges.                     */
/*                                                                    */
/*   Entrades : --                                                    */
/*   Sortides : Identificador de la cua de missatges.                 */
/*                                                                    */
/**********************************************************************/
int ini_mis()
{
	int id_mis;

	id_mis = msgget(IPC_PRIVATE,0600);	/* reserva d'un conjunt de missatge
						   del sistema, de forma privada, per al proces
						   actual i als seus fills. Nomes reservem un.
						   Nomes el propietari podra utilitzar-lo */
	if (id_mis == 1)
	{ fprintf (stderr,"error al inicialitzar la bustia\n");
		exit(2);
	}

	return(id_mis);
}

/**********************************************************************/
/*                                                                    */
/* Accio : elim_mis                                                   */
/*                                                                    */
/* Procediment que ens elimina una cua de missatges.                  */
/*                                                                    */
/*   Entrades : id_mis : Identificador de la cua de missatges.        */
/*   Sortides : --                                                    */
/*                                                                    */
/**********************************************************************/
void elim_mis(int id_mis)
{
	struct msqid_ds a[1];
	int r;

	r = msgctl(id_mis,IPC_RMID,a); 	/* esborrem la cua */
	if (r != 0)
	{ fprintf (stderr,"error al eliminar la bustia\n");
		exit(2);
	}
}

/**********************************************************************/
/*                                                                    */
/* Accio : sendM                                                      */
/*                                                                    */
/* Procediment que fa l'operacio "Send" d'un missatge sobre la cua de */
/* missatges.                                                         */
/*                                                                    */
/*   Entrades : id_mis : Identificador del missatge.                  */
/*              missatge : vector de bytes a enviar.                  */
/*              nbytes : nombre de bytes que conte el missatge        */
/*   Sortides : --                                                    */
/*                                                                    */
/**********************************************************************/
void sendM (int id_mis, void * missatge, int nbytes)
{
	tmis mis;
	int r;

	if (nbytes > TAM_MAX_MIS )
	{ fprintf (stderr,"el missatge es massa gran\n");
		exit(2);
	}

	mis.tipus = UNIC;			/* estableix el tipus de missatge
						   es considera un unic tipus */
	memcpy(mis.missatge,missatge,nbytes);	/*copia el missatge a l'estructura mis*/

	r = msgsnd(id_mis,&mis,nbytes,(int)0); 	/* encua el missatge */

	if (r!=0 )
	{ fprintf (stderr,"error al enviar el missatge\n");
		exit(2);
	}
}

/**********************************************************************/
/*                                                                    */
/* Accio : receiveM                                                   */
/*                                                                    */
/* Procediment que fa l'operacio "Receive" sobre la cua de missatges. */
/*                                                                    */
/*   Entrades : id_mis : Identificador de la cua de missatges.        */
/*                                                                    */
/*   Sortides : retorna el nombre de bytes del missatge rebut.        */
/*              missatge : adrec,a on es magatzemara el missatge      */
/*                                                                    */
/**********************************************************************/
int receiveM (int id_mis, void * missatge)
{
	tmis mis;
	int r;

	r = msgrcv(id_mis,&mis,TAM_MAX_MIS,(long)0,(int)0);  
	/* desencua un missatge */
	if (r==-1 )
	{ fprintf (stderr,"error al rebre el missatge\n");
		exit(2);
	}

	memcpy(missatge,mis.missatge,r);	/*copia el missatge a l'estructura mis*/

	return(r);
}
