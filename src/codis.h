#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define FITXER "historial"
// MSG = "ORIGEN:ACCIO:OPONENT"
// len(msg) = 6
#define LEN_MISSATGE 6
#define LEN_PUNTUACIO 100
// ACCIONS
#define RETURN_USUARI '0'
#define COLISIONA_INVERS '1'
#define COLISIONA_NO_INVERS '2'
#define JUGADOR_PREPARAT '3'
#define COLISIONA_PARET '4'
#define FINALITZA_PARTIDA '5'
#define JUGADOR_FINALITZA '6'

// OPONENTS
#define PARET '+'
#define NINGU '-'

// AUXILIARS
#define IGUALS 0
#define NUL '9'
#define GESTOR '*'

// GESTIO MISSATGES
void creaMissatge(char msg[], char origen, char accio, char oponent);
void llegeixMissatge(char msg[], char *origen, char *accio, char *oponent);
