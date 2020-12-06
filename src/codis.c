#include"codis.h"

void creaMissatge(char msg[], char origen, char accio, char oponent) {
	sprintf(msg, "%c:%c:%c", origen, accio, oponent);
	msg[5] = '\0';
}

void llegeixMissatge(char msg[], char *origen, char *accio, char *oponent) {
	(*origen) = msg[0];
	(*accio) = msg[2];
	(*oponent) = msg[4];
}
