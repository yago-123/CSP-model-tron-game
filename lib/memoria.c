/**********************************************************************/
/*                                                                    */
/*  Codi de les rutines per a comparticio de memoria entre diferents  */
/*  processos.                                                        */
/*                                                                    */
/*  Semantica : Disposar d'una rutina que ens permeti reservar una    */
/* 		zona de memoria.                                      */ 
/*  Implementacio : Amb les rutines de reserva de memoria de UNIX     */
/*                  de forma que es crida a "shmget" per reservar     */
/*                  recurs IPC privat, alocatacio de Memoria amb      */
/*                  "shmat", eliminacio del recurs IPC, mantenint     */
/*                  la zona de memoria fins al final del proces.      */
/*  Notes : -                                                         */
/*                                                                    */
/*  Autor : Carles Aliagas                                            */
/**********************************************************************/
#include "memoria.h"

/**********************************************************************/
/*                                                                    */
/*  Accio : ini_mem                                                   */
/*                                                                    */
/*  Procediment que crea una zona de memoria compartida a ser utilit- */
/*  zada per diversos procesos					      */
/*                                                                    */
/*  Entrada : tam : nombre de bytes de la zona de memoria a crear     */
/*                                                                    */
/*  Sortida : identificador de la zona de memoria creada              */
/*                                                                    */
/**********************************************************************/
int ini_mem(int tam)
{
	int id_shm;

	id_shm = shmget(IPC_PRIVATE,tam,0600);	/* agafem un recurs IPC */
	assert(id_shm != -1);

	return(id_shm);
}

/**********************************************************************/
/*                                                                    */
/*  Accio : elim_mem                                                  */
/*                                                                    */
/*  Procediment que elimina el IPC associat a una zona de memoria     */
/*  compartida. Tots els processos que hagin alocatat aquesta zona    */
/*  podran continuar fent-la servir encara que el IPC hagi desaparegut*/
/*  Quan finalment el proces mori, aquestes zones quedaran lliures    */
/*                                                                    */
/*  Entrada : id_shm : identificador de la zona de memoria compartida */
/*                                                                    */
/*  Sortida : --						      */
/*                                                                    */
/**********************************************************************/
void elim_mem(int id_shm)
{
	struct shmid_ds a[1];
	int r;

	r = shmctl (id_shm,IPC_RMID,a);	/*esborrem el recurs IPC. La memoria no 
					  s'eliminara fins que tots els processos no
					  l'alliberin */
	assert(r == 0);
}

/**********************************************************************/
/*                                                                    */
/*  Accio : map_mem                                                   */
/*                                                                    */
/*  Procediment que donada una zona de memoria compartida li asigna   */
/*  un espai logic dins del proces que la crida			      */
/*                                                                    */
/*  Entrada : id_shm : identificador de la zona de memoria compartida */
/*                                                                    */
/*  Sortida : adreca logica que apunta a la zona de memoria compartida*/
/*                                                                    */
/**********************************************************************/
void * map_mem(int id_shm)
{
	void * adr;

	adr = (void*)shmat(id_shm,NULL,0);	/* alocatem espai compartit i obtenim
						   la seva adrec,a */
	assert(adr != (void*)-1);

	return(adr);
}

