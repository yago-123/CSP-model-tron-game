all: 
	gcc -Wall -c lib/memoria.c -o lib/memoria.o
	gcc -Wall -c lib/winsuport2.c -o lib/winsuport2.o
	gcc -Wall -c lib/semafor.c -o lib/semafor.o
	gcc -Wall -c lib/missatge.c -o lib/missatge.o
	gcc -Wall src/tron.c src/tron.h src/codis.c lib/winsuport2.o lib/memoria.o lib/semafor.o lib/missatge.o -o tron -lcurses -lpthread
	gcc -Wall src/oponent.c src/oponent.h src/codis.c lib/winsuport2.o lib/memoria.o lib/semafor.o lib/missatge.o -o oponent -lcurses -lpthread

clean: 
	rm -f lib/*.o
	rm -f tron
	rm -f oponent
