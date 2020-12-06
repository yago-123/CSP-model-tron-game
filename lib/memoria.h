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
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <assert.h>
#include <stdio.h>

int ini_mem(int tam);

void elim_mem(int id_shm);

void * map_mem(int id_shm);
