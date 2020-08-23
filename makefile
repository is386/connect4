run:
	./connect4 ${GAMES} ${DIMENSION}

test:
	./connect4 t
	./connect4 0 0
	./connect4 -1 -1
	./connect4 1 3

build:
	gcc -o connect4 connect4.c csapp.c -lpthread

clean:
	rm connect4 -f
