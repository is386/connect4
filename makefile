build:
	gcc -o connect4 connect4.c csapp.c -lpthread

run:
	./connect4 ${GAMES} ${DIMENSION}

clean:
	rm connect4 -f
